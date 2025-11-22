#include "CPlayer.h"
#include "../trace.h"
#include "../States/Dashing.h"
#include "../States/Kicking.h"
#include "../States/Standing.h"
#include "../States/Falling.h"
#include "../States/Jumping.h"
#include "CWall.h"
#include "../Framework/CollisionManager.h"
#include <algorithm>

CPlayer::CPlayer()
	: _input(InputController::getInstance())
{
	_input.Attach(this);

	_sprite = SpriteManager::getInstance().getSprite(eID::XMAN);
	_sprite->setFrameRect(SpriteManager::getInstance().getSourceRect(eID::XMAN, "stand_1"));
	_sprite->setZIndex(1.0f);

	auto movement = std::make_unique<Movement>(VECTOR2ZERO, VECTOR2ZERO, _sprite.get());
	_component.insert(make_pair("Movement", std::move(movement)));

	auto gravity = std::make_unique<Gravity>(VECTOR2ZERO, movement.get());
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
	setState(std::make_unique<Standing>(this));

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

	// Jump buffering: if player presses jump slightly before landing, remember it (seconds)
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
	_input.Detach(this);
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
		this->setState(_currentIndexState);
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
		this->setState(std::make_unique<Jumping>(this));
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

void CPlayer::setState(std::unique_ptr<PlayerState> newState)
{
	// Set new state
	_playerState = std::move(newState);
	// Set current index of state for new index of state
	_currentIndexState = _playerState->getState();
	GAMELOG("Player state changed to: %d", static_cast<int>(_currentIndexState));
	// Restart animation
	setState(_currentIndexState);
}

void CPlayer::setState(eStatus status)
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

void CPlayer::eventKeyUp(KeyEventArg *e)
{
	if ((_isJumping) && (e->_key == DIK_X))
	{
		_isJumping = false;
		this->setState(std::make_unique<Falling>(this));
	}

	if ((e->_key == DIK_C) && (_currentIndexState == eStatus::DASH))
	{
		this->setState(std::make_unique<Standing>(this));
	}
}

void CPlayer::eventKeyDown(KeyEventArg *e)
{
	// Handle jump (DIK_X) with buffering and coyote time:
	if (e->_key == DIK_X)
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
			this->setState(std::make_unique<Jumping>(this));
		}
		else
		{
			// Can't jump now -> buffer the jump for a short time so it triggers on landing
			_jumpBufferTimer = _jumpBufferTime;
		}
	}

	if ((e->_key == DIK_C))
	{
		if ((_currentIndexState == eStatus::STAND) ||
			(_currentIndexState == eStatus::RUN) ||
			(_currentIndexState == eStatus::STAND_SHOOT) ||
			(_currentIndexState == eStatus::RUN_SHOOT))
		{
			GAMELOG("Dashing state triggered by key down C");
			this->setState(std::make_unique<Dashing>(this));
		}
	}

	if ((e->_key == DIK_Z) && (_allowShoot))
	{
		_timeShoot = 0.f;
		_allowShoot = false;
		_currentIndexState = eStatus(static_cast<int>(_currentIndexState) | static_cast<int>(eStatus::SHOOT));
		this->setState(_currentIndexState);
	}

	// (Thêm / chèn vào trong CPlayer::eventKeyDown)
	if ((e->_key == DIK_V))
	{
		if ((_currentIndexState == eStatus::STAND) ||
			(_currentIndexState == eStatus::RUN) ||
			(_currentIndexState == eStatus::STAND_SHOOT) ||
			(_currentIndexState == eStatus::RUN_SHOOT))
		{
			this->setState(std::make_unique<Kicking>(this));
		}
	}
}

void CPlayer::setPosition(GVector2 position)
{
	_sprite->setPosition(position);
}

GVector2 CPlayer::getPosition()
{
	return _sprite->getPosition();
}

void CPlayer::setScale(float scale)
{
	_sprite->setScale(scale);
}

IComponent *CPlayer::getComponent(string name)
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

void CPlayer::setMoveDirection(eMoveDirection moveDirection)
{
	_MoveDirection = moveDirection;
}

eMoveDirection CPlayer::getMoveDirection()
{
	return _MoveDirection;
}

// ICollidable implementation
RECT CPlayer::getBoundingBox() const
{
	// Use sprite's bounding box which accounts for scale, rotation, and origin
	return _sprite->getBounding();
}

void CPlayer::onCollision(ICollidable *other)
{
	// Handle collision with wall or other
	// Assume other is CWall
	if (dynamic_cast<CWall *>(other))
	{
		// Get bounding boxes
		RECT playerBox = getBoundingBox();
		RECT wallBox = other->getBoundingBox();

		// Calculate overlaps
		float overlapX = min(playerBox.right - wallBox.left, wallBox.right - playerBox.left);
		float overlapY = min(playerBox.top - wallBox.bottom, wallBox.top - playerBox.bottom);
		GAMELOG("[Colliding] Overlap X: %.2f, Overlap Y: %.2f", overlapX, overlapY);

		// Adjust position to resolve overlap
		GVector2 pos = getPosition();
		if (overlapX < overlapY)
		{
			// Push horizontally to resolve overlap
			if (playerBox.left < wallBox.left)
			{
				GAMELOG("[Colliding] Pushing player left by %.2f", overlapX);
				pos.x -= overlapX; // Push left
			}
			else
			{
				GAMELOG("[Colliding] Pushing player right by %.2f", overlapX);
				pos.x += overlapX; // Push right
			}
			// Stop horizontal velocity
			setVelocity(GVector2(0, getVelocity().y));
			this->setState(std::make_unique<Standing>(this));
		}
		else
		{
			// Push vertically to resolve overlap
			if (playerBox.top < wallBox.top)
			{
				GAMELOG("[Colliding] Pushing player up by %.2f", overlapY);
				pos.y -= overlapY; // Push up
			}
			else
			{
				GAMELOG("[Colliding] Pushing player down by %.2f", overlapY);
				pos.y += overlapY; // Push down
			}
			// Stop vertical velocity
			setVelocity(GVector2(getVelocity().x, 0));
		}
		setPosition(pos);
	}
}

GVector2 CPlayer::getVelocity()
{
	Movement *movement = static_cast<Movement *>(getComponent("Movement"));
	return movement ? movement->getVelocity() : VECTOR2ZERO;
}

void CPlayer::setVelocity(GVector2 vel)
{
	Movement *movement = static_cast<Movement *>(getComponent("Movement"));
	if (movement)
	{
		movement->setVelocity(vel);
	}
}

void CPlayer::OnCollisionWith(CCollisionEvent *e)
{
	// Handle collision event
	// For example, if colliding with wall
	if (dynamic_cast<CWall *>(e->obj))
	{
		// Adjust position based on nx, ny
		GVector2 pos = getPosition();
		pos.x += e->nx * 0.01f; // Push factor
		pos.y += e->ny * 0.01f;
		setPosition(pos);
		// Stop velocity if blocking
		if (e->nx != 0)
			setVelocity(GVector2(0, getVelocity().y));
		if (e->ny != 0)
			setVelocity(GVector2(getVelocity().x, 0));
	}
}

void CPlayer::GetSpeed(float &vx, float &vy)
{
	GVector2 vel = getVelocity();
	vx = vel.x;
	vy = vel.y;
}

void CPlayer::SetPosition(float x, float y)
{
	setPosition(GVector2(x, y));
}

bool CPlayer::IsBlocking()
{
	return false; // Player is not blocking
}
