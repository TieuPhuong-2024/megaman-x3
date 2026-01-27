#include "CGame.h"
#include <memory>
#include "trace.h"
#include "Scene/PlayScene.h"

std::unique_ptr<Graphics> CGame::s_hWindows = nullptr;
bool CGame::s_bIsExited = false;

CGame::CGame(HINSTANCE hInstance, LPCWSTR strName, int width, int height, int fps, int isFullScreen)
{
	s_hWindows = std::make_unique<Graphics>(hInstance, strName, width, height, fps, isFullScreen);
	_D3DXSprite = nullptr;
	_deltaTime = 0;
	_oldTime = 0;
	_frameRate = 0;
}

void CGame::init()
{
	if (!s_hWindows)
	{
		throw;
	}

	s_hWindows->m_Wnd = static_cast<WNDPROC>(wWinProc);
	s_hWindows->initWindow();
	GameTime::getInstance()->init();
	DeviceManager::getInstance()->Init(*s_hWindows);
	if (!InputController::getInstance()->init(s_hWindows->getWnd(), s_hWindows->getHINSTANCE()))
	{
		throw std::runtime_error("Failed to initialize input controller");
	}

	this->_frameRate = 1000.0f / s_hWindows->getFrameRate(); // 1000/30 = 33 millisecond

	D3DXCreateSprite(DeviceManager::getInstance()->getDevice(), &_D3DXSprite);
	this->loadResource();

	_oldTime = GameTime::getInstance()->getTotalGameTime();
	_deltaTime = 0;
}

CGame::~CGame()
{
	GAMELOG("~CGame");
}

void CGame::release()
{
	DeviceManager::release();
	GameTime::release();
	if (_D3DXSprite)
		_D3DXSprite->Release();
	_D3DXSprite = nullptr;
}

void CGame::update(float deltaTime)
{
}

void CGame::draw()
{
}

void CGame::run()
{
	MSG msg;
	while (s_bIsExited == false)
	{
#pragma region Translate Message
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				s_bIsExited = true;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} // don't mention it.  see e book if you want more info
#pragma endregion

		auto gameTime = GameTime::getInstance();
		gameTime->updateGameTime(); // game time isn't run if don't call updateGameTime
		_deltaTime = gameTime->getTotalGameTime() - _oldTime;

		if (_deltaTime >= _frameRate)
		{
			_oldTime += _frameRate;
			auto time = gameTime->getElapsedGameTime();
			if (time > _frameRate * 2)
				time = _frameRate;

			InputController::getInstance()->update();
			update(time);
			render();
		}
		else
			Sleep(_frameRate - _deltaTime); // sleep every frame for high performance
	}
}

void CGame::render()
{
	if (GetActiveWindow() != s_hWindows->getWnd())
		return;

	auto deviceManager = DeviceManager::getInstance();
	if (deviceManager->getDevice()->BeginScene() != DI_OK)
		return;

	deviceManager->clearScreen();
	draw();

	deviceManager->getDevice()->EndScene();
	deviceManager->present();
}

void CGame::loadResource()
{
}

void CGame::exit()
{
	s_bIsExited = true;
}

Graphics *CGame::getWindows()
{
	return s_hWindows.get();
}

LRESULT CGame::wWinProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(wnd, uMsg, wParam, lParam);
	}
	return 0;
}
