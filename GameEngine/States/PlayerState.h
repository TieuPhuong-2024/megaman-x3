#pragma once

// Core player/state includes
#include "../Framework/define.h"
#include "../Object/CPlayer.h"
#include <memory>

// Forward declarations to avoid circular includes
class Movement;
class Gravity;

class PlayerState
{
public:
	PlayerState(CPlayer *player);
	~PlayerState();

	virtual void update(float deltaTime);
	virtual void updateInput(float deltaTime);
	virtual eStatus getState() = 0;

	// Request a state change on the player
	void setState(std::unique_ptr<PlayerState> newState, float transitionTime = 0.0f);

	// Provide read access to the player for states and other systems
	CPlayer *getPlayer();
	Movement *getMovement() { return _movement; }
	Gravity *getGravity() { return _gravity; }

private:
	// The player instance shared across states
	CPlayer *_player;

	// Cached component pointers for convenience (populated in ctor)
	Movement *_movement;
	Gravity *_gravity;
};