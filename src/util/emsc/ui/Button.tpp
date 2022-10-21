#pragma once
#include "Button.hpp"
#include <type_traits>

template<typename Fn>
eui::Button::Button(Fn fn, std::enable_if_t<std::is_invocable_r_v<void, Fn>>*)
: eui::Button(std::function<bool(void)>{[fn{std::move(fn)}] { fn(); return false; }}) { }

template<typename Fn>
std::enable_if_t<std::is_invocable_r_v<void, Fn>>
eui::Button::setCb(Fn fn) {
	setCb(std::function<bool(void)>{[fn{std::move(fn)}] { fn(); return false; }});
}
