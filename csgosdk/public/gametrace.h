//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef GAMETRACE_H
#define GAMETRACE_H
#ifdef _WIN32
#pragma once
#endif


#include "cmodel.h"
#include "utlvector.h"
#include "ihandleentity.h"
#include "ispatialpartition.h"

#if defined( CLIENT_DLL )
class C_BaseEntity;
#else
class CBaseEntity;
#endif


//-----------------------------------------------------------------------------
// Purpose: A trace is returned when a box is swept through the world
// NOTE: eventually more of this class should be moved up into the base class!!
//-----------------------------------------------------------------------------
class CGameTrace : public CBaseTrace
{
public:

	// Returns true if hEnt points at the world entity.
	// If this returns true, then you can't use GetHitBoxIndex().
	bool DidHitWorld() const;

	// Returns true if we hit something and it wasn't the world.
	bool DidHitNonWorldEntity() const;

	// Gets the entity's network index if the trace has hit an entity.
	// If not, returns -1.
	int GetEntityIndex() const;

	// Returns true if there was any kind of impact at all
	bool DidHit() const;
	bool IsVisible() const;
	// The engine doesn't know what a CBaseEntity is, so it has a backdoor to 
	// let it get at the edict.
#if defined( ENGINE_DLL )
	void SetEdict( edict_t *pEdict );
	edict_t* GetEdict() const;
#endif	

public:

	float			fractionleftsolid;	// time we left a solid, only valid if we started in solid
	csurface_t		surface;			// surface hit (impact surface)
	int				hitgroup;			// 0 == generic, non-zero is specific body part
	short			physicsbone;		// physics bone hit by trace in studio
	unsigned short	worldSurfaceIndex;	// Index of the msurface2_t, if applicable
#if defined( CLIENT_DLL )
	C_BaseEntity *m_pEnt;
#else
	CBaseEntity *m_pEnt;
#endif
	// NOTE: this member is overloaded.
	// If hEnt points at the world entity, then this is the static prop index.
	// Otherwise, this is the hitbox index.
	int			hitbox;					// box hit by trace in studio

	CGameTrace() {}

	//private:
	// No copy constructors allowed
	CGameTrace(const CGameTrace& other) :
		fractionleftsolid(other.fractionleftsolid),
		surface(other.surface),
		hitgroup(other.hitgroup),
		physicsbone(other.physicsbone),
		worldSurfaceIndex(other.worldSurfaceIndex),
		m_pEnt(other.m_pEnt),
		hitbox(other.hitbox)
	{
		startpos = other.startpos;
		endpos = other.endpos;
		plane = other.plane;
		fraction = other.fraction;
		contents = other.contents;
		dispFlags = other.dispFlags;
		allsolid = other.allsolid;
		startsolid = other.startsolid;
	}

	CGameTrace& CGameTrace::operator=(const CGameTrace& other)
	{
		startpos = other.startpos;
		endpos = other.endpos;
		plane = other.plane;
		fraction = other.fraction;
		contents = other.contents;
		dispFlags = other.dispFlags;
		allsolid = other.allsolid;
		startsolid = other.startsolid;
		fractionleftsolid = other.fractionleftsolid;
		surface = other.surface;
		hitgroup = other.hitgroup;
		physicsbone = other.physicsbone;
		worldSurfaceIndex = other.worldSurfaceIndex;
		m_pEnt = other.m_pEnt;
		hitbox = other.hitbox;
		return *this;
	}
};


//-----------------------------------------------------------------------------
// Returns true if there was any kind of impact at all
//-----------------------------------------------------------------------------
inline bool CGameTrace::DidHit() const
{
	return fraction < 1 || allsolid || startsolid;
}

inline bool CGameTrace::IsVisible() const
{
	return fraction > 0.97f;
}


typedef CGameTrace trace_t;

//=============================================================================

class ITraceListData
{
public:
	virtual ~ITraceListData() {}

	virtual void Reset() = 0;
	virtual bool IsEmpty() = 0;
	// CanTraceRay will return true if the current volume encloses the ray
	// NOTE: The leaflist trace will NOT check this.  Traces are intersected
	// against the culled volume exclusively.
	virtual bool CanTraceRay(const Ray_t &ray) = 0;
};
#endif // GAMETRACE_H

