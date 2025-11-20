#include "CMegaman.h"
#include "util.h"
#include "Framework/SceneManager.h"
#include "Framework/define.h"
#include "Framework/DebugDraw.h"
#include "States/PlayerState.h"

CMegaman::CMegaman(HINSTANCE hInstance, LPCWSTR title) : CGame(hInstance, title, WINDOW_WIDTH, WINDOW_HEIGHT)
{
}

CMegaman::~CMegaman()
{
}

void CMegaman::init()
{
	CGame::init();
	DebugDraw::getInstance()->init(_pDeviceManager->getDevice());
	SceneManager::getInstance()->addScene(new PlayScene);
}

void CMegaman::release()
{
	CGame::release();
	DebugDraw::release();
	SceneManager::getInstance()->getCurrentScene()->release();
	SceneManager::getInstance()->clearScenes();
}

void CMegaman::update(float deltaTime)
{
	// Update player
	SceneManager::getInstance()->getCurrentScene()->update(deltaTime);
}

void CMegaman::draw()
{
	// Start drawing
	_D3DXSprite->Begin(D3DXSPRITE_ALPHABLEND);

	// Draw scene
	SceneManager::getInstance()->getCurrentScene()->draw(_D3DXSprite);

	// End drawing
	_D3DXSprite->End();

	// Debug drawing
	DebugDraw::getInstance()->begin();
	// Add debug draw calls here if needed
	DebugDraw::getInstance()->end();
}

void CMegaman::loadResource()
{
	// Load resource here
	Utils::loadResource(_D3DXSprite);
	Utils::loadResource(_D3DXSprite);
	Utils::loadStage();
	Utils::loadSound(s_hWindows->getWnd());
}

void CMegaman::updateInput(float deltaTime)
{
	// Toggle debug draw with F1
	if (_pInput->isKeyPressed(DIK_F1))
	{
		DebugDraw::getInstance()->toggle();
	}

	// Update input for current scene
	SceneManager::getInstance()->getCurrentScene()->updateInput(deltaTime);
}
