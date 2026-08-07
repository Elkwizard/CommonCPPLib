#include "events.hpp"

namespace util {
	CallbackID Event::add(const Callback& fn) {
		CallbackID id = nextID++;
		callbacks.emplace(id, fn);
		return id;
	}

	void Event::remove(CallbackID id) {
		callbacks.erase(id);
	}

	void Event::run() const {
		for (const auto& callback : callbacks)
			callback.second();
	}

	EventHandler::EventHandler(Event& _event, const Event::Callback& fn) : event(_event) {
		id = event.add(fn);
	}

	EventHandler::~EventHandler() {
		event.remove(id);
	}
}