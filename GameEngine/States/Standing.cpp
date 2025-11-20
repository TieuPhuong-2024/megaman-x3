#include "Standing.h"
#include "Running.h"
#include "Jumping.h"
#include "Falling.h"

#define VELOCITY_X 250.f
#define ACCELERATE_X 9.81f
#define EPSILON 1.0f

Standing::Standing()
{
	// Ensure player is stationary while standing
	_movement->setVelocity(VECTOR2ZERO);
	_movement->setAccelerate(VECTOR2ZERO);

	// Mark gravity as shallow when grounded so vertical velocity doesn't accumulate
	_gravity->setStatus(eGravityStatus::SHALLOWED);
}

Standing* Standing::create()
{
	return new Standing;
}

void Standing::update(float deltaTime)
{
	// If gravity indicates we're falling (walked off an edge or similar), transition
	if (_gravity->getStatus() == eGravityStatus::FALLING__DOWN)
	{
		this->setState(new Falling);
		return;
	}

	// If horizontal velocity is significant (external forces), transition to Running
	if (fabs(_movement->getVelocity().x) > EPSILON)
	{
		this->setState(new Running);
		return;
	}

	// Keep movement cleared while standing
	_movement->setVelocity(VECTOR2ZERO);
	_movement->setAccelerate(VECTOR2ZERO);
}

void Standing::updateInput(float deltaTime)
{
	// Default: no horizontal move
	_movement->setVelocity(VECTOR2ZERO);
	_movement->setAccelerate(VECTOR2ZERO);
	_player->setMoveDirection(eMoveDirection::NONE);

	// Horizontal movement: transition to Running and apply velocity/acceleration
	if (InputController::getInstance()->isKeyDown(DIK_LEFTARROW))
	{
		_player->setFlipX(true);
		_player->setMoveDirection(eMoveDirection::MOVE_LEFT);
		_movement->setVx(-VELOCITY_X);
		_movement->setAccelx(-ACCELERATE_X);
		this->setState(new Running);
		return;
	}
	else if (InputController::getInstance()->isKeyDown(DIK_RIGHTARROW))
	{
		_player->setFlipX(false);
		_player->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		_movement->setVx(VELOCITY_X);
		_movement->setAccelx(ACCELERATE_X);
		this->setState(new Running);
		return;
	}

	// If gravity indicates falling (e.g., stepped off a platform), transition
	if (_gravity->getStatus() == eGravityStatus::FALLING__DOWN)
	{
		this->setState(new Falling);
		return;
	}

	// Jump/shoot inputs are handled by CPlayer's key events (eventKeyDown); do not duplicate here.
}

eStatus Standing::getState()
{
	return eStatus::STAND;
}
