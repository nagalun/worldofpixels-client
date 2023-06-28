#include "Camera.hpp"
#include "util/emsc/time.hpp"

#include <emscripten.h>
#include <cmath>
#include <cstdio>
#include <chrono>

static const float timeConstantMs = 230.f;// -(1000.0f / 60.0f) / std::log(0.93f);

Camera::Camera()
: x(0.f),
  y(0.f),
  zoom(16.f),
  momentumDx(0.f),
  momentumDy(0.f),
  momentumStartTs(0.f) { }

Camera::~Camera() { }

float Camera::getX() const {
	float integ;
	float fract = std::modf(x, &integ);
	return integ + std::round(fract * zoom) / zoom;
}

float Camera::getY() const {
	float integ;
	float fract = std::modf(y, &integ);
	return integ + std::round(fract * zoom) / zoom;
}

float Camera::getDx() const {
	float elapsedMs = getTime() * 1000.f - momentumStartTs;
	float curExp = std::exp(-elapsedMs / timeConstantMs);
	float curDx = momentumDx * curExp;
	return curDx;
}

float Camera::getDy() const {
	float elapsedMs = getTime() * 1000.f - momentumStartTs;
	float curExp = std::exp(-elapsedMs / timeConstantMs);
	float curDy = momentumDy * curExp;
	return curDy;
}

float Camera::getZoom() const {
	return zoom;
}

void Camera::setPos(float nx, float ny) {
	x = nx;
	y = ny;
	recalculateCursorPosition();
}

void Camera::setZoom(float nz) {
	zoom = nz;
	recalculateCursorPosition();
}

void Camera::setZoom(float nz, float ox, float oy) {
	float dz = std::abs(nz - zoom);
	float dx = (ox - x) / nz * dz;
	float dy = (oy - y) / nz * dz;
	if (nz < zoom) {
		dx = -dx;
		dy = -dy;
	}
	zoom = nz;
	translate(dx, dy);
}

void Camera::getWorldPosFromScreenPos(float sx, float sy, float *wx, float *wy) const {
	float z = getZoom();
	float worldX = getX();
	float worldY = getY();
	float screenX = sx;
	float screenY = sy;
	double screenW;
	double screenH;
	getScreenSize(&screenW, &screenH);
	// coords origin to the center of the screen
	screenX -= screenW / 2.f;
	screenY -= screenH / 2.f;

	// apply to the camera coords and zoom
	worldX += screenX / z;
	worldY += screenY / z;

	*wx = worldX;
	*wy = worldY;
}

void Camera::translate(float dx, float dy) {
	setPos(x + dx, y + dy);
}

void Camera::setMomentum(float dx, float dy) {
	momentumDx = dx;
	momentumDy = dy;
	if (momentumDx != 0.f || momentumDy != 0.f) {
		momentumStartTs = getTime(true) * 1000.f;
	}
}

bool Camera::applyMomentum(float now, float dt) {
	if (momentumDx == 0.f && momentumDy == 0.f) {
		return false;
	}

	now *= 1000.f;
	dt *= 1000.f;

	float elapsedMs = now - momentumStartTs;
	float curExp = std::exp(-elapsedMs / timeConstantMs);
	float lastExp = std::exp(-(elapsedMs - dt) / timeConstantMs);
	lastExp = lastExp > 1.f ? 1.f : lastExp;

	float curDx = momentumDx * curExp;
	float curDy = momentumDy * curExp;
	float lastDx = momentumDx * lastExp;
	float lastDy = momentumDy * lastExp;
	float diffDx = lastDx - curDx;
	float diffDy = lastDy - curDy;

	translate(diffDx, diffDy);

	// stop when remaining movement difference is less than a screen pixel
	if (std::abs(curDx) + std::abs(curDy) < 1.f / zoom) {
		momentumDx = 0.f;
		momentumDy = 0.f;
		momentumStartTs = 0.f;
	}

	return momentumDx != 0.f || momentumDy != 0.f;
}
