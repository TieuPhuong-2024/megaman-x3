#include "Jumping.h"
#include "Falling.h"
#include "../trace.h"

#define VELOCITY_X		250.f
#define ACCELERATE_X	78.48f
#define VELOCITY_Y		250.f
#define ACCELERATE_Y	250.f
#define MAX_AIR_VX		350.f
#define AIR_FRICTION	600.0f

Jumping::Jumping()
{
	// Initialize upward velocity once when the jump begins.
	_movement->setAccely(0.0f);
	_movement->setVy(VELOCITY_Y);

	// Ensure gravity is active for aerial motion.
	_gravity->setStatus(eGravityStatus::FALLING__DOWN);
	_gravity->setgy(-ACCELERATE_Y);

	_jumpCount = 1;
}

Jumping::~Jumping()
{
}

eStatus Jumping::getState()
{
	return eStatus::JUMP;
}

void Jumping::update(float deltaTime)
{
	// If vertical velocity reaches zero or becomes negative, we've reached apex -> Falling
	float vy = _movement->getVelocity().y;
	printLog("Jumping vy: %f\n", vy);
	if (vy <= 0.0f)
	{
		printLog("Transitioning to Falling\n");
		this->setState(new Falling, 0.1f); // Smooth transition
		return;
	}

	// Clamp horizontal speed to a reasonable max while airborne
	auto vx = _movement->getVelocity().x;
	if (vx > MAX_AIR_VX) _movement->setVx(MAX_AIR_VX);
	if (vx < -MAX_AIR_VX) _movement->setVx(-MAX_AIR_VX);
}

void Jumping::updateInput(float deltaTime)
{
	// Allow air control: change horizontal acceleration/velocity based on input.
	// Do NOT reset vertical velocity here (it's set once in constructor).
	if (InputController::getInstance()->isKeyDown(DIK_LEFTARROW))
	{
		_player->setFlipX(true);
		_player->setMoveDirection(eMoveDirection::MOVE_LEFT);
		_movement->setAccelx(-ACCELERATE_X);
		// Provide immediate control by nudging velocity, but do not override vertical velocity.
		_movement->setVx(max(-MAX_AIR_VX, _movement->getVelocity().x - VELOCITY_X * 0.1f));
	}
	else if (InputController::getInstance()->isKeyDown(DIK_RIGHTARROW))
	{
		_player->setFlipX(false);
		_player->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		_movement->setAccelx(ACCELERATE_X);
		_movement->setVx(min(MAX_AIR_VX, _movement->getVelocity().x + VELOCITY_X * 0.1f));
	}
	else
	{
		// No horizontal input: apply air friction to slowly reduce horizontal speed
		_player->setMoveDirection(eMoveDirection::NONE);
		auto vx = _movement->getVelocity().x;
		float decel = AIR_FRICTION * (deltaTime / 1000.0f);
		if (vx > 0.0f)
		{
			vx -= decel;
			if (vx < 0.0f) vx = 0.0f;
		}
		else if (vx < 0.0f)
		{
			vx += decel;
			if (vx > 0.0f) vx = 0.0f;
		}
		_movement->setVx(vx);
		_movement->setAccelx(0.0f);
	}

	// Vertical motion remains controlled by gravity and was initialized in constructor.
}
