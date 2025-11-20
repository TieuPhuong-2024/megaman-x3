#pragma once

// Core player/state includes
#include "../Framework/define.h"
#include "../Object/CPlayer.h"

// Forward declarations to avoid circular includes
class Movement;
class Gravity;

class PlayerState
{
public:
	PlayerState();
	~PlayerState();

	virtual void update(float deltaTime);
	virtual void updateInput(float deltaTime);
	virtual eStatus getState() = 0;

protected:
	// The player instance shared across states
	static CPlayer* _player;

	// Cached component pointers for convenience (populated in ctor)
	Movement*		_movement;
	Gravity*		_gravity;

public:
	// Request a state change on the player
	void setState(PlayerState* newState, float transitionTime = 0.0f);

	// Set the player instance used by states (kept static for existing design)
	static void setPlayer(CPlayer* player);

	// Provide read access to the player for states and other systems
	static CPlayer* getPlayer();
};