#pragma once
#include "../Framework/define.h"
#include "../Object/BaseObject.h"
#include "../States/PlayerState.h"
#include <memory>
#include <map>
#include "../Framework/Animation.h"

class Player : public BaseObject
{
public:
    Player(GVector2 position);
    ~Player();

    void init();
    void update(float dt);
    void draw(LPD3DXSPRITE spriteHandle, Viewport *viewport);
    void release();

    // State management
    void setState(PlayerState *newState);
    eStatus getCurrentStatus() const;
    PlayerState *getCurrentState() const { return _currentState.get(); }

    // Component accessors
    Movement *getMovement() const { return _movement.get(); }
    Gravity *getGravity() const { return _gravity.get(); }
    Animation *getAnimation() const { return _animation.get(); }

    // Position and bounding box
    GVector2 getPosition() const { return _position; }
    void setPosition(GVector2 position) { _position = position; }

    virtual RECT GetBoundingBox() const override;
    virtual GVector2 GetVelocity() const override;

    // Direction
    eDirection getFacingDirection() const { return _facingDirection; }
    void setFacingDirection(eDirection direction) { _facingDirection = direction; }

    // Animation management
    void setAnimation(eStatus status);

private:
    GVector2 _position;
    GVector2 _scale;
    eDirection _facingDirection;

    // Components
    std::unique_ptr<Movement> _movement;
    std::unique_ptr<Gravity> _gravity;
    std::unique_ptr<Animation> _animation;

    // State machine
    std::unique_ptr<PlayerState> _currentState;

    // Animation mapping
    std::map<eStatus, std::string> _animationMap;

    // Internal helper methods
    void initAnimations();
    void initComponents();
};