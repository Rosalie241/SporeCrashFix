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

static void DisplayError(const char* fmt, ...)
{
	char buf[200];

	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	MessageBoxA(NULL, buf, "SporeCrashFix", MB_OK | MB_ICONERROR);
}

static uint32_t RvaToAddress(uint32_t rva_addr)
{
	return (uint32_t)((uint32_t)(GetModuleHandle(NULL)) + rva_addr);
}

// The game seems to crash randomly when respawning in the creature stage,
// the original function can't seem to handle arg3 when it's nullptr,
// to fix the crash, return true whenever arg3 is nullptr
static_detour(GameFunctionDetour, bool(void*, void*, void*)) {
	bool detoured(void* arg1, void* arg2, void* arg3)
	{
		if (arg3 == nullptr)
		{
			return true;
		}

		return original_function(arg1, arg2, arg3);
	}
};

void Dispose()
{
	// This method is called when the game is closing
}

void AttachDetours()
{
	// RVA of function = 0x965BE0
	GameFunctionDetour::attach(Address(ModAPI::ChooseAddress(0xD65140, 0xD65BE0)));

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

