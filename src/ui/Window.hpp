#pragma once

#include "Object.hpp"

#include <string_view>

namespace eui {

struct WindowOptions {
	std::string_view title = "";
	int x = 0;
	int y = 0;
	unsigned int width = 200;
	unsigned int height = 200;
	unsigned int minWidth = 50;
	unsigned int minHeight = 25;
	bool closeable = true;
	bool resizable = true;
	Object content = {};
};

class Window : public Object {
	Object titleBar;
	Object content;
	unsigned int width;
	unsigned int height;
	unsigned int minWidth;
	unsigned int minHeight;

public:
	Window(WindowOptions = {});

	void move(int x, int y);
	void resize(unsigned int width, unsigned int height);

	void setTitle(std::string_view);

	Object& getContent();
	unsigned int getHeight() const;
	unsigned int getMinHeight() const;
	unsigned int getMinWidth() const;
	Object& getTitleBar();
	void setTitleBar(Object titleBar);
	unsigned int getWidth() const;
};

} // namespace eui
