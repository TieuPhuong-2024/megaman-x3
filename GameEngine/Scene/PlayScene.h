#pragma once
#include "../Object/CPlayer.h"
#include "../Framework/Scene.h"
#include <vector>
#include "../Object/CWall.h"

class TileMap;

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene();

	bool  init() override;
	void  updateInput(float dt) override;
	void  update(float dt) override;
	void  draw(LPD3DXSPRITE spriteHandle) override;
	void  release() override;
	void  updateViewport(float dt);

private:
	CPlayer* _player;
	TileMap* _tileMap;
	std::vector<CWall*> _walls;
};