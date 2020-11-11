#include "Window.hpp"

#include <cstdio>

namespace eui {

Window::Window(WindowOptions wo)
: content(std::move(wo.content)),
  width(wo.width),
  height(wo.height),
  minWidth(wo.minWidth),
  minHeight(wo.minHeight) {
	setTitle(wo.title);
}

void Window::move(int x, int y) {
	static char prop[11 * 2 + 16 + 1] = {0};
	int written = std::sprintf(prop, "translate(%ipx,%ipx)", x, y);

	setProperty("style.transform", std::string_view(prop, written));
}

void Window::resize(unsigned int width, unsigned int height) {

}

void Window::setTitle(std::string_view title) {
	titleBar.setProperty("textContent", title);
}

unsigned int Window::getHeight() const {
	return height;
}

unsigned int Window::getMinHeight() const {
	return minHeight;
}

unsigned int Window::getMinWidth() const {
	return minWidth;
}

Object& Window::getTitleBar() {
	return titleBar;
}

void Window::setTitleBar(Object titleBar) {
	this->titleBar = std::move(titleBar);
}

unsigned int Window::getWidth() const {
	return width;
}

Object& Window::getContent() {
	return content;
}

} // namespace eui
