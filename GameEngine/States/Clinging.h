#pragma once
#include "PlayerState.h"

/*
 Clinging state:
 - Intended to be used when player is contacting a wall while airborne.
 - While clinging:
    * Gravity effect is reduced so the player descends slowly.
    * Horizontal velocity is locked (player clings in place on the wall).
    * Player can press jump to perform a wall-jump (transition to Jumping).
    * Clinging has a timeout (max cling time) after which the player will transition to Falling.
 - This header declares the Clinging state class; implementation should follow the project's
   pattern in a .cpp, but the interface and default parameters are provided here.
*/

class Clinging : public PlayerState
{
public:
	Clinging(CPlayer* player);
	~Clinging();

	// Called every frame to update internal timers or apply modified physics.
	void update(float deltaTime) override;

	// Called to handle input while in cling state (e.g. jump to wall-jump, drop off).
	void updateInput(float deltaTime) override;

	// Return the associated eStatus value for this state.
	eStatus getState() override;

private:
	// Time (in seconds) the player has been clinging.
	float _clingTime{ 0.0f };

	// Maximum time allowed for cling before falling (seconds).
	const float _maxClingTime{ 1.5f };

	// Gravity to apply while clinging (weaker downward pull).
	const float _clingGravityY{ -2.0f };

	// Horizontal zeroing to keep the player fixed along X while clinging.
	const float _clingVx{ 0.0f };
};