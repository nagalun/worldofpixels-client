#pragma once

#include <functional>
#include <cstdint>

#include <util/gl/GlContext.hpp>

namespace gl {

class WebGlContext : public GlContext {
	std::int32_t ctxInfo;
	const char * targetCanvas;
	const char * targetSizeElem;
	mutable Size sizeCache;
	bool renderLoopSet;
	bool renderPaused;

public:
	WebGlContext(const char * targetCanvas = "#canvas", const char * targetSizeElem = "#canvas", bool forceWebgl1 = false);
	~WebGlContext();

	WebGlContext(WebGlContext&& other);
	WebGlContext& operator=(WebGlContext&& other);

	bool resize(int w, int h);
	GlContext::Size getSize() const;
	GlContext::Size getDipSize() const;
	void setTitle(const char*);
	double getTime() const;

	bool ok() const;

	bool pauseRendering();
	bool resumeRendering();
	void startRenderLoop(void (*)(void *), void * user);
	void stopRenderLoop();

	bool activateRenderingContext(bool forceWebgl1 = false);
	void destroyRenderingContext();
	void giveUp();

	static int emEvent(int eventType, const void *, void * r);

private:
	bool activateRenderingContextAs(bool webgl1);
};

} /* namespace gl */
