#pragma once
#include "../Observer.h"
#include "../Framework/Viewport.h"
#include "../Framework/Sprite.h"
#include "../Framework/Animation.h"
#include "../Framework/ICollidable.h"
#include <memory>

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
	void draw(ID3DXSprite *spriteHandler, Viewport *viewport);
	void setState(std::unique_ptr<PlayerState> newState);
	void setState(eStatus status);

	void eventKeyUp(KeyEventArg *e) override;
	void eventKeyDown(KeyEventArg *e) override;

private:
	std::shared_ptr<Sprite> _sprite;
	map<eStatus, std::unique_ptr<Animation>> _spriteAnimation;
	eStatus _currentIndexState;
	map<string, std::unique_ptr<IComponent>> _component;
	InputController &_input;

	std::unique_ptr<PlayerState> _playerState;

	bool _isFlipX;

	eMoveDirection _MoveDirection;

	// Legacy flag retained for compatibility with existing logic.
	bool _isJumping;

	bool _allowShoot;

	float _timeShoot;

	// --- Jump / air control extended state ---
	// Maximum number of jumps allowed before requiring grounding (e.g., 2 for double-jump).
	int _maxJumps = 2;

	// Remaining jumps currently available.
	int _remainingJumps = 0;

	// Jump buffering: if the player pressed jump slightly before landing, remember it for this many seconds.
	// (seconds)
	float _jumpBufferTime = 0.12f;

	// Internal timer counting down the buffered jump (seconds). Zero or negative means none buffered.
	float _jumpBufferTimer = 0.0f;

	// Coyote time: forgiving window after walking off an edge where a jump is still allowed (seconds).
	float _coyoteTime = 0.12f;

	// Internal timer counting down the coyote window (seconds). Zero or negative means not in coyote.
	float _coyoteTimer = 0.0f;

	// Whether the player is considered grounded (set by collision system via notifyGrounded)
	bool _isGrounded = true;

	// --- End jump / air control extended state ---

	// Collision
	RECT _boundingBox;

	bool _isBlockingLeft = false;
	bool _isBlockingRight = false;

public:
	void setPosition(GVector2 position);
	GVector2 getPosition();

	void setScale(float scale);

	IComponent *getComponent(string name);

	GVector2 getVelocity();

	void setVelocity(GVector2 vel);

	void setFlipX(bool isFlip);
	bool getFlipX();

	void setMoveDirection(eMoveDirection moveDirection);
	eMoveDirection getMoveDirection();

	// ICollidable
	RECT getBoundingBox() const override;
	void onCollision(ICollidable *other) override;
	void OnCollisionWith(CCollisionEvent *e) override;
	void GetSpeed(float &vx, float &vy) override;
	void SetPosition(float x, float y) override;
	bool IsBlocking() override;
	bool IsBlockingLeft() { return _isBlockingLeft; };
	bool IsBlockingRight() { return _isBlockingRight; };
};