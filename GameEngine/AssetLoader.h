#pragma once
#include <windows.h>

struct ID3DXSprite;
class EventArg;

class AssetLoader
{
private:
	static AssetLoader* _instance;
public:
	static AssetLoader* GetInstance();

	void loadResource(ID3DXSprite* sprite);
	void loadSound(HWND hWnd);
	void loadStage();
};

