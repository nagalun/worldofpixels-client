#pragma once

#include <cstdint>
#include "util/emsc/ui/Object.hpp"
#include "util/emsc/ui/Button.hpp"
#include "util/emsc/ui/AutoStacking.hpp"
#include <string_view>
#include <variant>
#include <optional>

struct EmscriptenMouseEvent;

namespace eui {

struct WindowOptions {
	std::variant<std::string_view, Object> title;
	bool moveable = true;
	bool closeable = true;
	Object content = {};
};

class Window : public AutoStacking {
	Object titleBar;
	std::optional<Button> closeBtn;
	Object content;
	eui::EventHandle dragStartEvt;
	eui::EventHandle dragEvt;
	eui::EventHandle dragEndEvt;
	int x;
	int y;
	int horizAnchor;
	int vertAnchor;
	int moveLastX;
	int moveLastY;
	bool moveable;
	bool movingToCenter;
	bool closeable;
	bool closed;

public:
	Window(WindowOptions = {});
	explicit Window(std::string_view title, bool moveable = true, bool closeable = true);
	explicit Window(bool moveable, bool closeable);
	Window(Window&&) noexcept;
	const Window& operator=(Window&&) noexcept;
	virtual ~Window();

	void moveToCenter(bool slowMethod = false, bool hideUntilCentered = false);
	virtual void move(int x, int y, std::int8_t horizAnchor = -1, std::int8_t vertAnchor = -1);
	virtual bool isClosed();
	virtual bool open();
	virtual bool close();
	bool toggle();

	Object& getTitle();
	void setTitle(Object titleBar);
	void setTitle(std::string_view);

	Object& getContent();
	void setContent(Object content);

private:
	bool pointerDownTitle();
	bool pointerUp();
	bool pointerMove();
};

} // namespace eui
