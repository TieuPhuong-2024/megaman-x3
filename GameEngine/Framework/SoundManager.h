

#ifndef __SOUND_MANAGER__
#define __SOUND_MANAGER__
#include "define.h"
#include "dxaudio.h"

#include <map>
#include <wincon.h>



class SoundManager
{
public:
	~SoundManager();
	static SoundManager* getInstance();
	void Play(eSoundId);
	void Stop(eSoundId);
	void PlayLoop(eSoundId);
	bool IsPlaying(eSoundId);
	std::map<eSoundId, CSound*>* getListSound();

private:
	SoundManager();
	static SoundManager* _instance;
	std::map<eSoundId, CSound*> _listSound;
	CSound* currentSound;
	float volume;
};

#endif // !__SOUND_MANAGER__
