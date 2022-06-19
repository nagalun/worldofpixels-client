#include "Camera.hpp"

#include <cmath>

Camera::Camera()
: x(0.f),
  y(0.f),
  zoom(8.f) { }

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
}

void Camera::setZoom(float nz) {
	zoom = nz;
}

void Camera::translate(float dx, float dy) {
	setPos(x + dx, y + dy);
}

