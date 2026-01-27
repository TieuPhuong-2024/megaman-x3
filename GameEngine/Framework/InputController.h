#ifndef __INPUT_CTRL_H__
#define __INPUT_CTRL_H__

#pragma warning(disable : 4467)

#include "define.h"
#include "Graphics.h"
#include "Event.h"
#include <vector>
#include "define.h"
#include <list>

#define KEYBOARD_BUFFER_SIZE 1024

class InputController
{
public:
	~InputController();

	static InputController *getInstance();
	static void release();

	bool init(HWND, HINSTANCE); // should be called in game::init()
	void update();				// should be called in main game loop
	int isKeyDown(int keycode); // check if keycode is down.
	int isKeyUp(int nKeyCode);
	bool isKeyPressed(int keycode);
	bool isKeyRelease(int keycode);

private:
	InputController();
	static InputController *m_instance;

	LPDIRECTINPUT m_pDirectInput;
	LPDIRECTINPUTDEVICE8 m_pKeyboard;
	BYTE m_keyBuffer[256]; // BYTE = unsigned char
	HWND m_hWnd;		   // reference to another place. Don't release!!!
	DIDEVICEOBJECTDATA m_keyEvents[KEYBOARD_BUFFER_SIZE];

	bool m_previousKeyBuffer[256];
};

#endif // !__INPUT_CTRL_H__