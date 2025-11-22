#include "Falling.h"
#include "Standing.h"
#include "Clinging.h"
#include <memory>

#define GRAVITY -150.f
#define VELOCITY_X 200.f
#define ACCELERATE_X 9.81f
#define VELOCITY_Y -250.f
#define MAX_FALL_SPEED -800.f

Falling::Falling(CPlayer *player) : PlayerState(player)
{
	getGravity()->setStatus(eGravityStatus::FALLING__DOWN);
	getGravity()->setgy(GRAVITY);

	// Ensure a baseline downward velocity when entering falling state
	if (getMovement()->getVelocity().y > VELOCITY_Y)
	{
		getMovement()->setVy(VELOCITY_Y);
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
	if (getGravity()->getStatus() == eGravityStatus::SHALLOWED)
	{
		this->setState(std::make_unique<Standing>(getPlayer()), 0.1f); // Smooth landing transition
		return;
	}

	// Clamp maximum fall speed to avoid excessively fast falling
	auto vy = getMovement()->getVelocity().y;
	if (vy < MAX_FALL_SPEED)
	{
		getMovement()->setVy(MAX_FALL_SPEED);
	}
}

void Falling::updateInput(float deltaTime)
{
	// Allow horizontal control while falling (air control). This gives the player
	// the ability to steer mid-air without making air control too strong.
	if (InputController::getInstance().isKeyDown(DIK_LEFTARROW))
	{
		getPlayer()->setFlipX(true);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_LEFT);
		getMovement()->setAccelx(-ACCELERATE_X);
		getMovement()->setVx(-VELOCITY_X);
	}
	else if (InputController::getInstance().isKeyDown(DIK_RIGHTARROW))
	{
		getPlayer()->setFlipX(false);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		getMovement()->setAccelx(ACCELERATE_X);
		getMovement()->setVx(VELOCITY_X);
	}
	else
	{
		// No horizontal input: give limited air friction (handled inside Movement update)
		getPlayer()->setMoveDirection(eMoveDirection::NONE);
		getMovement()->setAccelx(0.0f);
	}

	// Maintain a minimum downward velocity baseline so the player doesn't float.
	auto currentVy = getMovement()->getVelocity().y;
	if (currentVy > VELOCITY_Y)
	{
		getMovement()->setVy(VELOCITY_Y);
	}

	// Note:
	// - To support wall-cling, collision logic should call setState(new Clinging)
	//   when appropriate (player contacting a wall while airborne).
}