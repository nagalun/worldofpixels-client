#pragma once

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/AutoStacking.hpp>
#include <string_view>
#include <variant>

struct EmscriptenMouseEvent;

namespace eui {

struct WindowOptions {
	std::variant<std::string_view, Object> title;
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

class Window : public AutoStacking {
	Object titleBar;
	Object content;
	unsigned int width;
	unsigned int height;
	unsigned int minWidth;
	unsigned int minHeight;
	int moveLastX;
	int moveLastY;
	bool moving;

public:
	Window(WindowOptions = {}, std::string_view containerSelector = "#eui-container");

	void move(int x, int y);
	void resize(unsigned int width, unsigned int height);

	Object& getTitle();
	void setTitle(Object titleBar);
	void setTitle(std::string_view);

	Object& getContent();
	void setContent(Object content);

	unsigned int getMinWidth() const;
	unsigned int getMinHeight() const;
	unsigned int getWidth() const;
	unsigned int getHeight() const;

private:
	bool pointerDownTitle(int buttons);
	bool pointerUp(int buttons);
	void pointerMove(int x, int y);
	static int handleMouseEvent(int type, const EmscriptenMouseEvent * ev, void * data);
};

} // namespace eui
