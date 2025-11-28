#pragma once
#include "BaseObject.h"
#include <d3dx9.h>
#include "../Framework/define.h"

class CWall : public BaseObject
{
public:
	CWall(float x, float y, float width, float height, int id);
	CWall();
	~CWall();

	// Collision interface
	RECT GetBoundingBox() const override;
	GVector2 GetPosition() const;
	void SetPosition(GVector2 pos);
	void GetSpeed(float &vx, float &vy) const;
	void SetPosition(float x, float y);
	GVector2 GetVelocity() const;
	void SetVelocity(GVector2 vel);

	float GetWidth() const { return _width; }
	void SetWidth(float width) { _width = width; }
	float GetHeight() const { return _height; }
	void SetHeight(float height) { _height = height; }
	float GetX() const { return _x; }
	void SetX(float x) { _x = x; }
	float GetY() const { return _y; }
	void SetY(float y) { _y = y; }

private:
	float _width;
	float _height;
	float _x;
	float _y;
};