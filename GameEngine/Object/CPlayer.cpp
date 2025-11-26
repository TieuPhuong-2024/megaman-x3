#include "CPlayer.h"
#include "../trace.h"
#include "../States/Dashing.h"
#include "../States/Kicking.h"
#include "../States/Standing.h"
#include "../States/Falling.h"
#include "../States/Jumping.h"
#include "CWall.h"
#include "../Framework/Collision.h"
#include <algorithm>

CPlayer::CPlayer()
{
	// Setup new Event system listener with RAII automatic cleanup
	_keyEventListener = std::make_unique<Framework::EventListenerScope<Framework::KeyEvent>>(
		[this](std::shared_ptr<Framework::Event> event)
		{
			handleKeyEvent(event);
		},
		"cplayer_key_handler");

	_sprite = SpriteManager::getInstance().getSprite(eID::XMAN);
	_sprite->setFrameRect(SpriteManager::getInstance().getSourceRect(eID::XMAN, "stand_1"));
	_sprite->setZIndex(1.0f);

	auto movement = std::make_unique<Movement>(VECTOR2ZERO, VECTOR2ZERO, _sprite.get());
	_component.insert(make_pair("Movement", std::move(movement)));

	auto gravity = std::make_unique<Gravity>(VECTOR2ZERO, (Movement *)_component["Movement"].get());
	_component.insert(make_pair("Gravity", std::move(gravity)));

	_spriteAnimation[eStatus::STAND] = std::make_unique<Animation>(_sprite.get(), 0.15f);
	_spriteAnimation[eStatus::STAND]->addFrameRect(eID::XMAN, "standing_1", "standing_2", NULL);

	_spriteAnimation[eStatus::STAND_SHOOT] = std::make_unique<Animation>(_sprite.get(), 0.15f);
	_spriteAnimation[eStatus::STAND_SHOOT]->addFrameRect(eID::XMAN, "StandingShoot_1", "StandingShoot_2", NULL);

	_spriteAnimation[eStatus::RUN] = std::make_unique<Animation>(_sprite.get(), 0.05f);
	_spriteAnimation[eStatus::RUN]->addFrameRect(eID::XMAN, "StartRunning_1", "StartRunning_2",
												 "running_1", "running_2", "running_3", "running_4", "running_5",
												 "running_6", "running_7", "running_8", "running_9", NULL);
	_spriteAnimation[eStatus::RUN]->animateFromTo(3, 10);

	_spriteAnimation[eStatus::RUN_SHOOT] = std::make_unique<Animation>(_sprite.get(), 0.05f);
	_spriteAnimation[eStatus::RUN_SHOOT]->addFrameRect(eID::XMAN,
													   "RunningShoot_1", "RunningShoot_2", "RunningShoot_3",
													   "RunningShoot_4", "RunningShoot_5", "RunningShoot_6",
													   "RunningShoot_7", "RunningShoot_8", "RunningShoot_9", NULL);

	_spriteAnimation[eStatus::JUMP] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::JUMP]->setLoop(false);
	_spriteAnimation[eStatus::JUMP]->addFrameRect(eID::XMAN, "jumping_1", "jumping_2", "jumping_3", NULL);

	_spriteAnimation[eStatus::JUMP_SHOOT] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::JUMP_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::JUMP_SHOOT]->addFrameRect(eID::XMAN, "JumpingShoot_1", "JumpingShoot_2", "JumpingShoot_3",
														NULL);

	_spriteAnimation[eStatus::FALL] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::FALL]->setLoop(false);
	_spriteAnimation[eStatus::FALL]->addFrameRect(eID::XMAN, "falling_1", "falling_2", "falling_3", "falling_4", NULL);

	_spriteAnimation[eStatus::FALL_SHOOT] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::FALL_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::FALL_SHOOT]->addFrameRect(eID::XMAN, "FallingShoot_1", "FallingShoot_2", "FallingShoot_3",
														"FallingShoot_4", NULL);

	_spriteAnimation[eStatus::DASH] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::DASH]->setLoop(false);
	_spriteAnimation[eStatus::DASH]->addFrameRect(eID::XMAN, "dashing_1", "dashing_2", NULL);

	_spriteAnimation[eStatus::DASH_SHOOT] = std::make_unique<Animation>(_sprite.get(), 0.1f);
	_spriteAnimation[eStatus::DASH_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::DASH_SHOOT]->addFrameRect(eID::XMAN, "DashShoot_1", "DashShoot_2", NULL);

	// Default current index of state is standing
	_currentIndexState = eStatus::STAND;

	// Setting status for player
	_playerState = std::make_unique<Standing>(this);
	ChangeState(std::make_unique<Standing>(this));

	// Don't reverse
	_isFlipX = false;

	// Move direction default is not moving
	_MoveDirection = eMoveDirection::NONE;

	// Not jumping (legacy flag kept for compatibility)
	_isJumping = false;

	// --- Jump system: double-jump, coyote time, and jump buffer ---
	// Max number of jumps (2 = double jump). Can be tuned later via an exposed setter.
	_maxJumps = 2;
	// Remaining jumps available right after spawn/landing
	_remainingJumps = _maxJumps;

	// Jump buffering: if player pressed jump slightly before landing, remember it (seconds)
	_jumpBufferTime = 0.12f;
	_jumpBufferTimer = 0.0f;

	// Coyote time: forgiving window after leaving ground where jump is still allowed (seconds)
	_coyoteTime = 0.12f;
	_coyoteTimer = 0.0f;

	// Whether the player is currently considered grounded. This will be toggled in updateInput
	_isGrounded = true;
	// --- end jump system state ---

	// Allow to shoot
	_allowShoot = true;

	// Time shoot
	_timeShoot = 0.f;
}

