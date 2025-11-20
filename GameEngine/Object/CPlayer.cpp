#include "CPlayer.h"
#include "../trace.h"
#include "../States/Dashing.h"
#include "../States/Kicking.h"
#include "../States/Standing.h"
#include "../States/Falling.h"
#include "../States/Jumping.h"
#include "CWall.h"
#include "../Framework/CollisionManager.h"

CPlayer::CPlayer()
{
	_input = InputController::getInstance();
	_input->Attach(this);

	_sprite = SpriteManager::getInstance()->getSprite(eID::XMAN);
	_sprite->setFrameRect(SpriteManager::getInstance()->getSourceRect(eID::XMAN, "stand_1"));
	_sprite->setZIndex(1.0f);

	Movement* movement = new Movement(VECTOR2ZERO, VECTOR2ZERO, _sprite);
	_component.insert(make_pair("Movement", movement));

	Gravity* gravity = new Gravity(VECTOR2ZERO, movement);
	_component.insert(make_pair("Gravity", gravity));

	_spriteAnimation[eStatus::STAND] = new Animation(_sprite, 0.15f);
	_spriteAnimation[eStatus::STAND]->addFrameRect(eID::XMAN, "standing_1", "standing_2", NULL);

	_spriteAnimation[eStatus::STAND_SHOOT] = new Animation(_sprite, 0.15f);
	_spriteAnimation[eStatus::STAND_SHOOT]->addFrameRect(eID::XMAN, "StandingShoot_1", "StandingShoot_2", NULL);


	_spriteAnimation[eStatus::RUN] = new Animation(_sprite, 0.05f);
	_spriteAnimation[eStatus::RUN]->addFrameRect(eID::XMAN, "StartRunning_1", "StartRunning_2",
	                                             "running_1", "running_2", "running_3", "running_4", "running_5",
	                                             "running_6", "running_7", "running_8", "running_9", NULL);
	_spriteAnimation[eStatus::RUN]->animateFromTo(3, 10);

	_spriteAnimation[eStatus::RUN_SHOOT] = new Animation(_sprite, 0.05f);
	_spriteAnimation[eStatus::RUN_SHOOT]->addFrameRect(eID::XMAN,
	                                                   "RunningShoot_1", "RunningShoot_2", "RunningShoot_3",
	                                                   "RunningShoot_4", "RunningShoot_5", "RunningShoot_6",
	                                                   "RunningShoot_7", "RunningShoot_8", "RunningShoot_9", NULL);


	_spriteAnimation[eStatus::JUMP] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::JUMP]->setLoop(false);
	_spriteAnimation[eStatus::JUMP]->addFrameRect(eID::XMAN, "jumping_1", "jumping_2", "jumping_3", NULL);

	_spriteAnimation[eStatus::JUMP_SHOOT] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::JUMP_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::JUMP_SHOOT]->addFrameRect(eID::XMAN, "JumpingShoot_1", "JumpingShoot_2", "JumpingShoot_3",
	                                                    NULL);


	_spriteAnimation[eStatus::FALL] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::FALL]->setLoop(false);
	_spriteAnimation[eStatus::FALL]->addFrameRect(eID::XMAN, "falling_1", "falling_2", "falling_3", "falling_4", NULL);

	_spriteAnimation[eStatus::FALL_SHOOT] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::FALL_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::FALL_SHOOT]->addFrameRect(eID::XMAN, "FallingShoot_1", "FallingShoot_2", "FallingShoot_3",
	                                                    "FallingShoot_4", NULL);

	_spriteAnimation[eStatus::DASH] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::DASH]->setLoop(false);
	_spriteAnimation[eStatus::DASH]->addFrameRect(eID::XMAN, "dashing_1", "dashing_2", NULL);

	_spriteAnimation[eStatus::DASH_SHOOT] = new Animation(_sprite, 0.1f);
	_spriteAnimation[eStatus::DASH_SHOOT]->setLoop(false);
	_spriteAnimation[eStatus::DASH_SHOOT]->addFrameRect(eID::XMAN, "DashShoot_1", "DashShoot_2", NULL);

	// Default current index of state is standing
	_currentIndexState = eStatus::STAND;

	// Setting status for player
	PlayerState::setPlayer(this);
	setState(new Standing);

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

	_previousIndexState = eStatus::STAND;
}

CPlayer::~CPlayer()
{
	/* Delete sprite */
	SAFE_DELETE(_sprite);

	/* Delete sprite animation */
	for (auto & it : _spriteAnimation)
	{
		delete it.second;
	}

	/* Delete component */
	for (auto & it : _component)
	{
		delete it.second;
	}

	/* Delete the state of player */
	SAFE_DELETE(_playerState);


	_input->Detach(this);
}

