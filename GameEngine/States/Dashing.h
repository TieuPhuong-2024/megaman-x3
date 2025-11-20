#pragma once

#include "PlayerState.h"

class Dashing :
    public PlayerState
{
public:
	eStatus getState() override;
	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;

private:
	float _timeDash{ 0.f };
};
