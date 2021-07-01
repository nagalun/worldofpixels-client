#pragma once

#include <functional>
#include <cstdint>

#include <util/gl/GlContext.hpp>

namespace gl {

class WebGlContext : public GlContext {
	std::int32_t ctxInfo;
	const char * targetCanvas;
	mutable Size sizeCache;
	bool renderLoopSet;
	bool renderPaused;

public:
	WebGlContext(const char * targetCanvas = "#canvas");
	~WebGlContext();

	WebGlContext(WebGlContext&& other);
	WebGlContext& operator=(WebGlContext&& other);

	bool resize(int w, int h);
	GlContext::Size getSize() const;
	void setTitle(const char*);
	double getTime() const;

	bool ok() const;

	bool pauseRendering();
	bool resumeRendering();
	void startRenderLoop(void (*)(void *), void * user);
	void stopRenderLoop();

	bool activateRenderingContext(bool webgl1);
	void destroyRenderingContext();

	static int emEvent(int eventType, const void *, void * r);
};

} /* namespace gl */
