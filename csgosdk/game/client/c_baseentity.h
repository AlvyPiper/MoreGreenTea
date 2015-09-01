//===== Copyright 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: A base class for the client-side representation of entities.
//
//			This class encompasses both entities that are created on the server
//			and networked to the client AND entities that are created on the
//			client.
//
// $NoKeywords: $
//===========================================================================//

#ifndef C_BASEENTITY_H
#define C_BASEENTITY_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "IClientEntityInternal.h"
#include "engine/IVModelRender.h"
#include "client_class.h"
#include "IClientShadowMgr.h"
#include "ehandle.h"
#include "iclientunknown.h"
#include "client_thinklist.h"
#if !defined( NO_ENTITY_PREDICTION )
#include "predictableid.h"
#endif
#include "soundflags.h"
#include "shareddefs.h"
#include "networkvar.h"
#include "sharedvar.h"
#include "tier1/interpolatedvar.h"
#include "collisionproperty.h"
#include "particle_property.h"
#include "toolframework/itoolentity.h"
#include "tier0/threadtools.h"
#include "vscript/ivscript.h"
#include "vscript_shared.h"
#include "../../public/PlayerState.h"
#include "glow_outline_effect.h"

#ifndef LOG_BASEENTITY
#define LOG_BASEENTITY
#endif

#define SHOUTMEMBERENTITY(memberName, member) if (typeid(member) == typeid(float)) { LOG_BASEENTITY("0x%X | %s = %f (size 0x%X)", offsetof(class C_BaseEntity,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class C_BaseEntity,  member), memberName, member, sizeof(member)); } else { LOG_BASEENTITY("0x%X | %s = %d (size 0x%X)", offsetof(class C_BaseEntity,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class C_BaseEntity,  member), memberName, member, sizeof(member)); }

class C_Team;
class IPhysicsObject;
class IClientVehicle;
class CPredictionCopy;
class C_BasePlayer;
struct studiohdr_t;
class CStudioHdr;
class CDamageModifier;
class IRecipientFilter;
class CUserCmd;
struct solid_t;
class ISave;
class IRestore;
class C_BaseAnimating;
class C_BaseAnimatingOverlay;
class C_AI_BaseNPC;
struct EmitSound_t;
class C_RecipientFilter;
class CTakeDamageInfo;
class C_BaseCombatCharacter;
class CEntityMapData;
class ConVar;
class CClientAlphaProperty;
struct CSoundParameters;

typedef unsigned int			AimEntsListHandle_t;

#ifndef AI_CriteriaSet
#define AI_CriteriaSet ResponseRules::CriteriaSet 
#endif
namespace ResponseRules
{
	class CriteriaSet;
	class IResponseSystem;
};
using ResponseRules::IResponseSystem;

#define		INVALID_AIMENTS_LIST_HANDLE		(AimEntsListHandle_t)~0

extern void RecvProxy_LocalVelocity(const CRecvProxyData *pData, void *pStruct, void *pOut);

enum CollideType_t
{
	ENTITY_SHOULD_NOT_COLLIDE = 0,
	ENTITY_SHOULD_COLLIDE,
	ENTITY_SHOULD_RESPOND
};

class VarMapEntry_t
{
public:
	unsigned short		type;
	unsigned short		m_bNeedsToInterpolate;	// Set to false when this var doesn't
	// need Interpolate() called on it anymore.
	void				*data;
	IInterpolatedVar	*watcher;
};

struct VarMapping_t
{
	VarMapping_t()
	{
		m_nInterpolatedEntries = 0;
	}

	CUtlVector< VarMapEntry_t >	m_Entries;
	int							m_nInterpolatedEntries;
	float						m_lastInterpolationTime;
};



#define DECLARE_INTERPOLATION()


// How many data slots to use when in multiplayer.
#define MULTIPLAYER_BACKUP			150


struct serialentity_t;

typedef CHandle<C_BaseEntity> EHANDLE; // The client's version of EHANDLE.

typedef void (C_BaseEntity::*BASEPTR)(void);
typedef void (C_BaseEntity::*ENTITYFUNCPTR)(C_BaseEntity *pOther);

// For entity creation on the client
typedef C_BaseEntity* (*DISPATCHFUNCTION)(void);

#include "touchlink.h"
#include "groundlink.h"

#if !defined( NO_ENTITY_PREDICTION ) && defined( USE_PREDICTABLEID )
//-----------------------------------------------------------------------------
// Purpose: For fully client side entities we use this information to determine
//  authoritatively if the server has acknowledged creating this entity, etc.
//-----------------------------------------------------------------------------
struct PredictionContext
{
	PredictionContext()
	{
		m_bActive					= false;
		m_nCreationCommandNumber	= -1;
		m_pszCreationModule			= NULL;
		m_nCreationLineNumber		= 0;
		m_hServerEntity				= NULL;
	}

	// The command_number of the usercmd which created this entity
	bool						m_bActive;
	int							m_nCreationCommandNumber;
	char const					*m_pszCreationModule;
	int							m_nCreationLineNumber;
	// The entity to whom we are attached
	CHandle< C_BaseEntity >		m_hServerEntity;
};
#endif

//-----------------------------------------------------------------------------
// Purpose: think contexts
//-----------------------------------------------------------------------------
struct thinkfunc_t
{
	BASEPTR		m_pfnThink;
	string_t	m_iszContext;
	int			m_nNextThinkTick;
	int			m_nLastThinkTick;
};

#define CREATE_PREDICTED_ENTITY( className )	C_BaseEntity::CreatePredictedEntityByName( className, __FILE__, __LINE__ );


// Entity flags that only exist on the client.
#define ENTCLIENTFLAG_GETTINGSHADOWRENDERBOUNDS	0x0001		// Tells us if we're getting the real ent render bounds or the shadow render bounds.
#define ENTCLIENTFLAG_DONTUSEIK					0x0002		// Don't use IK on this entity even if its model has IK.
#define ENTCLIENTFLAG_ALWAYS_INTERPOLATE		0x0004		// Used by view models.

enum entity_list_ids_t
{
	ENTITY_LIST_INTERPOLATE = 0,
	ENTITY_LIST_TELEPORT,
	ENTITY_LIST_PRERENDER,
	ENTITY_LIST_SIMULATE,
	ENTITY_LIST_DELETE,
	NUM_ENTITY_LISTS
};

//-----------------------------------------------------------------------------
// Purpose: Base client side entity object
//-----------------------------------------------------------------------------
class C_BaseEntity : public IClientEntity
{
public:
	DECLARE_CLASS_NOBASE(C_BaseEntity);
	//friend class CPrediction;
	//friend void cc_cl_interp_all_changed(IConVar *pConVar, const char *pOldString, float flOldValue);

	DECLARE_DATADESC();
	//DECLARE_CLIENTCLASS();
	//DECLARE_PREDICTABLE();
	//DECLARE_ENT_SCRIPTDESC();

	C_BaseEntity();

protected:
	~C_BaseEntity();

public:
	//static C_BaseEntity				*CreatePredictedEntityByName(const char *classname, const char *module, int line, bool persist = false);
	//static void						UpdateVisibilityAllEntities();
	////virtual void					ModifyOrAppendCriteria(AI_CriteriaSet& set);
	//// FireBullets uses shared code for prediction.
	//virtual void					FireBullets(const FireBulletsInfo_t &info);
	//virtual bool					ShouldDrawUnderwaterBulletBubbles();
	//virtual bool					ShouldDrawWaterImpacts(void) { return true; }
	//virtual bool					HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest);
	//virtual ITraceFilter*			GetBeamTraceFilter(void);
	//virtual void					DispatchTraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	//virtual void					TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	//virtual void					DoImpactEffect(trace_t &tr, int nDamageType);
	//virtual void					MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType);
	//virtual int						GetTracerAttachment(void);
	//void							ComputeTracerStartPosition(const Vector &vecShotSrc, Vector *pVecTracerStart);
	//void							TraceBleed(float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType);
	//virtual int						BloodColor();
	//virtual const char*				GetTracerType();

	// called when entity is damaged by predicted attacks
	//virtual void					TakeDamage(const CTakeDamageInfo &info) { }
	//virtual void					Spawn(void);
	//virtual void					SpawnClientEntity(void);
	//virtual void					Precache(void);
	//virtual void					Activate();
	//void							ParseMapData(CEntityMapData *mapData);
	//virtual void					OnParseMapDataFinished();
	//virtual bool					KeyValue(const char *szKeyName, const char *szValue);
	//virtual bool					KeyValue(const char *szKeyName, float flValue);
	//virtual bool					KeyValue(const char *szKeyName, int nValue);
	//virtual bool					KeyValue(const char *szKeyName, const Vector &vecValue);
	//virtual bool					GetKeyValue(const char *szKeyName, char *szValue, int iMaxLen);
	//virtual void					InitSharedVars();
	//virtual void					Init(int, int);

	// Entities block Line-Of-Sight for NPCs by default.
	// Set this to false if you want to change this behavior.
	//void							SetBlocksLOS(bool bBlocksLOS);
	//bool							BlocksLOS(void);
	//void							SetAIWalkable(bool bBlocksLOS);
	//bool							IsAIWalkable(void);

public:
	void					SetRefEHandle(const CBaseHandle &handle)	{ m_RefEHandle = handle; };
	const CBaseHandle&		GetRefEHandle() const						{ return m_RefEHandle; };
	ICollideable*			GetCollideable()							{ return &m_Collision; }
	IClientNetworkable*		GetClientNetworkable()						{ return (IClientNetworkable*)(this + 0x8); }
	IClientRenderable*		GetClientRenderable()						{ return (IClientRenderable*)(this + 0x4); }
	IClientEntity*			GetIClientEntity()							{ return this; }
	C_BaseEntity*			GetBaseEntity()								{ return this; }
	IClientThinkable*		GetClientThinkable()						{ return (IClientThinkable*)(this + 0xC); }
	IClientAlphaProperty*	GetClientAlphaProperty()					{ return m_pClientAlphaProperty; }

protected:
	void							Release() {  }

public:
	const Vector&					GetAbsOrigin(void) const { return m_vecAbsOrigin; };
	const QAngle&					GetAbsAngles(void) const { return m_angRotation; };

	class CMouthInfo*				GetMouth(void) { return NULL; };
	bool							GetSoundSpatialization(SpatializationInfo_t& info) { return false; }
	bool							IsBlurred(void) { return m_bIsBlurred; }
	//virtual void					InitSharedVars(void) {};
	void							Interp_SetupMappings(VarMapping_t *map);

	// Returns 1 if there are no more changes (ie: we could call RemoveFromInterpolationList).
	int								Interp_Interpolate(VarMapping_t *map, float currentTime);
	void							Interp_RestoreToLastNetworked(VarMapping_t *map);
	void							Interp_UpdateInterpolationAmounts(VarMapping_t *map);
	void							Interp_HierarchyUpdateInterpolationAmounts();
	// Called by the CLIENTCLASS macros.
	//virtual bool					Init(int entnum, int iSerialNum);

	// Called in the destructor to shutdown everything.
	void							Term();

	// memory handling, uses calloc so members are zero'd out on instantiation
	void							*operator new(size_t stAllocateBlock);
	void							*operator new[](size_t stAllocateBlock);
	void							*operator new(size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine);
	void							*operator new[](size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine);
	void							operator delete(void *pMem);
	void							operator delete(void *pMem, int nBlockUse, const char *pFileName, int nLine) { operator delete(pMem); }


	// This just picks one of the routes to IClientUnknown.
	virtual IClientUnknown*			GetIClientUnknown()	{ return this; }
	virtual C_BaseAnimating*		GetBaseAnimating() { return (C_BaseAnimating*)this; }
	virtual C_BaseAnimatingOverlay*	GetBaseAnimatingOverlay() { return (C_BaseAnimatingOverlay*)this; };
	virtual void					SetClassname(const char *className);
	virtual Class_T					Classify() { return CLASS_NONE; }
	HSCRIPT							GetScriptInstance();

	// IClientUnknown overrides.
public:
	void							SetToolHandle(HTOOLHANDLE handle);
	HTOOLHANDLE						GetToolHandle() const;
	void							EnableInToolView(bool bEnable);
	bool							IsEnabledInToolView() const;
	void							SetToolRecording(bool recording);
	bool							IsToolRecording() const;
	bool							HasRecordedThisFrame() const;
	virtual void					RecordToolMessage();
	virtual void					OnToolStartRecording();
	// used to exclude entities from being recorded in the SFM tools
	void							DontRecordInTools();
	bool							ShouldRecordInTools() const;
	virtual							ResponseRules::IResponseSystem *GetResponseSystem();

public:
	virtual IClientModelRenderable*	GetClientModelRenderable()	{ return (IClientModelRenderable*)((DWORD)this + 0x4); }// (PVOID)(this + 0x4); }

