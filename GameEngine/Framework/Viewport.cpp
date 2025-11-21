#include "Viewport.h"

Viewport::Viewport()
{
}

Viewport::Viewport(float x, float y)
{
	_positionWorld.x = x;
	_positionWorld.y = y;
}

Viewport::Viewport(float x, float y, float width, float height)
{
	_positionWorld.x = x;
	_positionWorld.y = y;
	_width = width;
	_height = height;
	_zoom = 1.0f;
}

Viewport::~Viewport()
{
}

void Viewport::setPositionWorld(GVector2 position)
{
	if (_positionWorld != position)
		_positionWorld = position;
}

GVector2 Viewport::getPositionWorld()
{
	return _positionWorld;
}

void Viewport::setZoom(float zoom)
{
	_zoom = zoom;
	if (_zoom < 0.1f) _zoom = 0.1f; // min zoom
	if (_zoom > 5.0f) _zoom = 5.0f; // max zoom
}

float Viewport::getZoom()
{
	return _zoom;
}

float Viewport::getWidth()
{
	return _width;
}

float Viewport::getHeight()
{
	return _height;
}

GVector3 Viewport::getPositionInViewport(GVector3* position)
{
	D3DXMATRIX mt;
	D3DXVECTOR4 posViewport;

	D3DXMatrixIdentity(&mt);
	mt._11 = _zoom;
	mt._22 = -_zoom;
	mt._41 = (-_zoom) * _positionWorld.x;
	mt._42 = _zoom * _positionWorld.y;

	D3DXVec3Transform(&posViewport, position, &mt);

	return GVector3(posViewport.x, posViewport.y, posViewport.z);
}

bool Viewport::isContains(const RECT& rect)
{
	if (rect.right < _positionWorld.x)
		return false;
	if (rect.bottom > _positionWorld.y)
		return false;
	if (rect.left > _positionWorld.x + _width)
		return false;
	if (rect.top < _positionWorld.y - _height)
		return false;
	return true;
}

RECT Viewport::getBounding()
{
	RECT rect;
	rect.left = _positionWorld.x;
	rect.top = _positionWorld.y;
	rect.bottom = rect.top - _height;
	rect.right = rect.left + _width;

	return rect;
}
