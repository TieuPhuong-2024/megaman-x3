#include "Dashing.h"
#include "Jumping.h"
#include "Standing.h"
#include "../trace.h"
#include <memory>
#include "../Framework/InputController.h"

Dashing::Dashing(CPlayer *player) : PlayerState(player)
{
}

Dashing::~Dashing()
{
}

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
	auto moveDir = getPlayer()->getMoveDirection();
	auto currentVel = getMovement()->getVelocity();

	if (moveDir == eMoveDirection::MOVE_LEFT)
	{
		getPlayer()->setFlipX(true);
		getMovement()->setVx(-DASH_SPEED);
		getMovement()->setAccelx(0.0f); // lock horizontal accel during dash
	}
	else if (moveDir == eMoveDirection::MOVE_RIGHT)
	{
		getPlayer()->setFlipX(false);
		getMovement()->setVx(DASH_SPEED);
		getMovement()->setAccelx(0.0f);
	}
	else
	{
		// Fallback: infer direction from current velocity
		if (currentVel.x < 0)
		{
			getPlayer()->setFlipX(true);
			getMovement()->setVx(-DASH_SPEED);
			getMovement()->setAccelx(0.0f);
		}
		else
		{
			getPlayer()->setFlipX(false);
			getMovement()->setVx(DASH_SPEED);
			getMovement()->setAccelx(0.0f);
		}
	}

	// End dash after duration
	if (_timeDash >= DASH_DURATION)
	{
		GAMELOG("Dashing ended after %.2f seconds", _timeDash);
		_timeDash = 0.0f;
		// After dashing, go to standing to give player control back.
		this->setState(std::make_unique<Standing>(getPlayer()));
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
	if (InputController::getInstance().isKeyDown(DIK_X))
	{
		_timeDash = 0.0f; // reset dash timer
		this->setState(std::make_unique<Jumping>(getPlayer()));
		return;
	}

	// If dash duration ended it will be handled in update(); keep this function
	// minimal to avoid conflicting with the update() velocity logic.
}
