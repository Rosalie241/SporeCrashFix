//
// SporeCrashFix - https://github.com/Rosalie241/SporeFixCrash
//  Copyright (C) 2021 Rosalie Wanders <rosalie@mailbox.org>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3.
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <https://www.gnu.org/licenses/>.
//

// dllmain.cpp : Defines the entry point for the DLL application.
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"

//
// Helper functions
//

static void DisplayError(const char* fmt, ...)
{
	char buf[200];

	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	MessageBoxA(NULL, buf, "SporeCrashFix", MB_OK | MB_ICONERROR);
}

//
// Detoured Functions
//

// Creature stage
//

// The game seems to crash when the player dies when it has a 'buddy',
// the crash happens when arg3 is a nullptr
static_detour(CreatureStageFunction1Detour, bool(void*, void*, void*)) {
	bool detoured(void* arg1, void* arg2, void* arg3)
	{
		if (arg3 == nullptr)
		{
			return true;
		}

		return original_function(arg1, arg2, arg3);
	}
};

// Tribal stage
//

// The game seems to crash randomly in the tribal stage,
// the crash happens when arg3 is a nullptr
static_detour(TribalStageFunction1Detour, void* (void*, void*, void*, void*)) {
	void* detoured(void* arg1, void* arg2, void* arg3, void* arg4)
	{
		if (arg3 == nullptr)
		{
			return nullptr;
		}

		return original_function(arg1, arg2, arg3, arg4);
	}
};

#ifdef _DEBUG
LONG WINAPI UnhandledExceptionHandler(_EXCEPTION_POINTERS* ExceptionInfo)
{
	DisplayError("crash detected, attach a debugger now!");
	return EXCEPTION_CONTINUE_SEARCH;
}

static LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI* SetUnhandledExceptionFilter_real)(LPTOP_LEVEL_EXCEPTION_FILTER) = SetUnhandledExceptionFilter;
static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_detour(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	if ((DWORD_PTR)lpTopLevelExceptionFilter == (Address(ModAPI::ChooseAddress(0x0091eca0, 0x0091e9e0))))
	{
		return SetUnhandledExceptionFilter_real(UnhandledExceptionHandler);
	}

	return SetUnhandledExceptionFilter_real(lpTopLevelExceptionFilter);
}
#endif // _DEBUG

//
// Exported Functions
//

void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials
}

void Dispose()
{
	// This method is called when the game is closing
}

void AttachDetours()
{
	CreatureStageFunction1Detour::attach(Address(ModAPI::ChooseAddress(0x00d65140, 0x00d65be0)));
	TribalStageFunction1Detour::attach(Address(ModAPI::ChooseAddress(0x00d71c90, 0x00d72720)));


#ifdef _DEBUG
	DetourAttach(&(PVOID&)SetUnhandledExceptionFilter_real, SetUnhandledExceptionFilter_detour);
#endif // _DEBUG

	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

