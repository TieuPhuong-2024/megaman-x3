#pragma once
#include "define.h"

struct CCollisionEvent;

class ICollidable
{
public:
    virtual ~ICollidable() {}

    virtual RECT GetBoundingBox() const = 0;
    virtual void OnCollision(ICollidable* other) = 0; // Keep for compatibility
    virtual void OnCollisionWith(CCollisionEvent* e) = 0;
    virtual GVector2 GetPosition() = 0;
    virtual void SetPosition(GVector2 pos) = 0;
    virtual void GetSpeed(float& vx, float& vy) = 0;
    virtual void SetPosition(float x, float y) = 0;
    virtual GVector2 GetVelocity() = 0;
    virtual void SetVelocity(GVector2 vel) = 0;
    virtual bool IsBlocking() = 0;
};