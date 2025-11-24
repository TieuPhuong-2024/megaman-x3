#pragma once
#include "define.h"

struct CCollisionEvent;

class ICollidable
{
public:
    virtual ~ICollidable() {}

    virtual RECT getBoundingBox() const = 0;
    virtual void onCollision(ICollidable* other) = 0; // Keep for compatibility
    virtual void OnCollisionWith(CCollisionEvent* e) = 0;
    virtual GVector2 getPosition() = 0;
    virtual void setPosition(GVector2 pos) = 0;
    virtual void GetSpeed(float& vx, float& vy) = 0;
    virtual void SetPosition(float x, float y) = 0;
    virtual GVector2 getVelocity() = 0;
    virtual void setVelocity(GVector2 vel) = 0;
    virtual bool IsBlocking() = 0;
};