	// Methods of IClientRenderable
	virtual const Vector&			GetRenderOrigin(void);
	virtual const QAngle&			GetRenderAngles(void);
	virtual Vector					GetObserverCamOrigin(void) { return GetRenderOrigin(); }	// Return the origin for player observers tracking this target
	virtual const matrix3x4_t &		RenderableToWorldTransform();
	virtual bool					IsTransparent(void);
	virtual int 					GetRenderFlags(void);
	virtual const model_t*			GetModel(void) const { return model; }
	virtual int						DrawModel(int flags, const RenderableInstance_t &instance);
	virtual bool					LODTest() { return true; }   // NOTE: UNUSED
	virtual void					GetRenderBounds(Vector& mins, Vector& maxs);
	virtual IPVSNotify*				GetPVSNotifyInterface();
	virtual void					GetRenderBoundsWorldspace(Vector& absMins, Vector& absMaxs);
	virtual void					GetShadowRenderBounds(Vector &mins, Vector &maxs, ShadowType_t shadowType);
	virtual void					GetColorModulation(float* color); // Determine the color modulation amount
	virtual void					OnThreadedDrawSetup() { }

public:
	virtual bool					TestCollision(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);
	virtual bool					TestHitboxes(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);

	// To mimic server call convention
	C_BaseEntity*					GetOwnerEntity(void) const;
	void							SetOwnerEntity(C_BaseEntity *pOwner);
	C_BaseEntity*					GetEffectEntity(void) const;
	void							SetEffectEntity(C_BaseEntity *pEffectEnt);
	bool							IsAbleToHaveFireEffect(void) const;
	// This function returns a value that scales all damage done by this entity.
	// Use CDamageModifier to hook in damage modifiers on a guy.
	virtual float					GetAttackDamageScale(void);

	// IClientNetworkable implementation.
public:
	virtual void					NotifyShouldTransmit(ShouldTransmitState_t state);
	// save out interpolated values
	virtual void					PreDataUpdate(DataUpdateType_t updateType);
	virtual void					PostDataUpdate(DataUpdateType_t updateType);
	virtual void					OnDataUnchangedInPVS(void);
	virtual void					ValidateModelIndex(void);
	// pvs info. NOTE: Do not override these!!
	virtual void					SetDormant(bool bDormant);
	virtual bool					IsDormant(void);
	virtual void					OnSetDormant(bool bDormant) {}

	// Tells the entity that it's about to be destroyed due to the client receiving
	// an uncompressed update that's caused it to destroy all entities & recreate them.
	virtual void					SetDestroyedOnRecreateEntities(void);
	virtual int						GetEFlags() const;
	virtual void					SetEFlags(int iEFlags);
	void							AddEFlags(int nEFlagMask);
	void							RemoveEFlags(int nEFlagMask);
	bool							IsEFlagSet(int nEFlagMask) const;
	// checks to see if the entity is marked for deletion
	bool							IsMarkedForDeletion(void);
	virtual int						entindex(void) const;
	// This works for client-only entities and returns the GetEntryIndex() of the entity's handle,
	// so the sound system can get an IClientEntity from it.
	int								GetSoundSourceIndex() const;
	// Server to client message received
	virtual void					ReceiveMessage(int classID, bf_read &msg);
	virtual void*					GetDataTableBasePtr();

	// IClientThinkable.
public:
	virtual void					ClientThink();
	virtual ClientThinkHandle_t		GetThinkHandle();
	virtual void					SetThinkHandle(ClientThinkHandle_t hThink);

public:
	void AddVar(void *data, IInterpolatedVar *watcher, int type, bool bSetup = false);
	void RemoveVar(void *data, bool bAssert = true);
	VarMapping_t*	GetVarMapping();

public:
	CCollisionProperty*			CollisionProp(); 	// An inline version the game code can use
	const CCollisionProperty*	CollisionProp() const;
	CParticleProperty*			ParticleProp();
	const CParticleProperty*	ParticleProp() const;
	CClientAlphaProperty*		AlphaProp();
	const CClientAlphaProperty*	AlphaProp() const;
	// Simply here for game shared 
	bool						IsFloating();
	virtual bool				ShouldSavePhysics();
	// save/restore stuff
	virtual void				OnSave();
	virtual void				OnRestore();
	// capabilities for save/restore
	virtual int					ObjectCaps(void);
	// only overload these if you have special data to serialize
	virtual int					Save(ISave &save);
	virtual int					Restore(IRestore &restore);

private:
	int							SaveDataDescBlock(ISave &save, datamap_t *dmap);
	int							RestoreDataDescBlock(IRestore &restore, datamap_t *dmap);
	// Called after spawn, and in the case of self-managing objects, after load
	virtual bool				CreateVPhysics();
	// Convenience routines to init the vphysics simulation for this object.
	// This creates a static object.  Something that behaves like world geometry - solid, but never moves
	IPhysicsObject*				VPhysicsInitStatic(void);
	// This creates a normal vphysics simulated object
	IPhysicsObject*				VPhysicsInitNormal(SolidType_t solidType, int nSolidFlags, bool createAsleep, solid_t *pSolid = NULL);
	// This creates a vphysics object with a shadow controller that follows the AI
	// Move the object to where it should be and call UpdatePhysicsShadowToCurrentPosition()
	IPhysicsObject*				VPhysicsInitShadow(bool allowPhysicsMovement, bool allowPhysicsRotation, solid_t *pSolid = NULL);

private:
	// called by all vphysics inits
	bool						VPhysicsInitSetup();

public:
	void						VPhysicsSetObject(IPhysicsObject *pPhysics);
	// destroy and remove the physics object for this entity
	virtual void				VPhysicsDestroyObject(void);
	// Purpose: My physics object has been updated, react or extract data
	virtual void				VPhysicsUpdate(IPhysicsObject *pPhysics);
	virtual void				VPhysicsShadowUpdate(IPhysicsObject *pPhysics);
	inline IPhysicsObject*		VPhysicsGetObject(void) const { return m_pPhysicsObject; }
	virtual int					VPhysicsGetObjectList(IPhysicsObject **pList, int listMax);
	virtual bool				VPhysicsIsFlesh(void);
	virtual void				VPhysicsCompensateForPredictionErrors(uchar const*);

	// IClientEntity implementation.
public:
	virtual bool				SetupBones(matrix3x4a_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	virtual void				SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights);
	virtual bool				UsesFlexDelayedWeights() { return false; }
	virtual void				DoAnimationEvents(void);	// see also GetVectors()
	inline Vector				Forward() const;			///< get my forward (+x) vector
	inline Vector				Left() const;				///< get my left    (+y) vector
	inline Vector				Up() const;					///< get my up      (+z) vector
	const Vector&				GetNetworkOrigin() const;
	const QAngle&				GetNetworkAngles() const;
	void						SetNetworkOrigin(const Vector& org);
	void						SetNetworkAngles(const QAngle& ang);
	const Vector&				GetLocalOrigin(void) const;
	void						SetLocalOrigin(const Vector& origin);
	vec_t						GetLocalOriginDim(int iDim) const;		// You can use the X_INDEX, Y_INDEX, and Z_INDEX defines here.
	void						SetLocalOriginDim(int iDim, vec_t flValue);
	const QAngle&				GetLocalAngles(void) const;
	void						SetLocalAngles(const QAngle& angles);
	vec_t						GetLocalAnglesDim(int iDim) const;		// You can use the X_INDEX, Y_INDEX, and Z_INDEX defines here.
	void						SetLocalAnglesDim(int iDim, vec_t flValue);
	virtual const Vector&		GetPrevLocalOrigin() const;
	virtual const QAngle&		GetPrevLocalAngles() const;
	virtual void				Teleport(const Vector*, const QAngle*, const Vector*);
	void						SetLocalTransform(const matrix3x4_t &localTransform);
	void						SetModelName(string_t name);
	string_t					GetModelName(void) const;
	int							GetModelIndex(void) const { return m_nModelIndex; };
	virtual void				SetModelIndex(int index);
	// These methods return a *world-aligned* box relative to the absorigin of the entity.
	// This is used for collision purposes and is *not* guaranteed
	// to surround the entire entity's visual representation
	// NOTE: It is illegal to ask for the world-aligned bounds for
	// SOLID_BSP objects
	virtual const Vector&		WorldAlignMins() const;
	virtual const Vector&		WorldAlignMaxs() const;
	// This defines collision bounds *in whatever space is currently defined by the solid type*
	//	SOLID_BBOX:		World Align
	//	SOLID_OBB:		Entity space
	//	SOLID_BSP:		Entity space
	//	SOLID_VPHYSICS	Not used
	void						SetCollisionBounds(const Vector& mins, const Vector &maxs);
	// NOTE: These use the collision OBB to compute a reasonable center point for the entity
	virtual const Vector&		WorldSpaceCenter() const;
	// FIXME: Do we want this?
	const Vector&				WorldAlignSize() const;
	bool						IsPointSized() const;
	// Returns a radius of a sphere 
	// *centered at the world space center* bounding the collision representation 
	// of the entity. NOTE: The world space center *may* move when the entity rotates.
	float						BoundingRadius() const;
	// Used when the collision prop is told to ask game code for the world-space surrounding box
	virtual void				ComputeWorldSpaceSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs);
	// Returns the entity-to-world transform
	matrix3x4_t&				EntityToWorldTransform();
	const matrix3x4_t&			EntityToWorldTransform() const;
	// Some helper methods that transform a point from entity space to world space + back
	void						EntityToWorldSpace(const Vector &in, Vector *pOut) const;
	void						WorldToEntitySpace(const Vector &in, Vector *pOut) const;
	// This function gets your parent's transform. If you're parented to an attachment,
	// this calculates the attachment's transform and gives you that.
	// You must pass in tempMatrix for scratch space - it may need to fill that in and return it instead of 
	// pointing you right at a variable in your parent.
	matrix3x4_t&				GetParentToWorldTransform(matrix3x4_t &tempMatrix);
	virtual void				GetVectors(Vector* forward, Vector* right, Vector* up) const;
	// Sets abs angles, but also sets local angles to be appropriate
	void						SetAbsOrigin(const Vector& origin);
	void						SetAbsAngles(const QAngle& angles);
	void						AddFlag(int flags);
	void						RemoveFlag(int flagsToRemove);
	void						ToggleFlag(int flagToToggle);
	int							GetFlags(void) const;
	void						ClearFlags();
	void						SetDistanceFade(float flMinDist, float flMaxDist);
	void						SetGlobalFadeScale(float flFadeScale);
	float						GetMinFadeDist() const;
	float						GetMaxFadeDist() const;
	float						GetGlobalFadeScale() const;
	MoveType_t					GetMoveType(void) const;
	MoveCollide_t				GetMoveCollide(void) const;
	virtual SolidType_t			GetSolid(void) const;
	virtual int			 		GetSolidFlags(void) const;
	bool						IsSolidFlagSet(int flagMask) const;
	void						SetSolidFlags(int nFlags);
	void						AddSolidFlags(int nFlags);
	void						RemoveSolidFlags(int nFlags);
	bool						IsSolid() const;
	// Attachments
	virtual int					LookupAttachment(const char *pAttachmentName) { return -1; }
	virtual bool				GetAttachment(int number, matrix3x4_t &matrix);
	virtual bool				GetAttachment(int number, Vector &origin);
	virtual	bool				GetAttachment(int number, Vector &origin, QAngle &angles);
	virtual void				ComputeLightingOrigin(int, Vector, const matrix3x4_t&, Vector&);
	virtual bool				GetAttachmentVelocity(int number, Vector &originVel, Quaternion &angleVel);
	virtual void				InvalidateAttachments() {}

	// Team handling
	virtual C_Team*				GetTeam(void);
	virtual int					GetTeamNumber(void) const;
	virtual int					GetPendingTeamNumber(void) const;
	virtual void				ChangeTeam(int iTeamNum);			// Assign this entity to a team.
	virtual int					GetRenderTeamNumber(void);
	virtual bool				InSameTeam(C_BaseEntity *pEntity);	// Returns true if the specified entity is on the same team as this one
	virtual bool				InLocalTeam(void);

	// ID Target handling
	virtual bool				IsValidIDTarget(void) { return false; }
	virtual char*				GetIDString(void) { return ""; };

	// See CSoundEmitterSystem
	void						EmitSound(const char *soundname, float soundtime = 0.0f, float *duration = NULL);  // Override for doing the general case of CPASAttenuationFilter( this ), and EmitSound( filter, entindex(), etc. );
	void						EmitSound(const char *soundname, HSOUNDSCRIPTHANDLE& handle, float soundtime = 0.0f, float *duration = NULL);  // Override for doing the general case of CPASAttenuationFilter( this ), and EmitSound( filter, entindex(), etc. );
	void						StopSound(const char *soundname);
	void						StopSound(const char *soundname, HSOUNDSCRIPTHANDLE& handle);
	void						GenderExpandString(char const *in, char *out, int maxlen);
	// All statics
	static float				GetSoundDuration(const char *soundname, char const *actormodel);
	static bool					GetParametersForSound(const char *soundname, CSoundParameters &params, const char *actormodel);
	static bool					GetParametersForSound(const char *soundname, HSOUNDSCRIPTHANDLE& handle, CSoundParameters &params, const char *actormodel);
	static void					EmitSound(IRecipientFilter& filter, int iEntIndex, const char *soundname, const Vector *pOrigin = NULL, float soundtime = 0.0f, float *duration = NULL);
	static void					EmitSound(IRecipientFilter& filter, int iEntIndex, const char *soundname, HSOUNDSCRIPTHANDLE& handle, const Vector *pOrigin = NULL, float soundtime = 0.0f, float *duration = NULL);
	static void					StopSound(int iEntIndex, const char *soundname);
	static soundlevel_t			LookupSoundLevel(const char *soundname);
	static soundlevel_t			LookupSoundLevel(const char *soundname, HSOUNDSCRIPTHANDLE& handle);
	static void					EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t & params);
	static void					EmitSound(IRecipientFilter& filter, int iEntIndex, const EmitSound_t & params, HSOUNDSCRIPTHANDLE& handle);
	static void					StopSound(int iEntIndex, int iChannel, const char *pSample, bool bIsStoppingSpeakerSound = false);
	static void					EmitAmbientSound(int entindex, const Vector& origin, const char *soundname, int flags = 0, float soundtime = 0.0f, float *duration = NULL);
	// These files need to be listed in scripts/game_sounds_manifest.txt
	static HSOUNDSCRIPTHANDLE	PrecacheScriptSound(const char *soundname);
	static void					PrefetchScriptSound(const char *soundname);
	// For each client who appears to be a valid recipient, checks the client has disabled CC and if so, removes them from 
	//  the recipient list.
	static void					RemoveRecipientsIfNotCloseCaptioning(C_RecipientFilter& filter);
	static void					EmitCloseCaption(IRecipientFilter& filter, int entindex, char const *token, CUtlVector< Vector >& soundorigins, float duration, bool warnifmissing = false);
	// Moves all aiments into their correct position for the frame
	static void					MarkAimEntsDirty();
	static void					CalcAimEntPositions();
	static bool					IsPrecacheAllowed();
	static void					SetAllowPrecache(bool allow);
	static bool					m_bAllowPrecache;
	static bool					IsSimulatingOnAlternateTicks();
	static bool					sm_bAccurateTriggerBboxChecks;	// SOLID_BBOX entities do a fully accurate trigger vs bbox check when this is set

	// C_BaseEntity local functions
