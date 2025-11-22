#include "SpriteManager.h"

#include <string>
#include "pugixml.hpp"

// Thread-safe Singleton implementation
SpriteManager &SpriteManager::getInstance()
{
	static SpriteManager instance;
	return instance;
}

// Constructor is defaulted in header

// Destructor với cleanup tự động
SpriteManager::~SpriteManager()
{
	clearAllSprites();
}

// Quản lý Sprite với shared_ptr
void SpriteManager::addSprite(eID id, std::shared_ptr<Sprite> sprite)
{
	if (!sprite)
	{
		return; // Không thêm sprite nullptr
	}

	// Kiểm tra sprite cũ
	auto it = _listSprite.find(id);
	if (it != _listSprite.end())
	{
		// Sprite cũ sẽ tự động bị giải phóng khi shared_ptr mất tham chiếu
		_listSprite.erase(it);
	}

	_listSprite[id] = sprite;
}

std::shared_ptr<Sprite> SpriteManager::getSprite(eID id) const
{
	auto it = _listSprite.find(id);
	return (it != _listSprite.end()) ? it->second : nullptr;
}

bool SpriteManager::hasSprite(eID id) const
{
	return _listSprite.find(id) != _listSprite.end();
}

void SpriteManager::removeSprite(eID id, bool releaseTexture)
{
	auto it = _listSprite.find(id);
	if (it != _listSprite.end())
	{
		if (releaseTexture && it->second)
		{
			it->second->release();
		}
		_listSprite.erase(it);
		// shared_ptr tự động cleanup khi ra khỏi scope
	}
}

void SpriteManager::clearAllSprites()
{
	// Giải phóng texture trước khi xóa
	for (auto &pair : _listSprite)
	{
		if (pair.second)
		{
			pair.second->release();
		}
	}
	_listSprite.clear();
}

// Load từ file
std::shared_ptr<Sprite> SpriteManager::loadXMLDoc(LPD3DXSPRITE spritehandle, LPCWSTR path)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path, pugi::parse_default | pugi::parse_pi);
	if (result == false)
	{
		OutputDebugString(L"Cannot loading file");
		return nullptr;
	}

	auto tileset_node = doc.child("map").child("tileset");
	int tilecount = tileset_node.attribute("tilecount").as_int();
	int columns = tileset_node.attribute("columns").as_int();
	auto image = tileset_node.child("image");

	std::string filename = image.attribute("source").as_string();
	std::wstring L_filename = std::wstring(filename.begin(), filename.end());
	std::wstring strpath = std::wstring(path);
	int index = strpath.find_last_of(L'//');
	strpath = strpath.substr(0, index);
	strpath += L"/" + L_filename;

	// Tạo shared_ptr cho Sprite mới
	return std::shared_ptr<Sprite>(new Sprite(spritehandle, (LPWSTR)strpath.c_str(), tilecount, columns));
}

void SpriteManager::loadXML(eID id, LPCWSTR XMLPath)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(XMLPath, pugi::parse_default | pugi::parse_pi);
	if (!result)
	{
		OutputDebugString(L"Cannot loading file");
		return;
	}

	pugi::xml_node firstNode = doc.first_child();

	for (auto node = firstNode.first_child(); node; node = node.next_sibling())
	{
		RECT r;
		GVector2 origin;

		LPCSTR nameTile = node.attribute("n").as_string();
		r.left = (long)node.attribute("x").as_int();
		r.top = (long)node.attribute("y").as_int();
		r.right = r.left + (long)node.attribute("w").as_int();
		r.bottom = r.top + (long)node.attribute("h").as_int();
		origin.x = node.attribute("pX").as_float();
		origin.y = node.attribute("pY").as_float();

		_sourceRectList[id][std::string(nameTile)] = r;
		_originList[id][std::string(nameTile)] = origin;
	}
}

// Sprite metadata
RECT SpriteManager::getSourceRect(eID id, std::string name) const
{
	auto it = _sourceRectList.find(id);
	if (it != _sourceRectList.end())
	{
		auto rectIt = it->second.find(name);
		if (rectIt != it->second.end())
		{
			return rectIt->second;
		}
	}
	// Return empty RECT nếu không tìm thấy
	return RECT{0, 0, 0, 0};
}

GVector2 SpriteManager::getOrigin(eID id, std::string name) const
{
	auto it = _originList.find(id);
	if (it != _originList.end())
	{
		auto originIt = it->second.find(name);
		if (originIt != it->second.end())
		{
			return originIt->second;
		}
	}
	// Return zero vector nếu không tìm thấy
	return GVector2(0.0f, 0.0f);
}
