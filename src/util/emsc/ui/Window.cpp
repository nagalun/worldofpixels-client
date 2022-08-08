#include "Window.hpp"
#include <cstdio>
#include <cstdint>

#include <emscripten/html5.h>

#include <util/misc.hpp>

namespace eui {

// "translate(-2147483648px,-2147483648px)"
// "-2147483648px" // w, h
constexpr std::size_t bufSz = 11 * 2 + 16;

static Object mktitle(std::string_view title) {
	Object t;
	t.setProperty("textContent", title);

	return t;
}

Window::Window(WindowOptions wo, std::string_view containerSelector)
: titleBar(wo.title.index() == 0 ? mktitle(std::get<0>(wo.title)) : std::move(std::get<1>(wo.title))),
  content(std::move(wo.content)),
  width(wo.width),
  height(wo.height),
  minWidth(wo.minWidth),
  minHeight(wo.minHeight),
  moveLastX(0),
  moveLastY(0),
  moving(false) {
	addClass("eui-win");

	titleBar.addClass("eui-win-title");
	titleBar.appendTo(*this);

	content.addClass("eui-win-content");
	content.appendTo(*this);

	const char * tbarSel = titleBar.getSelector().data();
	const char * csel = containerSelector.data();

	//emscripten_set_mousedown_callback(tbarSel, this, true, Window::handleMouseEvent);
	// there is a delay between moving the mouse and sending the event, so
	// mouseup can end up outside the window
	//emscripten_set_mousemove_callback(csel, this, false, Window::handleMouseEvent);
	//emscripten_set_mouseup_callback(csel, this, false, Window::handleMouseEvent);

	move(wo.x, wo.y);
	//resize(wo.width, wo.height);
}

void Window::move(int x, int y) {
	setProperty("style.transform", svprintf<bufSz>("translate(%ipx,%ipx)", x, y));
}

void Window::resize(unsigned int newWidth, unsigned int newHeight) {
	if (newWidth != width) {
		setProperty("style.width", svprintf<bufSz>("%ipx", newWidth));
		width = newWidth;
	}

	if (newHeight != height) {
		setProperty("style.height", svprintf<bufSz>("%ipx", newHeight));
		height = newHeight;
	}
}

Object& Window::getTitle() {
	return titleBar;
}

void Window::setTitle(Object nTitleBar) {
	titleBar = std::move(nTitleBar);
	titleBar.appendTo(*this);
	titleBar.addClass("eui-win-title");
}

void Window::setTitle(std::string_view title) {
	titleBar.setProperty("textContent", title);
}

Object& Window::getContent() {
	return content;
}

void Window::setContent(Object nContent) {
	content = std::move(nContent);
	content.appendTo(*this);
	content.addClass("eui-win-content");
}

unsigned int Window::getWidth() const {
	return width;
}

unsigned int Window::getHeight() const {
	return height;
}

unsigned int Window::getMinWidth() const {
	return minWidth;
}

unsigned int Window::getMinHeight() const {
	return minHeight;
}

bool Window::pointerDownTitle(int buttons) {
	moving = true;
	return false;
}

bool Window::pointerUp(int buttons) {
	if (moving && !(buttons & 1)) {
		moving = false;
	}

	return false;
}

void Window::pointerMove(int x, int y) {
	if (moving) {
		int deltaX = x - moveLastX;
		int deltaY = y - moveLastY;
		moveLastX = x;
		moveLastY = y;
		move(x, y);
	}
}

int Window::handleMouseEvent(int type, const EmscriptenMouseEvent *ev, void *data) {
	Window * win = static_cast<Window *>(data);
	std::printf("%d, %d\n", win->moving, type);

	switch (type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			win->pointerMove(ev->clientX, ev->clientY);
			return false;

		case EMSCRIPTEN_EVENT_MOUSEDOWN:
			return win->pointerDownTitle(ev->buttons);

		case EMSCRIPTEN_EVENT_MOUSEUP:
			return win->pointerUp(ev->buttons);
	}

	return false;
}

} // namespace eui
