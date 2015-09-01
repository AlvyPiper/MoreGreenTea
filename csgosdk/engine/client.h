//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#if !defined( CLIENT_H )
#define CLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include <qlimits.h>
#include <utllinkedlist.h>
#include <convar.h>
#include <checksum_crc.h>
#include <protocol.h>
#include <cdll_int.h>
#include <globalvars_base.h>
#include <soundflags.h>
#include <utlvector.h>
//#include "host.h"
//#include "event_system.h"
//#include "precache.h"
//#include "baseclientstate.h"
//#include "clientframe.h"

struct netadr_t
{
public:
	__int32 type; //0x0000 
	char ip[4]; //0x0004 
	unsigned short port; //0x0008 
};//Size=0x000C


struct model_t;

class ClientClass;
class CSfxTable;
class CPureServerWhitelist;

#define	MAX_DEMOS		32
#define	MAX_DEMONAME	32

struct AddAngle
{
	float total;
	float starttime;
};


//-----------------------------------------------------------------------------
// Purpose: CClientState should hold all pieces of the client state
//   The client_state_t structure is wiped completely at every server signon
//-----------------------------------------------------------------------------
class CClientState //: public CBaseClientState, public CClientFrameManager
{
	typedef struct CustomFile_s
	{
		CRC32_t			crc;	//file CRC
		unsigned int	reqID;	// download request ID
	} CustomFile_t;

public:
	CClientState();
	~CClientState();

public: // IConnectionlessPacketHandler interface:
		
	bool ProcessConnectionlessPacket(struct netpacket_s *packet);

public: // CBaseClientState overrides:
	void Disconnect(bool bShowMainMenu);
	void FullConnect( netadr_t &adr );
	bool SetSignonState ( int state, int count );
	void PacketStart(int incoming_sequence, int outgoing_acknowledged);
	void PacketEnd( void );
	void FileReceived( const char *fileName, unsigned int transferID );
	void FileRequested(const char *fileName, unsigned int transferID );
	void FileDenied(const char *fileName, unsigned int transferID );
	void ConnectionCrashed( const char * reason );
	void ConnectionClosing( const char * reason );
	const char *GetCDKeyHash( void );
	void SetFriendsID( uint friendsID, const char *friendsName );
	void SendClientInfo( void );
	void InstallStringTableCallback( char const *tableName );
	bool HookClientStringTable( char const *tableName );
	bool InstallEngineStringTableCallback( char const *tableName );

	void StartUpdatingSteamResources();
	void CheckUpdatingSteamResources();
	void CheckFileCRCsWithServer();
	void FinishSignonState_New();
	void ConsistencyCheck(bool bForce);
	void RunFrame();

	//void ReadEnterPVS( CEntityReadInfo &u );
	//void ReadLeavePVS( CEntityReadInfo &u );
	//void ReadDeltaEnt( CEntityReadInfo &u );
	//void ReadPreserveEnt( CEntityReadInfo &u );
	//void ReadDeletions( CEntityReadInfo &u );

	// In case the client DLL is using the old interface to set area bits,
	// copy what they've passed to us into the m_chAreaBits array (and 0xFF-out the m_chAreaPortalBits array).
	void UpdateAreaBits_BackwardsCompatible();

	// Used to be pAreaBits.
	unsigned char** GetAreaBits_BackwardCompatibility();

public: // IServerMessageHandlers
	