public:
	virtual void				UpdatePartitionListEntry();
	// This can be used to setup the entity as a client-only entity. 
	// Override this to perform per-entity clientside setup
	virtual bool				InitializeAsClientEntity(const char *pszModelName, bool bRenderWithViewModels);
	// This function gets called on all client entities once per simulation phase. If the entity
	// is in the simulate list.  It dispatches events like OnDataChanged()
	// return false if this entity no longer needs to simulate, true otherwise
	virtual bool				Simulate() { return false; }
	// This event is triggered during the simulation phase if an entity's data has changed. It is 
	// better to hook this instead of PostDataUpdate() because in PostDataUpdate(), server entity origins
	// are incorrect and attachment points can't be used.
	virtual void				OnDataChanged(DataUpdateType_t type);
	// This is called once per frame before any data is read in from the server.
	virtual void				OnPreDataChanged(DataUpdateType_t type);
	bool						IsStandable() const;
	bool						IsBSPModel() const;
	// If this is a vehicle, returns the vehicle interface
	virtual IClientVehicle*		GetClientVehicle() { return NULL; }
	// Returns the aiment render origin + angles
	virtual void				GetAimEntOrigin(IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles);
	// get network origin from previous update
	virtual const Vector&		GetOldOrigin();
	// Methods relating to traversing hierarchy
	C_BaseEntity*				GetMoveParent(void) const;
	C_BaseEntity*				GetRootMoveParent();
	C_BaseEntity*				FirstMoveChild(void) const;
	C_BaseEntity*				NextMovePeer(void) const;
	inline ClientEntityHandle_t	GetClientHandle() const	{ return ClientEntityHandle_t(m_RefEHandle); }
	inline bool					IsServerEntity(void);
	void						RenderWithViewModels(bool bEnable);
	bool						IsRenderingWithViewModels() const;
	void						DisableCachedRenderBounds(bool bDisabled);
	bool						IsCachedRenderBoundsDisabled() const;
	// NOTE: The goal of this function is different from IsTranslucent().
	// Here, we need to determine whether a renderable is inherently translucent
	// when run-time alpha modulation or any other game code is not taken into account
	virtual RenderableTranslucencyType_t ComputeTranslucencyType();
	virtual uint8				OverrideAlphaModulation(uint8 nAlpha) { return nAlpha; }
	virtual uint8				OverrideShadowAlphaModulation(uint8 nAlpha) { return nAlpha; }
	// Client code should call this under any circumstances where translucency type may change
	void						OnTranslucencyTypeChanged();
	// Client code should call this under any circumstances where splitscreen rendering may change
	void						OnSplitscreenRenderingChanged();
	virtual void				GetToolRecordingState(KeyValues *msg);
	virtual void				CleanupToolRecordingState(KeyValues *msg);
	// The value returned by here determines whether or not (and how) the entity
	// is put into the spatial partition.
	virtual CollideType_t		GetCollideType(void);
	virtual bool				ShouldDraw();
	virtual bool				ShouldSuppressForSplitScreenPlayer(int index);
	inline bool					IsVisible() const;
	inline bool					IsVisibleToAnyPlayer() const;
	void						UpdateVisibility();
	// Returns true if the entity changes its position every frame on the server but it doesn't
	// set animtime. In that case, the client returns true here so it copies the server time to
	// animtime in OnDataChanged and the position history is correct for interpolation.
	virtual bool				IsSelfAnimating();
	// Set appropriate flags and store off data when these fields are about to change
	virtual	void				OnLatchInterpolatedVariables(int flags);
	// For predictable entities, stores last networked value
	void						OnStoreLastNetworkedValue();
	// Initialize things given a new model.
	virtual CStudioHdr*			OnNewModel();
	virtual void				OnNewParticleEffect(const char *pszParticleName, CNewParticleEffect *pNewParticleEffect);
	virtual void				OnParticleEffectDeleted(CNewParticleEffect *pParticleEffect);
	bool						IsSimulatedEveryTick() const;
	bool						IsAnimatedEveryTick() const;
	void						SetSimulatedEveryTick(bool sim);
	void						SetAnimatedEveryTick(bool anim);
	void						Interp_Reset(VarMapping_t *map);
	virtual void				ResetLatched();
	//virtual float				GetInterpolationAmount(int flags);
	float						GetLastChangeTime(int flags);
	// Interpolate the position for rendering
	virtual bool				Interpolate(float currentTime);
	// Did the object move so far that it shouldn't interpolate?
	bool						Teleported(void);
	// Is this a submodel of the world ( *1 etc. in name ) ( brush models only )
	virtual bool				IsSubModel(void);
	// Deal with EF_* flags
	virtual bool				CreateLightEffects(void);
	void						AddToAimEntsList();
	void						RemoveFromAimEntsList();
	// Reset internal fields
	virtual void				Clear(void);
	// Helper to draw raw brush models
	virtual int					DrawBrushModel(bool bSort, bool bShadowDepth);
	// returns the material animation start time
	virtual float				GetTextureAnimationStartTime();
	// Indicates that a texture animation has wrapped
	virtual void				TextureAnimationWrapped();
	// Set the next think time. Pass in CLIENT_THINK_ALWAYS to have Think() called each frame.
	virtual void				SetNextClientThink(float nextThinkTime);
	// anything that has health can override this...
	virtual void				SetHealth(int iHealth) {}
	virtual int					GetHealth() const { return 0; }
	virtual int					GetMaxHealth() const { return 1; }

	// Returns the health fraction
	float						HealthFraction() const;
	// Should this object cast shadows?
	virtual ShadowType_t		ShadowCastType();
	// Should this object receive shadows?
	virtual bool				ShouldReceiveProjectedTextures(int flags);
	// Shadow-related methods
	virtual bool				IsShadowDirty();
	virtual void				MarkShadowDirty(bool bDirty);
	virtual IClientRenderable*	GetShadowParent();
	virtual IClientRenderable*	FirstShadowChild();
	virtual IClientRenderable*	NextShadowPeer();

	// Sets up a render handle so the leaf system will draw this entity.
	void						AddToLeafSystem();
	void						AddToLeafSystem(bool bRenderWithViewModels);
	// remove entity form leaf system again
	void						RemoveFromLeafSystem();

	// A method to apply a decal to an entity
	virtual void				AddDecal(const Vector& rayStart, const Vector& rayEnd, const Vector& decalCenter, int hitbox, int decalIndex,bool doTrace, trace_t& tr, int maxLODToDecal = ADDDECAL_TO_ALL_LODS);
	// A method to remove all decals from an entity
	void						RemoveAllDecals(void);
	// Is this a brush model?
	bool						IsBrushModel() const;
	// A random value 0-1 used by proxies to make sure they're not all in sync
	float						ProxyRandomValue() const { return m_flProxyRandomValue; }
	// The spawn time of this entity
	float						SpawnTime() const { return m_flSpawnTime; }
	virtual bool				IsClientCreated(void) const;
	virtual void				UpdateOnRemove(void);
	virtual void				SUB_Remove(void);

	// Prediction stuff
	void						CheckInitPredictable(const char *context);
	virtual C_BasePlayer*		GetPredictionOwner(void);
	void						AllocateIntermediateData(void);
	void						DestroyIntermediateData(void);
	void						ShiftIntermediateDataForward(int slots_to_remove, int previous_last_slot);
	void*						GetPredictedFrame(int framenumber);
	void*						GetOriginalNetworkDataObject(void);
	bool						IsIntermediateDataAllocated(void) const;
	virtual void				InitPredictable(C_BasePlayer *pOwner);
	void						ShutdownPredictable(void);
	int							GetSplitUserPlayerPredictionSlot();
	virtual void				SetPredictable(bool state);
	virtual void				HandlePredictionError(bool state);
	virtual bool				PredictionErrorShouldResetLatchedForAllPredictables();
	virtual bool				PredictionIsPhysicallySimulated();
	bool						GetPredictable(void) const;
	void						PreEntityPacketReceived(int commands_acknowledged);
	void						PostEntityPacketReceived(void);
	bool						PostNetworkDataReceived(int commands_acknowledged);
	bool						GetPredictionEligible(void) const;
	void						SetPredictionEligible(bool canpredict);
	enum						{ SLOT_ORIGINALDATA = -1, };
	void						SaveData(const char *context, int slot, int type);
	void						RestoreData(const char *context, int slot, int type);

	// Called after restoring data into prediction slots. This function is used in place of proxies
	// on the variables, so if some variable like m_nModelIndex needs to update other state (like 
	// the model pointer), it is done here.
	void						OnPostRestoreData();
	virtual char const *		DamageDecal(int bitsDamageType, int gameMaterial);
	virtual void				DecalTrace(trace_t *pTrace, char const *decalName);
	virtual void				ImpactTrace(trace_t *pTrace, int iDamageType, char *pCustomImpactName);
	virtual bool				ShouldPredict(void) { return false; };
	// interface function pointers
	void						(C_BaseEntity::*m_pfnThink)(void);
	virtual void				Think(void) { AssertMsg(m_pfnThink != &C_BaseEntity::Think, "Infinite recursion is infinitely bad."); if (m_pfnThink) (this->*m_pfnThink)(); }
	void						PhysicsDispatchThink(BASEPTR thinkFunc);

	// Toggle the visualization of the entity's abs/bbox
	enum { VISUALIZE_COLLISION_BOUNDS = 0x1, VISUALIZE_SURROUNDING_BOUNDS = 0x2, VISUALIZE_RENDER_BOUNDS = 0x4, };
	void						ClearBBoxVisualization(void);
	void						ToggleBBoxVisualization(int fVisFlags);
	void						DrawBBoxVisualizations(void);
	virtual bool				PreRender(int nSplitScreenPlayerSlot);
	bool						IsViewEntity() const;

	// Methods implemented on both client and server
public:
	void						SetSize(const Vector &vecMin, const Vector &vecMax); // UTIL_SetSize( pev, mins, maxs );
	virtual char const*			GetClassname(void);
	char const*					GetDebugName(void);
	virtual const char*			GetPlayerName() const { return NULL; }
	static int					PrecacheModel(const char *name);
	static bool					PrecacheSound(const char *name);
	static void					PrefetchSound(const char *name);
	void						Remove(); // UTIL_Remove( this );

