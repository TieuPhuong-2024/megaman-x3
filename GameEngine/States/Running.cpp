#include "Running.h"
#include "Standing.h"
#include "Dashing.h"
#include "Kicking.h"
#include "../trace.h"
#include <memory>

#define MAX_VELOCITY_LEFT -350.0f
#define MAX_VELOCITY_RIGHT 350.0f
#define VELOCITY_X 250.f
#define ACCELERATE_X 9.81f
#define FRICTION 800.0f
#define EPSILON 5.0f

Running::Running(CPlayer *player) : PlayerState(player)
{
}

void Running::update(float deltaTime)
{
	// Get current horizontal velocity
	float vx = getMovement()->getVelocity().x;

	// If player is actively moving left/right, ensure velocity is within allowed range
	if (getPlayer()->getMoveDirection() == eMoveDirection::MOVE_LEFT)
	{
		if (vx < MAX_VELOCITY_LEFT)
			getMovement()->setVx(MAX_VELOCITY_LEFT);
	}
	else if (getPlayer()->getMoveDirection() == eMoveDirection::MOVE_RIGHT)
	{
		if (vx > MAX_VELOCITY_RIGHT)
			getMovement()->setVx(MAX_VELOCITY_RIGHT);
	}
	else
	{
		// No input: apply friction to decelerate smoothly to zero instead of snapping
		if (fabs(vx) > 0.01f)
		{
			// deltaTime is milliseconds in this project; convert to seconds for calculations
			float decel = FRICTION * (deltaTime / 1000.0f);
			if (vx > 0.0f)
			{
				vx -= decel;
				if (vx < 0.0f)
					vx = 0.0f;
			}
			else
			{
				vx += decel;
				if (vx > 0.0f)
					vx = 0.0f;
			}
			getMovement()->setVx(vx);

			// If we've slowed enough, transition to Standing to give control back
			if (fabs(vx) <= EPSILON)
			{
				getMovement()->setVx(0.0f);
				this->setState(std::make_unique<Standing>(getPlayer()));
				return;
			}
		}
		else
		{
			// Already nearly stopped: go to standing
			getMovement()->setVx(0.0f);
			this->setState(std::make_unique<Standing>(getPlayer()));
			return;
		}
	}
}

void Running::updateInput(float deltaTime)
{
	// Handle horizontal input for running (maintain velocity & accel)
	if (InputController::getInstance().isKeyDown(DIK_LEFTARROW))
	{
		getPlayer()->setFlipX(true);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_LEFT);
		getMovement()->setVx(-VELOCITY_X);
		getMovement()->setAccelx(-ACCELERATE_X);
	}
	else if (InputController::getInstance().isKeyDown(DIK_RIGHTARROW))
	{
		getPlayer()->setFlipX(false);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		getMovement()->setVx(VELOCITY_X);
		getMovement()->setAccelx(ACCELERATE_X);
	}
	else
	{
		// No horizontal input: set move direction to NONE and let update() handle smooth decel
		getPlayer()->setMoveDirection(eMoveDirection::NONE);
	}
}

eStatus Running::getState()
{
	return eStatus::RUN;
}
