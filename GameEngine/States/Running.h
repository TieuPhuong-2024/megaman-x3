#pragma once
#include "PlayerState.h"

class Running : public PlayerState
{
public:
	Running();
	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;
	eStatus getState() override;
};