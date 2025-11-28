#pragma once

#include "../Framework/define.h"

class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();

	virtual RECT GetBoundingBox() const;
	virtual GVector2 GetVelocity() const;
	int GetID() const { return _id; }
	void SetID(int id) { _id = id; }

private:
	int _id;
};
