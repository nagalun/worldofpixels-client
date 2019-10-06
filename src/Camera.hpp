#pragma once

// camera should be the center of the viewport
class Camera {
	float x;
	float y;
	float zoom;

public:
	Camera();

	float getX() const;
	float getY() const;
	float getZoom() const;

	void setX(float);
	void setY(float);
	void setZoom(float);
	
	void translate(float, float);
};
