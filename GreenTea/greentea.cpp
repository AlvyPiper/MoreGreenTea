/*
	Name: GreenTea
	Author: Alvy Piper <alvycat@protonmail.com>
	Copyright: 2015
	Usage: A simple base to help people who have no clue where to start.
	Using content created by: Oubliette, Dude719, and Valve.
	
	Any content creator that wants their work removed can contact me at my contact email.
	
	Feel free to redistribute, modify, and share. Please give credit where it is due, though.
*/

#include "greentea.h"

using namespace GreenTea;

IClientEntityList			*cl::entitylist		= 0; //we extern'd our interfaces, so we have to actually call them here and setup interfaces
IVEngineClient				*cl::engine		= 0;
IBaseClientDLL				*cl::client		= 0;
IEngineTrace				*cl::enginetrace	= 0;
IVModelInfoClient			*cl::modelinfo		= 0;	
vgui::ISurface				*cl::surface		= 0;
vgui::IPanel				*cl::panel		= 0;

typedef void(__thiscall *PaintTraverseFn)(void*, unsigned int, bool, bool); //create a typedef for the original paint traverse
PaintTraverseFn org_painttraverse = 0; //original call
void __fastcall hook::painttraverse(void* thishook, void*, unsigned int vguipanel, bool forcerepaint, bool allowforce) //pass ECX and EDX registers to the first 2 arguments. you could use the stdcall calling convention for this, if you wanted.
{
	org_painttraverse(thishook, vguipanel, forcerepaint, allowforce); //call the original every frame, we would get a blackscreen otherwise

	const char* panel = cl::panel->GetName(vguipanel); //gets the name of panels

	if (!strcmp(panel, "FocusOverlayPanel")) //compares the name to "FocusOverlayPanel" because that is the panel we draw on, it's rather slow but still does the job.
	{
		Msg("We're drawing on FocusOverlayPanel.\n"); //print a message if we're drawing on the right panel, which we are.
	}
}

void greenteabegin()
{
	while (FindWindowA("Valve001", 0) == 0) //sleep if the game is not open
		Sleep(100);

	while (GetModuleHandleA("engine.dll") == 0 || GetModuleHandleA("client.dll") == 0) //...or if the game hasn't initalized fully
		Sleep(100);

	CreateInterfaceFn vgui2interface = (CreateInterfaceFn) GetProcAddress(GetModuleHandleA("vgui2.dll"), "CreateInterface");
	CreateInterfaceFn vguimatsurfaceinterface = (CreateInterfaceFn) GetProcAddress(GetModuleHandleA("vguimatsurface.dll"), "CreateInterface");
	CreateInterfaceFn engineinterface = (CreateInterfaceFn) GetProcAddress(GetModuleHandleA("engine.dll"), "CreateInterface");
	CreateInterfaceFn clientinterface = (CreateInterfaceFn) GetProcAddress(GetModuleHandleA("client.dll"), "CreateInterface");

	cl::enginetrace = (IEngineTrace*) engineinterface("EngineTraceClient004", 0); //interface versions can be found in IDA or OllyDbg using string search. pop open client.dll and search for the interface.
	cl::panel = (IPanel*) vgui2interface("VGUI_Panel009", 0);
	cl::surface = (ISurface*) vguimatsurfaceinterface("VGUI_Surface031", 0);
	cl::engine = (IVEngineClient*) engineinterface("VEngineClient013", 0);
	cl::client = (IBaseClientDLL*) clientinterface("VClient017", 0);
	cl::entitylist = (IClientEntityList*) clientinterface("VClientEntityList003", 0);
	cl::modelinfo = (IVModelInfoClient*) engineinterface("VModelInfoClient004", 0);

	Msg("enginetrace is 0x%x\npanel is 0x%x\nsurface is 0x%x\nengine is 0x%x\nclient is 0x%x\nentitylist is 0x%x\nmodelinfo is 0x%x\n", cl::enginetrace, cl::panel, cl::surface, cl::engine, cl::client, cl::entitylist, cl::modelinfo);

	// ^ this will print all the interfaces into the console, 0x0 would be NULL (and probably result in a game crash)

	CVMTHook *hk_painttraverse;

	hk_painttraverse = new CVMTHook((DWORD**)cl::panel); //creates a copy of the given table and overwrites the base pointer(?)
	org_painttraverse = (PaintTraverseFn) hk_painttraverse->HookMethod((DWORD) hook::painttraverse, 41); //index 41 is paint traverse
}

bool DllMain(HINSTANCE inst, DWORD dwreason, LPVOID lpreserved) 
{
	if (dwreason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE) inst);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE) greenteabegin, 0, 0, 0); //creates a thread which does all of the above.
	}
	TerminateThread(greenteabegin, 0); //kills thread once it's done

	return 1;
}
