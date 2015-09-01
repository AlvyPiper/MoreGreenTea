//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RECIPIENTFILTER_H
#define C_RECIPIENTFILTER_H
#ifdef _WIN32
#pragma once
#endif

#include "irecipientfilter.h"
#include "utlvector.h"
#include "c_baseentity.h"
#include "soundflags.h"
#include "bitvec.h"

class C_BasePlayer;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_RecipientFilter : public IRecipientFilter
{
public:
	C_RecipientFilter() { Reset(); }

	virtual			~C_RecipientFilter()
	{
	}

	virtual bool	IsReliable(void) const { return m_bReliable; }
	virtual int		GetRecipientCount(void) const { return m_Recipients.Count(); }
	virtual int		GetRecipientIndex(int slot) const
	{
		if (slot < 0 || slot >= GetRecipientCount())
			return -1;
		return m_Recipients[slot];
	}

	virtual bool	IsInitMessage(void) const { return false; };

public:

	void CopyFrom(const C_RecipientFilter& src)
	{
		m_bReliable = src.IsReliable();
		m_bInitMessage = src.IsInitMessage();

		m_bUsingPredictionRules = src.IsUsingPredictionRules();
		m_bIgnorePredictionCull = src.IgnorePredictionCull();

		int c = src.GetRecipientCount();
		for (int i = 0; i < c; ++i)
		{
			m_Recipients.AddToTail(src.GetRecipientIndex(i));
		}
	}

	void Reset(void)
	{
		m_bReliable = false;
		m_Recipients.RemoveAll();
		m_bUsingPredictionRules = false;
		m_bIgnorePredictionCull = false;
	}

	void MakeReliable(void)
	{
		m_bReliable = true;
	}

	void AddAllPlayers(void)
	{
		m_Recipients.RemoveAll();
		AddRecipient((C_BasePlayer*)(entitylist->GetClientEntity(1)));
	}

	void AddRecipientsByPVS(const Vector& origin)
	{
		AddAllPlayers();
	}

	void AddRecipientsByPAS(const Vector& origin)
	{
		AddAllPlayers();
	}

	void AddRecipient(C_BasePlayer *player)
	{
		if (!player)
			return;

		int index = player->index;

		// Already in list
		if (m_Recipients.Find(index) != m_Recipients.InvalidIndex())
			return;

		// this is a client side filter, only add the local player
		if (player->index == 1)
			return;

		m_Recipients.AddToTail(index);
	}

	void RemoveRecipient(C_BasePlayer *player)
	{
		if (!player)
			return;
		// Remove it if it's in the list
		m_Recipients.FindAndRemove(player->index);
	}

	void AddRecipientsByTeam(void *team)
	{
		AddAllPlayers();
	}

	void RemoveRecipientsByTeam(void *team)
	{
		Assert(0);
	}

	void UsePredictionRules(void);

	bool IsUsingPredictionRules(void) const;
	bool IgnorePredictionCull(void) const;
	void SetIgnorePredictionCull(bool ignore);

	void AddPlayersFromBitMask(CPlayerBitVec& playerbits)
	{
		FOR_EACH_VALID_SPLITSCREEN_PLAYER(hh)
		{
			C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer(hh);
			if (!pPlayer)
				continue;

			// only add the local player on client side
			if (!playerbits[pPlayer->index])
				continue;

			AddRecipient(pPlayer);
		}
	}

	void RemoveSplitScreenPlayers();

private:

	bool				m_bReliable;
	bool				m_bInitMessage;
	CUtlVector< int >	m_Recipients;
	// If using prediction rules, the filter itself suppresses local player
	bool				m_bUsingPredictionRules;
	// If ignoring prediction cull, then external systems can determine
	//  whether this is a special case where culling should not occur
	bool				m_bIgnorePredictionCull;
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player
//-----------------------------------------------------------------------------
class CSingleUserRecipientFilter : public C_RecipientFilter
{
public:
	CSingleUserRecipientFilter(C_BasePlayer *player)
	{
		AddRecipient(player);
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players, unreliable
//-----------------------------------------------------------------------------
class CBroadcastRecipientFilter : public C_RecipientFilter
{
public:
	CBroadcastRecipientFilter(void)
	{
		AddAllPlayers();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players, reliable
//-----------------------------------------------------------------------------
class CReliableBroadcastRecipientFilter : public CBroadcastRecipientFilter
{
public:
	CReliableBroadcastRecipientFilter(void)
	{
		MakeReliable();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player
//-----------------------------------------------------------------------------
class CPASFilter : public C_RecipientFilter
{
public:
	CPASFilter(const Vector& origin)
	{
		AddRecipientsByPAS(origin);
	}
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPASAttenuationFilter : public CPASFilter
{
public:
	CPASAttenuationFilter(C_BaseEntity *entity, float attenuation = ATTN_NORM) :
		CPASFilter(entity->GetAbsOrigin())
	{
	}

	CPASAttenuationFilter(const Vector& origin, float attenuation = ATTN_NORM) :
		CPASFilter(origin)
	{
	}

	CPASAttenuationFilter(C_BaseEntity *entity, const char *lookupSound) :
		CPASFilter(entity->GetAbsOrigin())
	{
	}

	CPASAttenuationFilter(const Vector& origin, const char *lookupSound) :
		CPASFilter(origin)
	{
	}

	CPASAttenuationFilter(C_BaseEntity *entity, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle) :
		CPASFilter(entity->GetAbsOrigin())
	{
	}

	CPASAttenuationFilter(const Vector& origin, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle) :
		CPASFilter(origin)
	{
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player
//-----------------------------------------------------------------------------
class CPVSFilter : public C_RecipientFilter
{
public:
	CPVSFilter(const Vector& origin)
	{
		AddRecipientsByPVS(origin);
	}
};

class CLocalPlayerFilter : public C_RecipientFilter
{
public:
	CLocalPlayerFilter(void);
};

#endif // C_RECIPIENTFILTER_H
