#pragma once

#include <cstdint>
#include <functional>

#include "Object.hpp"
#include "EventHandle.hpp"

struct EmscriptenMouseEvent;

namespace eui {

class Button : public Object {
	EventHandle eh;

public:
	Button(std::function<bool(void)> = nullptr);

	void setCb(std::function<bool(void)>);
};

} /* namespace eui */
