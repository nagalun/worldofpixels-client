#pragma once

// camera should be the center of the viewport
class Camera {
protected:
	float x;
	float y;
	float zoom;
	float momentumDx;
	float momentumDy;
	float momentumStartTs;

public:
	Camera();
	virtual ~Camera();

	float getX() const;
	float getY() const;
	float getDx() const;
	float getDy() const;
	float getZoom() const;
	void getWorldPosFromScreenPos(float sx, float sy, float * wx, float * wy) const;

	virtual double getScreenDpr() const = 0;
	virtual void getScreenSize(double * w, double * h) const = 0;
	virtual void setPos(float x, float y);
	virtual void setZoom(float z);
	virtual void setZoom(float z, float ox, float oy); // with non-center camera-coords origin

	virtual void translate(float dx, float dy);
	virtual void setMomentum(float dx, float dy);

protected:
	bool applyMomentum(float now, float dt);

private:
	virtual void recalculateCursorPosition() const = 0;
};
