#pragma once
#include <windows.h>

struct ID3DXSprite;
class EventArg;

class AssetLoader
{
public:
	static AssetLoader& GetInstance() {
		static AssetLoader instance;
		return instance;
	}

	void loadResource(ID3DXSprite* sprite);
	void loadSound(HWND hWnd);
	void loadStage();
};

