
#include "BaseObject.h"

BaseObject::BaseObject()
{
}

BaseObject::~BaseObject()
{
}

RECT BaseObject::GetBoundingBox() const
{
    return RECT();
}

GVector2 BaseObject::GetVelocity() const
{
    return GVector2();
}
