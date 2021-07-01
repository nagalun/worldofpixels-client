#include "GlContext.hpp"

namespace gl {

GlContext::~GlContext() { }

void GlContext::onRestored(std::function<void(void)> cb) {
	restoredCb = std::move(cb);
}

void GlContext::onLost(std::function<void(void)> cb) {
	lostCb = std::move(cb);
}

void GlContext::onResize(std::function<void(void)> cb) {
	resizeCb = std::move(cb);
}

} /* namespace gl */
