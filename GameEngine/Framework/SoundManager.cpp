

#include "SoundManager.h"

SoundManager* SoundManager::_instance;

SoundManager* SoundManager::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new SoundManager();
	}
	return _instance;
}

bool SoundManager::IsPlaying(eSoundId soundid)
{
	auto sound = _listSound[soundid];
	if (sound != nullptr)
	{
		return sound->IsSoundPlaying();
	}
	return false;
}

map<eSoundId, CSound*>* SoundManager::getListSound()
{
	return &_listSound;
}

void SoundManager::Play(eSoundId soundid)
{
	auto sound = _listSound[soundid];
	if (sound != nullptr)
	{
		PlaySound(sound);
		this->currentSound = sound;
	}
}
void SoundManager::PlayLoop(eSoundId soundid)
{
	auto sound = _listSound[soundid];
	if (sound != nullptr)
	{
		sound->Play(0, DSBPLAY_LOOPING);
	}
}
void SoundManager::Stop(eSoundId soundid)
{
	auto sound = _listSound[soundid];
	if (sound != nullptr)
	{
		StopSound(sound);
	}
}

SoundManager::SoundManager()
{
	currentSound = nullptr;
}

SoundManager::~SoundManager()
{
}