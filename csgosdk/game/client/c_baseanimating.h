//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//===========================================================================//
#ifndef C_BASEANIMATING_H
#define C_BASEANIMATING_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "studio.h"
#include "UtlVector.h"
#include "ragdoll.h"
#include "mouthinfo.h"
// Shared activities
#include "ai_activity.h"
#include "animationlayer.h"
#include "sequence_transitioner.h"
#include "bone_accessor.h"
#include "bone_merge_cache.h"
#include "ragdoll_shared.h"
#include "tier0/threadtools.h"
#include "datacache/idatacache.h"
#include "toolframework/itoolframework.h"
#include "engine/IVDebugOverlay.h"

#define LIPSYNC_POSEPARAM_NAME "mouth"
#define NUM_HITBOX_FIRES	10

#ifndef LOG_BASEANIMATING
#define LOG_BASEANIMATING
#endif

#define SHOUTMEMBERANIMATING(memberName, member) if (typeid(member) == typeid(float)) { LOG_BASEANIMATING("0x%X | %s = %f (size 0x%X)", offsetof(class C_BaseAnimating,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class C_BaseAnimating,  member), memberName, member, sizeof(member)); } else { LOG_BASEANIMATING("0x%X | %s = %d (size 0x%X)", offsetof(class C_BaseAnimating,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class C_BaseAnimating,  member), memberName, member, sizeof(member)); }

extern int g_iModelBoneCounter;

/*
class C_BaseClientShader
{
virtual void RenderMaterial( C_BaseEntity *pEntity, int count, const vec4_t *verts, const vec4_t *normals, const vec2_t *texcoords, vec4_t *lightvalues );
};
*/

class IRagdoll;
class C_ClientRagdoll;
class CIKContext;
class CIKState;
class ConVar;
class C_RopeKeyframe;
class CBoneBitList;
class CBoneList;
class KeyValues;
class CJiggleBones;
class IBoneSetup;

FORWARD_DECLARE_HANDLE(memhandle_t);
typedef unsigned short MDLHandle_t;

extern ConVar vcollide_wireframe;
extern IDataCache *datacache;

struct ClientModelRenderInfo_t : public ModelRenderInfo_t
{
	// Added space for lighting origin override. Just allocated space, need to set base pointer
	matrix3x4_t lightingOffset;

	// Added space for model to world matrix. Just allocated space, need to set base pointer
	matrix3x4_t modelToWorld;
};

struct RagdollInfo_t
{
	bool		m_bActive;
	float		m_flSaveTime;
	int			m_nNumBones;
	Vector		m_rgBonePos[MAXSTUDIOBONES];
	Quaternion	m_rgBoneQuaternion[MAXSTUDIOBONES];
};


class CAttachmentData
{
public:
	matrix3x4_t	m_AttachmentToWorld;
	QAngle	m_angRotation;
	Vector	m_vOriginVelocity;
	int		m_nLastFramecount : 31;
	int		m_bAnglesComputed : 1;
};


typedef unsigned int			ClientSideAnimationListHandle_t;

#define		INVALID_CLIENTSIDEANIMATION_LIST_HANDLE	(ClientSideAnimationListHandle_t)~0