	//PROCESS_NET_MESSAGE(Tick);
	//PROCESS_NET_MESSAGE(StringCmd);
	//PROCESS_SVC_MESSAGE(ServerInfo);
	//PROCESS_SVC_MESSAGE(ClassInfo);
	//PROCESS_SVC_MESSAGE(SetPause);
	//PROCESS_SVC_MESSAGE(VoiceInit);
	//PROCESS_SVC_MESSAGE(VoiceData);
	//PROCESS_SVC_MESSAGE(Sounds);
	//PROCESS_SVC_MESSAGE(FixAngle);
	//PROCESS_SVC_MESSAGE(CrosshairAngle);
	//PROCESS_SVC_MESSAGE(BSPDecal);
	//PROCESS_SVC_MESSAGE(GameEvent);
	//PROCESS_SVC_MESSAGE(UserMessage);
	//PROCESS_SVC_MESSAGE(EntityMessage);
	//PROCESS_SVC_MESSAGE(PacketEntities);
	//PROCESS_SVC_MESSAGE(TempEntities);
	//PROCESS_SVC_MESSAGE(Prefetch);

public:
	//char pad_0x0000[0x8]; //0x0000 vtable is at ptr + 0x8
	//N00000285* ClientStateVtable; //0x0008 
	char pad_0x000C[0x4]; //0x000C
	void* N00000017; //0x0010 
	void* N00000018; //0x0014 
	void* N00000019; //0x0018 
	void* N0000001A; //0x001C 
	void* N0000001B; //0x0020 
	void* N0000001C; //0x0024 
	void* N0000001D; //0x0028 
	void* N0000001E; //0x002C 
	void* N0000001F; //0x0030 
	void* N00000020; //0x0034 
	void* N00000021; //0x0038 
	void* N00000022; //0x003C 
	void* N00000023; //0x0040 
	void* N00000024; //0x0044 
	void* N00000025; //0x0048 
	void* N00000026; //0x004C 
	void* N00000027; //0x0050 
	void* N00000028; //0x0054 
	void* N00000029; //0x0058 
	void* N0000002A; //0x005C 
	void* N0000002B; //0x0060 
	void* N0000002C; //0x0064 
	void* N0000002D; //0x0068 
	void* N0000002E; //0x006C 
	void* N0000002F; //0x0070 
	void* N00000030; //0x0074 
	void* N0000039C; //0x0078 
	void* N00000032; //0x007C 
	void* N00000033; //0x0080 
	void* N00000034; //0x0084 
	void* N00000035; //0x0088 
	void* N00000036; //0x008C 
	__int32 m_Socket; //0x0090 
	INetChannel* m_NetChannel; //0x0094 
	__int32 m_nChallengeNr; //0x0098 
	char pad_0x009C[0x4C]; //0x009C
	__int32 m_nSignonState; //0x00E8 
	char pad_0x00EC[0x8]; //0x00EC
	float m_flNextCmdTime; //0x00F4 
	__int32 m_nServerCount; //0x00F8 
	__int32 m_nCurrentSequence; //0x00FC 
	char pad_0x0100[0x8]; //0x0100
	float m_ClockOffsets[16]; //0x0108 
	__int32 m_iCurClockOffset; //0x0148 
	__int32 m_nServerTick; //0x014C 
	__int32 m_nClientTick; //0x0150 
	__int32 m_nDeltaTick; //0x0154 
	unsigned char m_bPaused; //0x0158 
	char pad_0x0159[0x3]; //0x0159
	__int32 m_nViewEntity; //0x015C 
	char pad_0x0160[0x8]; //0x0160
	char m_szLevelName[260]; //0x0168 
	char m_szLevelNameShort[40]; //0x026C 
	char pad_0x0294[0x5EC]; //0x0294
	void* N00000233; //0x0880 
	void* N00000426; //0x0884 
	void* N00000427; //0x0888 
	void* N00000428; //0x088C 
	void* N00000429; //0x0890 
	char pad_0x0894[0x3130]; //0x0894


