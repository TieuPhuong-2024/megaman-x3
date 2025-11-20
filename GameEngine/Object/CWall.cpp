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

// ICollidable implementation
RECT CWall::getBoundingBox() const
{
    RECT bound;
    bound.left = _x;
    bound.bottom = _y;  // Đáy wall
    bound.right = _x + _width;
    bound.top = _y + _height;  // Đỉnh wall
    return bound;
}

GVector2 CWall::getPosition()
{
    return {_x, _y};
}

void CWall::setPosition(GVector2 pos)
{
    _x = pos.x;
    _y = pos.y;
}

void CWall::SetPosition(float x, float y)
{
    setPosition(GVector2(x, y));
}

bool CWall::IsBlocking()
{
    return true;
}
