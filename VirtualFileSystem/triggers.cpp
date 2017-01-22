#include "triggers.h"
#include "stdafx.h"

#include <iostream>
#include "vk.h"

AState AudioState;
GState GameState;
MState ModState;

HWND hWnd = 0;
HHOOK hHook;
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

bool ActiveWindow = false;

void FirstFrameSetup();
void GameLoop();

PDWORD FetchSuccessAudio();
PDWORD FetchFailureAudio();
PDWORD FetchBreathingAudio();
void OnKeyUp(int vk, bool shift, bool ctrl, bool alt);


PDWORD ProcessTriggers(UINT trigger) {
	switch (trigger) {
		case TRIGGER_SUCCESS:
		{
			return FetchSuccessAudio();
		}
		case TRIGGER_FAILURE:
		{
			return FetchFailureAudio();
		}
		case TRIGGER_DIALOG:
		{
			if (AudioState.Delay == 0) {
				return FetchBreathingAudio();
			}
			else {
				--AudioState.Delay;
				return nullptr;
			}
		}
		case TRIGGER_MAINLOOP: {
			if (hWnd == 0) {
				EnumWindows([](HWND wnd, LPARAM pPid) {
					DWORD wPid;
					GetWindowThreadProcessId(wnd, &wPid);
					if (wPid == pPid) {
						hWnd = wnd;
						return FALSE;
					}
					return TRUE;
				}, GetCurrentProcessId());
				if (hWnd != 0) {
					FirstFrameSetup();
				}
			}
			else {
				GameLoop();
			}
			return nullptr;
		}
		case TRIGGER_SMOKEVISIBLE: {
			return (PDWORD)ModState.SmokeVisible;
		}
		default:
			return nullptr;
	}
}

void GameLoop() {
	HWND active = GetActiveWindow();
	ActiveWindow = (hWnd == active);
}

void FirstFrameSetup() {
	// Keyboard Hook
#ifndef _DEBUG
	if (!(hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0)))
		tcout << "Failed to Hook Keyboard. Hotkeys will not function.";
#endif // !DEBUG

	// Enable Blindfold/EyeMask
	_DR1(GetModuleHandle(0), 0x1C96E4) = 1;
}

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (ActiveWindow) {
		KBDLLHOOKSTRUCT keyboard = *(KBDLLHOOKSTRUCT*)lParam;
		bool ctrl = GetKeyState(VK_CONTROL) & VK_BIT_DOWN;
		bool shift = GetKeyState(VK_SHIFT) & VK_BIT_DOWN;
		bool alt = keyboard.flags & LLKHF_ALTDOWN;
		switch (keyboard.vkCode)
		{
			case VK_LCONTROL:
			case VK_RCONTROL:
			case VK_CONTROL:
			case VK_LSHIFT:
			case VK_RSHIFT:
			case VK_SHIFT:
				return CallNextHookEx(nullptr, nCode, wParam, lParam);
		}
		switch (wParam)
		{
			case WM_KEYDOWN:
				break;
			case WM_KEYUP: {
				OnKeyUp(keyboard.vkCode, shift, ctrl, alt);
				break;
			}
			case WM_SYSKEYDOWN:
				break;
			case WM_SYSKEYUP:
				break;
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


PDWORD FetchSuccessAudio() {
	int num = rand();
	int sid = (num % 4);
	int id = (4 + sid);

	OggData *data = &AudioState.OggDataB[id];
	AudioState.Delay = (data->Size * 6) / 1000;
	tcout << "Playing Success Sound [" << id << "," << AudioState.Delay << "]\n";
	return (PDWORD)data;
}

PDWORD FetchFailureAudio() {
	int num = rand();
	int sid = (num % 4);
	int id = (8 + sid);

	OggData *data = &AudioState.OggDataB[id];
	AudioState.Delay = (data->Size * 6) / 1000;
	tcout << "Playing Failure Sound [" << id << "," << AudioState.Delay << "]\n";
	return (PDWORD)data;
}

PDWORD FetchBreathingAudio() {
	int num = rand();
	// Offset = 6 * Option (None,Gag,Blindfold) 
	// 2x Breathing
	// 4x Dialogue Pre
	bool breathe = num % 16 < 12;
	int sid = breathe
		? 0 + num % 2
		: 2 + num % 4;

	int id = (6 * GameState.Option) + sid;

	OggData *data = &AudioState.OggDataA[id];
	AudioState.Delay = (data->Size * 6) / 1000;
	tcout << "Playing " << ((sid >= 2) ? "Dialogue" : "Breathing") << " Sound [" << id << "," << AudioState.Delay << "]\n";
	return (PDWORD)data;
}

void OnKeyUp(int keycode, bool shift, bool ctrl, bool alt)
{
	if (ctrl && keycode == VK_KEY_S) {
		ModState.SmokeVisible = !ModState.SmokeVisible;
		tcout << "Smoke State: " << ModState.SmokeVisible << "\n";
	}
	if (ctrl && keycode == VK_KEY_B) {
		HMODULE base = GetModuleHandle(0);
		if (_DR1(base, 0x1C9554)) {
			_DR5(base, 0x1C9554, 0x1C, 0x54C, 0x5E8, 0x1E7C) = 2;
			tcout << "Blindfold Mode Set";
		}
	}
}
