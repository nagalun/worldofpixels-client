#include "Camera.hpp"

#include <cmath>
#include <cstdio>

Camera::Camera()
: x(0.f),
  y(0.f),
  zoom(16.f) { }

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
	int screenW;
	int screenH;
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

