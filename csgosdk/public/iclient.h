//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ICLIENT_H
#define ICLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include <inetmsghandler.h>
#include "tier0/platform.h"
#include "tier1/utlvector.h"

class IServer;
class INetMessage;
struct NetMessageCvar_t;
struct USERID_t;
class CMsg_CVars;

enum CrossPlayPlatform_t
{
	CROSSPLAYPLATFORM_UNKNOWN,
	CROSSPLAYPLATFORM_PC,
	CROSSPLAYPLATFORM_X360,
	CROSSPLAYPLATFORM_PS3,
	CROSSPLAYPLATFORM_LAST,
};

abstract_class IClient : public INetChannelHandler
{
public:
	virtual	~IClient() {}

	// connect client
	virtual void	Connect( const char * szName, int nUserID, INetChannel *pNetChannel, bool bFakePlayer, CrossPlayPlatform_t platform, const CMsg_CVars *pCvars = NULL ) = 0;

	// set the client in a pending state waiting for a new game
	virtual void	Inactivate( void ) = 0;
	
	// Reconnect without dropiing the netchannel
	virtual	void	Reconnect( void ) = 0;				// froce reconnect

	// disconnects a client with a given reason
	virtual void	Disconnect( const char *reason ) = 0;

	virtual int				GetPlayerSlot() const = 0; // returns client slot (usually entity number-1)
	virtual int				GetUserID() const = 0; // unique ID on this server 
	virtual const USERID_t	GetNetworkID() const = 0; // network wide ID
	virtual const char		*GetClientName() const = 0;	// returns client name
	virtual INetChannel		*GetNetChannel() = 0; // returns client netchannel
	virtual IServer			*GetServer() = 0; // returns the object server the client belongs to
	virtual const char		*GetUserSetting(const char *cvar) const = 0; // returns a clients FCVAR_USERINFO setting
	virtual const char		*GetNetworkIDString() const = 0; // returns a human readable representation of the network id

	// set/get client data rate in bytes/second
	virtual void	SetRate( int nRate, bool bForce ) = 0;
	virtual int		GetRate( void ) const = 0;
	
	// set/get updates/second rate
	virtual void	SetUpdateRate( int nUpdateRate, bool bForce ) = 0;
	virtual int		GetUpdateRate( void ) const = 0;	

	// clear complete object & free all memory 
	virtual void	Clear( void ) = 0;

	// returns the highest world tick number acknowledge by client
	virtual int		GetMaxAckTickCount() const = 0;
	
	// execute a client command
	virtual bool	ExecuteStringCommand( const char *s ) = 0;
	// send client a network message
	virtual bool	SendNetMsg(INetMessage &msg, bool bForceReliable = false, bool bVoice = false ) = 0;
	// send client a text message
	virtual void	ClientPrintf (const char *fmt, ...) = 0;

		// client has established network channels, nothing else
	virtual bool	IsConnected( void ) const = 0;
	// client is downloading signon data
	virtual bool	IsSpawned( void ) const = 0;
	// client active is ingame, receiving snapshots
	virtual bool	IsActive( void ) const = 0;
	// returns true, if client is not a real player
	virtual bool	IsFakeClient( void ) const = 0;
	// returns true, if client is a HLTV proxy
	virtual bool	IsHLTV( void ) const = 0;
	// returns true, if client hears this player
	virtual bool	IsHearingClient(int index) const = 0;
	// returns true, if client hears this player by proximity
	virtual bool	IsProximityHearingClient(int index) const = 0;

	virtual void	SetMaxRoutablePayloadSize( int nMaxRoutablePayloadSize ) = 0;

	// returns true, if client is a split screen user
	virtual bool	IsSplitScreenUser( void ) const = 0;

	virtual bool	CheckConnect( void ) = 0;

	virtual	bool	IsLowViolenceClient( void ) const = 0;

	virtual IClient	*GetSplitScreenOwner() = 0;

	// get the number of players on this client's machine
	virtual int		GetNumPlayers() = 0;

	virtual bool	IsHumanPlayer() const = 0;

	virtual CrossPlayPlatform_t		GetClientPlatform() const = 0;
};

#endif // ICLIENT_H
