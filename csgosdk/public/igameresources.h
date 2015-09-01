//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: IGameResources interface
//
// $NoKeywords: $
//=============================================================================//

#ifndef IGAMERESOURCES_H
#define IGAMERESOURCES_H

class Color;
class Vector;


abstract_class IGameResources
{
public:
	virtual	void unk();

	const char* GetName(int index)
	{
		string_t name = m_szName[index];
		return name.ToCStr();
	}

	int GetPing(int index)
	{
		return m_iPing[index];
	}

	int GetTeam(int index)
	{
		return m_iTeam[index];
	}

	Color GetTeamColor(int index)
	{
		return m_clrTeamColor[GetTeam(index)];
	}

	int GetHealth(int index)
	{
		return m_iHealth[index];
	}

	bool IsAlive(int index)
	{
		return m_bAlive[index];
	}

	char pad_0x4[0x9D4];
	string_t m_szName[MAX_PLAYERS + 1];
	int		m_iPing[MAX_PLAYERS + 1];
	int		m_iScore[MAX_PLAYERS + 1];
	int		m_iUnk[MAX_PLAYERS + 1];
	int		m_iDeaths[MAX_PLAYERS + 1];
	bool	m_bConnected[MAX_PLAYERS + 1];
	char	pad_0xF2D[0x3];
	int		m_iTeam[MAX_PLAYERS + 1];
	int		m_iPendingTeam[MAX_PLAYERS + 1];
	bool	m_bAlive[MAX_PLAYERS + 1];
	char	pad_0x1179[0x3];
	int		m_iHealth[MAX_PLAYERS + 1];
	Color	m_clrTeamColor[MAX_TEAMS];
};

extern IGameResources *GameResources( void ); // singelton accessor

#endif