CPlayer::~CPlayer()
{
	// EventListenerScope will automatically clean up in destructor
}

void CPlayer::update(float deltaTime)
{
	/* Update component */
	for (auto &it : _component)
	{
		it.second->update(deltaTime);
	}

	/* Update sprite animation */
	_spriteAnimation[_currentIndexState]->update(deltaTime);

	GAMELOG("Player update: state=%d, anim_index=%d", static_cast<int>(_currentIndexState), _spriteAnimation[_currentIndexState]->getIndex());

	/* Update player state */
	_playerState->update(deltaTime);
}

void CPlayer::updateInput(float deltaTime)
{
	_timeShoot += deltaTime / 1000.f;
	if ((_timeShoot > 1.f) && !_allowShoot)
	{
		_allowShoot = true;
		_timeShoot = 0.f;
		_currentIndexState = eStatus(static_cast<int>(_currentIndexState) & ~static_cast<int>(eStatus::SHOOT));
		this->SetStatus(_currentIndexState);
	}

	// Update jump-related timers (deltaTime is milliseconds in this project)
	float dtSeconds = deltaTime / 1000.0f;
	if (_coyoteTimer > 0.0f)
		_coyoteTimer -= dtSeconds;
	if (_jumpBufferTimer > 0.0f)
		_jumpBufferTimer -= dtSeconds;

	// Detect landing from state and reset jumps
	// (This is a simple/robust approach given the current structure: when player's state becomes STAND
	// we treat that as grounded/landing.)
	if ((_currentIndexState == eStatus::STAND) && !_isGrounded)
	{
		_isGrounded = true;
		_remainingJumps = _maxJumps;
		_jumpBufferTimer = 0.0f;
		_coyoteTimer = 0.0f;
	}

	// If we left ground (no longer standing) start coyote timer once
	if ((_currentIndexState != eStatus::STAND) && _isGrounded)
	{
		_isGrounded = false;
		_coyoteTimer = _coyoteTime;
	}

	// If player pressed jump slightly before landing (buffered) and now can jump -> perform it
	if ((_jumpBufferTimer > 0.0f) && ((_remainingJumps > 0) || (_coyoteTimer > 0.0f)))
	{
		_isJumping = true;
		_remainingJumps = (_remainingJumps > 0) ? (_remainingJumps - 1) : 0;
		_jumpBufferTimer = 0.0f;
		_coyoteTimer = 0.0f;
		this->ChangeState(std::make_unique<Jumping>(this));
	}

	// Update input for the current state (e.g., Running, Jumping, etc.)
	_playerState->updateInput(deltaTime);
}