class C_BaseAnimating : public C_BaseEntity, public IClientModelRenderable
{
public:
	DECLARE_CLASS(C_BaseAnimating, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();
	DECLARE_FRIEND_DATADESC_ACCESS();
	DECLARE_ENT_SCRIPTDESC();

	enum
	{
		NUM_POSEPAREMETERS = 24,
		NUM_BONECTRLS = 4
	};

	// Inherited from IClientUnknown
public:
	virtual IClientModelRenderable*	GetClientModelRenderable();

	// Inherited from IClientModelRenderable
public:
	virtual bool GetRenderData(void *pData, ModelDataCategory_t nCategory);

public:
	C_BaseAnimating();
	~C_BaseAnimating();

	virtual C_BaseAnimating*		GetBaseAnimating() { return this; }

	int GetRenderFlags(void);

	virtual bool	Interpolate(float currentTime);
	virtual bool	Simulate();
	virtual void	Release();

	float	GetAnimTimeInterval(void) const;

	// Get bone controller values.
	virtual void	GetBoneControllers(float controllers[MAXSTUDIOBONECTRLS]);
	virtual float	SetBoneController(int iController, float flValue);
	LocalFlexController_t GetNumFlexControllers(void);
	const char *GetFlexDescFacs(int iFlexDesc);
	const char *GetFlexControllerName(LocalFlexController_t iFlexController);
	const char *GetFlexControllerType(LocalFlexController_t iFlexController);

	virtual void	GetAimEntOrigin(IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles);

	// Computes a box that surrounds all hitboxes
	bool ComputeHitboxSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs);
	bool ComputeEntitySpaceHitboxSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs);

	// Gets the hitbox-to-world transforms, returns false if there was a problem
	bool HitboxToWorldTransforms(matrix3x4_t *pHitboxToWorld[MAXSTUDIOBONES]);

	// base model functionality
	float		  ClampCycle(float cycle, bool isLooping);
	virtual void GetPoseParameters(CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM]);
	virtual void CalcBoneMerge(CStudioHdr *hdr, int boneMask, CBoneBitList &boneComputed);
	virtual void BuildTransformations(CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed);
	void BuildJiggleTransformations(int boneIndex, const mstudiojigglebone_t *jiggleParams, const matrix3x4_t &goalMX);
	virtual void ApplyBoneMatrixTransform(matrix3x4_t& transform);
	virtual int	VPhysicsGetObjectList(IPhysicsObject **pList, int listMax);

	// model specific
	virtual bool SetupBones(matrix3x4a_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	virtual void UpdateIKLocks(float currentTime);
	virtual void CalculateIKLocks(float currentTime);
	virtual int DrawModel(int flags, const RenderableInstance_t &instance);
	virtual int	InternalDrawModel(int flags, const RenderableInstance_t &instance);
	virtual bool OnInternalDrawModel(ClientModelRenderInfo_t *pInfo);
	virtual bool OnPostInternalDrawModel(ClientModelRenderInfo_t *pInfo);
	void		DoInternalDrawModel(ClientModelRenderInfo_t *pInfo, DrawModelState_t *pState, matrix3x4_t *pBoneToWorldArray = NULL);
	void DrawWireFrame()
	{
		//if (IsRagdoll())
		//{
		//	m_pRagdoll->DrawWireframe();
		//}
		//else if (IsSolid() && CollisionProp()->GetSolid() == SOLID_VPHYSICS)
		//{
			vcollide_t *pCollide = modelinfo->GetVCollide(model);
			printf("pCollide: 0x%X\n", pCollide);
			if (pCollide /*&& pCollide->solidCount == 1*/)
			{
				static color32 debugColor = { 0, 255, 255, 255 };
				matrix3x4_t matrix;
				AngleMatrix(GetAbsAngles(), GetAbsOrigin(), matrix);
				engine->DebugDrawPhysCollide(pCollide->solids[0], NULL, matrix, debugColor);
			}
		//}
	}

	//
	virtual CMouthInfo *GetMouth();
	virtual void	ControlMouth(CStudioHdr *pStudioHdr);

	// override in sub-classes
	virtual void DoAnimationEvents(CStudioHdr *pStudio);
	virtual void FireEvent(const Vector& origin, const QAngle& angles, int event, const char *options);
	virtual void FireObsoleteEvent(const Vector& origin, const QAngle& angles, int event, const char *options);

	// Parses and distributes muzzle flash events
	virtual bool DispatchMuzzleEffect(const char *options, bool isFirstPerson);
	virtual void EjectParticleBrass(const char *pEffectName, const int iAttachment);

	// virtual	void AllocateMaterials( void );
	// virtual	void FreeMaterials( void );

	virtual CStudioHdr *OnNewModel(void);
	CStudioHdr	*GetModelPtr() const;
	void InvalidateMdlCache();

	virtual void SetPredictable(bool state);
	void UseClientSideAnimation();
	bool	IsUsingClientSideAnimation()	{ return m_bClientSideAnimation; }

	// C_BaseClientShader **p_ClientShaders;

	virtual	void StandardBlendingRules(CStudioHdr *pStudioHdr, Vector pos[], QuaternionAligned q[], float currentTime, int boneMask);
	void UnragdollBlend(CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime);

	void MaintainSequenceTransitions(IBoneSetup &boneSetup, float flCycle, Vector pos[], Quaternion q[]);
	virtual void AccumulateLayers(IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime);

	// Attachments
	virtual int	LookupAttachment(const char *pAttachmentName)
	{
		CStudioHdr* pStudioHdr = GetModelPtr();
		if (pStudioHdr && pStudioHdr->SequencesAvailable())
		{
			// Extract the bone index from the name
			for (int i = 0; i < pStudioHdr->GetNumAttachments(); i++)
			{
				if (!V_stricmp(pAttachmentName, pStudioHdr->pAttachment(i).pszName()))
				{
					return i + 1;
				}
			}
		}
		return 0;
	}

	int		LookupRandomAttachment(const char *pAttachmentNameSubstring);

	int		LookupPoseParameter(CStudioHdr *pStudioHdr, const char *szName);
	inline int LookupPoseParameter(const char *szName) { return LookupPoseParameter(GetModelPtr(), szName); }

	float	SetPoseParameter(CStudioHdr *pStudioHdr, const char *szName, float flValue);
	inline float SetPoseParameter(const char *szName, float flValue) { return SetPoseParameter(GetModelPtr(), szName, flValue); }
	float	SetPoseParameter(CStudioHdr *pStudioHdr, int iParameter, float flValue);
	inline float SetPoseParameter(int iParameter, float flValue) { return SetPoseParameter(GetModelPtr(), iParameter, flValue); }
	float	GetPoseParameter(int iParameter);

	float	GetPoseParameterRaw(int iPoseParameter);  // returns raw 0..1 value
	bool	GetPoseParameterRange(int iPoseParameter, float &minValue, float &maxValue);

	int		LookupBone(const char *szName);

	void GetBoneTransform(int iBone, matrix3x4_t &pBoneToWorld)
	{
		CStudioHdr *hdr = GetModelPtr();
		bool bWrote = false;
		if (hdr && iBone >= 0 && iBone < hdr->numbones())
		{
			if (hdr->boneFlags(iBone) & BONE_USED_BY_HITBOX)
			{
				GetCachedBoneMatrix(iBone, pBoneToWorld);
				bWrote = true;
			}
		}
		if (!bWrote)
		{
			MatrixCopy(EntityToWorldTransform(), pBoneToWorld);
		}
		Assert(GetModelPtr() && iBone >= 0 && iBone < GetModelPtr()->numbones());
	}

	bool GetBonePosition(int iBone, Vector &origin)
	{
		if (iBone < 0) 
			return false;

		QAngle angles;
		matrix3x4_t bonetoworld = GetBone(iBone);
		//GetBoneTransform(iBone, bonetoworld);
		MatrixAngles(bonetoworld, angles, origin);

		return true;
	}

	bool GetBoneAnglePosition(int iBone, Vector &origin, QAngle& angles)
	{
		if (iBone < 0)
			return false;
		matrix3x4_t bonetoworld;
		GetBoneTransform(iBone, bonetoworld);
		MatrixAngles(bonetoworld, angles, origin);

		return true;
	}

	void	CopySequenceTransitions(C_BaseAnimating *pCopyFrom);
	//bool solveIK(float a, float b, const Vector &Foot, const Vector &Knee1, Vector &Knee2);
	//void DebugIK( mstudioikchain_t *pikchain );

	virtual void					PreDataUpdate(DataUpdateType_t updateType);
	virtual void					PostDataUpdate(DataUpdateType_t updateType);

	virtual void					NotifyShouldTransmit(ShouldTransmitState_t state);
	virtual void					OnPreDataChanged(DataUpdateType_t updateType);
	virtual void					OnDataChanged(DataUpdateType_t updateType);

	// This can be used to force client side animation to be on. Only use if you know what you're doing!
	// Normally, the server entity should set this.
	void							ForceClientSideAnimationOn();

	void							AddToClientSideAnimationList();
	void							RemoveFromClientSideAnimationList();

	virtual bool					IsSelfAnimating();
	virtual void					ResetLatched();

	// implements these so ragdolls can handle frustum culling & leaf visibility
	virtual void					GetRenderBounds(Vector& theMins, Vector& theMaxs);
	virtual const Vector&			GetRenderOrigin(void);
	virtual const QAngle&			GetRenderAngles(void);

	virtual bool					GetSoundSpatialization(SpatializationInfo_t& info);

	// Attachments.
	bool							GetAttachment(const char *szName, Vector &absOrigin);
	bool							GetAttachment(const char *szName, Vector &absOrigin, QAngle &absAngles);

	// Inherited from C_BaseEntity
	virtual bool					GetAttachment(int number, Vector &origin);
	virtual bool					GetAttachment(int number, Vector &origin, QAngle &angles);
	virtual bool					GetAttachment(int number, matrix3x4_t &matrix);
	virtual bool					GetAttachmentVelocity(int number, Vector &originVel, Quaternion &angleVel);
	virtual void					InvalidateAttachments();

	// Returns the attachment in local space
	bool							GetAttachmentLocal(int iAttachment, matrix3x4_t &attachmentToLocal);
	bool							GetAttachmentLocal(int iAttachment, Vector &origin, QAngle &angles);
	bool                            GetAttachmentLocal(int iAttachment, Vector &origin);

	// Should this object cast render-to-texture shadows?
	virtual ShadowType_t			ShadowCastType();

	// Should we collide?
	virtual CollideType_t			GetCollideType(void);

	virtual bool					TestCollision(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);
	virtual bool					TestHitboxes(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);

	// returns true if we are of type C_ClientRagdoll
	virtual bool					IsClientRagdoll() const { return false; }

	// returns true if we're currently being ragdolled
	bool IsRagdoll() const { return (m_pRagdoll != NULL); }
	virtual C_BaseAnimating			*BecomeRagdollOnClient();
	virtual C_ClientRagdoll			*CreateClientRagdoll(bool bRestoring = false);
	C_BaseAnimating					*CreateRagdollCopy();
	bool							InitAsClientRagdoll(const matrix3x4_t *pDeltaBones0, const matrix3x4_t *pDeltaBones1, const matrix3x4_t *pCurrentBonePosition, float boneDt);
	void							IgniteRagdoll(C_BaseAnimating *pSource);
	void							TransferDissolveFrom(C_BaseAnimating *pSource);
	virtual void					SaveRagdollInfo(int numbones, const matrix3x4_t &cameraTransform, CBoneAccessor &pBoneToWorld);
	virtual bool					RetrieveRagdollInfo(Vector *pos, Quaternion *q);
	virtual void					Clear(void);
	void							ClearRagdoll();
	void							CreateUnragdollInfo(C_BaseAnimating *pRagdoll);
	void							ForceSetupBonesAtTime(matrix3x4a_t *pBonesOut, float flTime);
	virtual void					GetRagdollInitBoneArrays(matrix3x4a_t *pDeltaBones0, matrix3x4a_t *pDeltaBones1, matrix3x4a_t *pCurrentBones, float boneDt);

	// For shadows rendering the correct body + sequence...
	virtual int GetBody()			{ return m_nBody; }
	virtual int GetSkin()			{ return m_nSkin; }

	bool							IsOnFire() { return ((GetFlags() & FL_ONFIRE) != 0); }
	float							GetFrozenAmount() { return m_flFrozen; }

	inline float					GetPlaybackRate() const;
	inline void						SetPlaybackRate(float rate);

	void							SetModelScale(float scale)
	{
		m_flModelScale = scale;
	}

	inline float					GetModelScale() const { return m_flModelScale; }
	inline bool						IsModelScaleFractional() const;  /// very fast way to ask if the model scale is < 1.0f  (faster than if (GetModelScale() < 1.0f) )

	int	GetSequence();
	void SetSequence(int nSequence);
	inline void	ResetSequence(int nSequence);
	void OnNewSequence();

	float GetSequenceGroundSpeed(CStudioHdr *pStudioHdr, int iSequence);
	inline float GetSequenceGroundSpeed(int iSequence) 
	{ 
		return GetSequenceGroundSpeed(GetModelPtr(), iSequence);
	}

	bool IsSequenceLooping(CStudioHdr *pStudioHdr, int iSequence);
	inline bool	IsSequenceLooping(int iSequence) 
	{ 
		return IsSequenceLooping(GetModelPtr(), iSequence); 
	}

	float							GetSequenceMoveDist(CStudioHdr *pStudioHdr, int iSequence);
	void							GetSequenceLinearMotion(int iSequence, Vector *pVec);
	float							GetSequenceLinearMotionAndDuration(int iSequence, Vector *pVec);
	bool							GetSequenceMovement(int nSequence, float fromCycle, float toCycle, Vector &deltaPosition, QAngle &deltaAngles);
	void							GetBlendedLinearVelocity(Vector *pVec);
	void							SetMovementPoseParams(const Vector &vecLocalVelocity, int iMoveX, int iMoveY, int iXSign = 1, int iYSign = 1);
	int								LookupSequence(const char *label);
	int								LookupActivity(const char *label);
	char const						*GetSequenceName(int iSequence);
	char const						*GetSequenceActivityName(int iSequence);
	Activity						GetSequenceActivity(int iSequence);
	virtual void					StudioFrameAdvance(); // advance animation frame to some time in the future

	bool ExtractBbox(int nSequence, Vector &mins, Vector &maxs)
	{
		if (!m_pStudioHdr)
			return 0;
		if (!m_pStudioHdr->SequencesAvailable())
			return 0;

		mstudioseqdesc_t	&seqdesc = m_pStudioHdr->pSeqdesc(nSequence);
		mins = seqdesc.bbmin;
		maxs = seqdesc.bbmax;
		return 1;
	}

	// Clientside animation
	virtual float					FrameAdvance(float flInterval = 0.0f);
	virtual float					GetSequenceCycleRate(CStudioHdr *pStudioHdr, int iSequence);
	virtual void					UpdateClientSideAnimation();
	void							ClientSideAnimationChanged();
	virtual unsigned int			ComputeClientSideAnimationFlags();
	virtual void					ReachedEndOfSequence() { return; }

	float GetGroundSpeed(void) { return m_flGroundSpeed; }
	void SetCycle(float flCycle) { m_flCycle = flCycle; }
	float GetCycle() const	{ return m_flCycle; }

	void SetSkin(int iSkin) { m_nSkin = iSkin; }
	void SetBody(int iBody) { m_nBody = iBody; }

	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);

	const char *GetBodygroupName(int iGroup);
	int FindBodygroupByName(const char *name);
	int GetBodygroupCount(int iGroup);
	int GetNumBodyGroups(void);

	void SetHitboxSet(int setnum) {  m_nHitboxSet = setnum; }

	void SetHitboxSetByName(const char *setname)
	{
		const studiohdr_t* pstudiohdr = m_pStudioHdr->GetRenderHdr();
		if (!pstudiohdr)
		{
			m_nHitboxSet = -1;
			return;
		}

		for (int i = 0; i < pstudiohdr->numhitboxsets; i++)
		{
			mstudiohitboxset_t *set = pstudiohdr->pHitboxSet(i);
			if (!set)
				continue;
			if (!stricmp(set->pszName(), setname))
			{
				m_nHitboxSet = i;
				return;
			}
		}
		m_nHitboxSet = -1;
	}

	int	GetHitboxSet(void) 
	{ 
		return m_nHitboxSet; 
	}
	
	char const* GetHitboxSetName(void)
	{
		const studiohdr_t* pstudiohdr = m_pStudioHdr->GetRenderHdr();
		if (!pstudiohdr)
			return "";
		mstudiohitboxset_t *set = pstudiohdr->pHitboxSet(m_nHitboxSet);
		if (!set)
			return "";
		return set->pszName();
	}

	int GetHitboxSetCount(void)
	{
		const studiohdr_t* pstudiohdr = m_pStudioHdr->GetRenderHdr();
		if (!pstudiohdr)
			return 0;
		return pstudiohdr->numhitboxsets;
	}

	void DrawClientHitboxes(float duration = 0.03f, bool monocolor = false)
	{
		static Vector hullcolor[8] =
		{
			Vector(1.0, 1.0, 1.0),
			Vector(1.0, 0.5, 0.5),
			Vector(0.5, 1.0, 0.5),
			Vector(1.0, 1.0, 0.5),
			Vector(0.5, 0.5, 1.0),
			Vector(1.0, 0.5, 1.0),
			Vector(0.5, 1.0, 1.0),
			Vector(1.0, 1.0, 1.0)
		};

		CStudioHdr *pStudioHdr = GetModelPtr();
		if (!pStudioHdr)
			return;

		mstudiohitboxset_t *set = pStudioHdr->pHitboxSet(m_nHitboxSet);
		if (!set)
			return;

		Vector position;
		QAngle angles;

		int r = 255;
		int g = 0;
		int b = 0;

		for (int i = 0; i < set->numhitboxes; i++)
		{
			mstudiobbox_t *pbox = set->pHitbox(i);
			GetBoneAnglePosition(pbox->bone, position, angles);

			int j = (pbox->group % 8);
			r = (int)(255.0f * hullcolor[j][0]);
			g = (int)(255.0f * hullcolor[j][1]);
			b = (int)(255.0f * hullcolor[j][2]);

			g_pDebugOverlay->AddBoxOverlay2(position, pbox->bbmin, pbox->bbmax, angles, Color(0, 0, 0, 0), Color(r, g, b, 255), g_pGlobals->frametime);
		}
	}
	
	void							DrawSkeleton(CStudioHdr const* pHdr, int iBoneMask) const;

	C_BaseAnimating*				FindFollowedEntity();

	virtual bool					IsActivityFinished(void) { return m_bSequenceFinished; }
	inline bool						IsSequenceFinished(void);
	inline bool						SequenceLoops(void) { return m_bSequenceLoops; }

	// All view model attachments origins are stretched so you can place entities at them and
	// they will match up with where the attachment winds up being drawn on the view model, since
	// the view models are drawn with a different FOV.
	//
	// If you're drawing something inside of a view model's DrawModel() function, then you want the
	// original attachment origin instead of the adjusted one. To get that, call this on the 
	// adjusted attachment origin.
	virtual void					UncorrectViewModelAttachment(Vector &vOrigin) {}

	// Call this if SetupBones() has already been called this frame but you need to move the entity and rerender.
	void InvalidateBoneCache();

	bool IsBoneCacheValid() const // Returns true if the bone cache is considered good for this frame.
	{
		return m_iMostRecentModelBoneCounter == g_iModelBoneCounter;
	}

	void GetCachedBoneMatrix(int boneIndex, matrix3x4_t &out)
	{
		MatrixCopy(GetBone(boneIndex), out);
	}

	// Wrappers for CBoneAccessor.
	const matrix3x4a_t&				GetBone(int iBone) const;
	matrix3x4a_t&					GetBoneForWrite(int iBone);
	matrix3x4a_t*					GetBoneArrayForWrite();

	// Used for debugging. Will produce asserts if someone tries to setup bones or
	// attachments before it's allowed.
	// Use the "AutoAllowBoneAccess" class to auto push/pop bone access.
	// Use a distinct "tag" when pushing/popping - asserts when push/pop tags do not match.
	struct AutoAllowBoneAccess
	{
		AutoAllowBoneAccess(bool bAllowForNormalModels, bool bAllowForViewModels);
		~AutoAllowBoneAccess(void);
	};
	static void						PushAllowBoneAccess(bool bAllowForNormalModels, bool bAllowForViewModels, char const *tagPush);
	static void						PopBoneAccess(char const *tagPop);
	static void						ThreadedBoneSetup();
	static bool						InThreadedBoneSetup();
	static void						InitBoneSetupThreadPool();
	static void						ShutdownBoneSetupThreadPool();
	void							MarkForThreadedBoneSetup();
	static void						SetupBonesOnBaseAnimating(C_BaseAnimating *&pBaseAnimating);

	// Invalidate bone caches so all SetupBones() calls force bone transforms to be regenerated.
	static void						InvalidateBoneCaches();

	// Purpose: My physics object has been updated, react or extract data
	virtual void					VPhysicsUpdate(IPhysicsObject *pPhysics);

	void							DisableMuzzleFlash();		// Turn off the muzzle flash (ie: signal that we handled the server's event).
	virtual void					DoMuzzleFlash();	// Force a muzzle flash event. Note: this only QUEUES an event, so
	// ProcessMuzzleFlashEvent will get called later.
	bool							ShouldMuzzleFlash() const;	// Is the muzzle flash event on?

	// This is called to do the actual muzzle flash effect.
	virtual void ProcessMuzzleFlashEvent();

	// Update client side animations
	static void UpdateClientSideAnimations();

	// Load the model's keyvalues section and create effects listed inside it
	void							InitModelEffects(void);

	// Sometimes the server wants to update the client's cycle to get the two to run in sync (for proper hit detection)
	virtual void					SetServerIntendedCycle(float intended) { intended; }
	virtual float					GetServerIntendedCycle(void) { return -1.0f; }

	// For prediction
	int								SelectWeightedSequence(int activity);
	int								SelectWeightedSequenceFromModifiers(Activity activity, CUtlSymbol *pActivityModifiers, int iModifierCount);
	void							ResetSequenceInfo(void);
	float							SequenceDuration(void);
	float							SequenceDuration(CStudioHdr *pStudioHdr, int iSequence);
	inline float					SequenceDuration(int iSequence) { return SequenceDuration(GetModelPtr(), iSequence); }
	int								FindTransitionSequence(int iCurrentSequence, int iGoalSequence, int *piDir);
	void							RagdollMoved(void);
	virtual void					GetToolRecordingState(KeyValues *msg);
	virtual void					CleanupToolRecordingState(KeyValues *msg);
	void							SetReceivedSequence(void);
	virtual bool					ShouldResetSequenceOnNewModel(void);
	virtual bool					IsViewModel() const; // View models say yes to this.
	virtual bool					IsViewModelOrAttachment() const; // viewmodel or viewmodelattachmentmodel or lowerbody
	void							EnableJiggleBones(void);
	void							DisableJiggleBones(void);
	void							ScriptSetPoseParameter(const char *szName, float fValue);

