#pragma once
#include "../Framework/Scene.h"
#include <vector>
#include <memory>
#include "../Object/CWall.h"
#include "../Object/Player.h"

class TileMap;

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene();

	bool init() override;
	void update(float dt) override;
	void draw(LPD3DXSPRITE spriteHandle) override;
	void release() override;
	void updateViewport(float dt);

private:
	TileMap *_tileMap;
	std::vector<std::unique_ptr<CWall>> _walls;
	std::unique_ptr<Player> _player;
};