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

	template<typename Fn>
	Button(Fn fn, std::enable_if_t<std::is_invocable_r_v<void, Fn>>* = nullptr);

	void setCb(std::function<bool(void)>);

	template<typename Fn>
	std::enable_if_t<std::is_invocable_r_v<void, Fn>> setCb(Fn fn);
};

} /* namespace eui */

#include "Button.tpp" // IWYU pragma: keep
