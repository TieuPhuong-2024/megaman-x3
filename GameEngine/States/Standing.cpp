#include "Standing.h"
#include "Running.h"
#include "Jumping.h"
#include "Falling.h"
#include <memory>

#define VELOCITY_X 250.f
#define ACCELERATE_X 9.81f
#define EPSILON 1.0f

Standing::Standing(CPlayer *player) : PlayerState(player)
{
	// Ensure player is stationary while standing
	getMovement()->setVelocity(VECTOR2ZERO);
	getMovement()->setAccelerate(VECTOR2ZERO);

	// Mark gravity as shallow when grounded so vertical velocity doesn't accumulate
	getGravity()->setStatus(eGravityStatus::SHALLOWED);
}

void Standing::update(float deltaTime)
{
	// If gravity indicates we're falling (walked off an edge or similar), transition
	if (getGravity()->getStatus() == eGravityStatus::FALLING__DOWN)
	{
		this->setState(std::make_unique<Falling>(getPlayer()));
		return;
	}

	// If horizontal velocity is significant (external forces), transition to Running
	if (fabs(getMovement()->getVelocity().x) > EPSILON)
	{
		this->setState(std::make_unique<Running>(getPlayer()));
		return;
	}

	// Keep movement cleared while standing
	getMovement()->setVelocity(VECTOR2ZERO);
	getMovement()->setAccelerate(VECTOR2ZERO);
}

void Standing::updateInput(float deltaTime)
{
	// Default: no horizontal move
	getMovement()->setVelocity(VECTOR2ZERO);
	getMovement()->setAccelerate(VECTOR2ZERO);
	getPlayer()->setMoveDirection(eMoveDirection::NONE);

	auto &input = InputController::getInstance();

	// Horizontal movement: transition to Running and apply velocity/acceleration

	if (input.isKeyDown(DIK_RIGHTARROW) && getPlayer()->IsBlockingRight() == false)
	{
		getMovement()->setVelocity(GVector2(VELOCITY_X, 0));
		getMovement()->setAccelerate(GVector2(ACCELERATE_X, 0));
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		this->setState(std::make_unique<Running>(getPlayer()));
		return;
	}
	else if (input.isKeyDown(DIK_LEFTARROW) && getPlayer()->IsBlockingLeft() == false)
	{
		getMovement()->setVelocity(GVector2(-VELOCITY_X, 0));
		getMovement()->setAccelerate(GVector2(-ACCELERATE_X, 0));
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_LEFT);
		this->setState(std::make_unique<Running>(getPlayer()));
		return;
	}

	// If gravity indicates falling (e.g., stepped off a platform), transition
	if (getGravity()->getStatus() == eGravityStatus::FALLING__DOWN)
	{
		this->setState(std::make_unique<Falling>(getPlayer()));
		return;
	}

	// Jump/shoot inputs are handled by CPlayer's key events (eventKeyDown); do not duplicate here.
}

eStatus Standing::getState()
{
	return eStatus::STAND;
}
