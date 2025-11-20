#pragma once
#include "CGame.h"
#include "Object/CPlayer.h"
#include "Scene/PlayScene.h"

class CMegaman : public CGame
{
public:
	CMegaman(HINSTANCE hInstance, LPCWSTR title);
	~CMegaman();

	void init() override;
	void release() override;
	void update(float deltaTime) override;
	void draw() override;
	void loadResource() override;
	void updateInput(float deltaTime) override;

private:
};
