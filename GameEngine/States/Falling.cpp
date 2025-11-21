#include "Falling.h"
#include "Standing.h"
#include "Clinging.h"

#define GRAVITY			-150.f
#define VELOCITY_X		200.f
#define ACCELERATE_X	9.81f
#define VELOCITY_Y		-250.f
#define MAX_FALL_SPEED  -800.f


Falling::Falling()
{
	_gravity->setStatus(eGravityStatus::FALLING__DOWN);
	_gravity->setgy(GRAVITY);

	// Ensure a baseline downward velocity when entering falling state
	if (_movement->getVelocity().y > VELOCITY_Y)
	{
		_movement->setVy(VELOCITY_Y);
	}
}

Falling::~Falling()
{
}

eStatus Falling::getState()
{
	return eStatus::FALL;
}

void Falling::update(float deltaTime)
{
	// Landing detection: rely on collision system to trigger transition to Standing
	// If gravity is shallowed (from collision), transition to standing
	if (_gravity->getStatus() == eGravityStatus::SHALLOWED)
	{
		this->setState(new Standing, 0.1f); // Smooth landing transition
		return;
	}

	// Clamp maximum fall speed to avoid excessively fast falling
	auto vy = _movement->getVelocity().y;
	if (vy < MAX_FALL_SPEED)
	{
		_movement->setVy(MAX_FALL_SPEED);
	}
}

void Falling::updateInput(float deltaTime)
{
	// Allow horizontal control while falling (air control). This gives the player
	// the ability to steer mid-air without making air control too strong.
	if (InputController::getInstance()->isKeyDown(DIK_LEFTARROW))
	{
		_player->setFlipX(true);
		_player->setMoveDirection(eMoveDirection::MOVE_LEFT);
		_movement->setAccelx(-ACCELERATE_X);
		_movement->setVx(-VELOCITY_X);
	}
	else if (InputController::getInstance()->isKeyDown(DIK_RIGHTARROW))
	{
		_player->setFlipX(false);
		_player->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		_movement->setAccelx(ACCELERATE_X);
		_movement->setVx(VELOCITY_X);
	}
	else
	{
		// No horizontal input: give limited air friction (handled inside Movement update)
		_player->setMoveDirection(eMoveDirection::NONE);
		_movement->setAccelx(0.0f);
	}

	// Maintain a minimum downward velocity baseline so the player doesn't float.
	auto currentVy = _movement->getVelocity().y;
	if (currentVy > VELOCITY_Y)
	{
		_movement->setVy(VELOCITY_Y);
	}

	// Note:
	// - To support wall-cling, collision logic should call setState(new Clinging)
	//   when appropriate (player contacting a wall while airborne).
}