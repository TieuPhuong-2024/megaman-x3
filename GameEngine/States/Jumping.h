#pragma once
#include "PlayerState.h"

class Jumping : public PlayerState
{
public:
	Jumping();
	~Jumping();
	eStatus getState() override;
	void update(float deltaTime) override;
	void updateInput(float deltaTime) override;

private:
	int _jumpCount;
};