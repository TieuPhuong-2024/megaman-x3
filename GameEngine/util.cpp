#include "util.h"
#include "Framework/SpriteManager.h"
#include "Framework/StageManager.h"

void Utils::loadResource(ID3DXSprite* sprite)
{
	SpriteManager* spriteManager = SpriteManager::getInstance();

	Sprite* spr = new Sprite(sprite, L"Resource/XMan.png");
	spriteManager->getListSprite()->insert(make_pair(eID::XMAN, spr));
	spriteManager->loadXML(eID::XMAN, L"Resource/XMan.xml");

	spr = new Sprite(sprite, L"Resource/MAP_1.png");
	spr = spriteManager->loadXMLDoc(sprite, L"Resource/MAP_1.tmx");
	spr->setOrigin(VECTOR2ZERO);
	spriteManager->getListSprite()->insert(make_pair(eID::MAP_STAGE_MEGAMAN, spr));

}

void Utils::loadSound(HWND hWnd)
{
	// Load sound here
}

void Utils::loadStage()
{
	StageManager* stage = StageManager::getInstance();
	stage->getListStage()->insert(make_pair(eID::MAP_STAGE_MEGAMAN, "Resource/MAP_1.tmx"));
}