protected:
	// View models scale their attachment positions to account for FOV. To get the unmodified
	// attachment position (like if you're rendering something else during the view model's DrawModel call),
	// use TransformViewModelAttachmentToWorld.
	virtual void					FormatViewModelAttachment(int nAttachment, matrix3x4_t &attachmentToWorld) {}

	bool							IsBoneAccessAllowed() const;
	CMouthInfo&						MouthInfo();

	// Models used in a ModelPanel say yes to this
	virtual bool					IsMenuModel() const;

	// Allow studio models to tell C_BaseEntity what their m_nBody value is
	virtual int						GetStudioBody(void) { return m_nBody; }

	virtual bool					CalcAttachments();

	virtual bool					ComputeStencilState(ShaderStencilState_t *pStencilState);

	virtual bool					WantsInterpolatedVars() { return true; }

	virtual void					ResetSequenceLooping() { m_bSequenceFinished = false; }

private:
	// This method should return true if the bones have changed + SetupBones needs to be called
	virtual float					LastBoneChangedTime() { return FLT_MAX; }

	CBoneList*						RecordBones(CStudioHdr *hdr, matrix3x4_t *pBoneState);

	bool							PutAttachment(int number, const matrix3x4_t &attachmentToWorld);
	void							TermRopes();

	void							DelayedInitModelEffects(void);
	void							ParseModelEffects(KeyValues *modelKeyValues);

	void							UpdateRelevantInterpolatedVars();
	void							AddBaseAnimatingInterpolatedVars();
	void							RemoveBaseAnimatingInterpolatedVars();


	void							LockStudioHdr()
	{
		AUTO_LOCK(m_StudioHdrInitLock);
		const model_t *mdl = GetModel();
		if (mdl)
		{
			m_hStudioHdr = modelinfo->GetCacheHandle(mdl);
			if (m_hStudioHdr != MDLHANDLE_INVALID)
			{
				const studiohdr_t *pStudioHdr = mdlcache->LockStudioHdr(m_hStudioHdr);
				CStudioHdr *pStudioHdrContainer = NULL;
				if (!m_pStudioHdr)
				{
					if (pStudioHdr)
					{
						pStudioHdrContainer = new CStudioHdr;
						pStudioHdrContainer->Init(pStudioHdr, mdlcache);
					}
					else
					{
						m_hStudioHdr = MDLHANDLE_INVALID;
					}
				}
				else
				{
					pStudioHdrContainer = m_pStudioHdr;
				}

				Assert((pStudioHdr == NULL && pStudioHdrContainer == NULL) || pStudioHdrContainer->GetRenderHdr() == pStudioHdr);

				if (pStudioHdrContainer && pStudioHdrContainer->GetVirtualModel())
				{
					MDLHandle_t hVirtualModel = (MDLHandle_t)(int)(pStudioHdrContainer->GetRenderHdr()->virtualModel) & 0xffff;
					mdlcache->LockStudioHdr(hVirtualModel);
				}
				m_pStudioHdr = pStudioHdrContainer; // must be last to ensure virtual model correctly set up
			}
		}
	}

	void							UnlockStudioHdr();
	void							SetupBones_AttachmentHelper(CStudioHdr *pStudioHdr);

