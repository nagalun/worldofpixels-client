#pragma once

#include <functional>
#include <cstdint>

#include <util/gl/GlContext.hpp>
#include <Settings.hpp>

struct EmscriptenUiEvent;

namespace gl {

class WebGlContext : public GlContext {
	std::int32_t ctxInfo;
	const char * targetCanvas;
	const char * targetSizeElem;
	mutable Size<int> sizeCache;
	mutable Size<double> dipSizeCache;
	mutable Size<int> realSizeCache;
	mutable double dprCache;
	bool renderLoopSet;
	bool renderPaused;

	// this shouldn't be here
	decltype(Settings::nativeRes)::SlotKey sk;

public:
	WebGlContext(const char * targetCanvas = "#canvas", const char * targetSizeElem = "#canvas", bool forceWebgl1 = false);
	~WebGlContext();

	WebGlContext(WebGlContext&& other);
	WebGlContext& operator=(WebGlContext&& other);

	bool resize(int w, int h);
	bool resize(int drawingWidth, int drawingHeight, double elemWidth, double elemHeight);
	GlContext::Size<int> getSize() const;
	GlContext::Size<double> getDipSize() const;
	GlContext::Size<int> getRealSize() const;
	void setTitle(const char*);
	double getTime() const;
	double getDpr() const;

	bool ok() const;

	bool pauseRendering();
	bool resumeRendering();
	void startRenderLoop(void (*)(void *), void * user);
	void stopRenderLoop();

	bool activateRenderingContext(bool forceWebgl1 = false);
	void destroyRenderingContext();
	void giveUp();

	static int emEvent(int eventType, const void *, void * r);
	static int onEvtResize(int, const EmscriptenUiEvent *, void * ctx);

private:
	bool activateRenderingContextAs(bool webgl1);
};

} /* namespace gl */
