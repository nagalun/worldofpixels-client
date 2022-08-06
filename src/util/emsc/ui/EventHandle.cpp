#include "EventHandle.hpp"

#include <util/emsc/dom.hpp>

namespace eui {

EventHandle::EventHandle(std::uint32_t objId, std::string_view evt, std::function<bool(void)> cb)
: objId(objId),
  cb(std::move(cb)) {
	eui_elem_add_handler(objId, evt.data(), evt.size(), this, EventHandle::eventFired);
}

EventHandle::~EventHandle() {
	if (objId != 0) {
		eui_elem_del_handler(objId, this, EventHandle::eventFired);
	}
}

EventHandle& EventHandle::operator=(EventHandle &&other) {
	eui_elem_ch_handler(objId, this, EventHandle::eventFired, &other, EventHandle::eventFired);
	return *this;
}

EventHandle::EventHandle(EventHandle &&other)
: objId(std::exchange(other.objId, 0)),
  cb(std::exchange(other.cb, nullptr)) {
	eui_elem_ch_handler(objId, &other, EventHandle::eventFired, this, EventHandle::eventFired);
}

void EventHandle::setCb(std::function<bool(void)> newCb) {
	cb = std::move(newCb);
}

EventHandle::EventHandle()
: objId(0) { }

bool EventHandle::eventFired(void * data) {
	EventHandle* eh = static_cast<EventHandle*>(data);
	if (eh->cb) {
		return static_cast<EventHandle*>(data)->cb();
	}

	return false;
}

} /* namespace eui */
