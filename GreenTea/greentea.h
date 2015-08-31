/*
	Name: GreenTea
	Author: Alvy Piper (alvypiper@nikolaszorko.sexy)
	Copyright: 2015
	Usage: A simple base to help people who have no clue where to start.
	Using content created by: Oubliette, Dude719, and Valve.

	Any content creator that wants their work removed can contact me at my contact email.
	
	Feel free to redistribute, modify, and share. Please give credit where it is due, though.
*/

#pragma once

#define GAME_DLL
#define CLIENT_DLL
#define WIN32_LEAN_AND_MEAN

//includes
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdio>

#include <cdll_int.h>
#include "filesystem.h"
#include <cdll_client_int.h>
#include <mathlib\mathlib.h>
#include <iefx.h>
#include <engine\IEngineSound.h>
#include <ienginevgui.h>
#include <engine\ivdebugoverlay.h>
#include <vgui\ISurface.h>
#include <iprediction.h>
#include <igameevents.h>
#include <vgui\IVGui.h>
#include <input.h>
#include <con_nprint.h>
#include <game_controls\commandmenu.h>
#include <in_buttons.h>
#include <vphysics_interface.h>
#include <ivrenderview.h>
#include <vgui/ilocalize.h>
#include <engine\IEngineTrace.h>
#include "cdll_int.h"
#include "filesystem.h"
#include <convar.h>
#include <icvar.h>
#include <tier1.h>
#include <eiface.h>
#include <usercmd.h>
#include <checksum_md5.h>
#include <vstdlib/random.h>
#include "mathlib/vector.h"
#include <icliententity.h>
#include <icliententitylist.h>
#include <materialsystem/imaterial.h>
#include <materialsystem/imaterialsystem.h>
#include <inetchannelinfo.h>
#include <cdll_int.h>
#include <cdll_client_int.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <interface.h>
#include <mathlib\mathlib.h>
#include <cbase.h>

#include "vmt.h"

#include <steam/isteamclient.h>
#include <steam/isteamfriends.h>

#pragma comment( lib, "tier0.lib" )
#pragma comment( lib, "tier1.lib" )
#pragma comment( lib, "tier2.lib" )
#pragma comment( lib, "tier3.lib" )
#pragma comment( lib, "vgui_controls.lib" )
#pragma comment( lib, "mathlib.lib" )
#pragma comment( lib, "vstdlib.lib" )

namespace GreenTea //okay, this namespace has to be placed in every file as "using namespace GreenTea" (mainly cpp files).
{
	namespace cl
	{
		extern IClientEntityList			*entitylist; //cl::entitylist
		extern IVEngineClient				*engine;
		extern IBaseClientDLL				*client;
		extern IEngineTrace					*enginetrace;
		extern IVModelInfoClient			*modelinfo;
		extern vgui::ISurface				*surface;
		extern vgui::IPanel					*panel;
	}

	namespace hook
	{
		void __fastcall painttraverse(void* thishook, void*, unsigned int vguipanel, bool forcerepaint, bool allowforce); //hook::painttraverse(arg1, arg2, arg3, arg4)
	}
}
