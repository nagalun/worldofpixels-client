#include <util/emsc/ui/Window.hpp>
#include <cstdio>
#include <cstdint>

#include <emscripten/html5.h>

namespace eui {

// "translate(-2147483648px,-2147483648px)" + null
// "-2147483648px" + null // w, h
static char propBuf[11 * 2 + 16 + 1] = {0};

template<typename... Args>
std::string_view svprintf(const char * fmt, Args... args) {
	int written = std::snprintf(propBuf, sizeof(propBuf), fmt, args...);

	if (written < 0) {
		written = 0;
	} else if ((std::uint32_t)written > sizeof(propBuf)) {
		written = sizeof(propBuf);
	}

	return std::string_view(propBuf, written);
}

static Object mktitle(std::string_view title) {
	Object t("span");
	t.setProperty("textContent", title);

	return t;
}

Window::Window(WindowOptions wo, std::string_view containerSelector)
: AutoStacking(containerSelector),
  titleBar(wo.title.index() == 0 ? mktitle(std::get<0>(wo.title)) : std::move(std::get<1>(wo.title))),
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

	emscripten_set_mousedown_callback(tbarSel, this, true, Window::handleMouseEvent);
	// there is a delay between moving the mouse and sending the event, so
	// mouseup can end up outside the window
	emscripten_set_mousemove_callback(csel, this, true, Window::handleMouseEvent);
	emscripten_set_mouseup_callback(csel, this, true, Window::handleMouseEvent);

	move(wo.x, wo.y);
	//resize(wo.width, wo.height);
}

void Window::move(int x, int y) {
	setProperty("style.transform", svprintf("translate(%ipx,%ipx)", x, y));
}

void Window::resize(unsigned int width, unsigned int height) {
	setProperty("style.width", svprintf("%ipx", width));
	setProperty("style.height", svprintf("%ipx", height));
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

bool Window::pointerDown(int buttons) {
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
		//moveLastX =
	}
}

int Window::handleMouseEvent(int type, const EmscriptenMouseEvent *ev, void *data) {
	Window * win = static_cast<Window *>(data);

	switch (type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			win->pointerMove(ev->clientX, ev->clientY);
			return false;

		case EMSCRIPTEN_EVENT_MOUSEDOWN:
			return win->pointerDown(ev->buttons);

		case EMSCRIPTEN_EVENT_MOUSEUP:
			return win->pointerUp(ev->buttons);
	}

	return false;
}

} // namespace eui
