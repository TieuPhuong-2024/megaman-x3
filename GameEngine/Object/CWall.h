#pragma once
#include "BaseObject.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "../Framework/ICollidable.h"
#include "../Framework/Viewport.h"

class CWall : public BaseObject, public ICollidable
{
public:
	CWall(float x, float y, float width, float height, int id);
	CWall();
	~CWall();

	void update(float deltaTime);
	void draw(LPD3DXSPRITE spriteHandle, Viewport* viewport);

	// ICollidable
	RECT getBoundingBox() const override;
	void onCollision(ICollidable* other) override;
	void OnCollisionWith(CCollisionEvent* e) override;
	GVector2 getPosition() override;
	void setPosition(GVector2 pos) override;
	void GetSpeed(float& vx, float& vy) override;
	void SetPosition(float x, float y) override;
	GVector2 getVelocity() override;
	void setVelocity(GVector2 vel) override;
	bool IsBlocking() override;

	RECT GetBoundingBox() const { return _boundingBox; }
	void SetBoundingBox(RECT bbox) { this->_boundingBox = bbox; }
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
	RECT        _boundingBox;
	float       _width;
	float       _height;
	float       _x;
	float       _y;
	int         _id;
};