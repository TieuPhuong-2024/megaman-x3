#pragma once
#include "PlayerState.h"

class Standing : public PlayerState
{
public:
	Standing();
	static Standing* create();

	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;
	eStatus getState() override;

};