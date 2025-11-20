#pragma once
#include "Animation.h"

class BlendTree
{
public:
	BlendTree(Animation* animA, Animation* animB);
	~BlendTree();

	void update(float dt, float blendParam); // blendParam 0 = A, 1 = B
	void draw(LPD3DXSPRITE spriteHandle, Viewport* viewport);

private:
	Animation* _animA;
	Animation* _animB;
	float _blendParam;
};