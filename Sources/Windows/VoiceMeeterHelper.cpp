
#include <stdio.h>

#include "pch.h"
#include <iostream>


#include "VoiceMeeterHelper.h"


/*******************************************************************************/
/**                           GET VOICEMEETER DIRECTORY                       **/
/*******************************************************************************/

static wchar_t  uninstDirKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define INSTALLER_UNINST_KEY	L"VB:Voicemeeter {17359A74-1236-5467}"


void RemoveNameInPath(wchar_t * szPath)
{
	long ll;
	ll = (long)wcslen(szPath);
	while ((ll > 0) && (szPath[ll] != L'\\')) ll--;
	if (szPath[ll] == L'\\') szPath[ll] = 0;
}

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

static BOOL __cdecl RegistryGetVoicemeeterFolder(wchar_t * szDir)
{
	wchar_t  szKey[256];
	wchar_t  sss[1024];
	DWORD nnsize = 1024;
	HKEY hkResult;
	LONG rep;
	DWORD pptype = REG_SZ;
	sss[0] = 0;

	// build Voicemeeter uninstallation key
	wcscpy_s(szKey, 256, uninstDirKey);
	wcscat_s(szKey, 256, L"\\");
	wcscat_s(szKey, 256, INSTALLER_UNINST_KEY);

	// open key
	rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkResult);
	if (rep != ERROR_SUCCESS)
	{
		// if not present we consider running in 64bit mode and force to read 32bit registry
		rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ | KEY_WOW64_32KEY, &hkResult);
	}
	if (rep != ERROR_SUCCESS) return FALSE;
	// read uninstall profram path
	rep = RegQueryValueEx(hkResult, L"UninstallString", 0, &pptype, (unsigned char *)sss, &nnsize);
	RegCloseKey(hkResult);

	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	// remove name to get the path only
	RemoveNameInPath(sss);
	if (nnsize > 512) nnsize = 512;
	wcsncpy_s(szDir, 1024, sss, nnsize);

	return TRUE;
}


