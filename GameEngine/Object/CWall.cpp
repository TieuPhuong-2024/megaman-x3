#include "CWall.h"

CWall::CWall(float x, float y, float width, float height, int id)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;
    _id = id;

    _boundingBox.left = (LONG)x;
    _boundingBox.top = (LONG)y;
    _boundingBox.right = (LONG)(x + width);
    _boundingBox.bottom = (LONG)(y + height);
}

CWall::CWall()
{
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;
    _id = 0;

    _boundingBox.left = 0;
    _boundingBox.top = 0;
    _boundingBox.right = 0;
    _boundingBox.bottom = 0;
}

CWall::~CWall()
{
}

void CWall::update(float deltaTime)
{
}

void CWall::draw(LPD3DXSPRITE spriteHandle, Viewport *viewport)
{
}

// ICollidable implementation
RECT CWall::getBoundingBox() const
{
    return _boundingBox;
}

void CWall::onCollision(ICollidable* other)
{
    // Wall doesn't move, so no action
}

GVector2 CWall::getPosition()
{
    // Center of bounding box
    float x = (_boundingBox.left + _boundingBox.right) / 2.0f;
    float y = (_boundingBox.top + _boundingBox.bottom) / 2.0f;
    return GVector2(x, y);
}

void CWall::setPosition(GVector2 pos)
{
    float width = _boundingBox.right - _boundingBox.left;
    float height = _boundingBox.bottom - _boundingBox.top;
    _boundingBox.left = (LONG)(pos.x - width / 2);
    _boundingBox.right = (LONG)(pos.x + width / 2);
    _boundingBox.top = (LONG)(pos.y - height / 2);
    _boundingBox.bottom = (LONG)(pos.y + height / 2);
}

GVector2 CWall::getVelocity()
{
    return VECTOR2ZERO;  // Walls don't move
}

void CWall::setVelocity(GVector2 vel)
{
    // Do nothing
}

void CWall::OnCollisionWith(CCollisionEvent* e)
{
    // Wall doesn't react
}

void CWall::GetSpeed(float& vx, float& vy)
{
    vx = 0;
    vy = 0;
}

void CWall::SetPosition(float x, float y)
{
    setPosition(GVector2(x, y));
}

bool CWall::IsBlocking()
{
    return true;
}