	//float		m_flLastServerTickTime;		// the timestamp of last message
	//bool		insimulation;
	//
	//int			oldtickcount;		// previous tick
	//float		m_tickRemainder;	// client copy of tick remainder
	//float		m_frameTime;		// dt of the current frame
	//
	//int			lastoutgoingcommand;// Sequence number of last outgoing command
	//int			chokedcommands;		// number of choked commands
	//int			last_command_ack;	// last command sequence number acknowledged by server
	//int			command_ack;		// current command sequence acknowledged by server
	//int			m_nSoundSequence;	// current processed reliable sound sequence number
	//
	////
	//// information that is static for the entire time connected to a server
	////
	//bool		ishltv;			// true if HLTV server/demo
	//
	//CRC32_t		serverCRC;              // To determine if client is playing hacked .map. (entities lump is skipped)
	//CRC32_t		serverClientSideDllCRC; // To determine if client is playing on a hacked client dll.
	//
	//unsigned char	m_chAreaBits[MAX_AREA_STATE_BYTES];
	//unsigned char	m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];
	//bool			m_bAreaBitsValid; // Have the area bits been set for this level yet?
	//
	//// refresh related state
	//QAngle		viewangles;
	//CUtlVector< AddAngle >	addangle;
	//float		addangletotal;
	//float		prevaddangletotal;
	//int			cdtrack;			// cd audio
	//
	//CustomFile_t	m_nCustomFiles[MAX_CUSTOM_FILES]; // own custom files CRCs
	//
	//uint		m_nFriendsID;
	//char		m_FriendsName[MAX_PLAYER_NAME_LENGTH];
	//
	//
	//CUtlFixedLinkedList< CEventInfo > events;	// list of received events
	//
	//// demo loop control
	//int			demonum;		                  // -1 = don't play demos
	//char		demos[MAX_DEMOS][MAX_DEMONAME];	  // when not playing
	//
	//public:
	//
	//// If 'insimulation', returns the time (in seconds) at the client's current tick.
	//// Otherwise, returns the exact client clock.
	//float				GetTime() const;
	//
	//
	//bool				IsPaused() const;
	//
	//float				GetFrameTime( void ) const;
	//void				SetFrameTime( float dt ) { m_frameTime = dt; }
	//
	//float				GetClientInterpAmount();		// Formerly cl_interp, now based on cl_interp_ratio and cl_updaterate.
	//	
	//void				Clear( void );
	//
	//void				DumpPrecacheStats(  const char * name );
	//
	//// Public API to models
	//model_t				*GetModel( int index );
	//void				SetModel( int tableIndex );
	//int					LookupModelIndex( char const *name );
	//
	//// Public API to generic
	//char const			*GetGeneric( int index );
	//void				SetGeneric( int tableIndex );
	//int					LookupGenericIndex( char const *name );
	//
	//// Public API to sounds
	//CSfxTable			*GetSound( int index );
	//char const			*GetSoundName( int index );
	//void				SetSound( int tableIndex );
	//int					LookupSoundIndex( char const *name );
	//void				ClearSounds();
	//
	//// Public API to decals
	//char const			*GetDecalName( int index );
	//void				SetDecal( int tableIndex );
	//
	//// customization files code
	//void				CheckOwnCustomFiles(); // load own custom file
	//void				CheckOthersCustomFile(CRC32_t crc); // check if we have to download custom files from server
	//void				AddCustomFile( int slot, const char *resourceFile);
	//
	//public:
	//INetworkStringTable *m_pModelPrecacheTable;	
	//INetworkStringTable *m_pGenericPrecacheTable;	
	//INetworkStringTable *m_pSoundPrecacheTable;
	//INetworkStringTable *m_pDecalPrecacheTable;
	//INetworkStringTable *m_pInstanceBaselineTable;
	//INetworkStringTable *m_pLightStyleTable;
	//INetworkStringTable *m_pUserInfoTable;
	//INetworkStringTable *m_pServerStartupTable;
	//INetworkStringTable *m_pDownloadableFileTable;
	//
	//CPrecacheItem		model_precache[ MAX_MODELS ];
	//CPrecacheItem		generic_precache[ MAX_GENERIC ];
	//CPrecacheItem		sound_precache[ MAX_SOUNDS ];
	//CPrecacheItem		decal_precache[ MAX_BASE_DECALS ];
	//
	//WaitForResourcesHandle_t m_hWaitForResourcesHandle;
	//bool m_bUpdateSteamResources;
	//bool m_bShownSteamResourceUpdateProgress;
	//bool m_bDownloadResources;
	//bool m_bCheckCRCsWithServer;
	//float m_flLastCRCBatchTime;
	//
	//// This is only kept around to print out the whitelist info if sv_pure is used.	
	//CPureServerWhitelist *m_pPureServerWhitelist;
	//
	//private:
	//
	//// Note: This is only here for backwards compatibility. If it is set to something other than NULL,
	//// then we'll copy its contents into m_chAreaBits in UpdateAreaBits_BackwardsCompatible.
	//byte		*m_pAreaBits;
	//
	//// Set to false when we first connect to a server and true later on before we
	//// respond to a new whitelist.
	//bool		m_bMarkedCRCsUnverified;
};  //CClientState

extern	CClientState	cl;


#endif // CLIENT_H
