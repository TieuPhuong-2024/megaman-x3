#ifndef __SPRITEMANAGER_H__
#define __SPRITEMANAGER_H__

#include <map>
#include <memory>
#include <fstream>

#include "define.h"
#include "Sprite.h"

class SpriteManager
{
public:
	// Thread-safe Singleton
	static SpriteManager &getInstance();

	// Delete copy constructor và assignment operator
	SpriteManager(const SpriteManager &) = delete;
	SpriteManager &operator=(const SpriteManager &) = delete;

	// Quản lý Sprite với shared_ptr
	void addSprite(eID id, std::shared_ptr<Sprite> sprite);
	std::shared_ptr<Sprite> getSprite(eID id) const;
	bool hasSprite(eID id) const;
	void removeSprite(eID id, bool releaseTexture = true);
	void clearAllSprites();

	// Load từ file
	std::shared_ptr<Sprite> loadXMLDoc(LPD3DXSPRITE spritehandle, LPCWSTR path);
	void loadXML(eID id, LPCWSTR XMLPath);

	// Sprite metadata
	RECT getSourceRect(eID id, std::string name) const;
	GVector2 getOrigin(eID id, std::string name) const;

	// Destructor
	~SpriteManager();

private:
	SpriteManager() = default;

	std::map<eID, std::shared_ptr<Sprite>> _listSprite;
	std::map<eID, std::map<std::string, RECT>> _sourceRectList;
	std::map<eID, std::map<std::string, GVector2>> _originList;
};

#endif // !__SPRITEMANAGER_H__
