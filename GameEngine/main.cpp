
#include <Windows.h>
#include "CGame.h"
#include "CMegaman.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdLine, int nCmdShow)
{
	CGame* game = new CMegaman(hInstance, L"MEGAMAN X3");
	game->init();
	game->run();
	game->release();
	return 0;
}