#pragma once

#include <voicemeeter/VoicemeeterRemote.h>


class VoiceMeeterHelper
{
public:
	VoiceMeeterHelper();
	~VoiceMeeterHelper();

	bool Poll();
	float GetMute(int stripNumber);
	void ToggleMute(int stripNumber);


private:
	long InitializeDLLInterfaces(void);

		T_VBVMR_INTERFACE iVMR;
		HMODULE G_H_Module;
};