#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <Windows.h>
#include "define.h"

class Graphics
{
public:
	Graphics(HINSTANCE hInstance, LPCWSTR name, int width, int height, int fps, int isFullScreen);
	~Graphics();
	void initWindow();

	int isFullScreen();
	int getWidth();
	int getHeight();
	int getFrameRate();
	HWND getWnd();
	HINSTANCE getHINSTANCE();

	WNDPROC m_Wnd;

protected:
	HINSTANCE	_hInstance;
	HWND		_hWnd;

	LPCWSTR		_wcName;
	int			_isFullScreen;

	int			_width;
	int			_height;
	int			_fps;

	Graphics();
private:
};

typedef Graphics* pGraphics;


#endif // !__GRAPHICS_H__
