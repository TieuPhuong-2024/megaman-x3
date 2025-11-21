#pragma once
#include "../Observer.h"
#include "../Framework/Viewport.h"
#include "../Framework/Sprite.h"
#include "../Framework/Animation.h"
#include "../Framework/ICollidable.h"

class PlayerState;

enum class eMoveDirection
{
	NONE,
	MOVE_LEFT,
	MOVE_RIGHT
};

class CPlayer : public IObserverKeyUp, public ICollidable
{
public:
	CPlayer();
	~CPlayer();

	void update(float deltaTime);
	void updateInput(float deltaTime);
	void draw(ID3DXSprite* spriteHandler, Viewport* viewport);
	void setState(PlayerState* newState);
	void setState(eStatus status);

	void eventKeyUp(KeyEventArg* e) override;
	void eventKeyDown(KeyEventArg* e) override;

	void setInputController(InputController* input);
	InputController* getInputController();

	// Jump-related public API (exposed for states and other systems)
	// Attempt to perform a jump (honors jump buffer, coyote time, and remaining jumps).
	void tryJump();

	// Query whether the player is currently allowed to jump (checks remaining jumps or coyote).
	bool canJump() const;

	// Called by collision/physics when player becomes grounded to reset jump counts and clear timers.
	void notifyGrounded();

	// Called by collision/physics when player leaves the ground (starts coyote timer).
	void notifyLeftGround();

	// Configuration accessors for tuning jump behavior at runtime
	void setMaxJumps(int maxJumps);
	int getMaxJumps() const;
	int getRemainingJumps() const;

	void setJumpBufferTime(float seconds);
	float getJumpBufferTime() const;

	void setCoyoteTime(float seconds);
	float getCoyoteTime() const;

private:
	Sprite*						_sprite;
	map<eStatus, Animation*>	_spriteAnimation;
	eStatus						_currentIndexState;
	map<string, IComponent*>	_component;
	InputController*			_input;

	PlayerState*				_playerState;

	bool						_isFlipX;

	eMoveDirection				_MoveDirection;

	// Legacy flag retained for compatibility with existing logic.
	bool						_isJumping;

	bool						_allowShoot;

	float						_timeShoot;

	// --- Jump / air control extended state ---
	// Maximum number of jumps allowed before requiring grounding (e.g., 2 for double-jump).
	int							_maxJumps = 2;

	// Remaining jumps currently available.
	int							_remainingJumps = 0;

	// Jump buffering: if the player pressed jump slightly before landing, remember it for this many seconds.
	// (seconds)
	float						_jumpBufferTime = 0.12f;

	// Internal timer counting down the buffered jump (seconds). Zero or negative means none buffered.
	float						_jumpBufferTimer = 0.0f;

	// Coyote time: forgiving window after walking off an edge where a jump is still allowed (seconds).
	float						_coyoteTime = 0.12f;

	// Internal timer counting down the coyote window (seconds). Zero or negative means not in coyote.
	float						_coyoteTimer = 0.0f;

	// Whether the player is considered grounded (set by collision system via notifyGrounded)
	bool						_isGrounded = true;

	// --- End jump / air control extended state ---

	// Collision
	RECT 						_boundingBox;

public:
	void setPosition(GVector2 position);
	GVector2 getPosition();

	void setScale(float scale);

	IComponent* getComponent(string name);

	GVector2 getVelocity();

	void setVelocity(GVector2 vel);

	void setFlipX(bool isFlip);
	bool getFlipX();

	void setMoveDirection(eMoveDirection moveDirection);
	eMoveDirection getMoveDirection();

	// ICollidable
	RECT getBoundingBox() const override;
	void onCollision(ICollidable* other) override;
	void OnCollisionWith(CCollisionEvent* e) override;
	void GetSpeed(float& vx, float& vy) override;
	void SetPosition(float x, float y) override;
	bool IsBlocking() override;
};