public: // Members
	void dump();
	char								pad_0x9DC[0xC];
	CRagdoll*							m_pRagdoll; // 0x9E8
	CBaseAnimating*						m_pClientsideRagdoll; // 0x9EC
	int									m_nHitboxSet; // 0x9F0 // Hitbox set to use (default 0)
	CSequenceTransitioner				m_SequenceTransitioner;
	int									m_nPrevSequence;
	CRangeCheckedVar<float,-2,2,0>		m_flCycle;
	float								m_flPlaybackRate;// Animation playback framerate
	int									m_nSkin;// Texture group to use
	int									m_nBody;// Object bodygroup
	int									m_nNewSequenceParity;
	int									m_nResetEventsParity;
	int									m_nPrevNewSequenceParity;
	int									m_nPrevResetEventsParity;
	float								m_flEncodedController[MAXSTUDIOBONECTRLS];
	unsigned char						m_nMuzzleFlashParity;
	CIKContext							*m_pIk;
	int									m_iEyeAttachment;
	bool								m_bStoreRagdollInfo;
	RagdollInfo_t						*m_pRagdollInfo;
	Vector								m_vecForce;
	int									m_nForceBone;
	unsigned long						m_iMostRecentModelBoneCounter;
	unsigned long						m_iMostRecentBoneSetupRequest;
	C_BaseAnimating *					m_pNextForThreadedBoneSetup;
	int									m_iPrevBoneMask;
	int									m_iAccumulatedBoneMask;
	CBoneAccessor						m_BoneAccessor;
	CThreadFastMutex					m_BoneSetupLock;
	ClientSideAnimationListHandle_t		m_ClientSideAnimationListHandle;
	bool								m_bClientSideFrameReset; // 0xA90 // Client-side animation
	char								pad_0xA91[0x37]; // 0xA91
	float								m_flFrozen; // 0xAC8
	bool								m_bCanUseFastPath; // Can we use the fast rendering path?
	float								m_flGroundSpeed;	// computed linear movement rate for current sequence
	float								m_flLastEventCheck;	// cycle index of when events were last checked
	bool								m_bSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	bool								m_bSequenceLoops;	// true if the sequence loops
	bool								m_bIsUsingRelativeLighting;
	CMouthInfo							m_mouth; // Mouth lipsync/envelope following values
	CNetworkVar(float, m_flModelScale);
	int									m_ScaleType;
	CUtlLinkedList<C_RopeKeyframe*, unsigned short> m_Ropes; // Ropes that got spawned when the model was created.
	int									m_nRestoreSequence;
	float								m_flPrevEventCycle; // event processing info
	int									m_nEventSequence;
	float								m_flPoseParameter[MAXSTUDIOPOSEPARAM]; // Animation blending factors
	CInterpolatedVarArray<float, MAXSTUDIOPOSEPARAM> m_iv_flPoseParameter;
	float								m_flOldPoseParameters[MAXSTUDIOPOSEPARAM];
	CInterpolatedVarArray<float, MAXSTUDIOBONECTRLS> m_iv_flEncodedController;
	float								m_flOldEncodedController[MAXSTUDIOBONECTRLS];
	bool								m_bClientSideAnimation; // Clientside animation
	bool								m_bLastClientSideFrameReset;
	Vector								m_vecPreRagdollMins;
	Vector								m_vecPreRagdollMaxs;
	bool								m_builtRagdoll;
	bool								m_bReceivedSequence;
	bool								m_bIsStaticProp;
	int									m_nSequence; // Current animation sequence
	CInterpolatedVar<CRangeCheckedVar<float,-2,2,0>> m_iv_flCycle; // Current cycle location from server
	float								m_flOldCycle;
	float								m_prevClientCycle;
	float								m_prevClientAnimTime;
	bool								m_bBonePolishSetup; // True if bone setup should latch bones for demo polish subsystem
	CBoneMergeCache*					m_pBoneMergeCache;	// This caches the strcmp lookups that it has to do
	int									m_nPrevSkin;
	float								m_flOldModelScale;
	int									m_nOldSequence;
	int									m_nPrevBody;
	CUtlVector<matrix3x4a_t, CUtlMemoryAligned<matrix3x4a_t, 16>> m_CachedBoneData; // never access this directly. Use m_BoneAccessor.
	float								m_flLastBoneSetupTime;
	CJiggleBones*						m_pJiggleBones;
	bool								m_isJiggleBonesEnabled;
	CUtlVector<CAttachmentData>			m_Attachments; // Calculated attachment points
	EHANDLE								m_hLightingOrigin;
	unsigned char						m_nOldMuzzleFlashParity;
	bool								m_bInitModelEffects;
	bool								m_bSuppressAnimSounds;
	mutable CStudioHdr*					m_pStudioHdr;
	mutable MDLHandle_t					m_hStudioHdr;
	CThreadFastMutex					m_StudioHdrInitLock;
	CUtlReference<CNewParticleEffect>	m_ejectBrassEffect;
	int									m_iEjectBrassAttachment;
	float								bigfloat1; // D40
	float								bigfloat2;
	float								bigfloat3;
	char								pad_0xD4C[0x4];// end of Animating, begninning of AnimatingOverlay
	// 0xD50

	//////////////// statics ///////////////////////////
	static bool						m_bBoneListInUse;
	static CBoneList				m_recordingBoneList;
	////////////////////////////////////////////////////
};