void CPlayer::update(float deltaTime)
{
	/* Update component */
	for (auto & it : _component)
	{
		it.second->update(deltaTime);
	}

	/* Update sprite animation */
	_spriteAnimation[_currentIndexState]->update(deltaTime);

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
		_status = eStatus(static_cast<int>(_currentIndexState) & ~static_cast<int>(eStatus::SHOOT));
		this->setState(_status);
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
		this->setState(new Jumping);
	}

	// Update input for the current state (e.g., Running, Jumping, etc.)
	_playerState->updateInput(deltaTime);
}

void CPlayer::draw(ID3DXSprite* spriteHandler, Viewport* viewport)
{
	// Flip sprite to coordinate-x
	_sprite->setFlipX(_isFlipX);
	// Draw sprite animation
	_spriteAnimation[_currentIndexState]->draw(spriteHandler, viewport);
}

void CPlayer::setState(PlayerState* newState)
{
	// Delete previous state
	SAFE_DELETE(_playerState);
	// Set new state
	_playerState = newState;
	// Set current index of state for new index of state
	_currentIndexState = newState->getState();
	// Restart animation
	setState(_currentIndexState);
}


void CPlayer::setState(eStatus status)
{
	// Save previous
	_previousIndexState = _currentIndexState;

	// Get index of previous state
	auto index = _spriteAnimation[_currentIndexState]->getIndex();
	// Get time animation of previous state
	auto timeAnimate = _spriteAnimation[_currentIndexState]->getTimeAnimate();
	// Set current index of state for new index of state
	_currentIndexState = status;
	// Set time animate
	_spriteAnimation[status]->setTimeAnimate(timeAnimate);
	// Restart animation
	_spriteAnimation[status]->restart(index);
}


void CPlayer::eventKeyUp(KeyEventArg* e)
{
	if ((_isJumping) && (e->_key == DIK_X))
	{
		_isJumping = false;
		this->setState(new Falling);
	}
}

void CPlayer::eventKeyDown(KeyEventArg* e)
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
			this->setState(new Jumping);
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
			this->setState(new Dashing);
		}
	}

	if ((e->_key == DIK_Z) && (_allowShoot))
	{
		_timeShoot = 0.f;
		_allowShoot = false;
		_status = eStatus(static_cast<int>(_currentIndexState) | static_cast<int>(eStatus::SHOOT));
		this->setState(_status);
	}

	// (Thêm / chèn vào trong CPlayer::eventKeyDown)
    if ((e->_key == DIK_V))
    {
        if ((_currentIndexState == eStatus::STAND) ||
            (_currentIndexState == eStatus::RUN) ||
            (_currentIndexState == eStatus::STAND_SHOOT) ||
            (_currentIndexState == eStatus::RUN_SHOOT))
        {
            this->setState(new Kicking());
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

IComponent* CPlayer::getComponent(string name)
{
	return _component[name];
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
	return const_cast<CPlayer*>(this)->_sprite->getBounding();
}

void CPlayer::onCollision(ICollidable* other)
{
	// Handle collision with wall or other
	// For now, stop movement if colliding with wall
	// Assume other is CWall
	if (dynamic_cast<CWall*>(other))
	{
		// Stop velocity
		setVelocity(VECTOR2ZERO);
		// Or adjust position to not overlap
	}
}

GVector2 CPlayer::getVelocity()
{
	Movement* movement = static_cast<Movement*>(getComponent("Movement"));
	return movement ? movement->getVelocity() : VECTOR2ZERO;
}

void CPlayer::setVelocity(GVector2 vel)
{
	Movement* movement = static_cast<Movement*>(getComponent("Movement"));
	if (movement)
	{
		movement->setVelocity(vel);
	}
}

void CPlayer::OnCollisionWith(CCollisionEvent* e)
{
	// Handle collision event
	// For example, if colliding with wall
	if (dynamic_cast<CWall*>(e->obj))
	{
		// Adjust position based on nx, ny
		GVector2 pos = getPosition();
		pos.x += e->nx * 0.01f; // Push factor
		pos.y += e->ny * 0.01f;
		setPosition(pos);
		// Stop velocity if blocking
		if (e->nx != 0) setVelocity(GVector2(0, getVelocity().y));
		if (e->ny != 0) setVelocity(GVector2(getVelocity().x, 0));
	}
}

void CPlayer::GetSpeed(float& vx, float& vy)
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
