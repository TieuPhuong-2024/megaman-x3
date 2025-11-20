#include "Clinging.h"
#include "Jumping.h"
#include "Falling.h"
#include "../trace.h"

#define CLING_HORIZONTAL_VELOCITY 0.0f
#define CLING_GRAVITY_Y -2.0f
#define WALL_JUMP_VX 300.0f

Clinging::Clinging()
{
	// Reset timer
	_clingTime = 0.0f;

	// Reduce gravity so the player descends slowly while clinging.
	_gravity->setgy(CLING_GRAVITY_Y);
	_gravity->setStatus(eGravityStatus::FALLING__DOWN);

	// Lock horizontal movement while clinging.
	_movement->setVx(CLING_HORIZONTAL_VELOCITY);
	_movement->setAccelx(0.0f);
}

Clinging::~Clinging()
{
	// restore gravity to a sane default? Leave to other states on transition.
}

eStatus Clinging::getState()
{
	return eStatus::CLING;
}

void Clinging::update(float deltaTime)
{
	// Accumulate cling time (deltaTime is in milliseconds in this project)
	_clingTime += deltaTime / 1000.0f;

	// Ensure horizontal velocity remains locked while clinging
	_movement->setVx(CLING_HORIZONTAL_VELOCITY);
	_movement->setAccelx(0.0f);

	// Maintain reduced gravity while clinging
	_gravity->setgy(CLING_GRAVITY_Y);

	// If we've exceed max cling time, drop into falling state
	if (_clingTime >= _maxClingTime)
	{
		_clingTime = 0.0f;
		// restore gravity behavior will be handled by Falling constructor
		this->setState(new Falling);
	}
}

void Clinging::updateInput(float deltaTime)
{
	// If player presses jump while clinging -> perform a wall-jump.
	// We implement a simple wall-jump by giving an initial horizontal push
	// away from the wall and then transitioning into the Jumping state so
	// vertical velocity/acceleration are handled consistently there.
	if (InputController::getInstance()->isKeyDown(DIK_X))
	{
		_clingTime = 0.0f;

		// Determine the direction to push the player horizontally.
		// If player is flipped (facing left), we assume they were clinging on left wall
		// and need to jump to the right; otherwise jump to the left.
		if (_player->getMoveDirection() == eMoveDirection::MOVE_LEFT || _player->getFlipX())
		{
			// push to the right
			_player->setFlipX(false);
			_movement->setVx(WALL_JUMP_VX);
			_player->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		}
		else
		{
			// push to the left
			_player->setFlipX(true);
			_movement->setVx(-WALL_JUMP_VX);
			_player->setMoveDirection(eMoveDirection::MOVE_LEFT);
		}

		// Transition into Jumping which will set vertical velocity/acceleration.
		this->setState(new Jumping);
		return;
	}

	// Allow player to drop off the wall if they press down (optional):
	if (InputController::getInstance()->isKeyDown(DIK_DOWN))
	{
		_clingTime = 0.0f;
		this->setState(new Falling);
		return;
	}

	// If player moves away from the wall (press the direction away from wall),
	// we should exit clinging and allow running/falling logic to resume.
	if (InputController::getInstance()->isKeyDown(DIK_LEFTARROW) ||
		InputController::getInstance()->isKeyDown(DIK_RIGHTARROW))
	{
		// If they press the direction away from the wall, stop clinging.
		// Simple heuristic: if any horizontal key is pressed, drop to Falling so the
		// player regains normal mid-air control. More advanced behavior can transition
		// to Running when grounded, etc.
		_clingTime = 0.0f;
		this->setState(new Falling);
		return;
	}

	// Otherwise remain clinging; other actions (shooting) are handled by CPlayer's
	// shoot mechanism (animation flags) so we don't process them here.
}