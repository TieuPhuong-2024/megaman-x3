#include "AssetLoader.h"
#include "Framework/SpriteManager.h"
#include "Framework/StageManager.h"

void AssetLoader::loadResource(ID3DXSprite *sprite)
{
	auto &spriteManager = SpriteManager::getInstance();

	// Load XMan sprite
	auto xmanSprite = std::shared_ptr<Sprite>(new Sprite(sprite, L"Resource/XMan.png"));
	spriteManager.addSprite(eID::XMAN, xmanSprite);
	spriteManager.loadXML(eID::XMAN, L"Resource/XMan.xml");

	// Load Map sprite với XMLDoc
	auto mapSprite = spriteManager.loadXMLDoc(sprite, L"Resource/MAP_1.tmx");
	if (mapSprite)
	{
		mapSprite->setOrigin(VECTOR2ZERO);
		mapSprite->setScale(GVector2(2.0f, 2.0f));
		spriteManager.addSprite(eID::MAP_STAGE_MEGAMAN, mapSprite);
	}
}

void AssetLoader::loadSound(HWND hWnd)
{
	// Load sound here
}

void AssetLoader::loadStage()
{
	StageManager &stage = StageManager::getInstance();
	stage.addStage(eID::MAP_STAGE_MEGAMAN, "Resource/MAP_1.tmx");
}