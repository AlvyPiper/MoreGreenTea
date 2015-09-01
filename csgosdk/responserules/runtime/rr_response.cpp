//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "rrbase.h"

#include <tier2/interval.h>

/*
#include "AI_Criteria.h"
#include "ai_speech.h"
#include <KeyValues.h>
#include "engine/IEngineSound.h"
*/

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace ResponseRules;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRR_Response::CRR_Response() : m_fMatchScore(0)
{
	m_Type = ResponseRules::RESPONSE_NONE;
	m_szResponseName[0] = 0;
	m_szMatchingRule[0]=0;
	m_szContext = NULL;
	m_bApplyContextToWorld = false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CRR_Response::CRR_Response( const CRR_Response &from ) : m_fMatchScore(0)
{
	// Assert( (void*)(&m_Type) == (void*)this );
	Invalidate();
	memcpy( this, &from, sizeof(*this) );
	m_szContext = NULL;
	SetContext( from.m_szContext );
	m_bApplyContextToWorld = from.m_bApplyContextToWorld;
}


//-----------------------------------------------------------------------------
CRR_Response &CRR_Response::operator=( const CRR_Response &from )
{
	// Assert( (void*)(&m_Type) == (void*)this );
	Invalidate();
	memcpy( this, &from, sizeof(*this) );
	m_szContext = NULL;
	SetContext( from.m_szContext );
	m_bApplyContextToWorld = from.m_bApplyContextToWorld;
	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRR_Response::~CRR_Response()
{
	if (m_szContext)
		delete[] m_szContext;
}

void CRR_Response::Invalidate()
{
	if (m_szContext)
	{
		delete[] m_szContext;
		m_szContext = NULL;
	}
	m_Type = ResponseRules::RESPONSE_NONE;
	m_szResponseName[0] = 0;
	// not really necessary:
	/*
	m_szMatchingRule[0]=0;
	m_szContext = NULL;
	m_bApplyContextToWorld = false;
	*/
}

// please do not new or delete CRR_Responses.
void CRR_Response::operator delete(void* p) 
{ 
	AssertMsg(false, "DO NOT new or delete CRR_Response s.");
	free(p);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *response - 
//			*criteria - 
//-----------------------------------------------------------------------------
void CRR_Response::Init( ResponseType_t type, const char *responseName, const ResponseParams& responseparams, const char *ruleName, const char *applyContext, bool bApplyContextToWorld )
{
	m_Type = type;
	Q_strncpy( m_szResponseName, responseName, sizeof( m_szResponseName ) );
	// Copy underlying criteria
	Q_strncpy( m_szMatchingRule, ruleName ? ruleName : "NULL", sizeof( m_szMatchingRule ) );
	m_Params = responseparams;
	SetContext( applyContext );
	m_bApplyContextToWorld = bApplyContextToWorld;
}

//-----------------------------------------------------------------------------
// Purpose: Debug-print the response. You can optionally pass in the criteria
// used to come up with this response (usually present in the calling function)
// if you want to print that as well. DO NOT store the entire criteria set in
// CRR_Response just to make this debug print cleaner.
//-----------------------------------------------------------------------------
void CRR_Response::Describe(  const CriteriaSet *pDebugCriteria )
{
	if ( pDebugCriteria )
	{
		DevMsg( "Search criteria:\n" );
		pDebugCriteria->Describe();
	}
	
	if ( m_szMatchingRule[ 0 ] )
	{
		DevMsg( "Matched rule '%s', ", m_szMatchingRule );
	}
	if ( m_szContext )
	{
		DevMsg( "Contexts to set '%s' on %s, ", m_szContext, m_bApplyContextToWorld ? "world" : "speaker" );
	}

	DevMsg( "response %s = '%s'\n", DescribeResponse( (ResponseType_t)m_Type ),  m_szResponseName );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
void CRR_Response::GetName( char *buf, size_t buflen ) const
{
	Q_strncpy( buf, m_szResponseName, buflen );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
void CRR_Response::GetResponse( char *buf, size_t buflen ) const
{
	GetName( buf, buflen );
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : char const
//-----------------------------------------------------------------------------
const char *CRR_Response::DescribeResponse( ResponseType_t type )
{
	if ( (int)type < 0 || (int)type >= ResponseRules::NUM_RESPONSES )
	{
		Assert( 0 );
		return "???CRR_Response bogus index";
	}

	switch( type )
	{
	default:
		{
			Assert( 0 );
		}
		// Fall through
	case ResponseRules::RESPONSE_NONE:
		return "RESPONSE_NONE";
	case ResponseRules::RESPONSE_SPEAK:
		return "RESPONSE_SPEAK";
	case ResponseRules::RESPONSE_SENTENCE:
		return "RESPONSE_SENTENCE";
	case ResponseRules::RESPONSE_SCENE:
		return "RESPONSE_SCENE";
	case ResponseRules::RESPONSE_RESPONSE:
		return "RESPONSE_RESPONSE";
	case ResponseRules::RESPONSE_PRINT:
		return "RESPONSE_PRINT";
	case ResponseRules::RESPONSE_ENTITYIO:
		return "RESPONSE_ENTITYIO";
	}

	return "RESPONSE_NONE";
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRR_Response::Release()
{
	delete this;
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : soundlevel_t
//-----------------------------------------------------------------------------
soundlevel_t CRR_Response::GetSoundLevel() const
{
	if ( m_Params.flags & ResponseParams::RG_SOUNDLEVEL )
	{
		return (soundlevel_t)m_Params.soundlevel;
	}

	return SNDLVL_TALKING;
}

float CRR_Response::GetRespeakDelay( void ) const
{
	if ( m_Params.flags & ResponseParams::RG_RESPEAKDELAY )
	{
		interval_t temp;
		m_Params.respeakdelay.ToInterval( temp );
		return RandomInterval( temp );
	}

	return 0.0f;
}

float CRR_Response::GetWeaponDelay( void ) const
{
	if ( m_Params.flags & ResponseParams::RG_WEAPONDELAY )
	{
		interval_t temp;
		m_Params.weapondelay.ToInterval( temp );
		return RandomInterval( temp );
	}

	return 0.0f;
}

bool CRR_Response::GetSpeakOnce( void ) const
{
	if ( m_Params.flags & ResponseParams::RG_SPEAKONCE )
	{
		return true;
	}

	return false;
}

bool CRR_Response::ShouldntUseScene( void ) const
{
	return ( m_Params.flags & ResponseParams::RG_DONT_USE_SCENE ) != 0;
}

bool CRR_Response::ShouldBreakOnNonIdle( void ) const
{
	return ( m_Params.flags & ResponseParams::RG_STOP_ON_NONIDLE ) != 0;
}

int CRR_Response::GetOdds( void ) const
{
	if ( m_Params.flags & ResponseParams::RG_ODDS )
	{
		return m_Params.odds;
	}
	return 100;
}

float CRR_Response::GetDelay() const
{
	if ( m_Params.flags & ResponseParams::RG_DELAYAFTERSPEAK )
	{
		interval_t temp;
		m_Params.delay.ToInterval( temp );
		return RandomInterval( temp );
	}
	return 0.0f;
}

float CRR_Response::GetPreDelay() const
{
	if ( m_Params.flags & ResponseParams::RG_DELAYBEFORESPEAK )
	{
		interval_t temp;
		m_Params.predelay.ToInterval( temp );
		return RandomInterval( temp );
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Sets context string
// Output : void
//-----------------------------------------------------------------------------
void CRR_Response::SetContext( const char *context )
{
	if (m_szContext)
	{
		delete[] m_szContext;
		m_szContext = NULL;
	}

	if ( context )
	{
		int len = Q_strlen( context );
		m_szContext = new char[ len + 1 ];
		Q_memcpy( m_szContext, context, len );
		m_szContext[ len ] = 0;
	}
}
