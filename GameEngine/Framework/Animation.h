#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include <vector>
#include <stdarg.h>
#include "define.h"
#include "Sprite.h"
#include "IComponent.h"
#include "SpriteManager.h"

class Animation : public IComponent
{
public:
	~Animation();
	Animation(Sprite* spriteSheet, float timeAnimate = 0.0f, bool loop = true);

	void nextFrame();

	void setIndex(int index);

	int getIndex();

	void update(float dt);

	void draw(LPD3DXSPRITE spriteHandle, Viewport* viewport);

	void setTimeAnimate(float time);

	float getTimeAnimate();

	void start();

	void stop();

	void canAnimate(bool can);
	bool isAnimate();
	void addFrameRect(RECT rect);

	void addFrameRect(eID id, const char* firstRectName, ...);

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

	RECT getCurrentRect() const { return _currentRect; }
	GVector2 getCurrentOrigin() const { return _currentOrigin; }
	Sprite* getSpriteSheet() const { return _spriteSheet; }

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

	Sprite* _spriteSheet;
	vector<RECT> _frameRectList;
	vector<GVector2> _frameOriginList;
	string _nameAnimation;
	RECT _currentRect;
	GVector2 _currentOrigin;

	bool _canFlashes;
	float _valueFlashes;
	D3DXCOLOR _flashColor;
};

#endif // !__ANIMATION_H__
