#include "Sprite.h"
#include "../trace.h"

Sprite::~Sprite()
{
}

Sprite::Sprite(LPD3DXSPRITE spriteHandle, LPCWSTR filePath, int totalFrames, int cols)
{
	_origin = GVector2(0.5f, 0.5f);
	_scale = GVector2(1.0f, 1.0f);
	_zIndex = 1;
	_rotate = 0.0f;
	_IsFlipX = false;
	_IsFlipY = false;

	GAMELOG("Loading sprite from file: %ls", filePath);
	auto rs = _texture.loadFromFile(spriteHandle, filePath);
	if (rs != D3D_OK)
		throw;

	_totalFrames = totalFrames;
	_columns = cols;
	_textureWidth = _texture.getWidth();
	_textureHeight = _texture.getHeight();
	_frameWidth = _textureWidth / cols;
	_frameHeight = _textureHeight * cols / totalFrames;
	_index = 0;
	_currentFrame = GVector2(0, 0);

	this->setIndex(0);
	this->updateBounding();

	_isDrawBounding = false;
	_surface = nullptr;

	// create surface
	DeviceManager::getInstance()->getDevice()->CreateOffscreenPlainSurface(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&_surface,
		NULL);
}

void Sprite::release()
{
	this->_texture.release();
}

void Sprite::render(LPD3DXSPRITE spriteHandle)
{
	_texture.render(
		spriteHandle,
		&_frameRect,
		GVector2((int)_position.x, (int)_position.y),
		_scale,
		_rotate,
		_origin,
		_zIndex);
}

void Sprite::render(LPD3DXSPRITE spriteHandle, Viewport *viewport)
{
	_texture.render(
		spriteHandle,
		&_frameRect,
		*viewport,
		_position,
		_scale,
		_rotate,
		_origin,
		_zIndex);

	// Vẽ bounding để xem
	if (_surface == nullptr || _isDrawBounding == false)
	{
		return;
	}

	RECT r;
	r.top = WINDOW_HEIGHT - _bound.top;
	r.left = _bound.left;
	r.bottom = WINDOW_HEIGHT - _bound.bottom;
	r.right = _bound.right;

	// Cache device pointer to avoid multiple getDevice() calls
	LPDIRECT3DDEVICE9 device = DeviceManager::getInstance()->getDevice();
	device->ColorFill(_surface, NULL, D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f));
	device->StretchRect(
		_surface,
		NULL,
		DeviceManager::getInstance()->getSurface(),
		&r,
		D3DTEXF_NONE);
}

void Sprite::setPosition(float x, float y, float z)
{
	GVector3 v(x, y, z);
	this->setPosition(v);
}

void Sprite::setPosition(GVector3 vector)
{
	this->_position = GVector2(vector.x, vector.y);
	this->updateBounding();
}

void Sprite::setPosition(GVector2 position)
{
	this->_position = GVector2(position.x, position.y);
	this->updateBounding();
}

void Sprite::setPositionX(float x)
{
	if (x == _position.x)
		return;

	_position.x = x;
	this->updateBounding();
}

void Sprite::setPositionY(float y)
{
	if (y == _position.y)
		return;

	_position.y = y;
	this->updateBounding();
}

void Sprite::setScale(GVector2 scale)
{
	if (scale == _scale)
		return;

	_scale = scale;
	this->updateBounding();
}

void Sprite::setScale(float scale)
{
	// Use AND to detect if BOTH x and y are already equal to scale (no change needed)
	if (scale == _scale.x && scale == _scale.y)
		return;

	_scale.x = scale;
	_scale.y = scale;
	this->updateBounding();
}

void Sprite::setScaleX(float sx)
{
	if (sx == _scale.x)
		return;

	_scale.x = sx;
	this->updateBounding();
}

void Sprite::setScaleY(float sy)
{
	if (sy == _scale.y)
		return;

	_scale.y = sy;
	this->updateBounding();
}

void Sprite::setRotate(float degree)
{
	if (degree == _rotate)
		return;

	_rotate = degree;
	this->updateBounding();
}

void Sprite::setOrigin(GVector2 origin)
{
	if (origin == _origin)
		return;

	_origin = origin;
	_anchorPoint = GVector2(_bound.left + _frameWidth * _scale.x * _origin.x, _bound.bottom + _frameHeight * _scale.y * _origin.y);

	this->updateBounding();
}

void Sprite::setZIndex(float z)
{
	if (z != _zIndex)
		_zIndex = z;
}

RECT Sprite::getBounding() const
{
	return _bound;
}

void Sprite::setFrameRect(RECT rect)
{
	_frameRect = rect;

	_frameWidth = abs(_frameRect.left - _frameRect.right);
	_frameHeight = abs(_frameRect.top - _frameRect.bottom);

	this->updateBounding();
}

void Sprite::setFrameRect(float top, float right, float bottom, float left)
{
	_frameRect.top = top;
	_frameRect.right = right;
	_frameRect.left = left;
	_frameRect.bottom = bottom;

	_frameWidth = abs(_frameRect.left - _frameRect.right);
	_frameHeight = abs(_frameRect.top - _frameRect.bottom);

	this->updateBounding();
}

