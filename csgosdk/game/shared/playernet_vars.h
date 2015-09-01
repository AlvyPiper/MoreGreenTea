//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERNET_VARS_H
#define PLAYERNET_VARS_H
#ifdef _WIN32
#pragma once
#endif

#include "shared_classnames.h"

#define NUM_AUDIO_LOCAL_SOUNDS	8

// These structs are contained in each player's local data and shared by the client & server

struct fogparams_t
{
	DECLARE_CLASS_NOBASE( fogparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

#ifndef CLIENT_DLL
	DECLARE_SIMPLE_DATADESC();
#endif

	bool operator !=( const fogparams_t& other ) const;
	CNetworkVector( dirPrimary );
	//Vector dirPrimary;
	CNetworkColor32( colorPrimary );
	CNetworkColor32( colorSecondary );
	CNetworkColor32( colorPrimaryLerpTo );
	CNetworkColor32( colorSecondaryLerpTo );
	CNetworkVar( float, start );
	CNetworkVar( float, end );
	CNetworkVar( float, farz );
	CNetworkVar( float, maxdensity );
	CNetworkVar( float, startLerpTo );
	CNetworkVar( float, endLerpTo );
	CNetworkVar( float, maxdensityLerpTo );
	CNetworkVar( float, lerptime );
	CNetworkVar( float, duration );
	CNetworkVar( bool, enable );
	CNetworkVar( bool, blend );
	int reserved;
	CNetworkVar( float, HDRColorScale );
};

// Crappy. Needs to be here because it wants to use 
#ifdef CLIENT_DLL
#define CFogController C_FogController
#endif

class CFogController;

struct fogplayerparams_t
{
	DECLARE_CLASS_NOBASE( fogplayerparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();
#ifndef CLIENT_DLL
	DECLARE_SIMPLE_DATADESC();
#endif
	CNetworkHandle( CFogController, m_hCtrl );
	float					m_flTransitionTime;
	color32					m_OldColor;
	float					m_flOldStart;
	float					m_flOldEnd;
	float					m_flOldMaxDensity;
	float					m_flOldHDRColorScale;
	float					m_flOldFarZ;
	color32					m_NewColor;
	float					m_flNewStart;
	float					m_flNewEnd;
	float					m_flNewMaxDensity;
	float					m_flNewHDRColorScale;
	float					m_flNewFarZ;
	fogplayerparams_t()
	{
		m_hCtrl.Set( NULL );
		m_flTransitionTime = -1.0f;
		m_OldColor.r = m_OldColor.g = m_OldColor.g = m_OldColor.a = 0.0f;
		m_flOldStart = 0.0f;
		m_flOldEnd = 0.0f;
		m_flOldMaxDensity = 1.0f;
		m_flOldHDRColorScale = 1.0f;
		m_flOldFarZ = 0;
		m_NewColor.r = m_NewColor.g = m_NewColor.g = m_NewColor.a = 0.0f;
		m_flNewStart = 0.0f;
		m_flNewEnd = 0.0f;
		m_flNewMaxDensity = 1.0f;
		m_flNewHDRColorScale = 1.0f;
		m_flNewFarZ = 0;
	}
};

struct sky3dparams_t
{
	DECLARE_CLASS_NOBASE( sky3dparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

#ifndef CLIENT_DLL
	DECLARE_SIMPLE_DATADESC();
#endif

	// 3d skybox camera data
	CNetworkVar( int, scale );
	CNetworkVector( origin );
	CNetworkVar( int, area );
	// 3d skybox fog data m_Local.m_skybox3d.fog
	CNetworkVarEmbedded( fogparams_t, fog );
};

struct audioparams_t
{
	DECLARE_CLASS_NOBASE( audioparams_t );
	DECLARE_EMBEDDED_NETWORKVAR();

#ifndef CLIENT_DLL
	DECLARE_SIMPLE_DATADESC();
#endif

	Vector localSound[NUM_AUDIO_LOCAL_SOUNDS];
	CNetworkVar( int, soundscapeIndex );	// index of the current soundscape from soundscape.txt
	CNetworkVar( int, localBits );			// if bits 0,1,2,3 are set then position 0,1,2,3 are valid/used
	CNetworkVar( int, entIndex );		// the entity setting the soundscape
};


#endif // PLAYERNET_VARS_H
