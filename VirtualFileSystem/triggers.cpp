#include "triggers.h"
#include "stdafx.h"

#include <iostream>

AState AudioState;
GState GameState;

PDWORD ProcessTriggers(UINT trigger) {
	int num = rand();
	switch (trigger) {
		case TRIGGER_SUCCESS:
		{
			int sid = (num % 4);
			int id = (4 + sid);
			OggData *data = &AudioState.OggDataB[id];
			AudioState.Delay = (data->Size * 6) / 1000;
			tcout << "Playing Success Sound [" << id << "," << AudioState.Delay << "]\n";
			return (PDWORD)data;
		}
		case TRIGGER_FAILURE:
		{
			int sid = (num % 4);
			int id = (8 + sid);
			OggData *data = &AudioState.OggDataB[id];
			AudioState.Delay = (data->Size * 6) / 1000;
			tcout << "Playing Failure Sound [" << id << "," << AudioState.Delay << "]\n";
			return (PDWORD)data;
		}
		case TRIGGER_DIALOG:
		{
			if (AudioState.Delay == 0) {

				int sid = (num % 16 < 8)
					? 2 + (num % 4)
					: 0 + (num % 2);

				int id = (6 * GameState.Option) + sid;
				OggData *data = &AudioState.OggDataA[id];
				AudioState.Delay = (data->Size * 6) / 1000;
				tcout << "Playing " << ((sid >= 2) ? "Dialogue" : "Breathing") << " Sound [" << id << "," << AudioState.Delay << "]\n";
				return (PDWORD)data;
			}
			else {
				--AudioState.Delay;
				return nullptr;
			}
		}
		default:
			return nullptr;
	}
}
