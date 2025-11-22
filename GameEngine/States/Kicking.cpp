#include "Kicking.h"
#include "Standing.h"
#include "Jumping.h"
#include "../trace.h"
#include "../Framework/define.h"
#include "../Framework/InputController.h"
#include <memory>

Kicking::Kicking(CPlayer *player) : PlayerState(player)
{
	_timeKick = 0.0f;
	_impulseApplied = false;

	// Lock horizontal acceleration during the kick and clear horizontal accel so
	// the impulse can control the brief movement.
	getMovement()->setAccelx(0.0f);

	// If the player was moving, we will apply a small impulse in that direction
	// when update runs for the first time. Do not apply it in the constructor
	// because some initialization in other systems might not be ready.
}

Kicking::~Kicking()
{
	// Nothing special to clean up here. Movement/Gravity will be adjusted by
	// the next state (Standing/Jumping/Falling) when transitioned.
}

eStatus Kicking::getState()
{
	return eStatus::KICK;
}

void Kicking::update(float deltaTime)
{
	// deltaTime is in milliseconds in this project, so convert to seconds
	_timeKick += deltaTime / 1000.0f;

	// Apply the kick impulse once at the start of the kick.
	if (!_impulseApplied)
	{
		_impulseApplied = true;

		// Prefer explicit player move direction if available
		auto dir = getPlayer()->getMoveDirection();
		if (dir == eMoveDirection::MOVE_LEFT)
		{
			getMovement()->setVx(-_kickImpulseVx);
		}
		else if (dir == eMoveDirection::MOVE_RIGHT)
		{
			getMovement()->setVx(_kickImpulseVx);
		}
		else
		{
			// Fallback: infer from current velocity
			auto currentVel = getMovement()->getVelocity();
			if (currentVel.x < 0.0f)
				getMovement()->setVx(-_kickImpulseVx);
			else
				getMovement()->setVx(_kickImpulseVx);
		}
	}

	// Keep horizontal acceleration locked while kicking
	getMovement()->setAccelx(0.0f);

	// End kick after duration and return control to the player (Standing)
	if (_timeKick >= _kickDuration)
	{
		_timeKick = 0.0f;
		_impulseApplied = false;
		this->setState(std::make_unique<Standing>(getPlayer()));
	}
}

void Kicking::updateInput(float deltaTime)
{
	// Allow jump input to transition out of the kick early.
	// This keeps the player responsive if they choose to jump during the kick.
	if (InputController::getInstance().isKeyDown(DIK_X))
	{
		_timeKick = 0.0f;
		_impulseApplied = false;
		this->setState(std::make_unique<Jumping>(getPlayer()));
		return;
	}

	// During the kick we intentionally ignore horizontal move inputs so the kick
	// animation/impulse is not interrupted by immediate left/right changes.
	// Other actions (like shooting) are handled elsewhere (animation flags).
}