long VoiceMeeterHelper::InitializeDLLInterfaces(void)
{
	wchar_t szDllName[1024];
	memset(&this->iVMR, 0, sizeof(T_VBVMR_INTERFACE));

	//get folder where is installed Voicemeeter
	if (RegistryGetVoicemeeterFolder(szDllName) == FALSE)
	{
		// voicemeeter not installed
		return -100;
	}
	//use right dll according O/S type
	if (sizeof(void*) == 8) wcscat_s(szDllName, 1024, L"\\VoicemeeterRemote64.dll");
	else wcscat_s(szDllName, 1024, L"\\VoicemeeterRemote.dll");

	// Load Dll
	G_H_Module = LoadLibrary(szDllName);
	if (G_H_Module == NULL) return -101;

	// Get function pointers
	iVMR.VBVMR_Login = (T_VBVMR_Login)GetProcAddress(G_H_Module, "VBVMR_Login");
	iVMR.VBVMR_Logout = (T_VBVMR_Logout)GetProcAddress(G_H_Module, "VBVMR_Logout");
	iVMR.VBVMR_RunVoicemeeter = (T_VBVMR_RunVoicemeeter)GetProcAddress(G_H_Module, "VBVMR_RunVoicemeeter");
	iVMR.VBVMR_GetVoicemeeterType = (T_VBVMR_GetVoicemeeterType)GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterType");
	iVMR.VBVMR_GetVoicemeeterVersion = (T_VBVMR_GetVoicemeeterVersion)GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterVersion");

	iVMR.VBVMR_IsParametersDirty = (T_VBVMR_IsParametersDirty)GetProcAddress(G_H_Module, "VBVMR_IsParametersDirty");
	iVMR.VBVMR_GetParameterFloat = (T_VBVMR_GetParameterFloat)GetProcAddress(G_H_Module, "VBVMR_GetParameterFloat");
	iVMR.VBVMR_GetParameterStringA = (T_VBVMR_GetParameterStringA)GetProcAddress(G_H_Module, "VBVMR_GetParameterStringA");
	iVMR.VBVMR_GetParameterStringW = (T_VBVMR_GetParameterStringW)GetProcAddress(G_H_Module, "VBVMR_GetParameterStringW");
	iVMR.VBVMR_GetLevel = (T_VBVMR_GetLevel)GetProcAddress(G_H_Module, "VBVMR_GetLevel");
	iVMR.VBVMR_GetMidiMessage = (T_VBVMR_GetMidiMessage)GetProcAddress(G_H_Module, "VBVMR_GetMidiMessage");

	iVMR.VBVMR_SetParameterFloat = (T_VBVMR_SetParameterFloat)GetProcAddress(G_H_Module, "VBVMR_SetParameterFloat");
	iVMR.VBVMR_SetParameters = (T_VBVMR_SetParameters)GetProcAddress(G_H_Module, "VBVMR_SetParameters");
	iVMR.VBVMR_SetParametersW = (T_VBVMR_SetParametersW)GetProcAddress(G_H_Module, "VBVMR_SetParametersW");
	iVMR.VBVMR_SetParameterStringA = (T_VBVMR_SetParameterStringA)GetProcAddress(G_H_Module, "VBVMR_SetParameterStringA");
	iVMR.VBVMR_SetParameterStringW = (T_VBVMR_SetParameterStringW)GetProcAddress(G_H_Module, "VBVMR_SetParameterStringW");

	iVMR.VBVMR_Output_GetDeviceNumber = (T_VBVMR_Output_GetDeviceNumber)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceNumber");
	iVMR.VBVMR_Output_GetDeviceDescA = (T_VBVMR_Output_GetDeviceDescA)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescA");
	iVMR.VBVMR_Output_GetDeviceDescW = (T_VBVMR_Output_GetDeviceDescW)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescW");
	iVMR.VBVMR_Input_GetDeviceNumber = (T_VBVMR_Input_GetDeviceNumber)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceNumber");
	iVMR.VBVMR_Input_GetDeviceDescA = (T_VBVMR_Input_GetDeviceDescA)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescA");
	iVMR.VBVMR_Input_GetDeviceDescW = (T_VBVMR_Input_GetDeviceDescW)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescW");

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	iVMR.VBVMR_AudioCallbackRegister = (T_VBVMR_AudioCallbackRegister)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackRegister");
	iVMR.VBVMR_AudioCallbackStart = (T_VBVMR_AudioCallbackStart)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStart");
	iVMR.VBVMR_AudioCallbackStop = (T_VBVMR_AudioCallbackStop)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStop");
	iVMR.VBVMR_AudioCallbackUnregister = (T_VBVMR_AudioCallbackUnregister)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackUnregister");
#endif
	// check pointers are valid
	if (iVMR.VBVMR_Login == NULL) return -1;
	if (iVMR.VBVMR_Logout == NULL) return -2;
	if (iVMR.VBVMR_RunVoicemeeter == NULL) return -2;
	if (iVMR.VBVMR_GetVoicemeeterType == NULL) return -3;
	if (iVMR.VBVMR_GetVoicemeeterVersion == NULL) return -4;
	if (iVMR.VBVMR_IsParametersDirty == NULL) return -5;
	if (iVMR.VBVMR_GetParameterFloat == NULL) return -6;
	if (iVMR.VBVMR_GetParameterStringA == NULL) return -7;
	if (iVMR.VBVMR_GetParameterStringW == NULL) return -8;
	if (iVMR.VBVMR_GetLevel == NULL) return -9;
	if (iVMR.VBVMR_SetParameterFloat == NULL) return -10;
	if (iVMR.VBVMR_SetParameters == NULL) return -11;
	if (iVMR.VBVMR_SetParametersW == NULL) return -12;
	if (iVMR.VBVMR_SetParameterStringA == NULL) return -13;
	if (iVMR.VBVMR_SetParameterStringW == NULL) return -14;
	if (iVMR.VBVMR_GetMidiMessage == NULL) return -15;

	if (iVMR.VBVMR_Output_GetDeviceNumber == NULL) return -30;
	if (iVMR.VBVMR_Output_GetDeviceDescA == NULL) return -31;
	if (iVMR.VBVMR_Output_GetDeviceDescW == NULL) return -32;
	if (iVMR.VBVMR_Input_GetDeviceNumber == NULL) return -33;
	if (iVMR.VBVMR_Input_GetDeviceDescA == NULL) return -34;
	if (iVMR.VBVMR_Input_GetDeviceDescW == NULL) return -35;

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	if (iVMR.VBVMR_AudioCallbackRegister == NULL) return -40;
	if (iVMR.VBVMR_AudioCallbackStart == NULL) return -41;
	if (iVMR.VBVMR_AudioCallbackStop == NULL) return -42;
	if (iVMR.VBVMR_AudioCallbackUnregister == NULL) return -43;
#endif
	return 0;
}


VoiceMeeterHelper::VoiceMeeterHelper() {
	if (this->InitializeDLLInterfaces() < 0) {
		throw "VoiceMeeterNotInstalled";
	}
	auto val = iVMR.VBVMR_Login();
	std::wcout << val;
}

VoiceMeeterHelper::~VoiceMeeterHelper() {
	if (iVMR.VBVMR_Logout != NULL) iVMR.VBVMR_Logout();
	if (iVMR.VBVMR_AudioCallbackUnregister != NULL) iVMR.VBVMR_AudioCallbackUnregister();
}

bool VoiceMeeterHelper::Poll()
{
	return iVMR.VBVMR_IsParametersDirty() != 0;
}

float VoiceMeeterHelper::GetMute(int stripNumber)
{
	float isMuted;
	char bla[30];
	sprintf_s(bla, 30, "Strip[%d].Mute", stripNumber);
	auto val = iVMR.VBVMR_GetParameterFloat(bla, &isMuted);
	std::wcout << val;
	return isMuted;
}

void VoiceMeeterHelper::ToggleMute(int stripNumber)
{
	auto oldValue = GetMute(stripNumber);
	char bla[30];
	sprintf_s(bla, 30, "Strip[%d].Mute", stripNumber);
	iVMR.VBVMR_SetParameterFloat(bla, 1.0f - oldValue);
}
