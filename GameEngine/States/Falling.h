#pragma once
#include "PlayerState.h"

class Falling : public PlayerState
{
public:
	Falling();
	~Falling();
	eStatus getState() override;
	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;
};