//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
// Client-side CBasePlayer

#ifndef C_STUDIOFLEX_H
#define C_STUDIOFLEX_H
#pragma once


#include "c_baseanimating.h"
#include "c_baseanimatingoverlay.h"
#include "sceneentity_shared.h"

#include "UtlVector.h"

#ifndef LOG_BASEFLEX
#define LOG_BASEFLEX
#endif

#define SHOUTMEMBERFLEX(memberName, member) if (typeid(member) == typeid(float)) { LOG_BASEFLEX("0x%X | %s = %f (size 0x%X)", offsetof(class C_BaseFlex,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class C_BaseFlex,  member), memberName, member, sizeof(member)); } else { LOG_BASEFLEX("0x%X | %s = %d (size 0x%X)", offsetof(class C_BaseFlex,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class C_BaseFlex,  member), memberName, member, sizeof(member)); }

// Purpose: Item in list of loaded scene files
class CFlexSceneFile
{
public:
	enum { MAX_FLEX_FILENAME = 128, };
	char			filename[ MAX_FLEX_FILENAME ];
	void			*buffer;
};

struct Emphasized_Phoneme;
class CSentence;

class C_BaseFlex : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_BaseFlex, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_BaseFlex();
	virtual				~C_BaseFlex();
	virtual void		Spawn();
	virtual IClientModelRenderable*	GetClientModelRenderable();
	virtual void		InitPhonemeMappings();
	void				SetupMappings( char const *pchFileRoot );
	virtual CStudioHdr* OnNewModel( void );
	virtual void		StandardBlendingRules( CStudioHdr *hdr, Vector pos[], QuaternionAligned q[], float currentTime, int boneMask );
	virtual void		OnThreadedDrawSetup();
	// model specific
	virtual void		BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );
	virtual	void		SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	virtual bool		UsesFlexDelayedWeights();
	virtual bool		GetSoundSpatialization( SpatializationInfo_t& info );
	virtual void		GetToolRecordingState( KeyValues *msg );
	// Called at the lowest level to actually apply a flex animation
	void				AddFlexAnimation( CSceneEventInfo *info );
	void				SetFlexWeight( LocalFlexController_t index, float value );
	float				GetFlexWeight( LocalFlexController_t index );
	// Look up flex controller index by global name
	LocalFlexController_t FindFlexController( const char *szName );
	C_BaseFlex(const C_BaseFlex &); // not defined, not accessible
	const flexsetting_t*FindNamedSetting(const flexsettinghdr_t *pSettinghdr, const char *expr);
	void				ProcessVisemes(Emphasized_Phoneme *classes, float *pGlobalFlexWeight);
	void				AddVisemesForSentence(float *pGlobalFlexWeight, Emphasized_Phoneme *classes, float emphasis_intensity, CSentence *sentence, float t, float dt, bool juststarted);
	void				AddViseme(float *pGlobalFlexWeight, Emphasized_Phoneme *classes, float emphasis_intensity, int phoneme, float scale, bool newexpression);
	bool				SetupEmphasisBlend(Emphasized_Phoneme *classes, int phoneme);
	void				ComputeBlendedSetting(Emphasized_Phoneme *classes, float emphasis_intensity);
	static int			AddGlobalFlexController(char *szName);
	static char const*	GetGlobalFlexControllerName(int idx);
	// Start the specifics of an scene event
	virtual bool		StartSceneEvent(CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, C_BaseEntity *pTarget);
	virtual bool		ProcessSequenceSceneEvent(CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event);
	// Remove all playing events
	void				ClearSceneEvents(CChoreoScene *scene, bool canceled);
	// Stop specifics of event
	virtual	bool		ClearSceneEvent(CSceneEventInfo *info, bool fastKill, bool canceled);
	// Add the event to the queue for this actor
	void				AddSceneEvent(CChoreoScene *scene, CChoreoEvent *event, C_BaseEntity *pTarget = NULL, bool bClientSide = false, C_SceneEntity *pSceneEntity = NULL);
	// Remove the event from the queue for this actor
	void				RemoveSceneEvent(CChoreoScene *scene, CChoreoEvent *event, bool fastKill);
	// Checks to see if the event should be considered "completed"
	bool				CheckSceneEvent(float currenttime, CChoreoScene *scene, CChoreoEvent *event);
	// Checks to see if a event should be considered "completed"
	virtual bool		CheckSceneEventCompletion(CSceneEventInfo *info, float currenttime, CChoreoScene *scene, CChoreoEvent *event);
	int					FlexControllerLocalToGlobal(const flexsettinghdr_t *pSettinghdr, int key);
	void				EnsureTranslations(const flexsettinghdr_t *pSettinghdr);
	// For handling scene files
	const void			*FindSceneFile(const char *filename);
	static void			InvalidateFlexCaches();
	bool				IsFlexCacheValid() const;
	Vector				SetViewTarget(CStudioHdr *pStudioHdr, const float *pGlobalFlexWeight);
	void				RunFlexRules(CStudioHdr *pStudioHdr, const float *pGlobalFlexWeight, float *dest);
	void				ProcessSceneEvents(bool bFlexEvents, float *pGlobalFlexWeight);
	bool				ProcessSceneEvent(float *pGlobalFlexWeight, bool bFlexEvents, CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event);
	bool				RequestStartSequenceSceneEvent(CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget);
	bool				ProcessFlexAnimationSceneEvent(CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event);
	bool				ProcessFlexSettingSceneEvent(float *pGlobalFlexWeight, CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event);
	void				AddFlexSetting(float *pGlobalFlexWeight, const char *expr, float scale, const flexsettinghdr_t *pSettinghdr, bool newexpression);
	bool				HasSceneEvents() const;
	void				StartChoreoScene(CChoreoScene *scene);
	void				RemoveChoreoScene(CChoreoScene *scene);
	struct FS_LocalToGlobal_t
	{
		explicit FS_LocalToGlobal_t() : m_Key(0), m_nCount(0), m_Mapping(0)
		{
		}

		explicit FS_LocalToGlobal_t(const flexsettinghdr_t *key) : m_Key(key), m_nCount(0), m_Mapping(0)
		{
		}

		void SetCount(int count)
		{
			Assert(!m_Mapping);
			Assert(count > 0);
			m_nCount = count;
			m_Mapping = new int[m_nCount];
			Q_memset(m_Mapping, 0, m_nCount * sizeof(int));
		}

		FS_LocalToGlobal_t(const FS_LocalToGlobal_t& src)
		{
			m_Key = src.m_Key;
			delete m_Mapping;
			m_Mapping = new int[src.m_nCount];
			Q_memcpy(m_Mapping, src.m_Mapping, src.m_nCount * sizeof(int));
			m_nCount = src.m_nCount;
		}

		~FS_LocalToGlobal_t()
		{
			delete m_Mapping;
			m_nCount = 0;
			m_Mapping = 0;
		}

		const flexsettinghdr_t	*m_Key;
		int						m_nCount;
		int						*m_Mapping;
	};

	static bool FlexSettingLessFunc(const FS_LocalToGlobal_t& lhs, const FS_LocalToGlobal_t& rhs);

	// shared flex controllers
	static int		g_numflexcontrollers;
	static char		*g_flexcontroller[MAXSTUDIOFLEXCTRL * 4]; // room for global set of flexcontrollers
	static float	s_pGlobalFlexWeight[MAXSTUDIOFLEXCTRL * 4];

	enum { PHONEME_CLASS_WEAK = 0, PHONEME_CLASS_NORMAL, PHONEME_CLASS_STRONG, NUM_PHONEME_CLASSES };
	struct Emphasized_Phoneme
	{
		// Global fields, setup at start
		char			classname[64];
		bool			required;
		// Global fields setup first time tracks played
		bool			basechecked;
		const flexsettinghdr_t *base;
		const flexsetting_t *exp;
		// Local fields, processed for each sentence
		bool			valid;
		float			amount;
	};

