#include "Dashing.h"
#include "Jumping.h"
#include "Standing.h"
#include "../trace.h"

eStatus Dashing::getState()
{
	return eStatus::DASH;
}

void Dashing::update(float deltaTime)
{
	// Accumulate dash time (deltaTime in ms, normalize to seconds)
	_timeDash += deltaTime / 1000.0f;

	// Dash configuration
	const float DASH_SPEED = 600.0f;   // horizontal speed while dashing
	const float DASH_DURATION = 0.25f; // seconds

	// Determine dash direction:
	// Prefer explicit move direction; otherwise infer from current velocity
	auto moveDir = _player->getMoveDirection();
	auto currentVel = _movement->getVelocity();

	if (moveDir == eMoveDirection::MOVE_LEFT)
	{
		_player->setFlipX(true);
		_movement->setVx(-DASH_SPEED);
		_movement->setAccelx(0.0f); // lock horizontal accel during dash
	}
	else if (moveDir == eMoveDirection::MOVE_RIGHT)
	{
		_player->setFlipX(false);
		_movement->setVx(DASH_SPEED);
		_movement->setAccelx(0.0f);
	}
	else
	{
		// Fallback: infer direction from current velocity
		if (currentVel.x < 0)
		{
			_player->setFlipX(true);
			_movement->setVx(-DASH_SPEED);
			_movement->setAccelx(0.0f);
		}
		else
		{
			_player->setFlipX(false);
			_movement->setVx(DASH_SPEED);
			_movement->setAccelx(0.0f);
		}
	}

	// End dash after duration
	if (_timeDash >= DASH_DURATION)
	{
		_timeDash = 0.0f;
		// After dashing, go to standing to give player control back.
		this->setState(new Standing);
	}
}

void Dashing::updateInput(float deltaTime)
{
	// During dash we only want to allow a limited set of actions:
	// - Allow jump input to transition into Jumping state
	// - Ignore regular left/right input since dash locks horizontal control
	// This keeps dash feeling responsive and prevents accidental overrides.

	// If player presses jump while dashing, transition to Jumping immediately.
	// (Jumping state will handle reset of vertical accel/velocity.)
	if (InputController::getInstance()->isKeyDown(DIK_X))
	{
		_timeDash = 0.0f; // reset dash timer
		this->setState(new Jumping);
		return;
	}

	// If dash duration ended it will be handled in update(); keep this function
	// minimal to avoid conflicting with the update() velocity logic.
}
