#include "Camera.hpp"

Camera::Camera()
: x(0),
  y(0),
  zoom(1) { }

float Camera::getX() const {
	return x;
}

float Camera::getY() const {
	return y;
}

float Camera::getZoom() const {
	return zoom;
}
