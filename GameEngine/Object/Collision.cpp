#include "Collision.h"
#include "../trace.h"

bool CCollision::IsColliding(const RECT &bbox1, const RECT &bbox2)
{
	float left = bbox2.left - bbox1.right;
	float top = bbox2.top - bbox1.bottom;
	float right = bbox2.right - bbox1.left;
	float bottom = bbox2.bottom - bbox1.top;

	return !(left > 0 || right < 0 || top < 0 || bottom > 0);
}

RECT CCollision::GetBroadphaseBox(BaseObject *object, float dt)
{
    auto velocity = object->GetVelocity() * dt / 1000.0f;
    auto myRect = object->GetBoundingBox();

    RECT rect;
    rect.top = velocity.y > 0 ? myRect.top + velocity.y : myRect.top;
    rect.bottom = velocity.y > 0 ? myRect.bottom : myRect.bottom + velocity.y;
    rect.left = velocity.x > 0 ? myRect.left : myRect.left + velocity.x;
    rect.right = velocity.x > 0 ? myRect.right + velocity.x : myRect.right;

    return rect;
}

CCollision &CCollision::GetInstance()
{
    static CCollision instance;
    return instance;
}

float CCollision::SweptAABB(BaseObject *obj, BaseObject *otherObj, float dt, float &normalX, float &normalY)
{
    RECT bbox1 = obj->GetBoundingBox();
    RECT bbox2 = otherObj->GetBoundingBox();

    RECT broadphaseBox = GetBroadphaseBox(obj, dt);
    if (!IsColliding(broadphaseBox, bbox2))
    {
        GAMELOG("No broadphase collision detected");
        normalX = 0.0f;
        normalY = 0.0f;
        return 1.0f;
    }

    GVector2 myVel = obj->GetVelocity() * dt / 1000.0f;
    GVector2 otherVel = otherObj->GetVelocity() * dt / 1000.0f;

    auto currentVel = myVel;
    if (otherVel != GVector2(0, 0))
    {
        currentVel = myVel - otherVel;
    }

    float dxEntry, dxExit;
    float dyEntry, dyExit;
    if (currentVel.x > 0.0f)
    {
        dxEntry = bbox2.left - bbox1.right;
        dxExit = bbox2.right - bbox1.left;
    }
    else
    {
        dxEntry = bbox2.right - bbox1.left;
        dxExit = bbox2.left - bbox1.right;
    }

    if (currentVel.y > 0.0f)
    {
        dyEntry = bbox2.bottom - bbox1.top;
        dyExit = bbox2.top - bbox1.bottom;
    }
    else
    {
        dyEntry = bbox2.top - bbox1.bottom;
        dyExit = bbox2.bottom - bbox1.top;
    }

    float txEntry, txExit;
    float tyEntry, tyExit;
    if (currentVel.x == 0.0f)
    {
        txEntry = -FLT_MAX;
        txExit = FLT_MAX;
    }
    else
    {
        txEntry = dxEntry / currentVel.x;
        txExit = dxExit / currentVel.x;
    }

    if (currentVel.y == 0.0f)
    {
        tyEntry = -FLT_MAX;
        tyExit = FLT_MAX;
    }
    else
    {
        tyEntry = dyEntry / currentVel.y;
        tyExit = dyExit / currentVel.y;
    }

    float entryTime = max(txEntry, tyEntry);
    float exitTime = min(txExit, tyExit);

    if (entryTime > exitTime || (txEntry < 0.0f && tyEntry < 0.0f) || txEntry > 1.0f || tyEntry > 1.0f)
    {
        GAMELOG("No collision detected after calculations");
        normalX = 0.0f;
        normalY = 0.0f;
        return 1.0f;
    }

    if (txEntry > tyEntry)
    {
        if (dxEntry < 0.0f)
        {
            normalX = 1.0f;
            normalY = 0.0f;
        }
        else
        {
            normalX = -1.0f;
            normalY = 0.0f;
        }
    }
    else
    {
        if (dyEntry < 0.0f)
        {
            normalX = 0.0f;
            normalY = 1.0f;
        }
        else
        {
            normalX = 0.0f;
            normalY = -1.0f;
        }
    }

    return entryTime;
}