void CPlayer::draw(ID3DXSprite *spriteHandler, Viewport *viewport)
{
	// Flip sprite to coordinate-x
	_sprite->setFlipX(_isFlipX);
	// Draw sprite animation
	_spriteAnimation[_currentIndexState]->draw(spriteHandler, viewport);
}

void CPlayer::ChangeState(std::unique_ptr<PlayerState> newState)
{
	// Set new state
	_playerState = std::move(newState);
	// Set current index of state for new index of state
	_currentIndexState = _playerState->getState();
	GAMELOG("Player state changed to: %d", static_cast<int>(_currentIndexState));
	// Restart animation
	SetStatus(_currentIndexState);
}

void CPlayer::SetStatus(eStatus status)
{
	// Get index of previous state
	auto index = _spriteAnimation[_currentIndexState]->getIndex();
	// Get time animation of previous state
	auto timeAnimate = _spriteAnimation[_currentIndexState]->getTimeAnimate();
	// Set current index of state for new index of state
	_currentIndexState = status;
	GAMELOG("Player animation state set to: %d", static_cast<int>(status));
	// Set time animate
	_spriteAnimation[status]->setTimeAnimate(timeAnimate);
	// Restart animation
	_spriteAnimation[status]->restart(index);
}

RECT CPlayer::GetBoundingBox() const
{
	return _sprite->getBounding();
}

void CPlayer::OnCollision(ICollidable* other)
{
}

void CPlayer::OnCollisionWith(CCollisionEvent* e)
{
	// e->nx, e->ny = collision normal (direction of collision)
	// e->obj = the object you collided with
	// e->t = collision time (0.0 to 1.0)

	if (e->ny != 0)  // collision on Y axis (vertical)
	{
		if (e->ny < 0)  // collision from above (jumping into ceiling)
		{
			// Handle ceiling collision - stop upward velocity
			GVector2 vel = this->GetVelocity();
			vel.y = 0;  // Stop upward movement
			this->SetVelocity(vel);
			GAMELOG("Player hit ceiling");
		}
		else if (e->ny > 0)  // collision from below (landing on ground)
		{
			// Handle ground collision - player is grounded
			_isGrounded = true;
			_remainingJumps = _maxJumps;  // Reset jump count on landing
			
			// Transition to Standing state if not already in a ground state
			if (_currentIndexState == eStatus::FALL || _currentIndexState == eStatus::FALL_SHOOT)
			{
				this->ChangeState(std::make_unique<Standing>(this));
			}
			GAMELOG("Player landed on ground");
		}
	}

	if (e->nx != 0)  // collision on X axis (left/right walls)
	{
		if (e->nx < 0)  // collision from right side (hitting wall on right)
		{
			_isBlockingRight = true;
			GAMELOG("Player blocked on right");
		}
		else if (e->nx > 0)  // collision from left side (hitting wall on left)
		{
			_isBlockingLeft = true;
			GAMELOG("Player blocked on left");
		}
	}
}

bool CPlayer::IsBlocking()
{
	return false;
}

void CPlayer::GetSpeed(float& vx, float& vy)
{
	vx = this->GetVelocity().x;
	vy = this->GetVelocity().y;
}

