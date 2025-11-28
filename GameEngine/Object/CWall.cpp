#include "CWall.h"

CWall::CWall(float x, float y, float width, float height, int id)
{
	_x = x;
	_y = y;
	_width = width;
	_height = height;
	_id = id;
}

CWall::CWall()
{
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
	_id = 0;
}

CWall::~CWall()
{
}

// Collision interface
RECT CWall::GetBoundingBox() const
{
	RECT bound;
	bound.left = (LONG)_x;
	bound.bottom = (LONG)_y;  // Đáy wall
	bound.right = (LONG)(_x + _width);
	bound.top = (LONG)(_y + _height);  // Đỉnh wall
	return bound;
}

GVector2 CWall::GetPosition() const
{
	return { _x, _y };
}

void CWall::SetPosition(GVector2 pos)
{
	_x = pos.x;
	_y = pos.y;
}

void CWall::SetPosition(float x, float y)
{
	SetPosition(GVector2(x, y));
}

void CWall::GetSpeed(float& vx, float& vy) const
{
	vx = 0.0f;
	vy = 0.0f;
}

GVector2 CWall::GetVelocity() const
{
	return GVector2(0.0f, 0.0f);
}

void CWall::SetVelocity(GVector2 vel)
{
	// Wall doesn't move
}
