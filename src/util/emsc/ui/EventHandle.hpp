#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace eui {

class EventHandle {
	std::uint32_t objId;
	std::function<bool(void)> cb;

	EventHandle(std::uint32_t objId, std::string_view evt, std::function<bool(void)>);

public:
	EventHandle();
	~EventHandle();
	EventHandle& operator=(EventHandle &&other);
	EventHandle(EventHandle &&other);
	EventHandle& operator=(const EventHandle &other) = delete;
	EventHandle(const EventHandle &other) = delete;

	void setCb(std::function<bool(void)>);

private:
	static bool eventFired(void *);

	friend class Object;
};

} /* namespace eui */