void CPlayer::handleKeyEvent(std::shared_ptr<Framework::Event> event)
{
	auto keyEvent = std::static_pointer_cast<Framework::KeyEvent>(event);
	int keyCode = keyEvent->getKeyCode();
	auto eventType = keyEvent->getEventType();

	if (eventType == Framework::KeyEvent::Type::KeyUp)
	{
		// Handle key release events
		if ((_isJumping) && (keyCode == DIK_X))
		{
			_isJumping = false;
			this->ChangeState(std::make_unique<Falling>(this));
		}

		if ((keyCode == DIK_C) && (_currentIndexState == eStatus::DASH))
		{
			this->ChangeState(std::make_unique<Standing>(this));
		}
	}
	else if (eventType == Framework::KeyEvent::Type::KeyDown)
	{
		// Handle jump (DIK_X) with buffering and coyote time:
		if (keyCode == DIK_X)
		{
			// If we can jump right now (have remaining jumps or within coyote window) -> perform jump
			if ((_remainingJumps > 0) || (_coyoteTimer > 0.0f))
			{
				_isJumping = true;
				// consume one jump (ensure non-negative)
				_remainingJumps = (_remainingJumps > 0) ? (_remainingJumps - 1) : 0;
				// consume coyote
				_coyoteTimer = 0.0f;
				_isGrounded = false;
				// clear any buffered jump
				_jumpBufferTimer = 0.0f;

				// Transition into Jumping to (re)apply vertical velocity
				this->ChangeState(std::make_unique<Jumping>(this));
			}
			else
			{
				// Can't jump now -> buffer the jump for a short time so it triggers on landing
				_jumpBufferTimer = _jumpBufferTime;
			}
		}

		if (keyCode == DIK_C)
		{
			if ((_currentIndexState == eStatus::STAND) ||
				(_currentIndexState == eStatus::RUN) ||
				(_currentIndexState == eStatus::STAND_SHOOT) ||
				(_currentIndexState == eStatus::RUN_SHOOT))
			{
				GAMELOG("Dashing state triggered by key down C");
				this->ChangeState(std::make_unique<Dashing>(this));
			}
		}

		if ((keyCode == DIK_Z) && (_allowShoot))
		{
			_timeShoot = 0.f;
			_allowShoot = false;
			_currentIndexState = eStatus(static_cast<int>(_currentIndexState) | static_cast<int>(eStatus::SHOOT));
			this->SetStatus(_currentIndexState);
		}

		if ((keyCode == DIK_V))
		{
			if ((_currentIndexState == eStatus::STAND) ||
				(_currentIndexState == eStatus::RUN) ||
				(_currentIndexState == eStatus::STAND_SHOOT) ||
				(_currentIndexState == eStatus::RUN_SHOOT))
			{
				this->ChangeState(std::make_unique<Kicking>(this));
			}
		}
	}
}

void CPlayer::SetPosition(GVector2 position)
{
	_sprite->setPosition(position);
}

void CPlayer::SetPosition(float x, float y)
{
	this->SetPosition(GVector2(x, y));
}

GVector2 CPlayer::GetPosition()
{
	return _sprite->getPosition();
}

void CPlayer::SetScale(float scale)
{
	_sprite->setScale(scale);
}

IComponent *CPlayer::GetComponent(string name)
{
	return _component[name].get();
}

void CPlayer::setFlipX(bool isFlip)
{
	if (_isFlipX != isFlip)
		_isFlipX = isFlip;
}

bool CPlayer::getFlipX()
{
	return _isFlipX;
}

void CPlayer::SetMoveDirection(eMoveDirection moveDirection)
{
	_MoveDirection = moveDirection;
}

eMoveDirection CPlayer::GetMoveDirection()
{
	return _MoveDirection;
}

GVector2 CPlayer::GetVelocity()
{
	Movement *movement = static_cast<Movement *>(GetComponent("Movement"));
	return movement ? movement->getVelocity() : VECTOR2ZERO;
}

void CPlayer::SetVelocity(GVector2 vel)
{
	Movement *movement = static_cast<Movement *>(GetComponent("Movement"));
	if (movement)
	{
		movement->setVelocity(vel);
	}
}