#pragma once

#include "debug.hpp"
#include "sync.hpp"
#include "timer.hpp"

#include <thread>
#include <unordered_set>
#include <vector>
#include <functional>
#include <memory>
#include <queue>
#include <algorithm>

namespace util {
	class Threadpool {
		public:
			class Group {
				private:
					Sync sync;
					int outstandingTasks = 0;

				public:
					Group() { }

					void startTask() { std::unique_lock guard { sync.mutex };
						outstandingTasks++;
					}

					void finishTask() {
						std::unique_lock guard { sync.mutex };
						outstandingTasks--;
						sync.condition.notify_one();
					}

					void join() {
						std::unique_lock lock { sync.mutex };
						sync.condition.wait(lock, [&] {
							return outstandingTasks == 0;
						});
						lock.unlock();
					}
			};

			using TaskID = unsigned long long;
			using Priority = double;
			using GetPriority = std::function<Priority()>;

		private:
			bool debug;
			using Task = std::function<void()>;
			struct TaskEntry {
				TaskID id = 0;
				Task task = nullptr;
				GetPriority getPriority = nullptr;
				Group* group = nullptr;

				TaskEntry() { }

				TaskEntry(TaskID _id, const Task& _task, const GetPriority& _getPriority, Group* _group) {
					id = _id;
					task = _task;
					getPriority = _getPriority;
					group = _group;
					if (group) group->startTask();
				}

				TaskEntry& operator =(TaskEntry&& other) {
					if (this != &other) {
						id = other.id;
						task = std::move(other.task);
						getPriority = std::move(other.getPriority);
						group = other.group;
					}
					return *this;	
				}

				TaskEntry(TaskEntry&& other) {
					*this = std::move(other);
				}

				TaskEntry& operator =(const TaskEntry& other) = delete;
				TaskEntry(const TaskEntry& other) = delete;
				
			};
			
			struct Thread {
				std::unique_ptr<std::thread> thread;
				TaskEntry task;
				int inx;
	
				Thread() { }

				Thread(const Thread& other) = delete;

				Thread(Thread&& other) {
					if (&other != this) {
						thread = std::move(other.thread);
						task = std::move(other.task);
						inx = std::move(other.inx);
					}
				}

				~Thread() {
					thread->join();
				}

				void start(Task method) {
					thread = std::make_unique<std::thread>(method);
				}

				void runTask() {
					task.task();
					if (task.group)
						task.group->finishTask();
				}
			};

			class TaskQueue {
				private:
					void clean() { Timer t("clean task queue");
						int toCheck = tasks.size() / 10 + 1;
						int checked = 0;
						for (auto it = tasks.begin(); it != tasks.end();) {
							if (checked++ >= toCheck)
								break;
							if ((*it).second.getPriority() > 0)
								it++;
							else tasks.erase(it);
						}
					}

				public:
					std::unordered_map<TaskID, TaskEntry> tasks;
					Sync sync;
					int totalThreads, busyThreads = 0;

					int size() {
						return tasks.size();
					}

					bool checkDuplicates() {
						std::unordered_set<TaskID> ids {};
						for (int i = 0; i < tasks.size(); i++) {
							if (ids.count(tasks[i].id)) return true;
							ids.insert(tasks[i].id);
						}
						return false;
					}

					void addTask(TaskID id, const Task& task, const GetPriority& getPriority, Group* group) {
						tasks.emplace(id, TaskEntry(id, task, getPriority, group));
					}

					bool cancelTask(TaskID id) {
						if (tasks.count(id)) {
							TaskEntry& task = tasks.at(id);
							if (task.group)
								task.group->finishTask();
						}
						return tasks.erase(id);
					}

					void notify() {
						sync.condition.notify_one();
					}

					bool getTask(TaskEntry& result) {
						clean();
						if (tasks.empty()) return false;
						
						Timer t("getTask");
						
						TaskID highestID;
						Priority highestPriority = 0;
						int counted = 0;
						int toCount = tasks.size() / 10 + 1;
						for (const auto& [id, task] : tasks) {
							if (counted++ >= toCount)
								break;

							Priority p = task.getPriority();
							if (p <= 0) continue;
							if (p > highestPriority) {
								highestPriority = p;
								highestID = id;
							}
						}

						result = std::move(tasks.extract(highestID).mapped());
						return true;
					}
			};

