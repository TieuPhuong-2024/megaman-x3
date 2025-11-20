#pragma once
#include <windows.h>

struct ID3DXSprite;
class EventArg;

class Utils
{
public:
	static void loadResource(ID3DXSprite* sprite);
	static void loadSound(HWND hWnd);
	static void loadStage();
};

