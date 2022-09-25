#include "EventHandle.hpp"

#include <util/emsc/dom.hpp>

namespace eui {

EventHandle::EventHandle(std::uint32_t objId, std::string_view evt, std::function<bool(void)> cb, bool passive)
: objId(objId),
  cb(std::move(cb)),
  enabled(true),
  valid(true) {
	eui_elem_add_handler(objId, evt.data(), evt.size(), this, EventHandle::eventFired, passive);
}

EventHandle::~EventHandle() {
	if (valid) {
		eui_elem_del_handler(objId, this, EventHandle::eventFired);
	}
}

EventHandle& EventHandle::operator=(EventHandle &&other) {
	if (valid) {
		eui_elem_del_handler(objId, this, EventHandle::eventFired);
	}
	objId = std::exchange(other.objId, 0);
	cb = std::exchange(other.cb, nullptr);
	enabled = std::exchange(other.enabled, false);
	valid = std::exchange(other.valid, false);
	if (valid) {
		eui_elem_ch_handler(objId, &other, EventHandle::eventFired, this, EventHandle::eventFired);
	}
	return *this;
}

EventHandle::EventHandle(EventHandle &&other)
: objId(std::exchange(other.objId, 0)),
  cb(std::exchange(other.cb, nullptr)),
  enabled(std::exchange(other.enabled, false)),
  valid(std::exchange(other.valid, false)) {
	if (valid) {
		eui_elem_ch_handler(objId, &other, EventHandle::eventFired, this, EventHandle::eventFired);
	}
}

void EventHandle::setCb(std::function<bool(void)> newCb) {
	cb = std::move(newCb);
}

EventHandle::EventHandle()
: objId(0),
  enabled(false),
  valid(false) { }

void EventHandle::setEnabled(bool state) {
	enabled = state;
	eui_elem_enable_handler(objId, this, EventHandle::eventFired, state);
}

bool EventHandle::eventFired(void * data) {
	EventHandle* eh = static_cast<EventHandle*>(data);
	if (eh->cb && eh->enabled) {
		return static_cast<EventHandle*>(data)->cb();
	}

	return false;
}

} /* namespace eui */
