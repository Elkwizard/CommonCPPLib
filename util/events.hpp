#pragma once

#include <unordered_map>
#include <functional>
#include <optional>

namespace util {
	using CallbackID = size_t;

	class EventHandler;
	
	class Event {
		public:
			using Callback = std::function<void()>;

		private:
			std::unordered_map<CallbackID, Callback> callbacks; 
			int nextID = 0;

			CallbackID add(const Callback& fn);
			void remove(CallbackID id);

		public:
			void run() const;

		friend EventHandler;
	};
	
	class EventHandler {
		private:
			CallbackID id;
			Event& event;

		public:
			EventHandler(Event& _event, const Event::Callback& fn);
			~EventHandler();
	};
}