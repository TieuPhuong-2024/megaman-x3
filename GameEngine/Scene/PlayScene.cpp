#include "PlayScene.h"
#include "../Framework/StageManager.h"
#include "../Framework/DebugDraw.h"
#include "../Framework/define.h"
#include "../Object/CWall.h"
#include "../trace.h"
#include "../Object/Collision.h"

PlayScene::PlayScene() : _tileMap(nullptr)
{
}

PlayScene::~PlayScene()
{
	GAMELOG("~PlayScene");
}

bool PlayScene::init()
{
	_viewport = std::unique_ptr<Viewport>(new Viewport(0, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT));
	_tileMap = StageManager::getInstance().getTileMap(eID::MAP_STAGE_MEGAMAN);

	// Add walls from tilemap
	_walls = _tileMap->GetWalls();
	
	// Create player at starting position
	_player = std::unique_ptr<Player>(new Player(GVector2(100.0f, 300.0f)));
	_player->init();
	
	return true;
}

void PlayScene::update(float dt)
{
	// Update viewport
	updateViewport(dt);

	// Update player
	if (_player)
	{
		_player->update(dt);
	}

	// Handle collisions between player and walls
	for (const auto &wall : _walls)
	{
	}
}

void PlayScene::draw(LPD3DXSPRITE spriteHandle)
{
	// Draw tile map
	_tileMap->draw(spriteHandle, _viewport.get());

	// Draw player
	if (_player)
	{
		_player->draw(spriteHandle, _viewport.get());
	}

	// Debug drawing
	auto &debugDraw = DebugDraw::getInstance();
	if (debugDraw.isEnabled())
	{
		// Draw player bounding box
		if (_player)
		{
			debugDraw.DrawBBoxInWorld(_player->GetBoundingBox(), _viewport.get(), D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)); // Green for player
		}

		for (const auto &wall : _walls)
		{
			debugDraw.DrawBBoxInWorld(wall->GetBoundingBox(), _viewport.get(), D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f)); // Magenta for walls
		}

		// Draw some sample primitives
		debugDraw.drawText("Debug Mode: ON (F1 to toggle)", GVector2(10, 10), D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	}
}

void PlayScene::release()
{
	GAMELOG("PlayScene::release() starting");
	_tileMap->release();
	GAMELOG("TileMap released");

	// Clean up player
	_player.reset();
	GAMELOG("Player released");

	// Clean up walls
	GAMELOG("Clearing walls");
	_walls.clear();
	GAMELOG("Walls cleared");
	GAMELOG("PlayScene::release() finished");
}

void PlayScene::updateViewport(float dt)
{
	GVector2 currentPosition = _viewport->getPositionWorld();
	GVector2 worldSize = _tileMap->getWorldSize();
}