inline void C_BaseAnimating::dump()
{
	printf("C_BaseAnimating -> Address = 0x%X\n", (DWORD)this);
	LOG_BASEANIMATING("C_BaseAnimating -> Address = 0x%X", (DWORD)this);
	SHOUTMEMBERANIMATING("m_pRagdoll", m_pRagdoll);
	SHOUTMEMBERANIMATING("m_pClientsideRagdoll", m_pClientsideRagdoll);
	SHOUTMEMBERANIMATING("m_nHitboxSet", m_nHitboxSet);
	SHOUTMEMBERANIMATING("m_nPrevSequence", m_nPrevSequence);
	SHOUTMEMBERANIMATING("m_flCycle", m_flCycle);
	SHOUTMEMBERANIMATING("m_flPlaybackRate", m_flPlaybackRate);
	SHOUTMEMBERANIMATING("m_nSkin", m_nSkin);
	SHOUTMEMBERANIMATING("m_nBody", m_nBody);
	SHOUTMEMBERANIMATING("m_nNewSequenceParity", m_nNewSequenceParity);
	SHOUTMEMBERANIMATING("m_nResetEventsParity", m_nResetEventsParity);
	SHOUTMEMBERANIMATING("m_nPrevNewSequenceParity", m_nPrevNewSequenceParity);
	SHOUTMEMBERANIMATING("m_nPrevResetEventsParity", m_nPrevResetEventsParity);
	SHOUTMEMBERANIMATING("m_flEncodedController", m_flEncodedController[0]);
	SHOUTMEMBERANIMATING("m_nMuzzleFlashParity", m_nMuzzleFlashParity);
	SHOUTMEMBERANIMATING("m_pIk", m_pIk);
	SHOUTMEMBERANIMATING("m_iEyeAttachment", m_iEyeAttachment);
	SHOUTMEMBERANIMATING("m_bStoreRagdollInfo", m_bStoreRagdollInfo);
	SHOUTMEMBERANIMATING("m_pRagdollInfo", m_pRagdollInfo);
	SHOUTMEMBERANIMATING("m_vecForce", m_vecForce[0]);
	SHOUTMEMBERANIMATING("m_nForceBone", m_nForceBone);
	SHOUTMEMBERANIMATING("m_iMostRecentModelBoneCounter", m_iMostRecentModelBoneCounter);
	SHOUTMEMBERANIMATING("m_iMostRecentBoneSetupRequest", m_iMostRecentBoneSetupRequest);
	SHOUTMEMBERANIMATING("m_pNextForThreadedBoneSetup", m_pNextForThreadedBoneSetup);
	SHOUTMEMBERANIMATING("m_iPrevBoneMask", m_iPrevBoneMask);
	SHOUTMEMBERANIMATING("m_iAccumulatedBoneMask", m_iAccumulatedBoneMask);
	SHOUTMEMBERANIMATING("m_BoneAccessor.m_pAnimating", m_BoneAccessor.m_pAnimating);
	SHOUTMEMBERANIMATING("m_BoneAccessor.m_pBones", m_BoneAccessor.m_pBones);
	SHOUTMEMBERANIMATING("m_BoneAccessor.m_ReadableBones", m_BoneAccessor.m_ReadableBones);
	SHOUTMEMBERANIMATING("m_BoneAccessor.m_WritableBones", m_BoneAccessor.m_WritableBones);
	SHOUTMEMBERANIMATING("m_ClientSideAnimationListHandle", m_ClientSideAnimationListHandle);
	SHOUTMEMBERANIMATING("m_bClientSideFrameReset", m_bClientSideFrameReset);
	SHOUTMEMBERANIMATING("m_flFrozen", m_flFrozen);
	SHOUTMEMBERANIMATING("m_bCanUseFastPath", m_bCanUseFastPath);
	SHOUTMEMBERANIMATING("m_flGroundSpeed", m_flGroundSpeed);
	SHOUTMEMBERANIMATING("m_flLastEventCheck", m_flLastEventCheck);
	SHOUTMEMBERANIMATING("m_bSequenceFinished", m_bSequenceFinished);
	SHOUTMEMBERANIMATING("m_bSequenceLoops", m_bSequenceLoops);
	SHOUTMEMBERANIMATING("m_bIsUsingRelativeLighting", m_bIsUsingRelativeLighting);
	SHOUTMEMBERANIMATING("m_mouth.mouthopen", m_mouth.mouthopen);
	SHOUTMEMBERANIMATING("m_mouth.sndcount", m_mouth.sndcount);
	SHOUTMEMBERANIMATING("m_mouth.sndavg", m_mouth.sndavg);
	SHOUTMEMBERANIMATING("m_mouth.m_nVoiceSources", m_mouth.m_nVoiceSources);
	SHOUTMEMBERANIMATING("m_mouth.m_needsEnvelope", m_mouth.m_needsEnvelope);
	SHOUTMEMBERANIMATING("m_flModelScale", m_flModelScale);
	SHOUTMEMBERANIMATING("m_ScaleType", m_ScaleType);
	SHOUTMEMBERANIMATING("m_nRestoreSequence", m_nRestoreSequence);
	SHOUTMEMBERANIMATING("m_flPrevEventCycle", m_flPrevEventCycle);
	SHOUTMEMBERANIMATING("m_nEventSequence", m_nEventSequence);
	SHOUTMEMBERANIMATING("m_flPoseParameter[24]", m_flPoseParameter[0]);
	SHOUTMEMBERANIMATING("m_flOldPoseParameters[24]", m_flOldPoseParameters[0]);
	SHOUTMEMBERANIMATING("m_flOldEncodedController", m_flOldEncodedController[0]);
	SHOUTMEMBERANIMATING("m_bClientSideAnimation", m_bClientSideAnimation);
	SHOUTMEMBERANIMATING("m_bLastClientSideFrameReset", m_bLastClientSideFrameReset);
	SHOUTMEMBERANIMATING("m_vecPreRagdollMins", m_vecPreRagdollMins[0]);
	SHOUTMEMBERANIMATING("m_vecPreRagdollMaxs", m_vecPreRagdollMaxs[0]);
	SHOUTMEMBERANIMATING("m_builtRagdoll", m_builtRagdoll);
	SHOUTMEMBERANIMATING("m_bReceivedSequence", m_bReceivedSequence);
	SHOUTMEMBERANIMATING("m_bIsStaticProp", m_bIsStaticProp);
	SHOUTMEMBERANIMATING("m_nSequence", m_nSequence);
	SHOUTMEMBERANIMATING("m_flOldCycle", m_flOldCycle);
	SHOUTMEMBERANIMATING("m_prevClientCycle", m_prevClientCycle);
	SHOUTMEMBERANIMATING("m_prevClientAnimTime", m_prevClientAnimTime);
	SHOUTMEMBERANIMATING("m_bBonePolishSetup", m_bBonePolishSetup);
	SHOUTMEMBERANIMATING("m_pBoneMergeCache", m_pBoneMergeCache);
	SHOUTMEMBERANIMATING("m_nPrevSkin", m_nPrevSkin);
	SHOUTMEMBERANIMATING("m_flOldModelScale", m_flOldModelScale);
	SHOUTMEMBERANIMATING("m_nOldSequence", m_nOldSequence);
	SHOUTMEMBERANIMATING("m_nPrevBody", m_nPrevBody);
	SHOUTMEMBERANIMATING("m_flLastBoneSetupTime", m_flLastBoneSetupTime);
	SHOUTMEMBERANIMATING("m_pJiggleBones", m_pJiggleBones);
	SHOUTMEMBERANIMATING("m_isJiggleBonesEnabled", m_isJiggleBonesEnabled);
	SHOUTMEMBERANIMATING("m_hLightingOrigin", m_hLightingOrigin);
	SHOUTMEMBERANIMATING("m_nOldMuzzleFlashParity", m_nOldMuzzleFlashParity);
	SHOUTMEMBERANIMATING("m_bInitModelEffects", m_bInitModelEffects);
	SHOUTMEMBERANIMATING("m_bSuppressAnimSounds", m_bSuppressAnimSounds);
	SHOUTMEMBERANIMATING("m_pStudioHdr", m_pStudioHdr);
	SHOUTMEMBERANIMATING("m_hStudioHdr", m_hStudioHdr);
	SHOUTMEMBERANIMATING("m_iEjectBrassAttachment", m_iEjectBrassAttachment);
	SHOUTMEMBERANIMATING("bigfloat1", bigfloat1);
	SHOUTMEMBERANIMATING("bigfloat2", bigfloat2);
	SHOUTMEMBERANIMATING("bigfloat3", bigfloat3);
	printf("\n");
}