public:
	// Returns the attachment point index on our parent that our transform is relative to.
	// 0 if we're relative to the parent's absorigin and absangles.
	unsigned char				GetParentAttachment() const;
	// Externalized data objects ( see sharreddefs.h for DataObjectType_t )
	bool						HasDataObjectType(int type) const;
	void						AddDataObjectType(int type);
	void						RemoveDataObjectType(int type);
	void*						GetDataObject(int type);
	void*						CreateDataObject(int type);
	void						DestroyDataObject(int type);
	void						DestroyAllDataObjects(void);
	// Determine approximate velocity based on updates from server
	virtual void				EstimateAbsVelocity(Vector& vel);

	#if !defined( NO_ENTITY_PREDICTION ) // The player drives simulation of this entity
	void						SetPlayerSimulated(C_BasePlayer *pOwner);
	bool						IsPlayerSimulated(void) const;
	CBasePlayer*				GetSimulatingPlayer(void);
	void						UnsetPlayerSimulated(void);
	#endif

	// Sorry folks, here lies TF2-specific stuff that really has no other place to go
	virtual bool				CanBePoweredUp(void) { return false; }
	virtual bool				AttemptToPowerup(int iPowerup, float flTime, float flAmount = 0, C_BaseEntity *pAttacker = NULL, CDamageModifier *pDamageModifier = NULL) { return false; }
	void						SetCheckUntouch(bool check);
	bool						GetCheckUntouch() const;
	virtual bool				IsCurrentlyTouching(void) const;
	virtual void				StartTouch(C_BaseEntity *pOther);
	virtual void				Touch(C_BaseEntity *pOther);
	virtual void				EndTouch(C_BaseEntity *pOther);
	void						(C_BaseEntity::*m_pfnTouch)(C_BaseEntity *pOther);
	void						PhysicsStep(void);

protected: // Static
	static bool					sm_bDisableTouchFuncs;	// Disables PhysicsTouch and PhysicsStartTouch function calls

public:
	touchlink_t*				PhysicsMarkEntityAsTouched(C_BaseEntity *other);
	void						PhysicsTouch(C_BaseEntity *pentOther);
	void						PhysicsStartTouch(C_BaseEntity *pentOther);
	// HACKHACK:Get the trace_t from the last physics touch call (replaces the even-hackier global trace vars)
	static const trace_t&		GetTouchTrace(void);
	// FIXME: Should be priv	ate, but I can't make em private just yet
	void						PhysicsImpact(C_BaseEntity *other, trace_t &trace);
	void						PhysicsMarkEntitiesAsTouching(C_BaseEntity *other, trace_t &trace);
	void						PhysicsMarkEntitiesAsTouchingEventDriven(C_BaseEntity *other, trace_t &trace);
	void						PhysicsTouchTriggers(const Vector *pPrevAbsOrigin = NULL);
	// Physics helper
	static void					PhysicsRemoveTouchedList(C_BaseEntity *ent);
	static void					PhysicsNotifyOtherOfUntouch(C_BaseEntity *ent, C_BaseEntity *other);
	static void					PhysicsRemoveToucher(C_BaseEntity *other, touchlink_t *link);
	groundlink_t*				AddEntityToGroundList(CBaseEntity *other);
	void						PhysicsStartGroundContact(CBaseEntity *pentOther);
	static void					PhysicsNotifyOtherOfGroundRemoval(CBaseEntity *ent, CBaseEntity *other);
	static void					PhysicsRemoveGround(CBaseEntity *other, groundlink_t *link);
	static void					PhysicsRemoveGroundList(CBaseEntity *ent);
	void						StartGroundContact(CBaseEntity *ground);
	void						EndGroundContact(CBaseEntity *ground);
	void						SetGroundChangeTime(float flTime);
	float						GetGroundChangeTime(void);

	// Remove this as ground entity for all object resting on this object
	void						WakeRestingObjects();
	bool						HasNPCsOnIt();
	bool						PhysicsCheckWater(void);
	void						PhysicsCheckVelocity(void);
	void						PhysicsAddHalfGravity(float timestep);
	void						PhysicsAddGravityMove(Vector &move);
	virtual unsigned int		PhysicsSolidMaskForEntity(void) const;
	void						SetGroundEntity(C_BaseEntity *ground);
	C_BaseEntity*				GetGroundEntity(void);
	C_BaseEntity*				GetGroundEntity(void) const { return const_cast<C_BaseEntity *>(this)->GetGroundEntity(); }
	void						PhysicsPushEntity(const Vector& push, trace_t *pTrace);
	void						PhysicsCheckWaterTransition(void);
	// Performs the collision resolution for fliers.
	void						PerformFlyCollisionResolution(trace_t &trace, Vector &move);
	void						ResolveFlyCollisionBounce(trace_t &trace, Vector &vecVelocity, float flMinTotalElasticity = 0.0f);
	void						ResolveFlyCollisionSlide(trace_t &trace, Vector &vecVelocity);
	void						ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity);
	void						PhysicsCheckForEntityUntouch(void);
	// Creates the shadow (if it doesn't already exist) based on shadow cast type
	void						CreateShadow();
	// Destroys the shadow; causes its type to be recomputed if the entity doesn't go away immediately.
	void						DestroyShadow();

protected:
	enum thinkmethods_t { THINK_FIRE_ALL_FUNCTIONS, THINK_FIRE_BASE_ONLY, THINK_FIRE_ALL_BUT_BASE, }; // think function handling

public:
	// Unlinks from hierarchy
	// Set the movement parent. Your local origin and angles will become relative to this parent.
	// If iAttachment is a valid attachment on the parent, then your local origin and angles 
	// are relative to the attachment on this entity.
	void						SetParent(C_BaseEntity *pParentEntity, int iParentAttachment = 0);
	bool						PhysicsRunThink(thinkmethods_t thinkMethod = THINK_FIRE_ALL_FUNCTIONS);
	bool						PhysicsRunSpecificThink(int nContextIndex, BASEPTR thinkFunc);
	virtual void				PhysicsSimulate(void);
	virtual bool				IsAlive(void);
	bool						IsInWorld(void) { return true; }
	bool						IsWorld() const { return entindex() == 0; }
	/////////////////
	virtual bool 				ShouldRegenerateOriginFromCellBits() const;
	virtual bool				IsPlayer(void) const { return false; };
	virtual bool				IsBaseCombatCharacter(void) { return false; };
	virtual C_BaseCombatCharacter*MyCombatCharacterPointer(void) { return (CBaseCombatCharacter*)this; }
	virtual bool				IsNPC(void) { return false; }
	C_AI_BaseNPC*				MyNPCPointer(void);
	virtual bool				IsSprite(void) const { return false; }
	virtual bool				IsProp(void) const { return false; }
	// TF2 specific
	virtual bool				IsBaseObject(void) const { return false; }
	virtual bool				IsBaseCombatWeapon(void) const { return false; }
	virtual class C_BaseCombatWeapon*MyCombatWeaponPointer() { return NULL; }
	// Entities like the player, weapon models, and view models have special logic per-view port related to visibility and the model to be used, etc.
	virtual bool				ShouldDrawForSplitScreenUser(int nSlot);
	void						SetBlurState(bool bShouldBlur);
	virtual bool				IsBaseTrain(void) const { return false; }
	// Returns the eye point + angles (used for viewing + shooting)
	virtual Vector				EyePosition(void);
	virtual const QAngle&		EyeAngles(void);
	virtual const QAngle&		LocalEyeAngles(void);	// Direction of eyes in local space (pl.v_angle)
	// position of ears
	virtual Vector				EarPosition(void);
	Vector						EyePosition(void) const; // position of eyes
	const QAngle&				EyeAngles(void) const; // Direction of eyes in world space
	const QAngle&				LocalEyeAngles(void) const;	// Direction of eyes
	Vector						EarPosition(void) const; // position of ears
	// Called by physics to see if we should avoid a collision test....
	virtual bool				ShouldCollide(int collisionGroup, int contentsMask) const;
	// Sets physics parameters
	void						SetFriction(float flFriction);
	void						SetGravity(float flGravity);
	float						GetGravity(void) const;
	// Sets the model from a model index 
	void						SetModelByIndex(int nModelIndex);
	// Set model... (NOTE: Should only be used by client-only entities
	// Returns false if the model name is bogus
	bool						SetModel(const char *pModelName);
	virtual void				SetModelPointer(const model_t *pModel);
	// Access movetype and solid.
	void						SetMoveType(MoveType_t val, MoveCollide_t moveCollide = MOVECOLLIDE_DEFAULT);	// Set to one of the MOVETYPE_ defines.
	void						SetMoveCollide(MoveCollide_t val);	// Set to one of the MOVECOLLIDE_ defines.
	void						SetSolid(SolidType_t val);	// Set to one of the SOLID_ defines.
	// NOTE: Setting the abs velocity in either space will cause a recomputation
	// in the other space, so setting the abs velocity will also set the local vel
	void						SetLocalVelocity(const Vector &vecVelocity);
	void						SetAbsVelocity(const Vector &vecVelocity);
	const Vector&				GetLocalVelocity() const;
	const Vector&				GetAbsVelocity() const;	
	void						ApplyLocalVelocityImpulse(const Vector &vecImpulse);
	void						ApplyAbsVelocityImpulse(const Vector &vecImpulse);
	void						ApplyLocalAngularVelocityImpulse(const AngularImpulse &angImpulse);
	// NOTE: Setting the abs velocity in either space will cause a recomputation
	// in the other space, so setting the abs velocity will also set the local vel
	void						SetLocalAngularVelocity(const QAngle &vecAngVelocity);
	const QAngle&				GetLocalAngularVelocity() const;
	//void						SetAbsAngularVelocity( const QAngle &vecAngAbsVelocity );
	//const QAngle&				GetAbsAngularVelocity( ) const;
	const Vector&				GetBaseVelocity() const;
	void						SetBaseVelocity(const Vector& v);
	virtual const Vector&		GetViewOffset() const;
	virtual void				SetViewOffset(const Vector& v);
	virtual void				GetGroundVelocityToApply(Vector &vecGroundVel) { vecGroundVel = vec3_origin; }
	//// TrackIR
	//const Vector&				GetEyeOffset() const;
	//void						SetEyeOffset(const Vector& v);
	//const QAngle &			GetEyeAngleOffset() const;
	//void						SetEyeAngleOffset(const QAngle & qa);
	//// TrackIR    
	// Invalidates the abs state of all children
	void						InvalidatePhysicsRecursive(int nChangeFlags);
	ClientRenderHandle_t		GetRenderHandle() const;
	void						SetRemovalFlag(bool bRemove);
	bool						HasSpawnFlags(int nFlags) const;
	// Effects...
	bool						IsEffectActive(int nEffectMask) const;
	void						AddEffects(int nEffects);
	void						RemoveEffects(int nEffects);
	int							GetEffects(void) const;
	void						ClearEffects(void);
	void						SetEffects(int nEffects);
	// Computes the abs position of a point specified in local space
	void						ComputeAbsPosition(const Vector &vecLocalPosition, Vector *pAbsPosition);
	// Computes the abs position of a direction specified in local space
	void						ComputeAbsDirection(const Vector &vecLocalDirection, Vector *pAbsDirection);	
	// These methods encapsulate MOVETYPE_FOLLOW, which became obsolete
	void						FollowEntity(CBaseEntity *pBaseEntity, bool bBoneMerge = true);
	void						StopFollowingEntity();	// will also change to MOVETYPE_NONE
	bool						IsFollowingEntity();
	CBaseEntity*				GetFollowedEntity();
	// For shadows rendering the correct body + sequence...
	virtual int					GetBody() { return 0; }
	virtual int					GetSkin() { return 0; }
	const Vector&				ScriptGetForward(void) { static Vector vecForward; GetVectors(&vecForward, NULL, NULL); return vecForward; }
	const Vector&				ScriptGetLeft(void) { static Vector vecLeft; GetVectors(NULL, &vecLeft, NULL); return vecLeft; }
	const Vector&				ScriptGetUp(void) { static Vector vecUp; GetVectors(NULL, NULL, &vecUp); return vecUp; }
	// Stubs on client
	void						NetworkStateManualMode(bool activate)		{ }
	void						NetworkStateChanged()						{ }
	void						NetworkStateChanged(void *pVar)				{ }
	void						NetworkStateSetUpdateInterval(float N)		{ }
	void						NetworkStateForceUpdate()					{ }
	// Think functions with contexts
	int							RegisterThinkContext(const char *szContext);
	BASEPTR						ThinkSet(BASEPTR func, float flNextThinkTime = 0, const char *szContext = NULL);
	void						SetNextThink(float nextThinkTime, const char *szContext = NULL);
	float						GetNextThink(const char *szContext = NULL);
	float						GetLastThink(const char *szContext = NULL);
	int							GetNextThinkTick(const char *szContext = NULL);
	int							GetLastThinkTick(const char *szContext = NULL);
	// These set entity flags (EFL_*) to help optimize queries
	void						CheckHasThinkFunction(bool isThinkingHint = false);
	void						CheckHasGamePhysicsSimulation();
	bool						WillThink();
	bool						WillSimulateGamePhysics();
	int							GetFirstThinkTick();	// get first tick thinking on any context
	float						GetAnimTime() const;
	void						SetAnimTime(float at);
	float						GetSimulationTime() const;
	void						SetSimulationTime(float st);
	float						GetCreateTime()										{ return 0; }
	void						SetCreateTime(float flCreateTime)					{  }
	int							GetCreationTick() const;
	// Gets the model instance + shadow handle
	virtual ModelInstanceHandle_t GetModelInstance() { return m_ModelInstance; }
	void						SetModelInstance(ModelInstanceHandle_t hInstance) { m_ModelInstance = hInstance; }
	bool						SnatchModelInstance(C_BaseEntity * pToEntity);
	virtual ClientShadowHandle_t GetShadowHandle() const	{ return m_ShadowHandle; }
	virtual ClientRenderHandle_t&RenderHandle();
	virtual void				CreateModelInstance();
	// Sets the origin + angles to match the last position received
	void						MoveToLastReceivedPosition(bool force = false);

