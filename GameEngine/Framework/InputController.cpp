#include "InputController.h"
#include "../Object/Player.h"

InputController *InputController::m_instance = nullptr;

InputController::InputController()
{
	m_pDirectInput = nullptr;
	m_pKeyboard = nullptr;
	ZeroMemory(m_keyBuffer, 256);
}

InputController::~InputController()
{
	if (m_pDirectInput != nullptr)
		this->m_pDirectInput->Release();
	if (m_pKeyboard != nullptr)
		this->m_pKeyboard->Release();
}

InputController *InputController::getInstance()
{
	if (m_instance == nullptr)
		m_instance = new InputController();
	return m_instance;
}

void InputController::release()
{
}

bool InputController::init(HWND hWnd, HINSTANCE hInstance)
{
	this->m_hWnd = hWnd;
	HRESULT l_rs = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void **>(&m_pDirectInput),
		nullptr);
	if (l_rs != DI_OK)
		return false;

	l_rs = m_pDirectInput->CreateDevice(GUID_SysKeyboard, reinterpret_cast<LPDIRECTINPUTDEVICEW *>(&m_pKeyboard), nullptr);
	if (l_rs != DI_OK)
		return false;

	l_rs = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (l_rs != DI_OK)
		return false;

	l_rs = m_pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (l_rs != DI_OK)
		return false;

	// Set Property for keyboard buffer.
	DIPROPDWORD l_dipdw;

	l_dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	l_dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	l_dipdw.diph.dwHow = DIPH_DEVICE;
	l_dipdw.diph.dwObj = 0;
	l_dipdw.dwData = KEYBOARD_BUFFER_SIZE;

	l_rs = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &l_dipdw.diph);
	if (l_rs != DI_OK)
		return false;

	l_rs = m_pKeyboard->Acquire();
	if (l_rs != DI_OK)
		return false;

	return true;
}

void InputController::update()
{
	for (int i = 0; i < 256; i++)
	{
		m_previousKeyBuffer[i] = ((m_keyBuffer[i] & 0x80) > 0);
	}

	// collect state of all of keys.
	m_pKeyboard->GetDeviceState(sizeof(this->m_keyBuffer), m_keyBuffer);
	if (isKeyDown(DIK_ESCAPE))
	{
		PostMessage(m_hWnd, WM_QUIT, 0, 0);
	}

	DWORD l_dw = KEYBOARD_BUFFER_SIZE;
	HRESULT l_rs = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_keyEvents, &l_dw, 0);

	for (DWORD i = 0; i < l_dw; i++) // not use for each statement.
	{
		int l_nKeycode = m_keyEvents[i].dwOfs;
		int l_nKeyState = m_keyEvents[i].dwData;
		if ((l_nKeyState & 0x80) > 0)
		{
		}
		else
		{
		}
	}
}

int InputController::isKeyDown(int keycode)
{
	return ((m_keyBuffer[keycode] & 0x80) > 0);
}

int InputController::isKeyUp(int nKeyCode)
{
	return !isKeyDown(nKeyCode);
}

bool InputController::isKeyPressed(int keycode)
{
	// Don't use
	return isKeyDown(keycode) && !(m_previousKeyBuffer[keycode]);
}

bool InputController::isKeyRelease(int keycode)
{
	// Don't use
	return !isKeyDown(keycode) && (m_previousKeyBuffer[keycode]);
}