enum
{
	RAGDOLL_FRICTION_OFF = -2,
	RAGDOLL_FRICTION_NONE,
	RAGDOLL_FRICTION_IN,
	RAGDOLL_FRICTION_HOLD,
	RAGDOLL_FRICTION_OUT,
};

class C_ClientRagdoll : public C_BaseAnimating, public IPVSNotify
{
public:
	C_ClientRagdoll(bool bRestoring = true, bool fullInit = true);
public:
	DECLARE_CLASS(C_ClientRagdoll, C_BaseAnimating);
	DECLARE_DATADESC();

	// inherited from IClientUnknown
	virtual IClientModelRenderable*	GetClientModelRenderable();

	// inherited from IPVSNotify
	virtual void OnPVSStatusChanged(bool bInPVS);

	virtual void Release(void);
	virtual void SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights);
	virtual void ImpactTrace(trace_t *pTrace, int iDamageType, char *pCustomImpactName);
	void ClientThink(void);
	void ReleaseRagdoll(void) { m_bReleaseRagdoll = true; }
	bool ShouldSavePhysics(void) { return true; }
	virtual void	OnSave();
	virtual void	OnRestore();
	virtual int ObjectCaps(void) { return BaseClass::ObjectCaps() | FCAP_SAVE_NON_NETWORKABLE; }
	virtual IPVSNotify*				GetPVSNotifyInterface() { return this; }

	void	HandleAnimatedFriction(void);
	virtual void SUB_Remove(void);

	void	FadeOut(void);
	virtual float LastBoneChangedTime();
	// returns true if we are of type C_ClientRagdoll
	virtual bool					IsClientRagdoll() const { return true; }

	inline bool IsFadingOut() { return m_bFadingOut; }

	bool m_bFadeOut;
	bool m_bImportant;
	float m_flEffectTime;
	bool m_bReleaseRagdoll;
	int m_iCurrentFriction;
	int m_iMinFriction;
	int m_iMaxFriction;
	float m_flFrictionModTime;
	float m_flFrictionTime;

	int  m_iFrictionAnimState;

	bool m_bFadingOut;

	float m_flScaleEnd[NUM_HITBOX_FIRES];
	float m_flScaleTimeStart[NUM_HITBOX_FIRES];
	float m_flScaleTimeEnd[NUM_HITBOX_FIRES];
};

