#include "Window.hpp"
#include <cstdio>

#include "util/emsc/dom.hpp"
#include "util/misc.hpp"

namespace eui {

// "translate(-100%,-100%) translate(-2147483648px,-2147483648px)"
// "-2147483648px" // w, h
constexpr std::size_t bufSz = 61;

static Object mktitle(std::string_view title) {
	Object t;
	t.setProperty("textContent", title);

	return t;
}

Window::Window(WindowOptions wo)
: titleBar(wo.title.index() == 0 ? mktitle(std::get<0>(wo.title)) : std::move(std::get<1>(wo.title))),
  content(std::move(wo.content)),
  dragStartEvt(titleBar.createHandler("mousedown touchstart", std::bind(&Window::pointerDownTitle, this))),
  dragEvt(Object::createWindowHandler("mousemove touchmove", std::bind(&Window::pointerMove, this))),
  dragEndEvt(Object::createWindowHandler("mouseup touchend", std::bind(&Window::pointerUp, this))),
  x(0),
  y(0),
  horizAnchor(-1),
  vertAnchor(-1),
  moveLastX(0),
  moveLastY(0),
  moveable(wo.moveable),
  movingToCenter(false),
  closeable(wo.closeable),
  closed(true) {
	addClass("eui-win");

	titleBar.addClass("eui-win-title");
	titleBar.appendTo(*this);

	if (closeable) {
		closeBtn.emplace(std::bind(&Window::close, this));
		closeBtn->addClass("eui-win-close");
		closeBtn->setAttribute("title", "Close");
		closeBtn->appendTo(*this);
	}

	content.addClass("eui-win-content");
	content.appendTo(*this);

	if (moveable) {
		addClass("moveable");
	} else {
		dragStartEvt.setEnabled(false);
	}

	dragEvt.setEnabled(false);
	dragEndEvt.setEnabled(false);

//	bringUp();
//	moveToCenter(true, true);
}

Window::Window(std::string_view title, bool moveable, bool closeable)
: Window(WindowOptions{title, moveable, closeable}) { } // @suppress("Symbol is not resolved")

Window::Window(bool moveable, bool closeable)
: Window("", moveable, closeable) { }

Window::Window(Window&& o) noexcept
: AutoStacking(std::move(o)),
  titleBar(std::move(o.titleBar)),
  closeBtn(std::move(o.closeBtn)),
  content(std::move(o.content)),
  dragStartEvt(std::move(o.dragStartEvt)),
  dragEvt(std::move(o.dragEvt)),
  dragEndEvt(std::move(o.dragEndEvt)),
  x(std::move(o.x)),
  y(std::move(o.y)),
  horizAnchor(std::move(o.horizAnchor)),
  vertAnchor(std::move(o.vertAnchor)),
  moveLastX(std::move(o.moveLastX)),
  moveLastY(std::move(o.moveLastY)),
  moveable(std::move(o.moveable)),
  movingToCenter(std::move(o.movingToCenter)),
  closeable(std::move(o.closeable)),
  closed(std::move(o.closed)) {
	dragStartEvt.setCb(std::bind(&Window::pointerDownTitle, this));
	dragEvt.setCb(std::bind(&Window::pointerMove, this));
	dragEndEvt.setCb(std::bind(&Window::pointerUp, this));
	if (closeBtn.has_value()) {
		closeBtn->setCb(std::bind(&Window::close, this));
	}
}

const Window& Window::operator =(Window&& o) noexcept {
	AutoStacking::operator =(std::move(o));
	titleBar = std::move(o.titleBar);
	closeBtn = std::move(o.closeBtn);
	content = std::move(o.content);
	dragStartEvt = std::move(o.dragStartEvt);
	dragEvt = std::move(o.dragEvt);
	dragEndEvt = std::move(o.dragEndEvt);
	x = std::move(o.x);
	y = std::move(o.y);
	horizAnchor = std::move(o.horizAnchor);
	vertAnchor = std::move(o.vertAnchor);
	moveLastX = std::move(o.moveLastX);
	moveLastY = std::move(o.moveLastY);
	moveable = std::move(o.moveable);
	movingToCenter = std::move(o.movingToCenter);
	closeable = std::move(o.closeable);
	closed = std::move(o.closed);

	dragStartEvt.setCb(std::bind(&Window::pointerDownTitle, this));
	dragEvt.setCb(std::bind(&Window::pointerMove, this));
	dragEndEvt.setCb(std::bind(&Window::pointerUp, this));
	if (closeBtn.has_value()) {
		closeBtn->setCb(std::bind(&Window::close, this));
	}

	return *this;
}

Window::~Window() { }

void Window::moveToCenter(bool slowMethod, bool hideUntilCentered) {
	if (slowMethod) { // just css/js things
		movingToCenter = true;
		if (hideUntilCentered) {
			setProperty("style.visibility", "hidden");
		}

		eui_wait_n_frames(2, this, [] (void * d) {
			Window& w = *static_cast<Window*>(d);
			w.setProperty("style.visibility", "");
			if (w.movingToCenter) {
				w.movingToCenter = false;
				w.moveToCenter(false);
			}

			return false;
		});

		return;
	}

	int ww, wh, ew, eh;
	eui_get_vp_size(&ww, &wh);
	getOffsetSize(&ew, &eh);
	move(ww / 2 - ew / 2, wh / 2 - eh / 2, -1, -1);
}

void Window::move(int newX, int newY, std::int8_t newHorizAnchor, std::int8_t newVertAnchor) {

	if (newX == x && newY == y && horizAnchor == newHorizAnchor && vertAnchor == newVertAnchor) {
		return;
	}

	// cancel moving to center if move is called before it finishes
	movingToCenter = false;

	if (horizAnchor != newHorizAnchor) {
		setProperty("style.right", newHorizAnchor > 0 ? "0" : "");
	}

	if (vertAnchor != newVertAnchor) {
		setProperty("style.bottom", newVertAnchor > 0 ? "0" : "");
	}

	setProperty("style.transform", svprintf<bufSz>("translate(%i%%,%i%%) translate(%ipx,%ipx)",
			newHorizAnchor > 0 ? 100 : 0,
			newVertAnchor > 0 ? 100 : 0,
			newX, newY));

	x = newX;
	y = newY;
	horizAnchor = newHorizAnchor;
	vertAnchor = newVertAnchor;
}

bool Window::isClosed() {
	return closed;
}

bool Window::open() {
	if (closed) {
		bringUp(true);
		closed = false;
		setClickBringUpEnabled(true);
	}

	return true;
}

bool Window::close() {
	if (!closed) {
		remove();
		movingToCenter = false;
		closed = true;
		setClickBringUpEnabled(false);
	}

	return true;
}

bool Window::toggle() {
	bool cl = isClosed();
	if (cl) {
		open();
	} else {
		close();
	}

	return cl;
}


//void Window::resize(unsigned int newWidth, unsigned int newHeight) {
//	if (newWidth != width) {
//		setProperty("style.width", svprintf<bufSz>("%ipx", newWidth));
//		width = newWidth;
//	}
//
//	if (newHeight != height) {
//		setProperty("style.height", svprintf<bufSz>("%ipx", newHeight));
//		height = newHeight;
//	}
//}

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

bool Window::pointerDownTitle() {
	int cx, cy;
	eui_get_evt_pointer_coords(&cx, &cy, true);
	moveLastX = cx;
	moveLastY = cy;
	dragEvt.setEnabled(true);
	dragEndEvt.setEnabled(true);
	return false;
}

bool Window::pointerUp() {
	dragEvt.setEnabled(false);
	dragEndEvt.setEnabled(false);

	return false;
}

bool Window::pointerMove() {
	int cx, cy, ww, wh;
	eui_get_evt_pointer_coords(&cx, &cy, true, &ww, &wh);

	int deltaX = cx - moveLastX;
	int deltaY = cy - moveLastY;
	moveLastX = cx;
	moveLastY = cy;

	std::int8_t newHorizAnchor = cx > ww / 2 ? 1 : -1;
	std::int8_t newVertAnchor = cy > wh / 2 ? 1 : -1;

	int absX = horizAnchor > 0 ? ww + x : x;
	int absY = vertAnchor > 0 ? wh + y : y;
	absX += deltaX;
	absY += deltaY;
	int newX = newHorizAnchor > 0 ? absX - ww : absX;
	int newY = newVertAnchor > 0 ? absY - wh : absY;

	move(newX, newY, newHorizAnchor, newVertAnchor);

	return false;
}

} // namespace eui
