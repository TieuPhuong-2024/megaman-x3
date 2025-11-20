#ifndef __SPRITEMANAGER_H__
#define __SPRITEMANAGER_H__

#include <map> // map contain a pair data with key and value
// http://www.cplusplus.com/reference/map/map
#include <fstream>

#include "define.h"
#include "Sprite.h"

class SpriteManager
{
public:
	static SpriteManager* getInstance();
	static void release();

	std::map<eID, Sprite*>* getListSprite();
	Sprite* getSprite(eID id);
	RECT getSourceRect(eID id, std::string name);
	GVector2 getOrigin(eID id, std::string name);

	void loadSpriteInfo(eID id, const char* fileInfoPath);
	Sprite* loadXMLDoc(LPD3DXSPRITE spritehandle, LPCWSTR path);
	void loadXML(eID id, LPCWSTR XMLPath);

	void releaseSprite(eID id);
	void releaseTexture(eID id);

	~SpriteManager(void);
private:
	SpriteManager(void);
	static SpriteManager* s_instance;
	std::map<eID, Sprite*> _listSprite;
	std::map<eID, std::map<std::string, RECT>> _sourceRectList;
	std::map<eID, std::map<std::string, GVector2>> _originList;
};


#endif // !__SPRITEMANAGER_H__