void Sprite::setFrameRect(float x, float y, int width, int height)
{
	_frameRect.top = y;
	_frameRect.right = x + width;
	_frameRect.left = x;
	_frameRect.bottom = y + height;

	_frameWidth = width;
	_frameHeight = height;

	this->updateBounding();
}

RECT Sprite::getFrameRect() const
{
	return _frameRect;
}

RECT Sprite::getFrameRectByIndex(int index) const
{
	index = index % _totalFrames;
	RECT rect;
	rect.left = (long)_currentFrame.x * _frameWidth;
	rect.right = _frameRect.left + _frameWidth;
	rect.top = (long)_currentFrame.y * _frameHeight;
	rect.bottom = _frameRect.top + _frameHeight;
	return rect;
}

void Sprite::nextFrame()
{
	if (_totalFrames <= 1)
		return;

	this->setIndex(_index + 1);
}

void Sprite::setIndex(int index)
{
	if (_index != index)
		_index = index;

	this->setCurrentFrame();
}

int Sprite::getFrameWidth() const
{
	return _frameWidth * abs(_scale.x);
}

int Sprite::getFrameHeight() const
{
	return _frameHeight * abs(_scale.y);
}

int Sprite::getTextureWidth() const
{
	return _textureWidth * abs(_scale.x);
}

int Sprite::getTextureHeight() const
{
	return _textureHeight * abs(_scale.y);
}

void Sprite::drawBounding(bool draw)
{
	if (draw != _isDrawBounding)
		_isDrawBounding = draw;
}

void Sprite::setOpacity(float opacity)
{
	if (_opacity == opacity)
		return;

	_opacity = opacity;
	_texture.setColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, _opacity));
}

float Sprite::getOpacity() const
{
	return _opacity;
}

void Sprite::setColor(D3DXCOLOR color)
{
	_color = color;
	_texture.setColor(_color);
}

D3DXCOLOR Sprite::getColor() const
{
	return _color;
}

void Sprite::setFrameRect()
{
	this->_frameRect.left = (long)_currentFrame.x * _frameWidth;
	this->_frameRect.right = _frameRect.left + _frameWidth;
	this->_frameRect.top = (long)_currentFrame.y * _frameHeight;
	this->_frameRect.bottom = _frameRect.top + _frameHeight;
}

void Sprite::setCurrentFrame()
{
	if (_index >= _totalFrames)
		_index = _index % _totalFrames;

	this->_currentFrame.x = _index % _columns;
	this->_currentFrame.y = _index / _columns;

	this->setFrameRect();
}

void Sprite::updateBounding()
{
	float scaleW = _frameWidth * abs(_scale.x);
	float scaleH = _frameHeight * abs(_scale.y);

	_bound.left = _position.x - scaleW * _origin.x;
	_bound.bottom = _position.y - scaleH * _origin.y;
	_bound.right = _bound.left + scaleW;
	_bound.top = _bound.bottom + scaleH;

	_anchorPoint = GVector2(_bound.left + scaleW * _origin.x, _bound.bottom + scaleH * _origin.y);

	// Skip rotation calculations if angle is 0 (optimization for most common case)
	if (_rotate == 0.0f)
	{
		return;
	}

	// Calculate rotated points and find min/max bounds
	float rad = -_rotate * (M_PI / 180);
	float sinVal = sin(rad);
	float cosVal = cos(rad);

	// Relative positions from anchor point
	float halfW = scaleW * 0.5f;
	float halfH = scaleH * 0.5f;

	// Four corners relative to anchor point
	float cornersX[4] = {-halfW, halfW, halfW, -halfW};
	float cornersY[4] = {halfH, halfH, -halfH, -halfH};

	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;

	for (int i = 0; i < 4; i++)
	{
		// Rotate around origin
		float rotatedX = cornersX[i] * cosVal - cornersY[i] * sinVal;
		float rotatedY = cornersX[i] * sinVal + cornersY[i] * cosVal;

		// Translate back to absolute position
		rotatedX += _anchorPoint.x;
		rotatedY += _anchorPoint.y;

		if (rotatedX < minX)
			minX = rotatedX;
		if (rotatedX > maxX)
			maxX = rotatedX;
		if (rotatedY < minY)
			minY = rotatedY;
		if (rotatedY > maxY)
			maxY = rotatedY;
	}

	_bound.left = (long)minX;
	_bound.top = (long)maxY;
	_bound.right = (long)maxX;
	_bound.bottom = (long)minY;
}

GVector2 Sprite::rotatePointAroundOrigin(GVector2 point, float angle, GVector2 origin)
{
	// Early return for no rotation (optimization)
	if (angle == 0.0f)
		return point;

	// nhân ma trận xoay
	/*
		x' = x.cos(t) - y.sin(t)
		y' = x.sin(t) + y.cos(t)

		t là góc quay theo radian
		vậy quanh quanh 1 điểm mình dời về góc rồi quay xong dời lại
	*/

	GVector2 newPoint;
	// trừ vì sprite xoay với cái này lệch 90*
	float rad = -angle * (M_PI / 180);

	float _sin = sin(rad);
	float _cos = cos(rad);

	// dời điểm về góc
	point -= origin;

	// xoay
	newPoint.x = point.x * _cos - point.y * _sin;
	newPoint.y = point.x * _sin + point.y * _cos;

	// dời về chổ cũ
	newPoint += origin;

	return newPoint;
}