			std::atomic<TaskID> nextTaskID = 0;
			TaskQueue taskQueue;
			TaskQueue backgroundTaskQueue;
			Group defaultGroup;
			std::atomic<bool> destroyed = false;

			std::vector<Thread> threads;

			TaskID addTask(TaskID id, Task task, GetPriority getPriority, Group* group) {
				bool background;
				{ std::scoped_lock lock { taskQueue.sync.mutex, backgroundTaskQueue.sync.mutex };
					background = !group || (
						!taskQueue.tasks.empty() &&
						backgroundTaskQueue.tasks.empty() &&
						backgroundTaskQueue.busyThreads < backgroundTaskQueue.totalThreads
					);
					(background ? backgroundTaskQueue : taskQueue).addTask(id, task, getPriority, group ? group : &defaultGroup);
				};
				(background ? backgroundTaskQueue : taskQueue).notify();
				return id;
			}

			void notifyAll() {
				taskQueue.sync.condition.notify_all();
				backgroundTaskQueue.sync.condition.notify_all();
			}

		public:
			Threadpool(float backgroundRatio = 0.5f, bool _debug = false) {
				debug = _debug;
				int totalThreads = std::thread::hardware_concurrency() - 1;
				taskQueue.totalThreads = totalThreads * backgroundRatio;
				backgroundTaskQueue.totalThreads = totalThreads - taskQueue.totalThreads;
				threads.resize(totalThreads);
				for (int i = 0; i < totalThreads; i++) {
					threads[i].start([=, this] {
						try {
							Thread& self = threads[i];
							TaskQueue& tasks = (i < taskQueue.totalThreads) ? taskQueue : backgroundTaskQueue;

							while (true) {
								std::unique_lock lock { tasks.sync.mutex };

								tasks.sync.condition.wait(lock, [&] {
									return destroyed || tasks.size() > 0;
								});

								if (destroyed)
									return;

								while (tasks.getTask(self.task)) {
									tasks.busyThreads++;
									lock.unlock();
									tasks.sync.condition.notify_all();
									self.runTask();
									lock.lock();
									tasks.busyThreads--;
								}
							}

						} catch (const std::exception& err) {
							log(err.what());
						}
					});
				}
			}

			~Threadpool() {
				join();
				destroyed = true;
				notifyAll();
			}

			int getForegroundThreads() const {
				return taskQueue.totalThreads;
			}

			int getBackgroundThreads() const {
				return backgroundTaskQueue.totalThreads;
			}

			int getScheduledBackgroundTasks() {
				return backgroundTaskQueue.size();	
			}
			
			void join() {
				defaultGroup.join();
				// if (joined) return;
				// joined = true; 

				// taskQueue.join();
				// backgroundTaskQueue.join();

				// for (auto& thread : threads)
				// 	thread.thread->join();
			}

			bool cancelTask(TaskID id) {
				{ std::unique_lock lock { taskQueue.sync.mutex };
					if (taskQueue.cancelTask(id)) return true;
				};
				
				{ std::unique_lock lock { backgroundTaskQueue.sync.mutex };
					if (backgroundTaskQueue.cancelTask(id)) return true;
				};
				
				return false;
			}

			TaskID getTaskID() {
				return ++nextTaskID;
			}

			TaskID addTask(const Task& task, const GetPriority& getPriority = []() { return 1; }) {
				return addTask(getTaskID(), task, getPriority, nullptr);
			}

			TaskID addTask(const Task& task, Priority priority) {
				return addTask(getTaskID(), task, [=]() { return priority; });
			}

			TaskID addTask(const Task& task, Group& count) {
				return addTask(getTaskID(), task, []() { return std::numeric_limits<Priority>::max(); }, &count);
			}
	
			TaskID addTask(TaskID id, const Task& task, const GetPriority& getPriority = []() { return 1; }) {
				return addTask(id, task, getPriority, nullptr);
			}

			TaskID addTask(TaskID id, const Task& task, Priority priority) {
				return addTask(id, task, [=]() { return priority; });
			}

			TaskID addTask(TaskID id, const Task& task, Group& count) {
				return addTask(id, task, []() { return std::numeric_limits<Priority>::max(); }, &count);
			}
	};
}