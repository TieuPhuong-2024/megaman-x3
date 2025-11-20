#include "PlayScene.h"
#include "../Framework/StageManager.h"
#include "../Framework/CollisionManager.h"
#include "../Framework/DebugDraw.h"
#include "../Object/CWall.h"
#include "../trace.h"

PlayScene::PlayScene() : _player(nullptr), _tileMap(nullptr)
{
}

PlayScene::~PlayScene()
{
}

bool PlayScene::init()
{
	_viewport = new Viewport(0, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
	_player = new CPlayer;
	_player->setPosition(GVector2(100.f, 50.f));
	_player->setScale(2);
	_tileMap = StageManager::getInstance()->getTileMap(eID::MAP_STAGE_MEGAMAN);

	// Add to collision manager
	CollisionManager::getInstance()->addObject(_player);

	// Add walls from tilemap
	_walls = _tileMap->GetWalls();

	return true;
}

void PlayScene::updateInput(float dt)
{
	// Update input for player
	_player->updateInput(dt);
}

void PlayScene::update(float dt)
{
	// Update viewport
	updateViewport(dt);

	// Update player
	_player->update(dt);

	// Update collisions
	CollisionManager::getInstance()->update();
}

void PlayScene::draw(LPD3DXSPRITE spriteHandle)
{
	// Draw tile map
	_tileMap->draw(spriteHandle, _viewport);

	// Draw player
	_player->draw(spriteHandle, _viewport);

	// Debug drawing
	DebugDraw* debugDraw = DebugDraw::getInstance();
	if (debugDraw->isEnabled())
	{
		// Draw collision boxes with viewport transform
		debugDraw->drawCollisionBox(_player, _viewport, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)); // Green for player
		
		// Debug first wall only to avoid spam
		static bool debugOnce = false;
		if (!debugOnce && !_walls.empty()) {
			CWall* wall = _walls[0];
			RECT bbox = wall->getBoundingBox();
			GVector2 vpPos = _viewport->getPositionWorld();
			GVector3 topLeft = _viewport->getPositionInViewport(&GVector3((float)bbox.left, (float)bbox.top, 0));
			GVector3 bottomRight = _viewport->getPositionInViewport(&GVector3((float)bbox.right, (float)bbox.bottom, 0));
			printLog("[Draw] First Wall: bbox(L=%d,T=%d,R=%d,B=%d) VP(%.0f,%.0f) -> screen(%.0f,%.0f,%.0f,%.0f)\n",
				bbox.left, bbox.top, bbox.right, bbox.bottom, vpPos.x, vpPos.y,
				topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
			debugOnce = true;
		}
		
		for (auto wall : _walls)
		{
			debugDraw->drawBoundingBox(wall->getBoundingBox(), _viewport, D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f)); // Magenta for walls
		}

		// Draw some sample primitives
		debugDraw->drawText("Debug Mode: ON (F1 to toggle)", GVector2(10, 10), D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	}
}

void PlayScene::release()
{
	SAFE_DELETE(_viewport);
	SAFE_DELETE(_player);
	_tileMap->release();
	SAFE_DELETE(_tileMap);

	// Clean up test walls
	for (auto wall : _walls) {
		SAFE_DELETE(wall);
	}
	_walls.clear();
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