protected:
	// Only meant to be called from subclasses
	void						DestroyModelInstance();
	// Interpolate entity
	static void					ProcessTeleportList();
	static void					ProcessInterpolatedList();
	static void					CheckInterpolatedVarParanoidMeasurement();
	// overrideable rules if an entity should interpolate
	virtual bool				ShouldInterpolate();
	// Call this in OnDataChanged if you don't chain it down!
	void						MarkMessageReceived();
	// Gets the last message time
	float						GetLastMessageTime() const		{ return m_flLastMessageTime; }
	// For non-players
	int							PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);

protected:
	// Two part guts of Interpolate(). Shared with C_BaseAnimating.
	enum { INTERPOLATE_STOP = 0, INTERPOLATE_CONTINUE };

	// Returns INTERPOLATE_STOP or INTERPOLATE_CONTINUE.
	// bNoMoreChanges is set to 1 if you can call RemoveFromInterpolationList on the entity.
	int							BaseInterpolatePart1(float &currentTime, Vector &oldOrigin, QAngle &oldAngles, int &bNoMoreChanges);
	void						BaseInterpolatePart2(Vector &oldOrigin, QAngle &oldAngles, int nChangeFlags);


public:
	// Accessors for above
	static int					GetPredictionRandomSeed(void);
	static void					SetPredictionRandomSeed(const CUserCmd *cmd);
	static C_BasePlayer*		GetPredictionPlayer(void);
	static void					SetPredictionPlayer(C_BasePlayer *player);
	static void					CheckCLInterpChanged();
	// Collision group accessors
	int							GetCollisionGroup() const;
	void						SetCollisionGroup(int collisionGroup);
	void						CollisionRulesChanged();
	static C_BaseEntity*		Instance(int iEnt);
	// Doesn't do much, but helps with trace results
	static C_BaseEntity*		Instance(IClientEntity *ent);
	static C_BaseEntity*		Instance(CBaseHandle hEnt);
	// For debugging shared code
	static bool					IsServer(void);
	static bool					IsClient(void);
	static char const*			GetDLLType(void);
	static void					SetAbsQueriesValid(bool bValid);
	static bool					IsAbsQueriesValid(void);
	// Enable/disable abs recomputations on a stack.
	static void					PushEnableAbsRecomputations(bool bEnable);
	static void					PopEnableAbsRecomputations();
	// This requires the abs recomputation stack to be empty and just sets the global state. 
	// It should only be used at the scope of the frame loop.
	static void					EnableAbsRecomputations(bool bEnable);
	static bool					IsAbsRecomputationsEnabled(void);
	static void					PreRenderEntities(int nSplitScreenPlayerSlot);
	static void					PurgeRemovedEntities();
	static void					SimulateEntities();
	// Bloat the culling bbox past the parent ent's bbox in local space if EF_BONEMERGE_FASTCULL is set.
	virtual void				BoneMergeFastCullBloat(Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs) const;
	// Accessors for color.
	const color24				GetRenderColor() const;
	byte						GetRenderColorR() const;
	byte						GetRenderColorG() const;
	byte						GetRenderColorB() const;
	byte						GetRenderAlpha() const;
	void						SetRenderColor(byte r, byte g, byte b);
	void						SetRenderColorR(byte r);
	void						SetRenderColorG(byte g);
	void						SetRenderColorB(byte b);
	void						SetRenderAlpha(byte a);
	void						SetRenderMode(RenderMode_t nRenderMode, bool bForceUpdate = false);
	RenderMode_t				GetRenderMode() const;
	void						SetRenderFX(RenderFx_t nRenderFX, float flStartTime = FLT_MAX, float flDuration = 0.0f);
	RenderFx_t					GetRenderFX() const;
	// Returns true if there was a change.
	bool						SetCellBits(int cellbits = CELL_BASEENTITY_ORIGIN_CELL_BITS);

	static void					RecvProxy_CellBits(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellX(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellZ(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellOrigin(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellOriginXY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void					RecvProxy_CellOriginZ(const CRecvProxyData *pData, void *pStruct, void *pOut);

	const char*					GetEntityName();
	static bool					IsInterpolationEnabled(); // Should we be interpolating?

	// Called after predicted entity has been acknowledged so that no longer needed entity can be deleted
	// Return true to force deletion right now, regardless of isbeingremoved
	virtual bool				OnPredictedEntityRemove(bool isbeingremoved, C_BaseEntity *predicted);
	bool						IsDormantPredictable(void) const;
	bool						BecameDormantThisPacket(void) const;
	void						SetDormantPredictable(bool dormant);
	int							GetWaterLevel() const;
	void						SetWaterLevel(int nLevel);
	int							GetWaterType() const;
	void						SetWaterType(int nType);
	float						GetElasticity(void) const;
	int							GetTextureFrameIndex(void);
	void						SetTextureFrameIndex(int iIndex);
	virtual bool				GetShadowCastDistance(float *pDist, ShadowType_t shadowType) const;
	virtual bool				GetShadowCastDirection(Vector *pDirection, ShadowType_t shadowType) const;
	virtual C_BaseEntity 		*GetShadowUseOtherEntity(void) const;
	virtual void				SetShadowUseOtherEntity(C_BaseEntity *pEntity);
	CInterpolatedVar<QAngle>&	GetRotationInterpolator();
	CInterpolatedVar<Vector>&	GetOriginInterpolator();
	virtual bool				AddRagdollToFadeQueue(void) { return true; }
	void						MarkRenderHandleDirty(); // Dirty bits
	void						HierarchyUpdateMoveParent(); // used by SourceTV since move-parents may be missing when child spawns.
	void						SetCPULevels(int nMinCPULevel, int nMaxCPULevel);
	void						SetGPULevels(int nMinGPULevel, int nMaxGPULevel);
	int							GetMinCPULevel() const;
	int							GetMaxCPULevel() const;
	int							GetMinGPULevel() const;
	int							GetMaxGPULevel() const;
	int							GetIndexForThinkContext(const char *pszContext); // Think contexts
	virtual int					GetStudioBody(void) { return 0; } // Allow studio models to tell us what their m_nBody value is
	bool						IsParentChanging(); // call this in postdataupdate to detect hierarchy changes
	friend void					OnRenderStart();
	bool						InitializeAsClientEntityByIndex(int iIndex, bool bRenderWithViewModels);
	static void					InterpolateServerEntities(); // Figure out the smoothly interpolated origin for all server entities.
	static void					AddVisibleEntities(); // Check which entities want to be drawn and add them to the leaf system.
	static void					ToolRecordEntities(); // For entities marked for recording, post bone messages to IToolSystems
	void						UpdateBaseVelocity(void);
	void						PhysicsPusher(void); // Physics-related private methods
	void						PhysicsNone(void);
	void						PhysicsNoclip(void);
	void						PhysicsParent(void);
	void						PhysicsStepRunTimestep(float timestep);
	void						PhysicsToss(void);
	void						PhysicsCustom(void);
	void						PhysicsRigidChild(void); // Simulation in local space of rigid children
	void						CalcAbsolutePosition() { } // Computes absolute position based on hierarchy
	void						CalcAbsoluteVelocity()
	{
		if ((m_iEFlags & EFL_DIRTY_ABSVELOCITY) == 0)
			return;

		AUTO_LOCK(m_CalcAbsoluteVelocityMutex);

		if ((m_iEFlags & EFL_DIRTY_ABSVELOCITY) == 0) // need second check in event another thread grabbed mutex and did the calculation
		{
			return;
		}

		m_iEFlags &= ~EFL_DIRTY_ABSVELOCITY;

		CBaseEntity *pMoveParent = GetMoveParent();
		if (!pMoveParent)
		{
			m_vecAbsVelocity = m_vecVelocity;
			return;
		}

		VectorRotate(m_vecVelocity, pMoveParent->EntityToWorldTransform(), m_vecAbsVelocity);

		// Add in the attachments velocity if it exists
		if (m_iParentAttachment != 0)
		{
			Vector vOriginVel;
			Quaternion vAngleVel;
			if (pMoveParent->GetAttachmentVelocity(m_iParentAttachment, vOriginVel, vAngleVel))
			{
				m_vecAbsVelocity += vOriginVel;
				return;
			}
		}

		// Now add in the parent abs velocity
		m_vecAbsVelocity += pMoveParent->GetAbsVelocity();
	}
	void						SimulateAngles(float flFrameTime); // Computes new angles based on the angular velocity
	virtual void				PerformCustomPhysics(Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity);
	void						AddStudioDecal(const Ray_t& ray, int hitbox, int decalIndex, bool doTrace, trace_t& tr, int maxLODToDecal = ADDDECAL_TO_ALL_LODS); // methods related to decal adding
	void						AddBrushModelDecal(const Ray_t& ray, const Vector& decalCenter, int decalIndex, bool doTrace, trace_t& tr);
	void						ComputePackedOffsets(void);
	int							GetIntermediateDataSize(void);
	void						UnlinkChild(C_BaseEntity *pParent, C_BaseEntity *pChild);
	void						LinkChild(C_BaseEntity *pParent, C_BaseEntity *pChild);
	void						HierarchySetParent(C_BaseEntity *pNewParent);
	void						UnlinkFromHierarchy();
	void						UpdateWaterState(); // Computes the water level + type
	void						PhysicsCheckSweep(const Vector& vecAbsStart, const Vector &vecAbsDelta, trace_t *pTrace); // Checks a sweep without actually performing the move
	void						MoveToAimEnt(); // FIXME: REMOVE!!!
	void						SetNextThink(int nContextIndex, float thinkTime); // Sets/Gets the next think based on context index
	void						SetLastThink(int nContextIndex, float thinkTime);
	float						GetNextThink(int nContextIndex) const;
	int							GetNextThinkTick(int nContextIndex) const;
	void						CleanUpAlphaProperty();
	virtual float*				GetRenderClipPane();
	float *						GetRenderClipPlane(void); // Rendering clip plane, should be 4 floats, return value of NULL indicates a disabled render clip plane
	void						AddToEntityList(entity_list_ids_t listId);
	void						RemoveFromEntityList(entity_list_ids_t listId);

	/* My accessors */
	QAngle						GetPunchAngle();
	QAngle						GetViewPunch();
	QAngle*						GetAimPunchPtr();
	QAngle*						GetViewPunchPtr();
	Vector						GetVelocity();
	Vector						GetEyePos();
	QAngle						GetEyeAngles();
	int							GetGlowIndex();
	void						UpdateEntityGlow(Vector color, float alpha, float somefloat);
	void						GetEyeVectors(Vector *pForward, Vector *pRight, Vector *pUp);
	Vector						GetPredictedEyePos();
	Vector						GetAbsOrigin();
	QAngle						GetAbsAngles();
	int							TickBase();
	CPlayerState*				GetPlayerState();
	C_BaseCombatWeapon*			GetWeapon();
	int							team();
	bool						IsScoped();
	int							health();
	int							InCross();
	void						dump();

public:
	string_t						m_iClassname;
	HSCRIPT							m_hScriptInstance;
	string_t						m_iszScriptId;
	VarMapping_t					m_VarMap;
	char pad_0x3C[0xC];
	int								index; // Determine what entity this corresponds to
	unsigned long					m_EntClientFlags;
	const model_t					*model;
	CNetworkColor32					(m_clrRender);
	int								m_cellbits;
	int								m_cellwidth;
	int								m_cellX;
	int								m_cellY;
	int								m_cellZ;
	Vector							m_vecCellOrigin; // cached cell offset position
	Vector							m_vecAbsVelocity;
	Vector							m_vecAbsOrigin;
	Vector							m_vecOrigin;
	QAngle							m_vecAngVelocity;
	QAngle							m_angAbsRotation;
	QAngle							m_angRotation;
	float							m_flGravity;
	float							m_flProxyRandomValue;
	unsigned long					m_iEFlags;
	unsigned char					m_nWaterType;
	bool							m_bDormant;
	int								m_fEffects;
	int								m_iTeamNum;
	int								m_iPendingTeamNum;
	int								m_nNextThinkTick;
	int								m_iHealth;
	int								m_fFlags; // Behavior flags
	Vector							m_vecViewOffset; // Object eye position
	Vector							m_vecVelocity; // Object velocity
	Vector							m_vecBaseVelocity;	// Base velocity
	QAngle							m_angNetworkAngles;
	Vector							m_vecNetworkOrigin; // Last values to come over the wire. Used for interpolation.
	float							m_flFriction;
	EHANDLE							m_hNetworkMoveParent; // The moveparent received from networking data
	EHANDLE							m_hOwnerEntity; // The owner!
	EHANDLE							m_hGroundEntity;
	char							m_iName[MAX_PATH];
	short							m_nModelIndex; // Object model index
	unsigned char					m_nRenderFX;
	unsigned char 					m_nRenderMode;
	unsigned char					m_MoveType;
	unsigned char					m_MoveCollide;
	unsigned char					m_nWaterLevel;
	unsigned char					m_lifeState;
	float							m_flAnimTime; // Time animation sequence or frame was last changed
	float							m_flOldAnimTime;
	float							m_flSimulationTime;
	float							m_flOldSimulationTime;
	char							pad_0x26C[0x2];
	ClientRenderHandle_t			m_hRender;	// link into spatial partition
	unsigned char 					m_nOldRenderMode;
	char							pad_0x271[0x3];
	CBitVec<1>						m_VisibilityBits;
	//char							pad_0x275[0x1];
	//bool							m_bReadyToDraw; // Interpolation says don't draw yet
	//bool							m_bClientSideRagdoll;
	int								m_nLastThinkTick;
	int								m_takedamage;
	float							m_flSpeed; // was pev->speed
	int								touchStamp; // used so we know when things are no longer touching
	CBaseHandle						m_RefEHandle; // Reference ehandle. Used to generate ehandles off this entity.
	int								m_HandleIndex;
	char							pad_0x290[0x4];
	HTOOLHANDLE						m_ToolHandle; // 0x294
	bool							m_bEnabledInToolView;
	bool							m_bToolRecording;
	//int							m_nLastRecordedFrame;
	bool							m_bRecordInTools; // should this entity be recorded in the tools
	bool							m_bPredictionEligible;
	IPhysicsObject					*m_pPhysicsObject;
	int								m_iCurrentThinkContext;
	int								m_spawnflags;
	int								m_nSimulationTick;
	CUtlVector<thinkfunc_t>			m_aThinkFunctions;
	//Vector						m_vecEyeOffset; // TrackIR
	//QAngle						m_EyeAngleOffset; // TrackIR  
	char							pad_0x2C0[0xB];
	bool							m_bDormantPredictable;
	int								m_nIncomingPacketEntityBecameDormant; // So we can clean it up
	float							m_flLastMessageTime; // Timestamp of message arrival
	float							m_flSpawnTime; // The spawn time of the entity
	ModelInstanceHandle_t			m_ModelInstance; // Model instance data..
	ClientShadowHandle_t			m_ShadowHandle; // Shadow data
	ClientThinkHandle_t				m_hThink;
	CBitVec<1>						m_ShadowBits; // Per-splitscreen user shadow visibility bits
	unsigned char					m_iParentAttachment; // 0x2E4 // 0 if we're relative to the parent's absorigin and absangles.
	unsigned char					m_iOldParentAttachment; // 0x2E5
	char							pad_0x2E6[0x6];
	float							m_fadeMinDist; // 0x2EC // Fades
	float							m_fadeMaxDist; // 0x2F0
	float							m_flFadeScale; // 0x2F4
	bool							m_bPredictable; // Prediction system
	bool							m_bRenderWithViewModels;
	bool							m_bDisableCachedRenderBounds;
	unsigned char					m_nSplitUserPlayerPredictionSlot;
	CHandle<C_BaseEntity>			m_pMoveParent; // Hierarchy
	CHandle<C_BaseEntity>			m_pMoveChild;
	CHandle<C_BaseEntity>			m_pMovePeer;
	CHandle<C_BaseEntity>			m_pMovePrevPeer;
	CHandle<C_BaseEntity>			m_hOldMoveParent;
	string_t						m_ModelName;
	CNetworkVarEmbedded				(CCollisionProperty, m_Collision);
	CNetworkVarEmbedded				(CParticleProperty, m_Particles);
	IClientAlphaProperty			*m_pClientAlphaProperty;
	float							m_flElasticity;			 // Physics state
	float							m_flShadowCastDistance;	
	EHANDLE							m_ShadowDirUseOtherEntity; 
	float							m_flGroundChangeTime;	
	Vector							m_vecOldOrigin;			
	QAngle							m_vecOldAngRotation;	
	CInterpolatedVar<Vector>		m_iv_vecOrigin;
	char							pad_0x3E8[0x14];
	CInterpolatedVar<QAngle>		m_iv_angRotation;	
	char							pad_0x428[0x14];
	matrix3x4_t						m_rgflCoordinateFrame; // Specifies the entity-to-world transform		
	int								m_CollisionGroup; // used to cull collision tests
	unsigned char					*m_pIntermediateData[MULTIPLAYER_BACKUP]; // 0x470 // For storing prediction results and pristine network state
	unsigned char					*m_pOldIntermediateData[MULTIPLAYER_BACKUP]; // 0x6C8
	int								unkint;
	unsigned char					*m_pOriginalData;
	EHANDLE							m_hUnknown;
	int								m_nIntermediateDataCount;
	bool							m_bIsSpectated;
	bool							m_bIsPlayerSimulated;
	CNetworkVar						(bool, m_bSimulatedEveryTick);	// 0x932
	CNetworkVar						(bool, m_bAnimatedEveryTick);	// 0x933
	CNetworkVar						(bool, m_bAlternateSorting);	// 0x934
	CNetworkVar						(bool, m_bSpotted);				// 0x935
	CNetworkVar						(bool, m_bSpottedBy);			// 0x936
	char							pad_0x937[0x41];				// 0x937
	unsigned char					m_bSpottedByMask;				// 0x978
	char							pad_0x979[0x7];					// 0x979
	unsigned char					m_nMinCPULevel;					// 0x980
	unsigned char					m_nMaxCPULevel;					// 0x981
	unsigned char					m_nMinGPULevel;					// 0x982
	unsigned char					m_nMaxGPULevel;					// 0x983
	int								m_iTextureFrameIndex;			// 0x984
	//unsigned char					m_fBBoxVisFlags; // Bbox visualization
	//bool							m_bIsValidIKAttachment;
	//unsigned char					m_DataChangeEventRef;
	EHANDLE							m_hEffectEntity;			// 0x988
	CHandle<CBasePlayer>			m_hPlayerSimulationOwner;	// 0x98C
	EHANDLE							m_hUnknown2;				// 0x990
	static int						m_nPredictionRandomSeed;
	static C_BasePlayer				*m_pPredictionPlayer;
	static bool						s_bAbsQueriesValid;
	static bool						s_bAbsRecomputationEnabled;
	static bool						s_bInterpolate;
	int								m_fDataObjectTypes;			// 0x994
	AimEntsListHandle_t				m_AimEntsListHandle;		// 0x998
	int								m_nCreationTick;			// 0x99C
	float							m_fRenderingClipPlane[4]; //world space clip plane when drawing
	unsigned short					m_bEnableRenderingClipPlane; //true to use the custom clip plane when drawing
	unsigned short					m_ListEntry[NUM_ENTITY_LISTS];	// Entry into each g_EntityList (or InvalidIndex() if not in the list).
	CThreadFastMutex				m_CalcAbsolutePositionMutex;
	CThreadFastMutex				m_CalcAbsoluteVelocityMutex;
	bool							m_bIsBlurred; // 0x9CB
	char							pad[0x4];	// 0x9CC
};

EXTERN_RECV_TABLE(DT_BaseEntity);

inline void C_BaseEntity::dump()
{
	printf("C_BaseEntity -> Address = 0x%X\n", this);
	LOG_BASEENTITY("C_BaseEntity -> Address = 0x%X", (DWORD)this);
	SHOUTMEMBERENTITY("m_iClassname", m_iClassname);
	SHOUTMEMBERENTITY("m_hScriptInstance", m_hScriptInstance);
	SHOUTMEMBERENTITY("m_iszScriptId", m_iszScriptId);
	SHOUTMEMBERENTITY("m_varMap", m_VarMap.m_Entries.m_Memory.m_pMemory);
	SHOUTMEMBERENTITY("index", index);
	SHOUTMEMBERENTITY("m_EntClientFlags", m_EntClientFlags);
	SHOUTMEMBERENTITY("model", model);
	SHOUTMEMBERENTITY("m_cellbits", m_cellbits);
	SHOUTMEMBERENTITY("m_cellwidth", m_cellwidth);
	SHOUTMEMBERENTITY("m_cellX", m_cellX);
	SHOUTMEMBERENTITY("m_cellY", m_cellY);
	SHOUTMEMBERENTITY("m_cellZ", m_cellZ);
	SHOUTMEMBERENTITY("m_vecCellOrigin", m_vecCellOrigin[0]);
	SHOUTMEMBERENTITY("m_vecAbsVelocity", m_vecAbsVelocity[0]);
	SHOUTMEMBERENTITY("m_vecAbsOrigin", m_vecAbsOrigin[0]);
	SHOUTMEMBERENTITY("m_vecOrigin", m_vecOrigin[0]);
	SHOUTMEMBERENTITY("m_vecAngVelocity", m_vecAngVelocity);
	SHOUTMEMBERENTITY("m_angAbsRotation", m_angAbsRotation);
	SHOUTMEMBERENTITY("m_angRotation", m_angRotation);
	SHOUTMEMBERENTITY("m_flGravity", m_flGravity);
	SHOUTMEMBERENTITY("m_flProxyRandomValue", m_flProxyRandomValue);
	SHOUTMEMBERENTITY("m_iEFlags", m_iEFlags);
	SHOUTMEMBERENTITY("m_nWaterType", m_nWaterType);
	SHOUTMEMBERENTITY("m_bDormant", m_bDormant);
	SHOUTMEMBERENTITY("m_fEffects", m_fEffects);
	SHOUTMEMBERENTITY("m_iTeamNum", m_iTeamNum);
	SHOUTMEMBERENTITY("m_iPendingTeamNum", m_iPendingTeamNum);
	SHOUTMEMBERENTITY("m_nNextThinkTick", m_nNextThinkTick);
	SHOUTMEMBERENTITY("m_iHealth", m_iHealth);
	SHOUTMEMBERENTITY("m_fFlags", m_fFlags);
	SHOUTMEMBERENTITY("m_vecViewOffset", m_vecViewOffset[0]);
	SHOUTMEMBERENTITY("m_vecVelocity", m_vecVelocity[0]);
	SHOUTMEMBERENTITY("m_vecBaseVelocity", m_vecBaseVelocity[0]);
	SHOUTMEMBERENTITY("m_angNetworkAngles", m_angNetworkAngles[0]);
	SHOUTMEMBERENTITY("m_vecNetworkOrigin", m_vecNetworkOrigin[0]);
	SHOUTMEMBERENTITY("m_flFriction", m_flFriction);
	SHOUTMEMBERENTITY("m_hNetworkMoveParent", m_hNetworkMoveParent);
	SHOUTMEMBERENTITY("m_hOwnerEntity", m_hOwnerEntity);
	SHOUTMEMBERENTITY("m_hGroundEntity", m_hGroundEntity);
	SHOUTMEMBERENTITY("m_iName", m_iName);
	SHOUTMEMBERENTITY("m_nModelIndex", m_nModelIndex);
	SHOUTMEMBERENTITY("m_nRenderFX", m_nRenderFX);
	SHOUTMEMBERENTITY("m_nRenderMode", m_nRenderMode);
	SHOUTMEMBERENTITY("m_MoveType", m_MoveType);
	SHOUTMEMBERENTITY("m_MoveCollide", m_MoveCollide);
	SHOUTMEMBERENTITY("m_nWaterLevel", m_nWaterLevel);
	SHOUTMEMBERENTITY("m_lifeState", m_lifeState);
	SHOUTMEMBERENTITY("m_flAnimTime", m_flAnimTime);
	SHOUTMEMBERENTITY("m_flOldAnimTime", m_flOldAnimTime);
	SHOUTMEMBERENTITY("m_flSimulationTime", m_flSimulationTime);
	SHOUTMEMBERENTITY("m_flOldSimulationTime", m_flOldSimulationTime);
	SHOUTMEMBERENTITY("m_hRender", m_hRender);
	SHOUTMEMBERENTITY("m_nOldRenderMode", m_nOldRenderMode);
	SHOUTMEMBERENTITY("m_VisibilityBits", m_VisibilityBits);
	SHOUTMEMBERENTITY("m_nLastThinkTick", m_nLastThinkTick);
	SHOUTMEMBERENTITY("m_takedamage", m_takedamage);
	SHOUTMEMBERENTITY("m_flSpeed", m_flSpeed);
	SHOUTMEMBERENTITY("touchStamp", touchStamp);
	SHOUTMEMBERENTITY("m_RefEHandle", m_RefEHandle);
	SHOUTMEMBERENTITY("m_HandleIndex", m_HandleIndex);
	SHOUTMEMBERENTITY("m_ToolHandle", m_ToolHandle);
	SHOUTMEMBERENTITY("m_bEnabledInToolView", m_bEnabledInToolView);
	SHOUTMEMBERENTITY("m_bToolRecording", m_bToolRecording);
	SHOUTMEMBERENTITY("m_bRecordInTools", m_bRecordInTools);
	SHOUTMEMBERENTITY("m_bPredictionEligible", m_bPredictionEligible);
	SHOUTMEMBERENTITY("m_pPhysicsObject", m_pPhysicsObject);
	SHOUTMEMBERENTITY("m_iCurrentThinkContext", m_iCurrentThinkContext);
	SHOUTMEMBERENTITY("m_spawnflags", m_spawnflags);
	SHOUTMEMBERENTITY("m_nSimulationTick", m_nSimulationTick);
	SHOUTMEMBERENTITY("m_aThinkFunctions", m_aThinkFunctions);
	SHOUTMEMBERENTITY("m_bDormantPredictable", m_bDormantPredictable);
	SHOUTMEMBERENTITY("m_nIncomingPacketEntityBecameDormant", m_nIncomingPacketEntityBecameDormant);
	SHOUTMEMBERENTITY("m_flLastMessageTime", m_flLastMessageTime);
	SHOUTMEMBERENTITY("m_flSpawnTime", m_flSpawnTime);
	SHOUTMEMBERENTITY("m_ModelInstance", m_ModelInstance);
	SHOUTMEMBERENTITY("m_ShadowHandle", m_ShadowHandle);
	SHOUTMEMBERENTITY("m_ShadowBits", m_ShadowBits);
	SHOUTMEMBERENTITY("m_hThink", m_hThink);
	SHOUTMEMBERENTITY("m_iParentAttachment", m_iParentAttachment);
	SHOUTMEMBERENTITY("m_iOldParentAttachment", m_iOldParentAttachment);
	SHOUTMEMBERENTITY("m_fadeMinDist", m_fadeMinDist);
	SHOUTMEMBERENTITY("m_fadeMaxDist", m_fadeMaxDist);
	SHOUTMEMBERENTITY("m_flFadeScale", m_flFadeScale);
	SHOUTMEMBERENTITY("m_bPredictable", m_bPredictable);
	SHOUTMEMBERENTITY("m_bRenderWithViewModels", m_bRenderWithViewModels);
	SHOUTMEMBERENTITY("m_bDisableCachedRenderBounds", m_bDisableCachedRenderBounds);
	SHOUTMEMBERENTITY("m_nSplitUserPlayerPredictionSlot", m_nSplitUserPlayerPredictionSlot);
	SHOUTMEMBERENTITY("m_pMoveParent", m_pMoveParent);
	SHOUTMEMBERENTITY("m_pMoveChild", m_pMoveChild);
	SHOUTMEMBERENTITY("m_pMovePeer", m_pMovePeer);
	SHOUTMEMBERENTITY("m_pMovePrevPeer", m_pMovePrevPeer);
	SHOUTMEMBERENTITY("m_hOldMoveParent", m_hOldMoveParent);
	SHOUTMEMBERENTITY("m_ModelName", m_ModelName);
	SHOUTMEMBERENTITY("m_Collision", m_Collision);
	SHOUTMEMBERENTITY("m_Particles", m_Particles);
	SHOUTMEMBERENTITY("m_pClientAlphaProperty", m_pClientAlphaProperty);
	SHOUTMEMBERENTITY("m_flElasticity", m_flElasticity);
	SHOUTMEMBERENTITY("m_flShadowCastDistance", m_flShadowCastDistance);
	SHOUTMEMBERENTITY("m_ShadowDirUseOtherEntity", m_ShadowDirUseOtherEntity);
	SHOUTMEMBERENTITY("m_flGroundChangeTime", m_flGroundChangeTime);
	SHOUTMEMBERENTITY("m_vecOldOrigin", m_vecOldOrigin[0]);
	SHOUTMEMBERENTITY("m_vecOldAngRotation", m_vecOldAngRotation[0]);
	SHOUTMEMBERENTITY("m_iv_vecOrigin", m_iv_vecOrigin.m_pValue); 
	SHOUTMEMBERENTITY("m_iv_angRotation", m_iv_angRotation.m_pValue);
	SHOUTMEMBERENTITY("m_rgflCoordinateFrame", m_rgflCoordinateFrame[0][0]);
	SHOUTMEMBERENTITY("m_CollisionGroup", m_CollisionGroup);
	SHOUTMEMBERENTITY("m_pIntermediateData", m_pIntermediateData);
	SHOUTMEMBERENTITY("m_pOldIntermediateData", m_pOldIntermediateData);
	SHOUTMEMBERENTITY("unkint", unkint);
	SHOUTMEMBERENTITY("m_pOriginalData", m_pOriginalData);
	SHOUTMEMBERENTITY("m_hUnknown", m_hUnknown);
	SHOUTMEMBERENTITY("m_nIntermediateDataCount", m_nIntermediateDataCount);
	SHOUTMEMBERENTITY("m_bIsSpectated", m_bIsSpectated);
	SHOUTMEMBERENTITY("m_bIsPlayerSimulated", m_bIsPlayerSimulated);
	SHOUTMEMBERENTITY("m_bSimulatedEveryTick", m_bSimulatedEveryTick);
	SHOUTMEMBERENTITY("m_bAnimatedEveryTick", m_bAnimatedEveryTick);
	SHOUTMEMBERENTITY("m_bAlternateSorting", m_bAlternateSorting);
	SHOUTMEMBERENTITY("m_bSpotted", m_bSpotted);
	SHOUTMEMBERENTITY("m_bSpottedBy", m_bSpottedBy);
	SHOUTMEMBERENTITY("pad_0x937", pad_0x937);
	SHOUTMEMBERENTITY("m_bSpottedByMask", m_bSpottedByMask);
	SHOUTMEMBERENTITY("pad_0x979", pad_0x979);
	SHOUTMEMBERENTITY("m_nMinCPULevel", m_nMinCPULevel);
	SHOUTMEMBERENTITY("m_nMaxCPULevel", m_nMaxCPULevel);
	SHOUTMEMBERENTITY("m_nMinGPULevel", m_nMinGPULevel);
	SHOUTMEMBERENTITY("m_nMaxGPULevel", m_nMaxGPULevel);
	SHOUTMEMBERENTITY("m_iTextureFrameIndex", m_iTextureFrameIndex);
	SHOUTMEMBERENTITY("m_hEffectEntity", m_hEffectEntity);
	SHOUTMEMBERENTITY("m_hPlayerSimulationOwner", m_hPlayerSimulationOwner);
	SHOUTMEMBERENTITY("m_hUnknown2", m_hUnknown2);
	SHOUTMEMBERENTITY("m_fDataObjectTypes", m_fDataObjectTypes);
	SHOUTMEMBERENTITY("m_AimEntsListHandle", m_AimEntsListHandle);
	SHOUTMEMBERENTITY("m_nCreationTick", m_nCreationTick);
	SHOUTMEMBERENTITY("m_fRenderingClipPlane", m_fRenderingClipPlane[0]);
	SHOUTMEMBERENTITY("m_bEnableRenderingClipPlane", m_bEnableRenderingClipPlane);
	SHOUTMEMBERENTITY("m_ListEntry", m_ListEntry[0]);
	SHOUTMEMBERENTITY("m_CalcAbsolutePositionMutex.m_ownerID", m_CalcAbsolutePositionMutex.m_ownerID);
	SHOUTMEMBERENTITY("m_CalcAbsoluteVelocityMutex.m_ownerID", m_CalcAbsoluteVelocityMutex.m_ownerID);
	SHOUTMEMBERENTITY("m_bIsBlurred", m_bIsBlurred);
	printf("\n");
}

inline bool FClassnameIs(C_BaseEntity *pEntity, const char *szClassname)
{
	Assert(pEntity);
	if (pEntity == NULL)
		return false;
	return !strcmp(pEntity->GetClassname(), szClassname) ? true : false;
}

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), 0, NULL )
#define SetContextThink( a, b, context ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), (b), context )

#ifdef _DEBUG
#define SetTouch( a ) TouchSet( static_cast <void (C_BaseEntity::*)(C_BaseEntity *)> (a), #a )

#else
#define SetTouch( a ) m_pfnTouch = static_cast <void (C_BaseEntity::*)(C_BaseEntity *)> (a)

#endif


inline CCollisionProperty *C_BaseEntity::CollisionProp()
{
	return &m_Collision;
}

inline const CCollisionProperty *C_BaseEntity::CollisionProp() const
{
	return &m_Collision;
}

inline CClientAlphaProperty *C_BaseEntity::AlphaProp()
{
	return (CClientAlphaProperty*)m_pClientAlphaProperty;
}

inline const CClientAlphaProperty *C_BaseEntity::AlphaProp() const
{
	return (CClientAlphaProperty*)m_pClientAlphaProperty;
}

//-----------------------------------------------------------------------------
// An inline version the game code can use
//-----------------------------------------------------------------------------
inline CParticleProperty *C_BaseEntity::ParticleProp()
{
	return &m_Particles;
}

inline const CParticleProperty *C_BaseEntity::ParticleProp() const
{
	return &m_Particles;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether this entity was created on the client.
//-----------------------------------------------------------------------------
inline bool C_BaseEntity::IsServerEntity(void)
{
	return index != -1;
}

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline matrix3x4_t &C_BaseEntity::EntityToWorldTransform()
{
	//Assert(s_bAbsQueriesValid);
	//CalcAbsolutePosition();
	return m_rgflCoordinateFrame;
}

inline const matrix3x4_t &C_BaseEntity::EntityToWorldTransform() const
{
	//Assert(s_bAbsQueriesValid);
	//const_cast<C_BaseEntity*>(this)->CalcAbsolutePosition();
	return m_rgflCoordinateFrame;
}

//-----------------------------------------------------------------------------
// Some helper methods that transform a point from entity space to world space + back
//-----------------------------------------------------------------------------
inline void C_BaseEntity::EntityToWorldSpace(const Vector &in, Vector *pOut) const
{
	if (m_angAbsRotation == vec3_angle)
		VectorAdd(in, m_vecAbsOrigin, *pOut);
	else
		VectorTransform(in, EntityToWorldTransform(), *pOut);
}

inline void C_BaseEntity::WorldToEntitySpace(const Vector &in, Vector *pOut) const
{
	if (m_angAbsRotation == vec3_angle)
		VectorSubtract(in, m_vecAbsOrigin, *pOut);
	else
		VectorITransform(in, EntityToWorldTransform(), *pOut);
}

inline const Vector &C_BaseEntity::GetAbsVelocity() const
{
	//Assert(s_bAbsQueriesValid);
	//const_cast<C_BaseEntity*>(this)->CalcAbsoluteVelocity();
	return m_vecAbsVelocity;
}

inline C_BaseEntity	*C_BaseEntity::Instance(IClientEntity *ent)
{
	return ent ? ent->GetBaseEntity() : NULL;
}

// For debugging shared code
inline bool	C_BaseEntity::IsServer(void)
{
	return false;
}

inline bool	C_BaseEntity::IsClient(void)
{
	return true;
}

inline const char *C_BaseEntity::GetDLLType(void)
{
	return "client";
}


//-----------------------------------------------------------------------------
// Methods relating to solid type + flags
//-----------------------------------------------------------------------------
inline void C_BaseEntity::SetSolidFlags(int nFlags)
{
	CollisionProp()->SetSolidFlags(nFlags);
}

inline bool C_BaseEntity::IsSolidFlagSet(int flagMask) const
{
	return CollisionProp()->IsSolidFlagSet(flagMask);
}

inline int	C_BaseEntity::GetSolidFlags(void) const
{
	return CollisionProp()->GetSolidFlags();
}

inline void C_BaseEntity::AddSolidFlags(int nFlags)
{
	CollisionProp()->AddSolidFlags(nFlags);
}

inline void C_BaseEntity::RemoveSolidFlags(int nFlags)
{
	CollisionProp()->RemoveSolidFlags(nFlags);
}

inline bool C_BaseEntity::IsSolid() const
{
	return CollisionProp()->IsSolid();
}

inline void C_BaseEntity::SetSolid(SolidType_t val)
{
	CollisionProp()->SetSolid(val);
}

inline SolidType_t C_BaseEntity::GetSolid() const
{
	return CollisionProp()->GetSolid();
}

inline void C_BaseEntity::SetCollisionBounds(const Vector& mins, const Vector &maxs)
{
	CollisionProp()->SetCollisionBounds(mins, maxs);
}


//-----------------------------------------------------------------------------
// Methods relating to bounds
//-----------------------------------------------------------------------------
inline const Vector& C_BaseEntity::WorldAlignMins() const
{
	Assert(!CollisionProp()->IsBoundsDefinedInEntitySpace());
	Assert(CollisionProp()->GetCollisionAngles() == vec3_angle);
	return CollisionProp()->OBBMins();
}

inline const Vector& C_BaseEntity::WorldAlignMaxs() const
{
	Assert(!CollisionProp()->IsBoundsDefinedInEntitySpace());
	Assert(CollisionProp()->GetCollisionAngles() == vec3_angle);
	return CollisionProp()->OBBMaxs();
}

inline const Vector& C_BaseEntity::WorldAlignSize() const
{
	Assert(!CollisionProp()->IsBoundsDefinedInEntitySpace());
	Assert(CollisionProp()->GetCollisionAngles() == vec3_angle);
	return CollisionProp()->OBBSize();
}

inline float CBaseEntity::BoundingRadius() const
{
	return CollisionProp()->BoundingRadius();
}

inline bool CBaseEntity::IsPointSized() const
{
	return CollisionProp()->BoundingRadius() == 0.0f;
}


//-----------------------------------------------------------------------------
// Methods relating to traversing hierarchy
//-----------------------------------------------------------------------------
inline C_BaseEntity *C_BaseEntity::GetMoveParent(void) const
{
	return m_pMoveParent;
}

inline C_BaseEntity *C_BaseEntity::FirstMoveChild(void) const
{
	return m_pMoveChild;
}

inline C_BaseEntity *C_BaseEntity::NextMovePeer(void) const
{
	return m_pMovePeer;
}

//-----------------------------------------------------------------------------
// Velocity
//-----------------------------------------------------------------------------
inline const Vector& C_BaseEntity::GetLocalVelocity() const
{
	return m_vecVelocity;
}

inline const QAngle& C_BaseEntity::GetLocalAngularVelocity() const
{
	return m_vecAngVelocity;
}

inline const Vector& C_BaseEntity::GetBaseVelocity() const
{
	return m_vecBaseVelocity;
}

inline void	C_BaseEntity::SetBaseVelocity(const Vector& v)
{
	m_vecBaseVelocity = v;
}

inline void C_BaseEntity::SetFriction(float flFriction)
{
	m_flFriction = flFriction;
}

inline void C_BaseEntity::SetGravity(float flGravity)
{
	m_flGravity = flGravity;
}

inline float C_BaseEntity::GetGravity(void) const
{
	return m_flGravity;
}

inline int C_BaseEntity::GetWaterLevel() const
{
	return m_nWaterLevel;
}

inline void C_BaseEntity::SetWaterLevel(int nLevel)
{
	m_nWaterLevel = nLevel;
}

inline float C_BaseEntity::GetElasticity(void)	const
{
	return m_flElasticity;
}

inline const color24 CBaseEntity::GetRenderColor() const
{
	color24 c = { m_clrRender->r, m_clrRender->g, m_clrRender->b };
	return c;
}

inline byte C_BaseEntity::GetRenderColorR() const
{
	return m_clrRender->r;
}

inline byte C_BaseEntity::GetRenderColorG() const
{
	return m_clrRender->g;
}

inline byte C_BaseEntity::GetRenderColorB() const
{
	return m_clrRender->b;
}

inline void C_BaseEntity::SetRenderColor(byte r, byte g, byte b)
{
	m_clrRender.SetR(r);
	m_clrRender.SetG(g);
	m_clrRender.SetB(b);
}

inline void C_BaseEntity::SetRenderColorR(byte r)
{
	m_clrRender.SetR(r);
}

inline void C_BaseEntity::SetRenderColorG(byte g)
{
	m_clrRender.SetG(g);
}

inline void C_BaseEntity::SetRenderColorB(byte b)
{
	m_clrRender.SetB(b);
}

inline RenderMode_t C_BaseEntity::GetRenderMode() const
{
	return (RenderMode_t)m_nRenderMode;
}

inline RenderFx_t C_BaseEntity::GetRenderFX() const
{
	return (RenderFx_t)m_nRenderFX;
}

inline void	C_BaseEntity::SetCPULevels(int nMinCPULevel, int nMaxCPULevel)
{
	m_nMinCPULevel = nMinCPULevel;
	m_nMaxCPULevel = nMaxCPULevel;
}

inline void C_BaseEntity::SetGPULevels(int nMinGPULevel, int nMaxGPULevel)
{
	m_nMinGPULevel = nMinGPULevel;
	m_nMaxGPULevel = nMaxGPULevel;
}

inline int C_BaseEntity::GetMinCPULevel() const
{
	return m_nMinCPULevel;
}

inline int C_BaseEntity::GetMaxCPULevel() const
{
	return m_nMaxCPULevel;
}

inline int C_BaseEntity::GetMinGPULevel() const
{
	return m_nMinGPULevel;
}

inline int C_BaseEntity::GetMaxGPULevel() const
{
	return m_nMaxGPULevel;
}


//-----------------------------------------------------------------------------
// checks to see if the entity is marked for deletion
//-----------------------------------------------------------------------------
inline bool C_BaseEntity::IsMarkedForDeletion(void)
{
	return (m_iEFlags & EFL_KILLME);
}

inline void C_BaseEntity::AddEFlags(int nEFlagMask)
{
	m_iEFlags |= nEFlagMask;
}

inline void C_BaseEntity::RemoveEFlags(int nEFlagMask)
{
	m_iEFlags &= ~nEFlagMask;
}

inline bool CBaseEntity::IsEFlagSet(int nEFlagMask) const
{
	return (m_iEFlags & nEFlagMask) != 0;
}

inline unsigned char CBaseEntity::GetParentAttachment() const
{
	return m_iParentAttachment;
}

inline ClientRenderHandle_t C_BaseEntity::GetRenderHandle() const
{
	return m_hRender;
}

inline ClientRenderHandle_t& C_BaseEntity::RenderHandle()
{
	return m_hRender;
}

inline int C_BaseEntity::team()
{
	return m_iTeamNum;
}

inline int C_BaseEntity::health()
{
	return m_iHealth;
}

inline int C_BaseEntity::InCross()
{
	return *(int*)((DWORD)this + 0x23DC);
}

inline Vector C_BaseEntity::GetAbsOrigin()
{
	return m_vecAbsOrigin;
}

inline QAngle C_BaseEntity::GetAbsAngles()
{
	return m_angAbsRotation;
}

inline Vector C_BaseEntity::GetVelocity()
{
	return m_vecAbsVelocity;
}

inline Vector C_BaseEntity::GetEyePos()
{
	return m_vecAbsOrigin + m_vecViewOffset;
}

inline Vector C_BaseEntity::GetPredictedEyePos()
{
	return GetEyePos() + GetVelocity() * g_pGlobals->interval_per_tick * g_pGlobals->interpolation_amount;
}

inline int C_BaseEntity::GetGlowIndex()
{
	return *(int*)((DWORD)this + 0x1DB0);
}


inline void C_BaseEntity::UpdateEntityGlow(Vector color, float alpha, float bloomVal = 1.0f)
{
	int index = g_pGlowObjectManager->GetIndex(this);
	g_pGlowObjectManager->SetEntity(index, this);
	g_pGlowObjectManager->SetColor(index, color);
	g_pGlowObjectManager->SetAlpha(index, alpha);
	g_pGlowObjectManager->SetBloom(index, bloomVal);
	g_pGlowObjectManager->SetRenderFlags(index, true, false);
}

inline void C_BaseEntity::GetEyeVectors(Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL)
{ 
	AngleVectors(GetEyeAngles(), pForward, pRight, pUp);
}


//-----------------------------------------------------------------------------
// Methods to cast away const
//-----------------------------------------------------------------------------
inline Vector C_BaseEntity::EyePosition(void) const
{
	return const_cast<C_BaseEntity*>(this)->EyePosition();
}

inline const QAngle &C_BaseEntity::EyeAngles(void) const		// Direction of eyes in world space
{
	return const_cast<C_BaseEntity*>(this)->EyeAngles();
}

inline const QAngle &C_BaseEntity::LocalEyeAngles(void) const	// Direction of eyes
{
	return const_cast<C_BaseEntity*>(this)->LocalEyeAngles();
}

inline Vector	C_BaseEntity::EarPosition(void) const			// position of ears
{
	return const_cast<C_BaseEntity*>(this)->EarPosition();
}

inline VarMapping_t* C_BaseEntity::GetVarMapping()
{
	return &m_VarMap;
}


//-----------------------------------------------------------------------------
// Should we be interpolating?
//-----------------------------------------------------------------------------
inline bool	C_BaseEntity::IsInterpolationEnabled()
{
	return s_bInterpolate;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : inline void
//-----------------------------------------------------------------------------
inline void C_BaseEntity::SetToolHandle(HTOOLHANDLE handle)
{
#ifndef NO_TOOLFRAMEWORK
	m_ToolHandle = handle;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : inline HTOOLHANDLE
//-----------------------------------------------------------------------------
inline HTOOLHANDLE C_BaseEntity::GetToolHandle() const
{
#ifndef NO_TOOLFRAMEWORK
	return m_ToolHandle;
#else
	return (HTOOLHANDLE)0;
#endif
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
inline bool C_BaseEntity::IsEnabledInToolView() const
{

	return false;
//#ifndef NO_TOOLFRAMEWORK
//	return m_bEnabledInToolView;
//#else
//	return false;
//#endif
}


//-----------------------------------------------------------------------------
// Client version of UTIL_Remove
//-----------------------------------------------------------------------------
inline void UTIL_Remove(C_BaseEntity *pEntity)
{
	pEntity->Remove();
}


inline bool C_BaseEntity::ShouldRecordInTools() const
{
#ifndef NO_TOOLFRAMEWORK
	return m_bRecordInTools;
#else
	return true;
#endif
}

inline bool C_BaseEntity::IsVisible() const
{
	ASSERT_LOCAL_PLAYER_RESOLVABLE();
	if (INVALID_CLIENT_RENDER_HANDLE == m_hRender)
		return false;
	return m_VisibilityBits.IsBitSet(GET_ACTIVE_SPLITSCREEN_SLOT());
}

inline bool C_BaseEntity::IsVisibleToAnyPlayer() const
{
	return !m_VisibilityBits.IsAllClear();
}

inline bool C_BaseEntity::HasSpawnFlags(int nFlags) const
{
	return (m_spawnflags & nFlags) != 0;
}

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline const char *C_BaseEntity::GetEntityName()
{
	return m_iName;
}

class CAbsQueryScopeGuard
{
public:
	CAbsQueryScopeGuard(bool state)
	{
		m_bSavedState = C_BaseEntity::IsAbsQueriesValid();
		C_BaseEntity::SetAbsQueriesValid(state);
	}
	~CAbsQueryScopeGuard()
	{
		C_BaseEntity::SetAbsQueriesValid(m_bSavedState);
}
private:
	bool	m_bSavedState;
};

#define ABS_QUERY_GUARD( state ) CAbsQueryScopeGuard s_AbsQueryGuard( state );

C_BaseEntity *CreateEntityByName(const char *className);

#if !defined( NO_ENTITY_PREDICTION )
class CEntIndexLessFunc
{
public:
	bool Less(C_BaseEntity * const & lhs, C_BaseEntity * const & rhs, void *pContext)
	{
		int e1 = lhs->entindex();
		int e2 = rhs->entindex();

		// if an entity has an invalid entity index, then put it at the end of the list
		e1 = (e1 == -1) ? MAX_EDICTS : e1;
		e2 = (e2 == -1) ? MAX_EDICTS : e2;

		return e1 < e2;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Maintains a list of predicted or client created entities
//-----------------------------------------------------------------------------
class CPredictableList
{
public:
	C_BaseEntity	*GetPredictable(int slot);
	int				GetPredictableCount(void) const;

protected:
	void			AddToPredictableList(C_BaseEntity *add);
	void			RemoveFromPredictablesList(C_BaseEntity *remove);

private:
	CUtlSortVector< C_BaseEntity *, CEntIndexLessFunc >	m_Predictables;

	friend class C_BaseEntity;
};

FORCEINLINE C_BaseEntity *CPredictableList::GetPredictable(int slot)
{
	return m_Predictables[slot];
}

FORCEINLINE int CPredictableList::GetPredictableCount(void) const
{
	return m_Predictables.Count();
}

FORCEINLINE int C_BaseEntity::GetSplitUserPlayerPredictionSlot()
{
#if defined( USE_PREDICTABLEID )
	Assert(m_bPredictable || m_pPredictionContext);
#else
	Assert(m_bPredictable);
#endif
	return m_nSplitUserPlayerPredictionSlot;
}

extern CPredictableList *GetPredictables(int nSlot);

// To temporarily muck with g_pGlobals->curtime
class CCurTimeScopeGuard
{
public:
	CCurTimeScopeGuard(float flNewCurTime, bool bOptionalCondition = true);
	~CCurTimeScopeGuard();
private:

	float	m_flSavedTime;
	bool	m_bActive;
};
#endif


#endif // C_BASEENTITY_H
