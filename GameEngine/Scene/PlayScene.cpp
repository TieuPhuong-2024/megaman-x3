#include "PlayScene.h"
#include "../Framework/StageManager.h"
#include "../Framework/DebugDraw.h"
#include "../Framework/define.h"
#include "../Framework/Collision/World.h"
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
	_player->SetPosition(GVector2(100.f, 200.f));
	_player->SetScale(2.0f);
	_tileMap = StageManager::getInstance().getTileMap(eID::MAP_STAGE_MEGAMAN);

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

	// Build list of collidable objects (walls, platforms, etc.)
	vector<CWall*> collidables;
	for (const auto& wall : _walls)
	{
		collidables.push_back(wall.get());
	}

	// Process collisions with walls
	// TODO: Implement collision detection using Framework\Collision system
	for (const auto& wall : collidables)
	{
		// Collision detection will be implemented using the Box2D-style collision system
		// Check if player collides with wall
		RECT playerBBox = _player->GetBoundingBox();
		RECT wallBBox = wall->GetBoundingBox();
		
		// Simple AABB collision check
		if (!(playerBBox.right < wallBBox.left || playerBBox.left > wallBBox.right ||
			  playerBBox.bottom < wallBBox.top || playerBBox.top > wallBBox.bottom))
		{
			// Collision detected
			// TODO: Implement more sophisticated collision response
		}
	}

	GAMELOG("Collision: IsGrounded=%d, BlockLeft=%d, BlockRight=%d", 
		_player->IsGrounded(), _player->IsBlockingLeft(), _player->IsBlockingRight());
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
		// Draw player bounding box
		debugDraw.DrawBBoxInWorld(_player->GetBoundingBox(), _viewport.get(), D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)); // Green for player

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
	auto playerX = _player->GetPosition().x;

	GVector2 new_position = GVector2(max(playerX - WINDOW_WIDTH / 2, 0), WINDOW_HEIGHT);

	if (_player->GetVelocity().x == 0)
	{
		new_position = currentPosition;
	}

	if (new_position.x + WINDOW_WIDTH > worldSize.x)
	{
		new_position.x = worldSize.x - WINDOW_WIDTH;
	}

	_viewport->setPositionWorld(new_position);
}