#pragma once

#include "Object.hpp"

#include <string_view>

namespace eui {

class Window : public Object {
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
	};

	Window(WindowOptions = {});

	void move(int x, int y);
	void resize(unsigned int width, unsigned int height);

	void setTitle(std::string_view);

	unsigned int getMinHeight() const;
	unsigned int getMinWidth() const;
	unsigned int setMinHeight() const;
	unsigned int setMinWidth() const;

	Object& getContent();

};

} // namespace eui
