#include "Animation.h"
#include "../trace.h"

Animation::Animation(Sprite *spriteSheet, float timeAnimate, bool loop, int expectedFrames)
{
	_spriteSheet = spriteSheet;
	_timeAnimate = timeAnimate;

	_canAnimate = true;
	_totalFrames = 0;
	_index = 0;
	_timer = 0;
	_valueFlashes = 0.5f;

	_startFrame = 0;
	_endFrame = _totalFrames - 1;

	// Reserve capacity for better performance
	_frames.reserve(expectedFrames);
	
	// Initialize optimization flags
	_needsSpriteUpdate = true;
	_lastFlashState = false;

	this->setIndex(0);
	this->setLoop(loop);
	_canFlashes = false;
	_flashColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

Animation::~Animation()
{
}

void Animation::nextFrame()
{
	this->setIndex(_index + 1);
}

int Animation::getIndex()
{
	return _index;
}

void Animation::setIndex(int index)
{
	if (index == _index || _totalFrames == 0)
		return;

	_index = index;

	if (_index < _startFrame)
		_index = _startFrame;

	if (_index > _endFrame)
	{
		if (_isLoop)
		{
			_index = _startFrame;
		}
		else
		{
			_index = _endFrame;
			this->stop();
		}
	}

	// Update current frame and mark for sprite update
	_currentFrame = _frames[_index];
	_needsSpriteUpdate = true;
}

void Animation::update(float dt)
{
	if (!_canFlashes && !_canAnimate)
		return;

	// Use precomputed time scale for better performance
	_timer += dt * MS_TO_SECONDS;
	if (_timer >= _timeAnimate)
	{
		if (_canAnimate)
			this->nextFrame();

		_timer -= _timeAnimate;

		if (_canFlashes)
		{
			// Cache flash state to reduce sprite updates
			bool currentFlashState = (_spriteSheet->getOpacity() == _valueFlashes);
			if (currentFlashState != _lastFlashState)
			{
				_lastFlashState = currentFlashState;
				if (currentFlashState)
				{
					_spriteSheet->setOpacity(_valueFlashes);
					_spriteSheet->setColor(D3DXCOLOR(_flashColor.r, _flashColor.g, _flashColor.b, _valueFlashes));
				}
				else
				{
					_spriteSheet->setOpacity(1.0f);
					_spriteSheet->setColor(D3DXCOLOR(_flashColor.r, _flashColor.g, _flashColor.b, 1.0f));
				}
			}
		}
	}
}

void Animation::setTimeAnimate(float time)
{
	_timeAnimate = time;
}

float Animation::getTimeAnimate()
{
	return _timeAnimate;
}

void Animation::start()
{
	if (_canAnimate == false)
		_canAnimate = true;
}

void Animation::stop()
{
	if (_canAnimate == true)
		_canAnimate = false;
}

void Animation::canAnimate(bool can)
{
	if (_canAnimate != can)
		_canAnimate = can;
}

bool Animation::isAnimate()
{
	return _canAnimate;
}

bool Animation::isLastAnimation()
{
	return !_isLoop && _index == _endFrame;
}

void Animation::addFrameRect(RECT rect)
{
	if (_frames.empty())
	{
		_currentFrame.rect = rect;
		_currentFrame.origin = VECTOR2ZERO;
		_needsSpriteUpdate = true;
	}

	_frames.emplace_back(rect, VECTOR2ZERO);
	_totalFrames = _frames.size();

	_endFrame = _totalFrames - 1;
}

void Animation::addFrameRect(eID id, const char *firstRectName, ...)
{
	va_list vl;
	const char *name;

	va_start(vl, firstRectName);

	name = firstRectName;

	while (name != NULL)
	{
		RECT rect = SpriteManager::getInstance().getSourceRect(id, name);
		GVector2 origin = SpriteManager::getInstance().getOrigin(id, name);
		
		if (_frames.empty())
		{
			_currentFrame.rect = rect;
			_currentFrame.origin = origin;
			_needsSpriteUpdate = true;
		}

		_frames.emplace_back(rect, origin);
		_totalFrames = _frames.size();
		_endFrame = _totalFrames - 1;
		
		name = va_arg(vl, const char *);
	}

	va_end(vl);
}

void Animation::addFrameOrigin(GVector2 origin)
{
	if (_frames.empty())
	{
		_currentFrame.origin = origin;
		_needsSpriteUpdate = true;
	}
	else
	{
		// Update the last frame's origin
		_frames.back().origin = origin;
	}
}

void Animation::setLoop(bool isLoop)
{
	if (isLoop != _isLoop)
		_isLoop = isLoop;
}

bool Animation::isLoop()
{
	return _isLoop;
}

void Animation::restart(int from)
{
	setIndex(from);

	if (_canAnimate == false)
		_canAnimate = true;
}

void Animation::enableFlashes(bool enable)
{
	if (_canFlashes == enable)
		return;

	_canFlashes = enable;
	_spriteSheet->setColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
}

void Animation::setValueFlashes(float value)
{
	if (_valueFlashes != value)
		_valueFlashes = value;
}

void Animation::animateFromTo(int from, int to, bool loop)
{
	if (from <= to)
	{
		_startFrame = from;
		_endFrame = to;
	}
	else
	{
		_startFrame = to;
		_endFrame = from;
	}

	this->setIndex(from);
	_isLoop = loop;
	_timer = 0.0f;

	if (_canAnimate == false)
		_canAnimate = true;
}

void Animation::setColorFlash(D3DXCOLOR color)
{
	_flashColor = color;
}

D3DXCOLOR Animation::getColorFlash()
{
	return _flashColor;
}

void Animation::draw(LPD3DXSPRITE spriteHandle, Viewport *viewport)
{
	if (_needsSpriteUpdate)
	{
		_spriteSheet->setFrameRect(_currentFrame.rect);
		_spriteSheet->setOrigin(_currentFrame.origin);
		_needsSpriteUpdate = false;
	}
	_spriteSheet->render(spriteHandle, viewport);
}
