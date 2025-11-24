#include "CMegaman.h"
#include "Framework/SceneManager.h"
#include "Framework/define.h"
#include "Framework/DebugDraw.h"
#include "AssetLoader.h"
#include "trace.h"
#include <memory>

CMegaman::CMegaman(HINSTANCE hInstance, LPCWSTR title) : CGame(hInstance, title, WINDOW_WIDTH, WINDOW_HEIGHT)
{
}

CMegaman::~CMegaman()
{
	GAMELOG("~CMegaman");
}

void CMegaman::init()
{
	CGame::init();
	DebugDraw::getInstance().init(_pDeviceManager->getDevice());
	SceneManager::getInstance().addScene(std::make_unique<PlayScene>());
}

void CMegaman::release()
{
	GAMELOG("CMegaman::release() starting");
	CGame::release();
	DebugDraw::release();
	GAMELOG("About to call clearScenes()");
	SceneManager::getInstance().clearScenes();
	GAMELOG("clearScenes() done");
	GAMELOG("CMegaman::release() finished");
}

void CMegaman::update(float deltaTime)
{
	// Update player
	SceneManager::getInstance().getCurrentScene()->update(deltaTime);
}

void CMegaman::draw()
{
	// Start drawing
	_D3DXSprite->Begin(D3DXSPRITE_ALPHABLEND);

	// Draw scene
	SceneManager::getInstance().getCurrentScene()->draw(_D3DXSprite);

	// End drawing
	_D3DXSprite->End();

	// Debug drawing
	DebugDraw::getInstance().begin();
	// Add debug draw calls here if needed
	DebugDraw::getInstance().end();
}

void CMegaman::loadResource()
{
	// Load resource here
	auto &assetLoader = AssetLoader::GetInstance();
	assetLoader.loadResource(_D3DXSprite);
	assetLoader.loadStage();
	assetLoader.loadSound(s_hWindows->getWnd());
}

void CMegaman::updateInput(float deltaTime)
{
	// Toggle debug draw with F1
	if (_pInput.isKeyPressed(DIK_F1))
	{
		DebugDraw::getInstance().toggle();
	}

	// Update input for current scene
	SceneManager::getInstance().getCurrentScene()->updateInput(deltaTime);
}
