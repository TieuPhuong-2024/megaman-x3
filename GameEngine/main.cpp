
#include <Windows.h>
#include <memory>
#include "CGame.h"
#include "CMegaman.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdLine, int nCmdShow)
{
	std::unique_ptr<CGame> game = std::make_unique<CMegaman>(hInstance, L"MEGAMAN X3");
	game->init();
	game->run();
	game->release();
	return 0;
}