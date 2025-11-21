#pragma once

#include "PlayerState.h"

class Dashing :
    public PlayerState
{
public:
	Dashing(CPlayer* player);
	~Dashing();
	eStatus getState() override;
	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;

private:
	float _timeDash{ 0.f };
};
