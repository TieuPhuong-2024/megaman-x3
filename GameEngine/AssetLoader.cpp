#include "AssetLoader.h"
#include "Framework/SpriteManager.h"
#include "Framework/StageManager.h"

void AssetLoader::loadResource(ID3DXSprite* sprite)
{
	SpriteManager* spriteManager = SpriteManager::getInstance();

	Sprite* spr = new Sprite(sprite, L"Resource/XMan.png");
	spriteManager->getListSprite()->insert(make_pair(eID::XMAN, spr));
	spriteManager->loadXML(eID::XMAN, L"Resource/XMan.xml");

	spr = new Sprite(sprite, L"Resource/MAP_1.png");
	spr = spriteManager->loadXMLDoc(sprite, L"Resource/MAP_1.tmx");
	spr->setOrigin(VECTOR2ZERO);
	spr->setScale(GVector2(2.0f, 2.0f));
	spriteManager->getListSprite()->insert(make_pair(eID::MAP_STAGE_MEGAMAN, spr));
}

void AssetLoader::loadSound(HWND hWnd)
{
	// Load sound here
}

void AssetLoader::loadStage()
{
	StageManager& stage = StageManager::getInstance();
	stage.addStage(eID::MAP_STAGE_MEGAMAN, "Resource/MAP_1.tmx");
}