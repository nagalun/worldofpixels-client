#pragma once

#include <functional>

namespace gl {

class GlContext {
protected:
	std::function<void(void)> restoredCb;
	std::function<void(void)> lostCb;
	std::function<void(void)> resizeCb;

public:
	template<typename T = int>
	struct Size {
		T w;
		T h;
	};

	GlContext() = default;
	virtual ~GlContext();

	GlContext(GlContext &&other) = default;
	GlContext(const GlContext &other) = delete;
	GlContext& operator=(GlContext &&other) = default;
	GlContext& operator=(const GlContext &other) = delete;

	virtual void setTitle(const char *) = 0;
	virtual bool resize(int w, int h) = 0;
	virtual Size<int> getSize() const = 0;
	virtual Size<double> getDipSize() const = 0;
	virtual double getTime() const = 0;
	virtual double getDpr() const = 0;

	virtual bool ok() const = 0;

	virtual bool pauseRendering() = 0;
	virtual bool resumeRendering() = 0;
	virtual void startRenderLoop(void (*)(void *), void * user) = 0;
	virtual void stopRenderLoop() = 0;

	void onRestored(std::function<void(void)>);
	void onLost(std::function<void(void)>);
	void onResize(std::function<void(void)>);
};

} /* namespace gl */
