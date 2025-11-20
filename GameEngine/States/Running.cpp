#include "Running.h"
#include "Standing.h"
#include "Dashing.h"
#include "Kicking.h"

#define MAX_VELOCITY_LEFT	-350.0f
#define MAX_VELOCITY_RIGHT	350.0f
#define VELOCITY_X			250.f
#define ACCELERATE_X		9.81f
#define FRICTION			800.0f
#define EPSILON				5.0f

Running::Running()
{
}

void Running::update(float deltaTime)
{
	// Get current horizontal velocity
	float vx = _movement->getVelocity().x;

	// If player is actively moving left/right, ensure velocity is within allowed range
	if (_player->getMoveDirection() == eMoveDirection::MOVE_LEFT)
	{
		if (vx < MAX_VELOCITY_LEFT)
			_movement->setVx(MAX_VELOCITY_LEFT);
	}
	else if (_player->getMoveDirection() == eMoveDirection::MOVE_RIGHT)
	{
		if (vx > MAX_VELOCITY_RIGHT)
			_movement->setVx(MAX_VELOCITY_RIGHT);
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
				if (vx < 0.0f) vx = 0.0f;
			}
			else
			{
				vx += decel;
				if (vx > 0.0f) vx = 0.0f;
			}
			_movement->setVx(vx);

			// If we've slowed enough, transition to Standing to give control back
			if (fabs(vx) <= EPSILON)
			{
				_movement->setVx(0.0f);
				this->setState(new Standing);
				return;
			}
		}
		else
		{
			// Already nearly stopped: go to standing
			_movement->setVx(0.0f);
			this->setState(new Standing);
			return;
		}
	}
}

void Running::updateInput(float deltaTime)
{
	// Priority inputs handled first: dash and kick should interrupt running
	if (InputController::getInstance()->isKeyDown(DIK_C))
	{
		// Start dash from running
		this->setState(new Dashing);
		return;
	}

	if (InputController::getInstance()->isKeyDown(DIK_V))
	{
		// Start kick from running
		this->setState(new Kicking);
		return;
	}

	// Handle horizontal input for running (maintain velocity & accel)
	if (InputController::getInstance()->isKeyDown(DIK_LEFTARROW))
	{
		_player->setFlipX(true);
		_player->setMoveDirection(eMoveDirection::MOVE_LEFT);
		_movement->setVx(-VELOCITY_X);
		_movement->setAccelx(-ACCELERATE_X);
	}
	else if (InputController::getInstance()->isKeyDown(DIK_RIGHTARROW))
	{
		_player->setFlipX(false);
		_player->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		_movement->setVx(VELOCITY_X);
		_movement->setAccelx(ACCELERATE_X);
	}
	else
	{
		// No horizontal input: set move direction to NONE and let update() handle smooth decel
		_player->setMoveDirection(eMoveDirection::NONE);
	}
}

eStatus Running::getState()
{
	return eStatus::RUN;
}