public: // members
	void dump();

	Vector											m_viewtarget;
	CInterpolatedVar<Vector>						m_iv_viewtarget;
	float											m_flexWeight[MAXSTUDIOFLEXCTRL]; // indexed by model local flexcontroller
	CInterpolatedVarArray<float, MAXSTUDIOFLEXCTRL>	m_iv_flexWeight;
	int												m_blinktoggle; 
	CUtlVector<CSceneEventInfo>						m_SceneEvents; // Array of active SceneEvents, in order oldest to newest
	CUtlVector<CChoreoScene*>						m_ActiveChoreoScenes;
	CUtlRBTree<FS_LocalToGlobal_t, unsigned short>	m_LocalToGlobal;
	float											m_blinktime;
	int												m_prevblinktoggle;
	int												m_iBlink;
	LocalFlexController_t							m_iEyeUpdown;
	LocalFlexController_t							m_iEyeRightleft;
	int												m_iMouthAttachment;
	float*											m_flFlexDelayedWeight;
	int												m_iMostRecentFlexCounter;
	Vector											m_CachedViewTarget;
	CUtlVector<float>								m_CachedFlexWeights;
	CUtlVector<float>								m_CachedDelayedFlexWeights;
	Emphasized_Phoneme								m_PhonemeClasses[NUM_PHONEME_CLASSES]; 
	char											pad_0x1134[0x4]; // C_BaseFlex ends, C_BaseCombatCharacter begins at 0x1138
	// 0x1138
};

