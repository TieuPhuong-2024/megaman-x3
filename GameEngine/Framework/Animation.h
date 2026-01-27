#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <vector>
#include <stdarg.h>
#include "define.h"
#include "Sprite.h"
#include "IComponent.h"
#include "SpriteManager.h"

struct AnimationFrame
{
	RECT rect;
	GVector2 origin;

	AnimationFrame() = default;
	AnimationFrame(const RECT &r, const GVector2 &o) : rect(r), origin(o) {}
};

class Animation : public IComponent
{
public:
	~Animation();
	Animation(Sprite *spriteSheet, float timeAnimate = 0.0f, bool loop = true, int expectedFrames = 8);

	static constexpr float MS_TO_SECONDS = 1.0f / 1000.0f;

	void nextFrame();

	void setIndex(int index);

	int getIndex();

	void update(float dt);

	void draw(LPD3DXSPRITE spriteHandle, Viewport *viewport);

	void setTimeAnimate(float time);

	float getTimeAnimate();

	void start();

	void stop();

	void canAnimate(bool can);
	bool isAnimate();
	void addFrameRect(RECT rect);

	void addFrameRect(eID id, const char *firstRectName, ...);

	void addFrameOrigin(GVector2 origin);

	void setLoop(bool isLoop);
	bool isLoop();

	void restart(int from = 0);

	void enableFlashes(bool enable);

	void setValueFlashes(float value);

	void animateFromTo(int from, int to, bool loop = true);

	void setColorFlash(D3DXCOLOR color);
	D3DXCOLOR getColorFlash();
	bool isLastAnimation();

	RECT getCurrentRect() const { return _currentFrame.rect; }
	GVector2 getCurrentOrigin() const { return _currentFrame.origin; }
	Sprite *getSpriteSheet() const { return _spriteSheet; }

private:
	bool _isAll;
	int _startFrame;
	int _endFrame;
	int _index;
	int _totalFrames;

	float _timeAnimate;
	float _timer;

	bool _canAnimate;
	bool _isLoop;

	Sprite *_spriteSheet;
	vector<AnimationFrame> _frames;
	string _nameAnimation;
	AnimationFrame _currentFrame;

	bool _canFlashes;
	float _valueFlashes;
	D3DXCOLOR _flashColor;

	// Optimization flags
	bool _needsSpriteUpdate;
	bool _lastFlashState;
};

#endif // !__ANIMATION_H__
