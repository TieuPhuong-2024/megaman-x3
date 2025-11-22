#include "Jumping.h"
#include "Falling.h"
#include "../trace.h"
#include <memory>

#define VELOCITY_X 250.f
#define ACCELERATE_X 78.48f
#define VELOCITY_Y 250.f
#define ACCELERATE_Y 250.f
#define MAX_AIR_VX 350.f
#define AIR_FRICTION 600.0f

Jumping::Jumping(CPlayer *player) : PlayerState(player)
{
	// Initialize upward velocity once when the jump begins.
	getMovement()->setAccely(0.0f);
	getMovement()->setVy(VELOCITY_Y);

	// Ensure gravity is active for aerial motion.
	getGravity()->setStatus(eGravityStatus::FALLING__DOWN);
	getGravity()->setgy(-ACCELERATE_Y);
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
	float vy = getMovement()->getVelocity().y;
	if (vy <= 0.0f)
	{
		printLog("Transitioning to Falling\n");
		this->setState(std::make_unique<Falling>(getPlayer()), 0.1f); // Smooth transition
		return;
	}

	// Clamp horizontal speed to a reasonable max while airborne
	auto vx = getMovement()->getVelocity().x;
	if (vx > MAX_AIR_VX)
		getMovement()->setVx(MAX_AIR_VX);
	if (vx < -MAX_AIR_VX)
		getMovement()->setVx(-MAX_AIR_VX);
}

void Jumping::updateInput(float deltaTime)
{
	// Allow air control: change horizontal acceleration/velocity based on input.
	// Do NOT reset vertical velocity here (it's set once in constructor).
	if (InputController::getInstance().isKeyDown(DIK_LEFTARROW))
	{
		getPlayer()->setFlipX(true);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_LEFT);
		getMovement()->setAccelx(-ACCELERATE_X);
		// Provide immediate control by nudging velocity, but do not override vertical velocity.
		getMovement()->setVx(max(-MAX_AIR_VX, getMovement()->getVelocity().x - VELOCITY_X * 0.1f));
	}
	else if (InputController::getInstance().isKeyDown(DIK_RIGHTARROW))
	{
		getPlayer()->setFlipX(false);
		getPlayer()->setMoveDirection(eMoveDirection::MOVE_RIGHT);
		getMovement()->setAccelx(ACCELERATE_X);
		getMovement()->setVx(min(MAX_AIR_VX, getMovement()->getVelocity().x + VELOCITY_X * 0.1f));
	}
	else
	{
		// No horizontal input: apply air friction to slowly reduce horizontal speed
		getPlayer()->setMoveDirection(eMoveDirection::NONE);
		auto vx = getMovement()->getVelocity().x;
		float decel = AIR_FRICTION * (deltaTime / 1000.0f);
		if (vx > 0.0f)
		{
			vx -= decel;
			if (vx < 0.0f)
				vx = 0.0f;
		}
		else if (vx < 0.0f)
		{
			vx += decel;
			if (vx > 0.0f)
				vx = 0.0f;
		}
		getMovement()->setVx(vx);
		getMovement()->setAccelx(0.0f);
	}

	// Vertical motion remains controlled by gravity and was initialized in constructor.
}
