#include "SpriteManager.h"

#include <string>
#include "pugixml.hpp"

SpriteManager* SpriteManager::s_instance = nullptr;

SpriteManager::SpriteManager(void)
{
	// do nothing.
}

SpriteManager::~SpriteManager(void)
{
	for (auto spr = _listSprite.begin(); spr != _listSprite.end(); ++spr)
	{
		spr->second->release(); // release image
		delete spr->second; // delete sprite
	}
	if (_listSprite.empty() == false)
		_listSprite.clear(); // remove all from MAP
}

Sprite* SpriteManager::loadXMLDoc(LPD3DXSPRITE spritehandle, LPCWSTR path)
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

	return new Sprite(spritehandle, (LPWSTR) strpath.c_str(), tilecount, columns);
}

Sprite* SpriteManager::getSprite(eID id)
{
	Sprite* it = this->_listSprite.find(id)->second;
	return new Sprite(*it); // get the copy version of Sprite
}

RECT SpriteManager::getSourceRect(eID id, std::string name)
{
	return _sourceRectList[id][name];
}

GVector2 SpriteManager::getOrigin(eID id, std::string name)
{
	return _originList[id][name];
}

void SpriteManager::loadSpriteInfo(eID id, const char* fileInfoPath)
{
	FILE* file;
	fopen_s(&file, fileInfoPath, "r");

	if (file)
	{
		while (!feof(file))
		{
			RECT rect;
			char name[100];
			fgets(name, 100, file);

			fscanf_s(file, "%s %d %d %d %d", name, 100, &rect.left, &rect.top, &rect.right, &rect.bottom);

			_sourceRectList[id][std::string(name)] = rect;
		}
		fclose(file);
	}
}

void SpriteManager::releaseSprite(eID id)
{
	Sprite* it = this->_listSprite.find(id)->second;
	delete it; // delete the sprite only, don't release image
	this->_listSprite.erase(id); // erase functions only remove the pointer from MAP, don't delete it.
}

void SpriteManager::releaseTexture(eID id)
{
	Sprite* spr = this->_listSprite.find(id)->second;
	spr->release(); // release image
	delete spr;
	this->_listSprite.erase(id); // erase functions only remove the pointer from MAP, don't delete it.
}

SpriteManager* SpriteManager::getInstance()
{
	if (s_instance == nullptr)
		s_instance = new SpriteManager();
	return s_instance;
}

void SpriteManager::release()
{
	delete s_instance; // _instance is static attribute, only static function can delete it.
	s_instance = nullptr;
}

std::map<eID, Sprite*>* SpriteManager::getListSprite()
{
	return &_listSprite;
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
