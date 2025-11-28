#pragma once

#include <Windows.h>
#include "BaseObject.h"

class CCollision {
public:
    static CCollision& GetInstance();
    ~CCollision() = default;
    CCollision(const CCollision&) = delete;
    CCollision& operator=(const CCollision&) = delete;
    CCollision(CCollision&&) = delete;
    CCollision& operator=(CCollision&&) = delete;

    float SweptAABB(BaseObject* obj, BaseObject* otherObj, float dt, float& normalX, float& normalY);
    
private:
    CCollision() = default;
    bool IsColliding(const RECT &bbox1, const RECT &bbox2);
    RECT GetBroadphaseBox(BaseObject* object, float dt);
};