#pragma once

// camera should be the center of the viewport
class Camera {
	float x;
	float y;
	float zoom;

public:
	Camera();
	virtual ~Camera();

	float getX() const;
	float getY() const;
	float getZoom() const;

	virtual void setPos(float, float);
	virtual void setZoom(float);
	
	virtual void translate(float, float);
};
