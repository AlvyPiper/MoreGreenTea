//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( USERCMD_H )
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "utlvector.h"
#include "imovehelper.h"
#include "checksum_crc.h"


class bf_read;
class bf_write;

class CEntityGroundContact
{
public:
	int					entindex;
	float				minheight;
	float				maxheight;
};

#ifndef LOG_USERCMD
#define LOG_USERCMD
#endif

#define SHOUTMEMBERUSERCMD(memberName, member) if (typeid(member) == typeid(float)) { LOG_USERCMD("0x%X | %s = %f (size 0x%X)", offsetof(class CUserCmd,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class CUserCmd,  member), memberName, member, sizeof(member)); } else { LOG_USERCMD("0x%X | %s = %d (size 0x%X)", offsetof(class CUserCmd,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class CUserCmd,  member), memberName, member, sizeof(member)); }

class CUserCmd
{
public:
	CUserCmd() { Reset(); }

	virtual ~CUserCmd() { };

	void Reset()
	{
		command_number = 0;
		tick_count = 0;
		viewangles.Init();
		aimdirection.Init();
		forwardmove = 0.0f;
		sidemove = 0.0f;
		upmove = 0.0f;
		buttons = 0;
		impulse = 0;
		weaponselect = 0;
		weaponsubtype = 0;
		random_seed = 0;
		mousedx = 0;
		mousedy = 0;
		hasbeenpredicted = false;
	}

	CUserCmd& operator =(const CUserCmd& src)
	{
		if (this == &src) return *this;
		command_number = src.command_number;
		tick_count = src.tick_count;
		viewangles = src.viewangles;
		aimdirection = src.aimdirection;
		forwardmove = src.forwardmove;
		sidemove = src.sidemove;
		upmove = src.upmove;
		buttons = src.buttons;
		impulse = src.impulse;
		weaponselect = src.weaponselect;
		weaponsubtype = src.weaponsubtype;
		random_seed = src.random_seed;
		mousedx = src.mousedx;
		mousedy = src.mousedy;
		hasbeenpredicted = src.hasbeenpredicted;

		return *this;
	}

	CUserCmd(const CUserCmd& src) { *this = src; }

	CRC32_t GetChecksum(void) const
	{
		CRC32_t crc;
		CRC32_Init(&crc);
		
		CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
		CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
		CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
		CRC32_ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
		CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
		CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
		CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
		CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
		CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
		CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
		CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
		CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
		CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
		CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));

		CRC32_Final( &crc );
		return crc;
	}

	int		command_number;		// 0x04 For matching server and client commands for debugging
	int		tick_count;			// 0x08 the tick the client created this command
	QAngle	viewangles;			// 0x0C Player instantaneous view angles.
	Vector	aimdirection;		// 0x18
	float	forwardmove;		// 0x24
	float	sidemove;			// 0x28
	float	upmove;				// 0x2C
	int		buttons;			// 0x30 Attack button states
	byte    impulse;			// 0x34
	int		weaponselect;		// 0x38 Current weapon id
	int		weaponsubtype;		// 0x3C
	int		random_seed;		// 0x40 For shared random functions
	short	mousedx;			// 0x44 mouse accum in x from create move
	short	mousedy;			// 0x46 mouse accum in y from create move
	bool	hasbeenpredicted;	// 0x48 Client only, tracks whether we've predicted this command at least once
	char	pad_0x4C[0x18];		// 0x4C Current sizeof( usercmd ) =  100  = 0x64

	void dump()
	{
		printf("CUserCmd -> Address = 0x%X\n", (DWORD)this);
		LOG_USERCMD("CUserCmd -> Address = 0x%X", (DWORD)this);
		SHOUTMEMBERUSERCMD("command_number", command_number);		
		SHOUTMEMBERUSERCMD("tick_count"	, tick_count);			
		SHOUTMEMBERUSERCMD("viewangles"	, viewangles);			
		SHOUTMEMBERUSERCMD("aimdirection", aimdirection);		
		SHOUTMEMBERUSERCMD("forwardmove", forwardmove);		
		SHOUTMEMBERUSERCMD("sidemove", sidemove);			
		SHOUTMEMBERUSERCMD("upmove"	, upmove);				
		SHOUTMEMBERUSERCMD("buttons", buttons);			
		SHOUTMEMBERUSERCMD("impulse", impulse);			
		SHOUTMEMBERUSERCMD("weaponselect", weaponselect);		
		SHOUTMEMBERUSERCMD("weaponsubtype", weaponsubtype);		
		SHOUTMEMBERUSERCMD("random_seed", random_seed);		
		SHOUTMEMBERUSERCMD("mousedx", mousedx);			
		SHOUTMEMBERUSERCMD("mousedy", mousedy);			
		SHOUTMEMBERUSERCMD("hasbeenpredicted", hasbeenpredicted);	
		SHOUTMEMBERUSERCMD("pad_0x4C[0x18]", pad_0x4C);		
		printf("\n");
	}

};

void ReadUsercmd(bf_read *buf, CUserCmd *move, CUserCmd *from);
void WriteUsercmd(bf_write *buf, const CUserCmd *to, const CUserCmd *from);

#endif // USERCMD_H