//-----------------------------------------------------------------------------
// Purpose: Serves the 90% case of calling SetSequence / ResetSequenceInfo.
//-----------------------------------------------------------------------------
inline void C_BaseAnimating::ResetSequence(int nSequence)
{
	SetSequence(nSequence);
	ResetSequenceInfo();
}

inline float C_BaseAnimating::GetPlaybackRate() const
{
	return m_flPlaybackRate * clamp(1.0f - m_flFrozen, 0.0f, 1.0f);
}

inline void C_BaseAnimating::SetPlaybackRate(float rate)
{
	m_flPlaybackRate = rate;
}

inline const matrix3x4a_t& C_BaseAnimating::GetBone(int iBone) const
{
	return m_BoneAccessor.GetBone(iBone);
}

inline matrix3x4a_t& C_BaseAnimating::GetBoneForWrite(int iBone)
{
	return m_BoneAccessor.GetBoneForWrite(iBone);
}

inline matrix3x4a_t* C_BaseAnimating::GetBoneArrayForWrite()
{
	return m_BoneAccessor.GetBoneArrayForWrite();
}

inline bool C_BaseAnimating::ShouldMuzzleFlash() const
{
	return m_nOldMuzzleFlashParity != m_nMuzzleFlashParity;
}

//-----------------------------------------------------------------------------
// Purpose: return a pointer to an updated studiomdl cache cache
//-----------------------------------------------------------------------------

