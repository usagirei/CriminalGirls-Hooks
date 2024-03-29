#pragma once

#include <inttypes.h>
#include "stdafx.h"

#define TRIGGER_SUCCESS            0xFFFFFFFF
#define TRIGGER_DIALOG             0xFFFFFFFE
#define TRIGGER_FAILURE            0xFFFFFFFD
#define TRIGGER_SMOKEVISIBLE       0xFFFFFFFC
#define TRIGGER_MAINLOOP           0xFFFFFFFB

struct OggData {
	uint8_t* Data;
	uint32_t Size;
};

struct GState{
	uint8_t Option;
	uint8_t MaxTries;
	uint8_t Tries;
	uint8_t MiniGame;
	uint8_t Level;
	char Girl[4] = { 0 };
	struct {
		char Girl[4] = { 0 };
		bool Status;
	} KnightStatus[9];
};

struct MState {
	bool SmokeVisible = true;
};

struct AState {
	OggData* OggDataA = nullptr;
	OggData* OggDataB = nullptr;

	uint32_t NumOggFilesA = 0;
	uint32_t NumOggFilesB = 0;

	char* TpkDataA = nullptr;
	char* TpkDataB = nullptr;

	char TpkDataAName[64];
	char TpkDataBName[64];

	uint32_t Delay = 0;
};

extern GState GameState;
extern AState AudioState;
extern MState ModState;

PDWORD ProcessTriggers(UINT trigger);