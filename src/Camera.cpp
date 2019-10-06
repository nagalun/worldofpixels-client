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

void Camera::setX(float nx) {
	x = nx;
}

void Camera::setY(float ny) {
	y = ny;
}

void Camera::setZoom(float nz) {
	zoom = nz;
}

void Camera::translate(float dx, float dy) {
	x += dx;
	y += dy;
}

