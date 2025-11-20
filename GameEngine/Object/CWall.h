#pragma once
#include "BaseObject.h"
#include <d3dx9.h>
#include "../Framework/ICollidable.h"

class CWall : public BaseObject
{
public:
	CWall(float x, float y, float width, float height, int id);
	CWall();
	~CWall();

	// ICollidable
	RECT getBoundingBox() const;
	GVector2 getPosition();
	void setPosition(GVector2 pos);
	void SetPosition(float x, float y);
	bool IsBlocking();

	float GetWidth() const { return _width; }
	void SetWidth(float width) { _width = width; }
	float GetHeight() const { return _height; }
	void SetHeight(float height) { _height = height; }
	float GetX() const { return _x; }
	void SetX(float x) { _x = x; }
	float GetY() const { return _y; }
	void SetY(float y) { _y = y; }
	float GetId() const { return _id; }
	void SetId(int id) { _id = id; }


private:
	float       _width;
	float       _height;
	float       _x;
	float       _y;
	int         _id;
};