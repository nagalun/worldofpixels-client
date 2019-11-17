#include "Camera.hpp"

Camera::Camera()
: x(0.f),
  y(0.f),
  zoom(1.f) { }

float Camera::getX() const {
	return x;
}

float Camera::getY() const {
	return y;
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