inline CStudioHdr *C_BaseAnimating::GetModelPtr() const
{
	return (m_pStudioHdr && m_pStudioHdr->IsValid()) ? m_pStudioHdr : NULL;
}


inline void C_BaseAnimating::InvalidateMdlCache()
{
	UnlockStudioHdr();
	if (m_pStudioHdr != NULL)
	{
		delete m_pStudioHdr;
		m_pStudioHdr = NULL;
	}
}


inline bool C_BaseAnimating::IsModelScaleFractional() const   /// very fast way to ask if the model scale is < 1.0f
{
	COMPILE_TIME_ASSERT(sizeof(m_flModelScale) == sizeof(int));
	return *((const int *)&m_flModelScale) < 0x3f800000;
}

//-----------------------------------------------------------------------------
// Sequence access
//-----------------------------------------------------------------------------
inline int C_BaseAnimating::GetSequence()
{
	return m_nSequence;
}

inline bool C_BaseAnimating::IsSequenceFinished(void)
{
	return m_bSequenceFinished;
}

inline float C_BaseAnimating::SequenceDuration(void)
{
	return SequenceDuration(GetSequence());
}


//-----------------------------------------------------------------------------
// Mouth
//-----------------------------------------------------------------------------
inline CMouthInfo& C_BaseAnimating::MouthInfo()
{
	return m_mouth;
}


// FIXME: move these to somewhere that makes sense
void GetColumn(matrix3x4_t& src, int column, Vector &dest);
void SetColumn(Vector &src, int column, matrix3x4_t& dest);

EXTERN_RECV_TABLE(DT_BaseAnimating);


extern void DevMsgRT(char const* pMsg, ...);

#endif // C_BASEANIMATING_H
