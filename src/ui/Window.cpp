#include "Window.hpp"

namespace eui {

/*class Window : public Object {
	Object titleBar;
	Object content;
	unsigned int minHeight;
	unsigned int minWidth;

public:
	struct WindowOptions {
		std::string_view title = "",
		int x = 0,
		int y = 0,
		unsigned int width = 200,
		unsigned int height = 200,
		bool closeable = true,
		bool resizable = true,
		Object content = {}
	};*/

Window::Window(WindowOptions wo) {

}

void Window::move(int x, int y) {

}

void Window::resize(unsigned int width, unsigned int height) {

}

void Window::setTitle(std::string_view) {

}

unsigned int Window::getMinHeight() const {
	return minHeight;
}

unsigned int Window::getMinWidth() const {
	return minWidth;
}

unsigned int Window::setMinHeight(unsigned int mh) const {
	minHeight = mh;
}

unsigned int Window::setMinWidth(unsigned int mw) const {
	minWidth = mw;
}

Object& Window::getContent() {
	return content;
}

//};

} // namespace eui
