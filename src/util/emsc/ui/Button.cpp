#include "Button.hpp"

#include <emscripten/html5.h>

namespace eui {

Button::Button(std::function<bool(void)> cb)
: Object("button"),
  eh(createHandler("click", std::move(cb))) { }

void Button::setCb(std::function<bool(void)> newCb) {
	eh.setCb(std::move(newCb));
}

} /* namespace eui */