inline void C_BaseFlex::dump()
{
	printf("C_BaseFlex -> Address = 0x%X\n", (DWORD)this);
	LOG_BASEFLEX("C_BaseFlex -> Address = 0x%X", (DWORD)this);
	SHOUTMEMBERFLEX("m_viewtarget", m_viewtarget[0]);
	SHOUTMEMBERFLEX("m_iv_viewtarget", m_iv_viewtarget.m_pValue);
	SHOUTMEMBERFLEX("m_flexWeight[96]", m_flexWeight[0]);
	SHOUTMEMBERFLEX("m_iv_flexWeight", m_iv_flexWeight.m_pValue);
	SHOUTMEMBERFLEX("m_blinktoggle", m_blinktoggle);
	SHOUTMEMBERFLEX("m_SceneEvents", m_SceneEvents.m_Memory.m_pMemory);
	SHOUTMEMBERFLEX("m_ActiveChoreoScenes", m_ActiveChoreoScenes.m_Memory.m_pMemory);
	SHOUTMEMBERFLEX("m_LocalToGlobal", m_LocalToGlobal.m_LessFunc);
	SHOUTMEMBERFLEX("m_blinktime", m_blinktime);
	SHOUTMEMBERFLEX("m_prevblinktoggle", m_prevblinktoggle);
	SHOUTMEMBERFLEX("m_iBlink", m_iBlink);
	SHOUTMEMBERFLEX("m_iEyeUpdown", m_iEyeUpdown);
	SHOUTMEMBERFLEX("m_iEyeRightleft", m_iEyeRightleft);
	SHOUTMEMBERFLEX("m_iMouthAttachment", m_iMouthAttachment);
	SHOUTMEMBERFLEX("m_flFlexDelayedWeight", m_flFlexDelayedWeight);
	SHOUTMEMBERFLEX("m_iMostRecentFlexCounter", m_iMostRecentFlexCounter);
	SHOUTMEMBERFLEX("m_CachedViewTarget", m_CachedViewTarget[0]);
	SHOUTMEMBERFLEX("m_CachedFlexWeights", m_CachedFlexWeights.m_Memory.m_pMemory);
	SHOUTMEMBERFLEX("m_CachedDelayedFlexWeights", m_CachedDelayedFlexWeights.m_Memory.m_pMemory);
	SHOUTMEMBERFLEX("m_PhonemeClasses[3]", m_PhonemeClasses[0].classname);
	SHOUTMEMBERFLEX("pad_0x1134[4]", pad_0x1134);
	printf("\n");
}


//-----------------------------------------------------------------------------
// Do we have active expressions?
//-----------------------------------------------------------------------------
inline bool C_BaseFlex::HasSceneEvents() const
{
	return m_SceneEvents.Count() != 0;
}


EXTERN_RECV_TABLE(DT_BaseFlex);

float *GetVisemeWeights( int phoneme );

#endif // C_STUDIOFLEX_H




