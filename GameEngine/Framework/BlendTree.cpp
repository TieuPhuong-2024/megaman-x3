#include "BlendTree.h"

BlendTree::BlendTree(Animation* animA, Animation* animB)
	: _animA(animA), _animB(animB), _blendParam(0.0f)
{
}

BlendTree::~BlendTree()
{
}

void BlendTree::update(float dt, float blendParam)
{
	_blendParam = blendParam;
	_animA->update(dt);
	_animB->update(dt);
}

void BlendTree::draw(LPD3DXSPRITE spriteHandle, Viewport* viewport)
{
	// Lerp between A and B based on _blendParam
	RECT rectA = _animA->getCurrentRect();
	RECT rectB = _animB->getCurrentRect();
	RECT blendedRect;
	blendedRect.left = (LONG)(rectA.left * (1.0f - _blendParam) + rectB.left * _blendParam);
	blendedRect.top = (LONG)(rectA.top * (1.0f - _blendParam) + rectB.top * _blendParam);
	blendedRect.right = (LONG)(rectA.right * (1.0f - _blendParam) + rectB.right * _blendParam);
	blendedRect.bottom = (LONG)(rectA.bottom * (1.0f - _blendParam) + rectB.bottom * _blendParam);

	GVector2 originA = _animA->getCurrentOrigin();
	GVector2 originB = _animB->getCurrentOrigin();
	GVector2 blendedOrigin = originA * (1.0f - _blendParam) + originB * _blendParam;

	// Assuming sprite is shared
	Sprite* spriteSheet = _animA->getSpriteSheet();
	spriteSheet->setFrameRect(blendedRect);
	spriteSheet->setOrigin(blendedOrigin);
	spriteSheet->render(spriteHandle, viewport);
}