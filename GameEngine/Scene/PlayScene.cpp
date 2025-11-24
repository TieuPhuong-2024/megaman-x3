#include "PlayScene.h"
#include "../Framework/StageManager.h"
#include "../Framework/CollisionManager.h"
#include "../Framework/DebugDraw.h"
#include "../Framework/define.h"
#include "../Object/CWall.h"
#include "../trace.h"

PlayScene::PlayScene() : _tileMap(nullptr)
{
}

PlayScene::~PlayScene()
{
	GAMELOG("~PlayScene");
}

bool PlayScene::init()
{
	_viewport = std::make_unique<Viewport>(0, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
	_player = std::make_unique<CPlayer>();
	_player->setPosition(GVector2(100.f, 200.f));
	_player->setScale(2.0f);
	_tileMap = StageManager::getInstance().getTileMap(eID::MAP_STAGE_MEGAMAN);

	// Add walls from tilemap
	_walls = _tileMap->GetWalls();

	return true;
}

void PlayScene::updateInput(float dt)
{
	// Update input for player
	_player->updateInput(dt);

	// Handle zoom
	InputController &input = InputController::getInstance();
	if (input.isKeyPressed(DIK_EQUALS) || input.isKeyPressed(DIK_ADD)) // + key
	{
		float currentZoom = _viewport->getZoom();
		_viewport->setZoom(currentZoom * 1.1f);
	}
	if (input.isKeyPressed(DIK_MINUS)) // - key
	{
		float currentZoom = _viewport->getZoom();
		_viewport->setZoom(currentZoom / 1.1f);
	}
}

void PlayScene::update(float dt)
{
	// Update viewport
	updateViewport(dt);

	// Update player
	_player->update(dt);

	// Simple AABB collision check between player and walls
	for (const auto &wall : _walls)
	{
		if (aabbOverlap(_player->getBoundingBox(), wall->getBoundingBox()))
		{
			GAMELOG("Collision detected between player and wall at position (%.2f, %.2f)", wall->getPosition().x, wall->getPosition().y);
			_player->onCollision(wall.get());
		}
	}
}

void PlayScene::draw(LPD3DXSPRITE spriteHandle)
{
	// Draw tile map
	_tileMap->draw(spriteHandle, _viewport.get());

	// Draw player
	_player->draw(spriteHandle, _viewport.get());

	// Debug drawing
	auto &debugDraw = DebugDraw::getInstance();
	if (debugDraw.isEnabled())
	{
		// Draw collision boxes with viewport transform
		debugDraw.DrawCollisionBoxInWorld(_player.get(), _viewport.get(), D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)); // Green for player

		for (const auto &wall : _walls)
		{
			debugDraw.DrawBBoxInWorld(wall->getBoundingBox(), _viewport.get(), D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f)); // Magenta for walls
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
	auto playerX = _player->getPosition().x;

	GVector2 new_position = GVector2(max(playerX - WINDOW_WIDTH / 2, 0), WINDOW_HEIGHT);

	if (_player->getVelocity().x == 0)
	{
		new_position = currentPosition;
	}

	if (new_position.x + WINDOW_WIDTH > worldSize.x)
	{
		new_position.x = worldSize.x - WINDOW_WIDTH;
	}

	_viewport->setPositionWorld(new_position);
}