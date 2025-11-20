#pragma once
#include <Windows.h>
#include "Framework/Graphics.h"
#include "Framework/DeviceManager.h"
#include "Framework/GameTime.h"
#include "Framework/InputController.h"

class CGame
{
public:
	virtual ~CGame() = default;
	explicit CGame(HINSTANCE hInstance, LPCWSTR = L"Window Game", int width = 800, int height = 600, int fps = 60, int isFullScreen = 0);
	virtual void init();
	virtual void release();

	virtual void updateInput(float deltaTime);
	virtual void update(float deltaTime);
	virtual void draw();

	virtual void run();
	virtual void render();

	virtual void loadResource();

	static void exit();

	static Graphics* getWindows();

	static LRESULT CALLBACK wWinProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	DeviceManager*		_pDeviceManager;
	GameTime*			_pGameTime;
	ID3DXSprite*		_D3DXSprite;
	InputController*	_pInput;
	static Graphics*	s_hWindows;

	static bool			s_bIsExited;

	float				_deltaTime;
	float				_oldTime;
	float				_frameRate;
};