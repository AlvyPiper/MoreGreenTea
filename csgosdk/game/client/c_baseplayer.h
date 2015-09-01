//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Client-side CBasePlayer.
//
//			- Manages the player's flashlight effect.
//
//===========================================================================//

#ifndef C_BASEPLAYER_H
#define C_BASEPLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_playerlocaldata.h"
#include "c_basecombatcharacter.h"
#include "playerstate.h"
#include "usercmd.h"
#include "shareddefs.h"
#include "timedevent.h"
#include "smartptr.h"
#include "fx_water.h"
#include "hintsystem.h"
#include "soundemittersystem/isoundemittersystembase.h"
#include "c_env_fog_controller.h"
#include "C_PostProcessController.h"
#include "C_ColorCorrection.h"

#ifndef LOG_BASEPLAYER
#define LOG_BASEPLAYER
#endif

#define SHOUTMEMBERPLAYER(memberName, member) if (typeid(member) == typeid(float)) { LOG_BASEPLAYER("0x%X | %s = %f (size 0x%X)", offsetof(class C_BasePlayer,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class C_BasePlayer,  member), memberName, member, sizeof(member)); } else { LOG_BASEPLAYER("0x%X | %s = %d (size 0x%X)", offsetof(class C_BasePlayer,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class C_BasePlayer,  member), memberName, member, sizeof(member)); }

class C_BaseCombatWeapon;
class C_BaseViewModel;
class C_FuncLadder;

extern int g_nKillCamMode;
extern int g_nKillCamTarget1;
extern int g_nKillCamTarget2;

class C_CommandContext
{
public:
	bool			needsprocessing;
	CUserCmd		cmd;
	int				command_number;
};

class C_PredictionError
{
public:
	float	time;
	Vector	error;
};

#define CHASE_CAM_DISTANCE		96.0f
#define WALL_OFFSET				6.0f


enum PlayerRenderMode_t
{
	PLAYER_RENDER_NONE = 0,
	PLAYER_RENDER_FIRSTPERSON,
	PLAYER_RENDER_THIRDPERSON,
};


bool IsInFreezeCam(void);

//-----------------------------------------------------------------------------
// Purpose: Base Player class
//-----------------------------------------------------------------------------
class C_BasePlayer : public C_BaseCombatCharacter
{
public:
	DECLARE_CLASS(C_BasePlayer, C_BaseCombatCharacter);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_BasePlayer();
	virtual			~C_BasePlayer();
	virtual void	Spawn(void);
	virtual void	SharedSpawn(); // Shared between client and server.
	Class_T		Classify(void) { return CLASS_PLAYER; }
	// IClientEntity overrides.
	virtual void	OnPreDataChanged(DataUpdateType_t updateType);
	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	PreDataUpdate(DataUpdateType_t updateType);
	virtual void	PostDataUpdate(DataUpdateType_t updateType);
	virtual void	ReceiveMessage(int classID, bf_read &msg);
	virtual void	OnRestore();
	virtual void	MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType);
	virtual void	GetToolRecordingState(KeyValues *msg);
	void	SetAnimationExtension(const char *pExtension);
	C_BaseViewModel			*GetViewModel(int viewmodelindex = 0);
	C_BaseCombatWeapon		*GetActiveWeapon(void) const;
	const char				*GetTracerType(void);
	// View model prediction setup
	virtual void			CalcView(Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
	virtual void			CalcViewModelView(const Vector& eyeOrigin, const QAngle& eyeAngles);
	// Handle view smoothing when going up stairs
	void					SmoothViewOnStairs(Vector& eyeOrigin);
	virtual float			CalcRoll(const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed);
	void					CalcViewRoll(QAngle& eyeAngles);
	virtual void			CalcViewBob(Vector& eyeOrigin);
	void					CreateWaterEffects(void);
	virtual void			SetPlayerUnderwater(bool state);
	void					UpdateUnderwaterState(void);
	bool					IsPlayerUnderwater(void) { return m_bPlayerUnderwater; }
	virtual	C_BaseCombatCharacter *ActivePlayerCombatCharacter(void) { return this; }
	virtual Vector			Weapon_ShootPosition();
	virtual bool			Weapon_CanUse(C_BaseCombatWeapon *pWeapon);
	virtual void			Weapon_DropPrimary(void) {}
	virtual Vector			GetAutoaimVector(float flScale);
	void					SetSuitUpdate(char *name, int fgroup, int iNoRepeat);
	// Input handling
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd *pCmd);
	virtual void	AvoidPhysicsProps(CUserCmd *pCmd);
	virtual void	PlayerUse(void);
	CBaseEntity		*FindUseEntity(void);
	virtual bool	IsUseableEntity(CBaseEntity *pEntity, unsigned int requiredCaps);
	// Data handlers
	virtual bool	IsPlayer(void) const { return true; }
	virtual int		GetHealth() const { return m_iHealth; }
	int				GetBonusProgress() const { return m_iBonusProgress; }
	int				GetBonusChallenge() const { return m_iBonusChallenge; }
	// observer mode
	virtual int				GetObserverMode() const;
	virtual CBaseEntity*	GetObserverTarget() const;
	void					SetObserverTarget(EHANDLE hObserverTarget);
	bool					AudioStateIsUnderwater(Vector vecMainViewOrigin);
	bool					IsObserver() const;
	bool					IsHLTV() const;
	bool					IsReplay() const;
	void					ResetObserverMode();
	bool					IsBot(void) const { return false; }

	// Eye position..
	virtual Vector			EyePosition();
	virtual const QAngle&	EyeAngles(); // Direction of eyes
	void					EyePositionAndVectors(Vector *pPosition, Vector *pForward, Vector *pRight, Vector *pUp);
	virtual const QAngle&	LocalEyeAngles(); // Direction of eyes
	// This can be overridden to return something other than m_pRagdoll if the mod uses separate entities for ragdolls.
	virtual IRagdoll* GetRepresentativeRagdoll() const;
	// override the initial bone position for ragdolls
	virtual void GetRagdollInitBoneArrays(matrix3x4a_t *pDeltaBones0, matrix3x4a_t *pDeltaBones1, matrix3x4a_t *pCurrentBones, float boneDt);
	// Returns eye vectors
	void			EyeVectors(Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL) { AngleVectors(EyeAngles(), pForward, pRight, pUp); }
	void			CacheVehicleView(void);	// Calculate and cache the position of the player in the vehicle
	bool			IsSuitEquipped(void) { return m_Local.m_bWearingSuit; };
	virtual void	TeamChange(int iNewTeam);
	// Flashlight
	void	Flashlight(void);
	void	UpdateFlashlight(void);
	void	TurnOffFlashlight(void);	// TERROR
	virtual const char *GetFlashlightTextureName(void) const { return NULL; } // TERROR
	virtual float GetFlashlightFOV(void) const { return 0.0f; } // TERROR
	virtual float GetFlashlightFarZ(void) const { return 0.0f; } // TERROR
	virtual float GetFlashlightLinearAtten(void) const { return 0.0f; } // TERROR
	virtual bool CastsFlashlightShadows(void) const { return true; } // TERROR
	virtual void GetFlashlightOffset(const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, Vector *pVecOffset) const;
	//Vector	m_vecFlashlightOrigin;
	//Vector	m_vecFlashlightForward;
	//Vector	m_vecFlashlightUp;
	//Vector	m_vecFlashlightRight;
	// Weapon selection code
	virtual bool			IsAllowedToSwitchWeapons(void) { return !IsObserver(); }
	virtual C_BaseCombatWeapon	*GetActiveWeaponForSelection(void);
	virtual C_BaseAnimating* GetRenderedWeaponModel();
	virtual bool			IsOverridingViewmodel(void) { return false; };
	virtual int				DrawOverriddenViewmodel(C_BaseViewModel *pViewmodel, int flags, const RenderableInstance_t &instance) { return 0; };
	virtual float			GetDefaultAnimSpeed(void) { return 1.0; }
	void					SetMaxSpeed(float flMaxSpeed) { m_flMaxspeed = flMaxSpeed; }
	float					MaxSpeed() const		{ return m_flMaxspeed; }
	// Should this object cast shadows?
	virtual ShadowType_t	ShadowCastType() { return SHADOWS_NONE; }
	virtual bool			ShouldReceiveProjectedTextures(int flags) { return false; }
	// Makes sure s_pLocalPlayer is properly initialized
	void					CheckForLocalPlayer(int nSplitScreenSlot);
	/// Is the passed in player one of the split screen users
	static bool				IsLocalPlayer(const C_BaseEntity *pl);
	/// is this player a local player ( call when you have already verified that your pointer really is a C_BasePlayer )
	inline bool				IsLocalPlayer(void) const;
	// Global/static methods
	virtual void			ThirdPersonSwitch(bool bThirdperson);
	bool					ShouldDrawLocalPlayer();
	static C_BasePlayer		*GetLocalPlayer(int nSlot = -1);
	static void				SetRemoteSplitScreenPlayerViewsAreLocalPlayer(bool bSet); //if true, calls to GetLocalPlayer() will return a remote splitscreen player when applicable.
	static bool				HasAnyLocalPlayer();
	static int				GetSplitScreenSlotForPlayer(C_BaseEntity *pl);
	void					AddSplitScreenPlayer(C_BasePlayer *pOther);
	void					RemoveSplitScreenPlayer(C_BasePlayer *pOther);
	CUtlVector<CHandle<C_BasePlayer>> &GetSplitScreenPlayers();
	bool					IsSplitScreenPartner(C_BasePlayer *pPlayer);
	bool					IsSplitScreenPlayer() const;
	int						GetSplitScreenPlayerSlot();
	virtual IClientModelRenderable*	GetClientModelRenderable();
	virtual bool			PreRender(int nSplitScreenPlayerSlot);
	int						GetUserID(void) const;
	virtual bool			CanSetSoundMixer(void);
	virtual C_BaseEntity	*GetSoundscapeListener();
	void					AddToPlayerSimulationList(C_BaseEntity *other);
	void					SimulatePlayerSimulatedEntities(void);
	void					RemoveFromPlayerSimulationList(C_BaseEntity *ent);
	void					ClearPlayerSimulationList(void);
	virtual void			PhysicsSimulate(void);
	virtual unsigned int	PhysicsSolidMaskForEntity(void) const { return MASK_PLAYERSOLID; }
	virtual bool			ShouldPredict(void); // Prediction stuff
	virtual C_BasePlayer	*GetPredictionOwner(void);
	virtual void			PreThink(void);
	virtual void			PostThink(void);
	virtual void			ItemPreFrame(void);
	virtual void			ItemPostFrame(void);
	virtual void			AbortReload(void);
	virtual void			SelectLastItem(void);
	virtual void			Weapon_SetLast(C_BaseCombatWeapon *pWeapon);
	virtual bool			Weapon_ShouldSetLast(C_BaseCombatWeapon *pOldWeapon, C_BaseCombatWeapon *pNewWeapon) { return true; }
	virtual bool			Weapon_ShouldSelectItem(C_BaseCombatWeapon *pWeapon);
	virtual	bool			Weapon_Switch(C_BaseCombatWeapon *pWeapon, int viewmodelindex = 0);		// Switch to given weapon if has ammo (false if failed)
	virtual C_BaseCombatWeapon *GetLastWeapon(void) { return m_hLastWeapon.Get(); }
	void					ResetAutoaim(void);
	virtual void 			SelectItem(const char *pstr, int iSubType = 0);
	virtual void			UpdateClientData(void);
	virtual float			GetFOV(void) const;
	virtual int				GetDefaultFOV(void) const;
	virtual bool			IsZoomed(void)	{ return false; }
	bool					SetFOV(CBaseEntity *pRequester, int FOV, float zoomRate, int iZoomStart = 0);
	void					ClearZoomOwner(void);
	float					GetFOVDistanceAdjustFactor();
	virtual void			ViewPunch(const QAngle &angleOffset);
	void					ViewPunchReset(float tolerance = 0);
	void					UpdateButtonState(int nUserCmdButtonMask);
	int						GetImpulse(void) const;
	virtual bool			Simulate();
	virtual bool			ShouldInterpolate();
	virtual bool			ShouldDraw();
	virtual int				DrawModel(int flags, const RenderableInstance_t &instance);
	// Called when not in tactical mode. Allows view to be overriden for things like driving a tank.
	virtual void			OverrideView(CViewSetup *pSetup);
	C_BaseEntity			*GetViewEntity(void) const { return m_hViewEntity; }
	const char *			GetPlayerName(); // returns the player name
	virtual const Vector	GetPlayerMins(void) const; // uses local player
	virtual const Vector	GetPlayerMaxs(void) const; // uses local player
	virtual void			UpdateCollisionBounds(void);
	bool					IsPlayerDead();
	bool					IsPoisoned(void) { return m_Local.m_bPoisoned; }
	virtual C_BaseEntity*	GetUseEntity(void) const;
	virtual C_BaseEntity*	GetPotentialUseEntity(void) const;
	IClientVehicle			*GetVehicle();
	const IClientVehicle	*GetVehicle() const;
	bool					IsInAVehicle() const	{ return (NULL != m_hVehicle.Get()) ? true : false; }
	virtual void			SetVehicleRole(int nRole);
	void					LeaveVehicle(void);
	bool					UsingStandardWeaponsInVehicle(void);
	virtual void			SetAnimation(PLAYER_ANIM playerAnim);
	float					GetTimeBase(void) const;
	float					GetFinalPredictedTime() const;
	bool					IsInVGuiInputMode() const;
	bool					IsInViewModelVGuiInputMode() const;
	C_CommandContext*		GetCommandContext();
	// Get the command number associated with the current usercmd we're running (if in predicted code).
	int						CurrentCommandNumber() const;
	const CUserCmd*			GetCurrentUserCommand() const;
	virtual const QAngle&	GetPunchAngle();
	void					SetPunchAngle(const QAngle &angle);
	float					GetWaterJumpTime() const;
	void					SetWaterJumpTime(float flWaterJumpTime);
	float					GetSwimSoundTime(void) const;
	void					SetSwimSoundTime(float flSwimSoundTime);
	float					GetDeathTime(void) { return m_flDeathTime; }
	void					SetPreviouslyPredictedOrigin(const Vector &vecAbsOrigin);
	const Vector&			GetPreviouslyPredictedOrigin() const;
	virtual float			GetMinFOV() const; // CS wants to allow small FOVs for zoomed-in AWPs.
	virtual void			DoMuzzleFlash();
	virtual void			PlayPlayerJingle();
	virtual void			UpdateStepSound(surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity);
	virtual void			PlayStepSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force);
	virtual surfacedata_t * GetFootstepSurface(const Vector &origin, const char *surfaceName);
	virtual void			GetStepSoundVelocities(float *velwalk, float *velrun);
	virtual void			SetStepSoundTime(stepsoundtimes_t iStepSoundTime, bool bWalking);
	// Called by prediction when it detects a prediction correction.
	// vDelta is the line from where the client had predicted the player to at the usercmd in question,
	// to where the server says the client should be at said usercmd.
	void					NotePredictionError(const Vector &vDelta);
	// Called by the renderer to apply the prediction error smoothing.
	void					GetPredictionErrorSmoothingVector(Vector &vOffset);
	virtual void			ExitLadder() {}
	surfacedata_t *			GetLadderSurface(const Vector &origin);
	void					ForceButtons(int nButtons);
	void					UnforceButtons(int nButtons);
	void					SetLadderNormal(Vector vecLadderNormal) { m_vecLadderNormal = vecLadderNormal; }
	const Vector&			GetLadderNormal(void) const { return m_vecLadderNormal; }
	int						GetLadderSurfaceProps(void) const { return m_ladderSurfaceProps; }
	// Hints
	virtual CHintSystem		*Hints(void) { return NULL; }
	bool					ShouldShowHints(void) { return Hints() ? Hints()->ShouldShowHints() : false; }
	bool 					HintMessage(int hint, bool bForce = false, bool bOnlyIfClear = false) { return Hints() ? Hints()->HintMessage(hint, bForce, bOnlyIfClear) : false; }
	void 					HintMessage(const char *pMessage) { if (Hints()) Hints()->HintMessage(pMessage); }
	virtual	IMaterial*		GetHeadLabelMaterial(void);
	// Fog
	virtual fogparams_t		*GetFogParams(void) { return &m_CurrentFog; }
	void					FogControllerChanged(bool bSnap);
	void					UpdateFogController(void);
	void					UpdateFogBlend(void);
	C_PostProcessController* GetActivePostProcessController() const;
	C_ColorCorrection*		GetActiveColorCorrection() const;
	void					IncrementEFNoInterpParity();
	int						GetEFNoInterpParity() const;
	float					GetFOVTime(void){ return m_flFOVTime; }
	PlayerRenderMode_t 		GetPlayerRenderMode(int nSlot);
	virtual void			OnAchievementAchieved(int iAchievement) {}
	// RecvProxies
	static void				RecvProxy_LocalVelocityX(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_LocalVelocityY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_LocalVelocityZ(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_ObserverTarget(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_ObserverMode(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_LocalOriginXY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_LocalOriginZ(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_NonLocalOriginXY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_NonLocalOriginZ(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_NonLocalCellOriginXY(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void				RecvProxy_NonLocalCellOriginZ(const CRecvProxyData *pData, void *pStruct, void *pOut);
	virtual bool			ShouldRegenerateOriginFromCellBits() const;
	virtual void			CalcPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	void					CalcVehicleView(IClientVehicle *pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov);
	virtual void			CalcObserverView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual Vector			GetChaseCamViewOffset(CBaseEntity *target);
	void					CalcChaseCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	void					CalcInEyeCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual void			CalcDeathCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual void			CalcRoamingView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual void			CalcFreezeCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	// Check to see if we're in vgui input mode...
	void DetermineVguiInputMode(CUserCmd *pCmd);
	// Used by prediction, sets the view angles for the player
	virtual void SetLocalViewAngles(const QAngle &viewAngles);
	virtual void SetViewAngles(const QAngle& ang);
	// used by client side player footsteps 
	surfacedata_t* GetGroundSurface();
	// Did we just enter a vehicle this frame?
	bool			JustEnteredVehicle();
	float GetStepSize(void) const { return m_Local.m_flStepSize; }
	virtual bool IsDucked(void) const { return m_Local.m_bDucked; }
	virtual bool IsDucking(void) const { return m_Local.m_bDucking; }
	virtual float GetFallVelocity(void) { return m_Local.m_flFallVelocity; }
	void ForceSetupBonesAtTimeFakeInterpolation(matrix3x4a_t *pBonesOut, float curtimeOffset);

	friend class CPrediction;
	friend class CASW_Prediction;
	// HACK FOR TF2 Prediction
	friend class CTFGameMovementRecon;
	friend class CGameMovement;
	friend class CTFGameMovement;
	friend class CCSGameMovement;
	friend class CHL2GameMovement;
	friend class CPortalGameMovement;
	friend class CASW_MarineGameMovement;
	friend class CPaintGameMovement;
	friend class CMoveHelperClient;

	const char *GetLastKnownPlaceName(void) const	{ return m_szLastPlaceName; }	// return the last nav place name the player occupied
	float GetLaggedMovementValue(void){ return m_flLaggedMovementValue; }
	bool  ShouldGoSouth(Vector vNPCForward, Vector vNPCRight); //Such a bad name.
	void SetOldPlayerZ(float flOld) { m_flOldPlayerZ = flOld; }
	const fogplayerparams_t& GetPlayerFog() const { return m_PlayerFog; }

	// Make sure no one calls this...
	C_BasePlayer& operator=(const C_BasePlayer& src);
	C_BasePlayer(const C_BasePlayer &); // not defined, not accessible

	struct StepSoundCache_t {
		StepSoundCache_t() : m_usSoundNameIndex(0) {}
		CSoundParameters	m_SoundParameters;
		unsigned short		m_usSoundNameIndex;
	};

	void dump();

public:// 0x1310
	int								m_iCoachingTeam;
	fogparams_t						m_CurrentFog;
	EHANDLE							m_hOldFogController;
	int								m_StuckLast;
	float							m_flDuckPercent;
	float							m_flTotalDuckTime;
	float							m_flLastOverDuckTime;
	CPlayerLocalData				m_Local; // Data for only the local player
	EHANDLE							m_hTonemapController;
	CPlayerState					pl; // Data common to all other players, too
	int								m_iFOV;				// field of view
	int								m_iFOVStart;		// starting value of the FOV changing over time (client only)
	int								m_afButtonLast;
	int								m_afButtonPressed;
	int								m_afButtonReleased;
	int								m_nButtons;
	int								m_nImpulse;
	CNetworkVar(int, m_ladderSurfaceProps);
	int								m_flPhysics;
	float							m_flFOVTime;		// starting time of the FOV zoom
	float							m_flWaterJumpTime;  // used to be called teleport_time
	float							m_flSwimSoundTime;
	float							m_flStepSoundTime;
	float							somefloat;
	float							m_surfaceFriction;
	Vector							m_vecLadderNormal;
	char							m_szAnimExtension[32];
	int								m_nOldTickBase;
	int								m_iBonusProgress;
	int								m_iBonusChallenge;
	float							m_flMaxspeed;
	EHANDLE							m_hZoomOwner;				// 0x1610
	char							pad_0x1614[0xC];			// 0x1614
	int								m_vphysicsCollisionState;	// 0x1620
	char							pad_0x1624[0x28];			// 0x1624
	CUserCmd						m_cmd;						// 0x164C
	int								m_afPhysicsFlags;			// 0x16B0
	EHANDLE							m_hVehicle;					// 0x16B4
	CHandle<C_BaseCombatWeapon>		m_hLastWeapon;				// 0x16B8
	CHandle< C_BaseViewModel >		m_hViewModel[2];			// 0x16C0 players own view models, left & right hand
	char							pad_0x16C4[32/*0x20*/];		// 0x16C4
	bool							m_fOnTarget; 				// 0x16E4 For weapon prediction
	EHANDLE							m_hUseEntity;				// 0x16E8
	int								m_iDefaultFOV;				// 0x16EC default FOV if no other zooms are occurring
	int								m_afButtonForced;			// 0x16F0 These are forced onto the player's inputs
	CUserCmd*						m_pCurrentCommand;			// 0x16F4
	EHANDLE							m_hViewEntity;				// 0x16F8
	bool							m_bShouldDrawPlayerWhileUsingViewEntity; // 0x16FC
	EHANDLE							m_hConstraintEntity;		// 0x1700 Movement constraints
	Vector							m_vecConstraintCenter;		// 0x1704
	float							m_flConstraintRadius;		// 0x1710
	float							m_flConstraintWidth;		// 0x1714
	float							m_flConstraintSpeedFactor;  // 0x1718
	bool							m_bConstraintPastRadius;	// 0x171C
	void*							somePtr;					// 0x1720
	int								someInt;					// 0x1724
	int								m_iObserverMode;			// 0x1728 if in spectator mode != 0
	bool							m_bActiveCameraMan;			// 0x172C
	bool							m_bCameraManXRay;
	bool							m_bCameraManOverview;
	bool							m_bCameraManScoreBoard;
	Vector							m_uCameraManGraphs;
	EHANDLE							m_hObserverTarget;			// current observer target
	float							m_flObserverChaseDistance;	// last distance to observer taget
	Vector							m_vecFreezeFrameStart;
	float							m_flFreezeFrameStartTime;	// Time at which we entered freeze frame observer mode
	float							m_flFreezeFrameDistance;
	bool							m_bWasFreezeFraming;		// 0x1758
	int								reserve1;					// 0x175C
	int								reserve2;					// 0x1760
	float							m_flDeathTime;				// 0x1764
	float							m_fForceTeam;				// 0x1768
	CInterpolatedVar< Vector >		m_iv_vecViewOffset;			// 0x176C
	char							pad_0x1798[24];				// 0x1798
	Vector							m_vecWaterJumpVel; 			// 0x17B0
	QAngle							m_vecOldViewAngles;			// 0x17BC
	bool							m_bWasFrozen;				// 0x17C8		
	int								m_nTickBase;				// 0x17CC
	int								m_nFinalPredictedTick;		// 0x17D0
	EHANDLE							m_hCurrentVguiScreen;		// 0x17D4
	bool							m_bFlashlightEnabled[MAX_SPLITSCREEN_PLAYERS];
	CUtlVector<CHandle<C_BaseEntity>> m_SimulatedByThisPlayer;	// 0x17DC
	float							m_flOldPlayerZ;				// 0x17F0
	float							m_flOldPlayerViewOffsetZ;	// 0x17F4
	Vector							m_vecVehicleViewOrigin;		// 0x17F8 Used to store the calculated view of the player while riding in a vehicle
	QAngle							m_vecVehicleViewAngles;		// 0x1804 Vehicle angles
	float							m_flVehicleViewFOV;			// 0x1810
	int								m_nVehicleViewSavedFrame;	// 0x1814 Used to mark which frame was the last one the view was calculated for
	int								m_iOldAmmo[MAX_AMMO_TYPES]; // 0x1818
	C_CommandContext				m_CommandContext;			// 0x1898
	float							m_flWaterSurfaceZ;
	bool							m_bResampleWaterSurface;
	TimedEvent						m_tWaterParticleTimer;
	CSmartPtr<WaterDebrisEffect>	m_pWaterEmitter;
	bool							m_bPlayerUnderwater;
	float							m_flNextAvoidanceTime;
	float							m_flAvoidanceRight;
	float							m_flAvoidanceForward;
	float							m_flAvoidanceDotForward;
	float							m_flAvoidanceDotRight;
	float							m_flLaggedMovementValue;
	// These are used to smooth out prediction corrections. They're most useful when colliding with vphysics objects.
	// The server will be sending constant prediction corrections, and these can help the errors not be so jerky.
	Vector							m_vecPredictionError;
	float							m_flPredictionErrorTime;
	Vector							m_vecPreviouslyPredictedOrigin; // Used to determine if non-gamemovement game code has teleported, or tweaked the player's origin
	char							m_szLastPlaceName[MAX_PLACE_NAME_LENGTH];	// received from the server
	int								m_surfaceProps; // Texture names and surface data, used by CGameMovement
	surfacedata_t*					m_pSurfaceData;	// 0x1968	
	char							m_chTextureType;
	bool							m_bSentFreezeFrame;
	float							m_flFreezeZOffset;
	byte							m_ubEFNoInterpParity;
	byte							m_ubOldEFNoInterpParity;
	char							pad_0x1978[68];			// 0x1978
	//CUtlVector<CHandle<CBasePlayer>> m_hSplitScreenPlayers;
	//CHandle< CBasePlayer > m_hSplitOwner;
	int								m_nSplitScreenSlot; //-1 == not a split player // 0x19B0
	bool							m_bIsLocalPlayer;
	StepSoundCache_t				m_StepSoundCache[2]; // One for left and one for right side of step
	CNetworkHandle(CPostProcessController, m_hPostProcessCtrl);	// active postprocessing controller
	CNetworkHandle(CColorCorrection, m_hColorCorrectionCtrl);		// active FXVolume color correction
	fogplayerparams_t				m_PlayerFog; // fog params
	int								unknownint;
	float							m_totalPlayTime;
	float							bigassfloat_1;
	float							bigassfloat_2;
	float							bigassfloat_3; // 0x1B88
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	char							pad_0x1B8C[92];
	ITexture*						m_pTexture; // 0x1BE8 
	char							pad_0x1BEC[12];
	/*IPlayerAnimState*/void*		m_pPlayerAnimState; // 0x1BF8 
	char							pad_0x1BFC[4];
	bool							m_bIsScoped; // 0x1C00 
	bool							m_bIsWalking; // 0x1C01 
	bool							m_bResumeZoom; // 0x1C02 
	int								m_iPlayerState; // 0x1C04 
	bool							m_bIsDefusing; // 0x1C08 
	bool							m_bIsGrabbingHostage; // 0x1C09 
	bool							m_bIsRescuing; // 0x1C0A 
	float							m_fImmuneToGunGameDamageTime; // 0x1C0C 
	char							pad_0x1C10[4]; //int
	bool							m_bGunGameImmunity; // 0x1C14 
	bool							m_bHasMovedSinceSpawn; // 0x1C15 
	bool							m_bMadeFinalGunGameProgressiveKill; // 0x1C16 
	int								m_iGunGameProgressiveWeaponIndex; // 0x1C18 
	int								m_iNumGunGameTRKillPoints; // 0x1C1C 
	int								m_iNumGunGameKillsWithCurrentWeapon; // 0x1C20 
	int								m_iNumRoundKills; // 0x1C24 
	char							pad_0x1C28[4]; //int
	float							m_fMolotovUseTime; // 0x1C2C 
	bool							m_bInBombZone; // 0x1C30 
	bool							m_bInBuyZone; // 0x1C31 
	bool							m_bInNoDefuseArea; // 0x1C32
	int								m_iThrowGrenadeCounter; // 0x1C34 
	bool							m_bWaitForNoAttack; // 0x1C38 
	bool							m_bIsRespawningForDMBonus; // 0x1C39
	float							m_flGuardianTooFarDistFrac; // 0x1C3C added on 26 MAY 15
	int								unknewint; // 0x1C40 added on 26 MAY 15
	bool							m_bKilledByTaser; // 0x1C3
	int								m_iMoveState; //0x1C48 
	bool							m_bCanMoveDuringFreezePeriod; //0x1C4C 
	bool							m_isCurrentGunGameLeader; //0x1C4D 
	bool							m_isCurrentGunGameTeamLeader; //0x1C4E 
	char							m_szArmsModel[257]; //0x1C4F 
	int								m_iAddonBits; //0x1D50 
	int								m_iPrimaryAddon; //0x1D54 
	int								m_iSecondaryAddon; //0x1D58 
	int								m_iProgressBarDuration; //0x1D5C 
	float							m_flProgressBarStartTime; //0x1D60 
	float							m_flStamina; //0x1D64 
	int								m_iDirection; //0x1D68 
	int								m_iShotsFired; //0x1D6C 
	int								m_nNumFastDucks; //0x1D70 
	bool							m_bDuckOverride; //0x1D74 
	bool							m_bNightVisionOn; //0x1D75 
	bool							m_bHasNightVision; //0x1D76 
	float							m_flVelocityModifier; //0x1D78 
	float							m_flGroundAccelLinearFracLastTime; //0x1D7C 
	int								m_iStartAccount; //0x1D80 
	int								m_totalHitsOnServer; //0x1D84 
	char							pad_0x1D88[0x4]; //0x1D88
	int								m_hRagdoll; //0x1D8C 
	int								m_hCarriedHostage; //0x1D90 
	int								m_hCarriedHostageProp; //0x1D94 
	char							pad_0x1D98[0x18]; //0x1D98
	float							m_flFlashMaxAlpha; //0x1DB0 
	float							m_flFlashDuration; //0x1DB4 
	float							m_flLastPrimaryAttack; //0x1DB8 
	char							pad_0x1DBC[0x4]; //0x1DBC
	Vector							m_flSomeOrigin; //0x1DC0 
	int								m_iGlowIndex; //0x1DCC 
	char							pad_0x1DD0[0x1C]; //0x1DD0
	CBaseEntity*					m_pAnimating; //0x1DEC 
	char							pad_0x1DF0[0xC]; //0x1DF0
	char							m_iMatchStats_Kills[120]; //0x1DFC 
	char							m_iMatchStats_Damage[120]; //0x1E74 
	char							m_iMatchStats_EquipmentValue[120]; //0x1EEC 
	char							m_iMatchStats_MoneySaved[120]; //0x1F64 
	char							m_iMatchStats_KillReward[120]; //0x1FDC 
	char							m_iMatchStats_LiveTime[120]; //0x2054 
	char							m_iMatchStats_Deaths[120]; //0x20CC 
	char							m_iMatchStats_Assists[120]; //0x2144 
	char							m_iMatchStats_HeadShotKills[120]; //0x21BC 
	char							m_iMatchStats_Objective[120]; //0x2234 
	char							m_iMatchStats_CashEarned[120]; //0x22AC 
	char							pad_0x2324[0xC]; //0x2324
	CountdownTimer					m_someTimer1; //0x2330 
	char							pad_0x233C[0x28]; //0x233C
	Vector							m_flOriginOfSomeSort; //0x2364 
	char							pad_0x2370[0x14]; //0x2370
	unsigned short					m_unRoundStartEquipmentValue; //0x2384 
	unsigned short					m_unFreezetimeEndEquipmentValue; //0x2386 
	int								m_rank_1; //0x2388 
	int								m_rank_2; //0x238C 
	int								m_rank_3; //0x2390 
	int								m_rank_4; //0x2394 
	int								m_rank_5; //0x2398 
	int								m_rank_6; //0x239C 
	int								m_unMusicID; //0x23A0 
	int								m_iAccount; //0x23A4 
	bool							m_bHasHelmet; // 0x23A8
	int								m_iClass; //0x23AC 
	int								m_ArmorValue; //0x23B0 
	Vector							m_angEyeAngles; //0x23B4 
	bool							m_bHasDefuser; //0x23C0 
	bool							m_bInHostageRescueZone; //0x23C1 
	char							pad_0x23C2[0x12]; //0x23C2
	bool							m_bHud_RadarHidden; //0x23D4 
	int								m_nLastKillerIndex; //0x23D8 
	int								m_nLastConcurrentKilled; //0x23DC 
	int								m_nDeathCamMusic; //0x23E0 
	CInterpolatedVar<Vector>		m_iv_angEyeAngles; //0x23E4 
	int								m_iCrossHairID; //0x2410 
	CountdownTimer					m_someTimer2; //0x2414 
	char							pad_0x2420[0x8]; //0x2420
	CountdownTimer					m_someTimer3; //0x2428 
	char							pad_0x2434[0x8]; //0x2434
	// keeps going
};

EXTERN_RECV_TABLE(DT_BasePlayer);


inline void C_BasePlayer::dump()
{
	printf("C_BasePlayer -> Address = 0x%X\n", (DWORD)this);
	SHOUTMEMBERPLAYER("m_iCoachingTeam", m_iCoachingTeam);
	SHOUTMEMBERPLAYER("m_CurrentFog.dirPrimary[0]", m_CurrentFog.dirPrimary.m_Value[0]);
	SHOUTMEMBERPLAYER("m_CurrentFog.colorPrimary", m_CurrentFog.colorPrimary);
	SHOUTMEMBERPLAYER("m_CurrentFog.colorSecondary", m_CurrentFog.colorSecondary);
	SHOUTMEMBERPLAYER("m_CurrentFog.colorPrimaryLerpTo", m_CurrentFog.colorPrimaryLerpTo);
	SHOUTMEMBERPLAYER("m_CurrentFog.colorSecondaryLerpTo", m_CurrentFog.colorSecondaryLerpTo);
	SHOUTMEMBERPLAYER("m_CurrentFog.start", m_CurrentFog.start);
	SHOUTMEMBERPLAYER("m_CurrentFog.end", m_CurrentFog.end);
	SHOUTMEMBERPLAYER("m_CurrentFog.farz", m_CurrentFog.farz);
	SHOUTMEMBERPLAYER("m_CurrentFog.maxdensity", m_CurrentFog.maxdensity);
	SHOUTMEMBERPLAYER("m_CurrentFog.startLerpTo", m_CurrentFog.startLerpTo);
	SHOUTMEMBERPLAYER("m_CurrentFog.endLerpTo", m_CurrentFog.endLerpTo);
	SHOUTMEMBERPLAYER("m_CurrentFog.maxdensityLerpTo", m_CurrentFog.maxdensityLerpTo);
	SHOUTMEMBERPLAYER("m_CurrentFog.lerptime", m_CurrentFog.lerptime);
	SHOUTMEMBERPLAYER("m_CurrentFog.duration", m_CurrentFog.duration);
	SHOUTMEMBERPLAYER("m_CurrentFog.enable", m_CurrentFog.enable);
	SHOUTMEMBERPLAYER("m_CurrentFog.blend", m_CurrentFog.blend);
	SHOUTMEMBERPLAYER("m_CurrentFog.reserved", m_CurrentFog.reserved);
	SHOUTMEMBERPLAYER("m_CurrentFog.HDRColorScale", m_CurrentFog.HDRColorScale);
	SHOUTMEMBERPLAYER("m_hOldFogController", m_hOldFogController);
	SHOUTMEMBERPLAYER("m_StuckLast", m_StuckLast);
	SHOUTMEMBERPLAYER("m_Local.m_chAreaBits", m_Local.m_chAreaBits);
	SHOUTMEMBERPLAYER("m_Local.m_chAreaPortalBits", m_Local.m_chAreaPortalBits);
	SHOUTMEMBERPLAYER("m_Local.m_nStepside", m_Local.m_nStepside);
	SHOUTMEMBERPLAYER("m_Local.m_nOldButtons", m_Local.m_nOldButtons);
	SHOUTMEMBERPLAYER("m_Local.m_flFOVRate", m_Local.m_flFOVRate);
	SHOUTMEMBERPLAYER("m_Local.m_iHideHUD", m_Local.m_iHideHUD);
	SHOUTMEMBERPLAYER("m_Local.m_nDuckTimeMsecs", m_Local.m_nDuckTimeMsecs);
	SHOUTMEMBERPLAYER("m_Local.m_nDuckJumpTimeMsecs", m_Local.m_nDuckJumpTimeMsecs);
	SHOUTMEMBERPLAYER("m_Local.m_nJumpTimeMsecs", m_Local.m_nJumpTimeMsecs);
	SHOUTMEMBERPLAYER("m_Local.m_flFallVelocity", m_Local.m_flFallVelocity);
	SHOUTMEMBERPLAYER("m_Local.m_flOldFallVelocity", m_Local.m_flOldFallVelocity);
	SHOUTMEMBERPLAYER("m_Local.m_flStepSize", m_Local.m_flStepSize);
	SHOUTMEMBERPLAYER("m_Local.m_viewPunchAngle", m_Local.m_viewPunchAngle);
	SHOUTMEMBERPLAYER("m_Local.m_aimPunchAngle", m_Local.m_aimPunchAngle);
	SHOUTMEMBERPLAYER("m_Local.m_aimPunchAngleVel", m_Local.m_aimPunchAngleVel);
	SHOUTMEMBERPLAYER("m_Local.m_bDucked", m_Local.m_bDucked);
	SHOUTMEMBERPLAYER("m_Local.m_bDucking", m_Local.m_bDucking);
	SHOUTMEMBERPLAYER("m_Local.m_bInDuckJump", m_Local.m_bInDuckJump);
	SHOUTMEMBERPLAYER("m_Local.m_bDrawViewmodel", m_Local.m_bDrawViewmodel);
	SHOUTMEMBERPLAYER("m_Local.m_bWearingSuit", m_Local.m_bWearingSuit);
	SHOUTMEMBERPLAYER("m_Local.m_bPoisoned", m_Local.m_bPoisoned);
	SHOUTMEMBERPLAYER("m_Local.m_bAllowAutoMovement", m_Local.m_bAllowAutoMovement);
	SHOUTMEMBERPLAYER("m_Local.m_bInLanding", m_Local.m_bInLanding);
	SHOUTMEMBERPLAYER("m_Local.m_flLandingTime", m_Local.m_flLandingTime);
	SHOUTMEMBERPLAYER("m_Local.m_vecClientBaseVelocity", m_Local.m_vecClientBaseVelocity);
	SHOUTMEMBERPLAYER("m_Local.m_bAutoAimTarget", m_Local.m_bAutoAimTarget);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.scale", m_Local.m_skybox3d.scale.m_Value);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.origin", m_Local.m_skybox3d.origin.m_Value[0]);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.area", m_Local.m_skybox3d.area.m_Value);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.dirPrimary", m_Local.m_skybox3d.fog.dirPrimary.m_Value[0]);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.colorPrimary", m_Local.m_skybox3d.fog.colorPrimary);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.colorSecondary", m_Local.m_skybox3d.fog.colorSecondary);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.colorPrimaryLerpTo", m_Local.m_skybox3d.fog.colorPrimaryLerpTo);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.colorSecondaryLerpTo", m_Local.m_skybox3d.fog.colorSecondaryLerpTo);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.start", m_Local.m_skybox3d.fog.start);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.end", m_Local.m_skybox3d.fog.end);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.farz", m_Local.m_skybox3d.fog.farz);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.maxdensity", m_Local.m_skybox3d.fog.maxdensity);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.startLerpTo", m_Local.m_skybox3d.fog.startLerpTo);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.endLerpTo", m_Local.m_skybox3d.fog.endLerpTo);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.maxdensityLerpTo", m_Local.m_skybox3d.fog.maxdensityLerpTo);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.lerptime", m_Local.m_skybox3d.fog.lerptime);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.duration", m_Local.m_skybox3d.fog.duration);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.blend	", m_Local.m_skybox3d.fog.blend);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.enable", m_Local.m_skybox3d.fog.enable);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.reserved", m_Local.m_skybox3d.fog.reserved);
	SHOUTMEMBERPLAYER("m_Local.m_skybox3d.fog.HDRColorScale", m_Local.m_skybox3d.fog.HDRColorScale);
	SHOUTMEMBERPLAYER("m_Local.m_audio.localSound", m_Local.m_audio.localSound);
	SHOUTMEMBERPLAYER("m_Local.m_audio.soundscapeIndex", m_Local.m_audio.soundscapeIndex);
	SHOUTMEMBERPLAYER("m_Local.m_audio.localBits", m_Local.m_audio.localBits);
	SHOUTMEMBERPLAYER("m_Local.m_audio.entIndex", m_Local.m_audio.entIndex);
	SHOUTMEMBERPLAYER("m_Local.m_bSlowMovement", m_Local.m_bSlowMovement);
	SHOUTMEMBERPLAYER("m_Local.reserved", m_Local.reserved);
	SHOUTMEMBERPLAYER("m_hTonemapController", m_hTonemapController);
	SHOUTMEMBERPLAYER("pl.deadflag", pl.deadflag);
	SHOUTMEMBERPLAYER("pl.v_angle", pl.v_angle[0]);
	SHOUTMEMBERPLAYER("m_iFOV", m_iFOV);
	SHOUTMEMBERPLAYER("m_iFOVStart", m_iFOVStart);
	SHOUTMEMBERPLAYER("m_afButtonLast", m_afButtonLast);
	SHOUTMEMBERPLAYER("m_afButtonPressed", m_afButtonPressed);
	SHOUTMEMBERPLAYER("m_afButtonReleased", m_afButtonReleased);
	SHOUTMEMBERPLAYER("m_nButtons", m_nButtons);
	SHOUTMEMBERPLAYER("m_nImpulse", m_nImpulse);
	SHOUTMEMBERPLAYER("m_ladderSurfaceProps", m_ladderSurfaceProps);
	SHOUTMEMBERPLAYER("m_flPhysics", m_flPhysics);
	SHOUTMEMBERPLAYER("m_flFOVTime", m_flFOVTime);
	SHOUTMEMBERPLAYER("m_flWaterJumpTime", m_flWaterJumpTime);
	SHOUTMEMBERPLAYER("m_flSwimSoundTime", m_flSwimSoundTime);
	SHOUTMEMBERPLAYER("m_flStepSoundTime", m_flStepSoundTime);
	SHOUTMEMBERPLAYER("somefloat", somefloat);
	SHOUTMEMBERPLAYER("m_surfaceFriction", m_surfaceFriction);
	SHOUTMEMBERPLAYER("m_vecLadderNormal[0]", m_vecLadderNormal[0]);
	SHOUTMEMBERPLAYER("m_szAnimExtension", m_szAnimExtension);
	SHOUTMEMBERPLAYER("m_nOldTickBase", m_nOldTickBase);
	SHOUTMEMBERPLAYER("m_iBonusProgress", m_iBonusProgress);
	SHOUTMEMBERPLAYER("m_iBonusChallenge", m_iBonusChallenge);
	SHOUTMEMBERPLAYER("m_flMaxspeed", m_flMaxspeed);
	SHOUTMEMBERPLAYER("m_hZoomOwner", m_hZoomOwner);
	SHOUTMEMBERPLAYER("pad_0x1614", pad_0x1614);
	SHOUTMEMBERPLAYER("m_vphysicsCollisionState", m_vphysicsCollisionState);
	SHOUTMEMBERPLAYER("pad_0x1624", pad_0x1624);
	SHOUTMEMBERPLAYER("m_cmd.command_number", m_cmd.command_number);
	SHOUTMEMBERPLAYER("m_cmd.tick_count", m_cmd.tick_count);
	SHOUTMEMBERPLAYER("m_cmd.viewangles[0]", m_cmd.viewangles[0]);
	SHOUTMEMBERPLAYER("m_cmd.aimdirection[0]", m_cmd.aimdirection[0]);
	SHOUTMEMBERPLAYER("m_cmd.forwardmove", m_cmd.forwardmove);
	SHOUTMEMBERPLAYER("m_cmd.sidemove", m_cmd.sidemove);
	SHOUTMEMBERPLAYER("m_cmd.upmove", m_cmd.upmove);
	SHOUTMEMBERPLAYER("m_cmd.buttons", m_cmd.buttons);
	SHOUTMEMBERPLAYER("m_cmd.impulse", m_cmd.impulse);
	SHOUTMEMBERPLAYER("m_cmd.weaponselect", m_cmd.weaponselect);
	SHOUTMEMBERPLAYER("m_cmd.weaponsubtype", m_cmd.weaponsubtype);
	SHOUTMEMBERPLAYER("m_cmd.random_seed", m_cmd.random_seed);
	SHOUTMEMBERPLAYER("m_cmd.mousedx", m_cmd.mousedx);
	SHOUTMEMBERPLAYER("m_cmd.mousedy", m_cmd.mousedy);
	SHOUTMEMBERPLAYER("m_cmd.hasbeenpredicted", m_cmd.hasbeenpredicted);
	SHOUTMEMBERPLAYER("m_cmd.pad_0x4C", m_cmd.pad_0x4C);
	SHOUTMEMBERPLAYER("m_afPhysicsFlags", m_afPhysicsFlags);
	SHOUTMEMBERPLAYER("m_hVehicle", m_hVehicle);
	SHOUTMEMBERPLAYER("m_hLastWeapon", m_hLastWeapon);
	SHOUTMEMBERPLAYER("m_hViewModel", m_hViewModel);
	SHOUTMEMBERPLAYER("pad_0x16C4", pad_0x16C4);
	SHOUTMEMBERPLAYER("m_fOnTarget", m_fOnTarget);
	SHOUTMEMBERPLAYER("m_hUseEntity", m_hUseEntity);
	SHOUTMEMBERPLAYER("m_iDefaultFOV", m_iDefaultFOV);
	SHOUTMEMBERPLAYER("m_afButtonForced", m_afButtonForced);
	SHOUTMEMBERPLAYER("m_pCurrentCommand", m_pCurrentCommand);
	SHOUTMEMBERPLAYER("m_hViewEntity", m_hViewEntity);
	SHOUTMEMBERPLAYER("m_bShouldDrawPlayerWhileUsingViewEntity", m_bShouldDrawPlayerWhileUsingViewEntity);
	SHOUTMEMBERPLAYER("m_hConstraintEntity", m_hConstraintEntity);
	SHOUTMEMBERPLAYER("m_vecConstraintCenter[0]", m_vecConstraintCenter[0]);
	SHOUTMEMBERPLAYER("m_flConstraintRadius", m_flConstraintRadius);
	SHOUTMEMBERPLAYER("m_flConstraintWidth", m_flConstraintWidth);
	SHOUTMEMBERPLAYER("m_flConstraintSpeedFactor", m_flConstraintSpeedFactor);
	SHOUTMEMBERPLAYER("m_bConstraintPastRadius", m_bConstraintPastRadius);
	SHOUTMEMBERPLAYER("somePtr", somePtr);
	SHOUTMEMBERPLAYER("someInt", someInt);
	SHOUTMEMBERPLAYER("m_iObserverMode", m_iObserverMode);
	SHOUTMEMBERPLAYER("m_bActiveCameraMan", m_bActiveCameraMan);
	SHOUTMEMBERPLAYER("m_bCameraManXRay", m_bCameraManXRay);
	SHOUTMEMBERPLAYER("m_bCameraManOverview", m_bCameraManOverview);
	SHOUTMEMBERPLAYER("m_bCameraManScoreBoard", m_bCameraManScoreBoard);
	SHOUTMEMBERPLAYER("m_uCameraManGraphs[0]", m_uCameraManGraphs[0]);
	SHOUTMEMBERPLAYER("m_hObserverTarget", m_hObserverTarget);
	SHOUTMEMBERPLAYER("m_flObserverChaseDistance", m_flObserverChaseDistance);
	SHOUTMEMBERPLAYER("m_vecFreezeFrameStart[0]", m_vecFreezeFrameStart[0]);
	SHOUTMEMBERPLAYER("m_flFreezeFrameStartTime", m_flFreezeFrameStartTime);
	SHOUTMEMBERPLAYER("m_flFreezeFrameDistance", m_flFreezeFrameDistance);
	SHOUTMEMBERPLAYER("m_bWasFreezeFraming", m_bWasFreezeFraming);
	SHOUTMEMBERPLAYER("reserve1", reserve1);
	SHOUTMEMBERPLAYER("reserve2", reserve2);
	SHOUTMEMBERPLAYER("m_flDeathTime", m_flDeathTime);
	SHOUTMEMBERPLAYER("m_fForceTeam", m_fForceTeam);
	SHOUTMEMBERPLAYER("pad_0x1798", pad_0x1798);
	SHOUTMEMBERPLAYER("m_vecWaterJumpVel[0]", m_vecWaterJumpVel[0]);
	SHOUTMEMBERPLAYER("m_vecOldViewAngles[0]", m_vecOldViewAngles[0]);
	SHOUTMEMBERPLAYER("m_bWasFrozen", m_bWasFrozen);
	SHOUTMEMBERPLAYER("m_nTickBase", m_nTickBase);
	SHOUTMEMBERPLAYER("m_nFinalPredictedTick", m_nFinalPredictedTick);
	SHOUTMEMBERPLAYER("m_hCurrentVguiScreen", m_hCurrentVguiScreen);
	SHOUTMEMBERPLAYER("m_bFlashlightEnabled[0]", m_bFlashlightEnabled[0]);
	SHOUTMEMBERPLAYER("m_flOldPlayerZ", m_flOldPlayerZ);
	SHOUTMEMBERPLAYER("m_flOldPlayerViewOffsetZ", m_flOldPlayerViewOffsetZ);
	SHOUTMEMBERPLAYER("m_vecVehicleViewOrigin[0]", m_vecVehicleViewOrigin[0]);
	SHOUTMEMBERPLAYER("m_vecVehicleViewAngles[0]", m_vecVehicleViewAngles[0]);
	SHOUTMEMBERPLAYER("m_flVehicleViewFOV", m_flVehicleViewFOV);
	SHOUTMEMBERPLAYER("m_nVehicleViewSavedFrame", m_nVehicleViewSavedFrame);
	SHOUTMEMBERPLAYER("m_iOldAmmo", m_iOldAmmo);
	SHOUTMEMBERPLAYER("m_CommandContext", m_CommandContext.needsprocessing);
	SHOUTMEMBERPLAYER("m_flWaterSurfaceZ", m_flWaterSurfaceZ);
	SHOUTMEMBERPLAYER("m_bResampleWaterSurface", m_bResampleWaterSurface);
	SHOUTMEMBERPLAYER("m_bPlayerUnderwater", m_bPlayerUnderwater);
	SHOUTMEMBERPLAYER("m_flNextAvoidanceTime", m_flNextAvoidanceTime);
	SHOUTMEMBERPLAYER("m_flAvoidanceRight", m_flAvoidanceRight);
	SHOUTMEMBERPLAYER("m_flAvoidanceForward", m_flAvoidanceForward);
	SHOUTMEMBERPLAYER("m_flAvoidanceDotForward", m_flAvoidanceDotForward);
	SHOUTMEMBERPLAYER("m_flAvoidanceDotRight", m_flAvoidanceDotRight);
	SHOUTMEMBERPLAYER("m_flLaggedMovementValue", m_flLaggedMovementValue);
	SHOUTMEMBERPLAYER("m_vecPredictionError[0]", m_vecPredictionError[0]);
	SHOUTMEMBERPLAYER("m_flPredictionErrorTime", m_flPredictionErrorTime);
	SHOUTMEMBERPLAYER("m_vecPreviouslyPredictedOrigin[0]", m_vecPreviouslyPredictedOrigin[0]);
	SHOUTMEMBERPLAYER("m_szLastPlaceName", m_szLastPlaceName);
	SHOUTMEMBERPLAYER("m_surfaceProps", m_surfaceProps);
	SHOUTMEMBERPLAYER("m_pSurfaceData", m_pSurfaceData);
	SHOUTMEMBERPLAYER("m_chTextureType", m_chTextureType);
	SHOUTMEMBERPLAYER("m_bSentFreezeFrame", m_bSentFreezeFrame);
	SHOUTMEMBERPLAYER("m_flFreezeZOffset", m_flFreezeZOffset);
	SHOUTMEMBERPLAYER("m_ubEFNoInterpParity", m_ubEFNoInterpParity);
	SHOUTMEMBERPLAYER("m_ubOldEFNoInterpParity", m_ubOldEFNoInterpParity);
	SHOUTMEMBERPLAYER("pad_0x1978", pad_0x1978);
	SHOUTMEMBERPLAYER("m_nSplitScreenSlot", m_nSplitScreenSlot);
	SHOUTMEMBERPLAYER("m_bIsLocalPlayer", m_bIsLocalPlayer);
	SHOUTMEMBERPLAYER("m_hPostProcessCtrl", m_hPostProcessCtrl);
	SHOUTMEMBERPLAYER("m_hColorCorrectionCtrl", m_hColorCorrectionCtrl);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_hCtrl", m_PlayerFog.m_hCtrl);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flTransitionTime", m_PlayerFog.m_flTransitionTime);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_OldColor", m_PlayerFog.m_OldColor);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flOldStart", m_PlayerFog.m_flOldStart);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flOldEnd", m_PlayerFog.m_flOldEnd);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flOldHDRColorScale", m_PlayerFog.m_flOldHDRColorScale);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flOldFarZ", m_PlayerFog.m_flOldFarZ);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_NewColor", m_PlayerFog.m_NewColor);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flNewStart", m_PlayerFog.m_flNewStart);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flNewEnd", m_PlayerFog.m_flNewEnd);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flNewMaxDensity", m_PlayerFog.m_flNewMaxDensity);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flNewHDRColorScale", m_PlayerFog.m_flNewHDRColorScale);
	SHOUTMEMBERPLAYER("m_PlayerFog.m_flNewFarZ", m_PlayerFog.m_flNewFarZ);
	SHOUTMEMBERPLAYER("unknownint", unknownint);
	SHOUTMEMBERPLAYER("m_totalTime", m_totalPlayTime);
	SHOUTMEMBERPLAYER("bigassfloat_1", bigassfloat_1);
	SHOUTMEMBERPLAYER("bigassfloat_2", bigassfloat_2);
	SHOUTMEMBERPLAYER("bigassfloat_3", bigassfloat_3);
	///////
	SHOUTMEMBERPLAYER("pad_0x1B8C[92]", pad_0x1B8C);
	SHOUTMEMBERPLAYER("m_pTexture", m_pTexture);
	SHOUTMEMBERPLAYER("pad_0x1BEC[12]", pad_0x1BEC);
	SHOUTMEMBERPLAYER("m_pPlayerAnimState", m_pPlayerAnimState);
	SHOUTMEMBERPLAYER("pad_0x1BFC[4]", pad_0x1BFC);
	SHOUTMEMBERPLAYER("m_bIsScoped", m_bIsScoped);
	SHOUTMEMBERPLAYER("m_bIsWalking", m_bIsWalking);
	SHOUTMEMBERPLAYER("m_bResumeZoom", m_bResumeZoom);
	SHOUTMEMBERPLAYER("m_iPlayerState",	m_iPlayerState);
	SHOUTMEMBERPLAYER("m_bIsDefusing", m_bIsDefusing);
	SHOUTMEMBERPLAYER("m_bIsGrabbingHostage", m_bIsGrabbingHostage);
	SHOUTMEMBERPLAYER("m_bIsRescuing", m_bIsRescuing);
	SHOUTMEMBERPLAYER("m_fImmuneToGunGameDamageTime", m_fImmuneToGunGameDamageTime);
	SHOUTMEMBERPLAYER("pad_0x1C10[4]", pad_0x1C10);
	SHOUTMEMBERPLAYER("m_bGunGameImmunity", m_bGunGameImmunity);
	SHOUTMEMBERPLAYER("m_bHasMovedSinceSpawn", m_bHasMovedSinceSpawn);
	SHOUTMEMBERPLAYER("m_bMadeFinalGunGameProgressiveKill", m_bMadeFinalGunGameProgressiveKill);
	SHOUTMEMBERPLAYER("m_iGunGameProgressiveWeaponIndex", m_iGunGameProgressiveWeaponIndex);
	SHOUTMEMBERPLAYER("m_iNumGunGameTRKillPoints", 	m_iNumGunGameTRKillPoints		);
	SHOUTMEMBERPLAYER("m_iNumGunGameKillsWithCurrentWeapon",m_iNumGunGameKillsWithCurrentWeapon);
	SHOUTMEMBERPLAYER("m_iNumRoundKills", m_iNumRoundKills);
	SHOUTMEMBERPLAYER("pad_0x1C28[4]", pad_0x1C28);
	SHOUTMEMBERPLAYER("m_fMolotovUseTime", m_fMolotovUseTime);
	SHOUTMEMBERPLAYER("m_bInBombZone", m_bInBombZone);
	SHOUTMEMBERPLAYER("m_bInBuyZone", m_bInBuyZone);
	SHOUTMEMBERPLAYER("m_bInNoDefuseArea", m_bInNoDefuseArea);
	SHOUTMEMBERPLAYER("m_iThrowGrenadeCounter", m_iThrowGrenadeCounter);
	SHOUTMEMBERPLAYER("m_bWaitForNoAttack", m_bWaitForNoAttack);
	SHOUTMEMBERPLAYER("m_bIsRespawningForDMBonus", m_bIsRespawningForDMBonus);
	SHOUTMEMBERPLAYER("m_flGuardianTooFarDistFrac", m_flGuardianTooFarDistFrac); // Added 26 MAY 15
	SHOUTMEMBERPLAYER("unknewint", unknewint); // Added 26 MAY 15
	SHOUTMEMBERPLAYER("m_bKilledByTaser", m_bKilledByTaser);
	SHOUTMEMBERPLAYER("m_iMoveState", m_iMoveState);
	SHOUTMEMBERPLAYER("m_bCanMoveDuringFreezePeriod", m_bCanMoveDuringFreezePeriod);
	SHOUTMEMBERPLAYER("m_isCurrentGunGameLeader", m_isCurrentGunGameLeader);
	SHOUTMEMBERPLAYER("m_isCurrentGunGameTeamLeader", m_isCurrentGunGameTeamLeader);
	SHOUTMEMBERPLAYER("m_szArmsModel[256]", m_szArmsModel);
	SHOUTMEMBERPLAYER("m_iAddonBits", m_iAddonBits);
	SHOUTMEMBERPLAYER("m_iPrimaryAddon", m_iPrimaryAddon);
	SHOUTMEMBERPLAYER("m_iSecondaryAddon",m_iSecondaryAddon);
	SHOUTMEMBERPLAYER("m_iProgressBarDuration", m_iProgressBarDuration);
	SHOUTMEMBERPLAYER("m_flProgressBarStartTime", m_flProgressBarStartTime);
	SHOUTMEMBERPLAYER("m_flStamina", m_flStamina);
	SHOUTMEMBERPLAYER("m_iDirection", m_iDirection);
	SHOUTMEMBERPLAYER("m_iShotsFired", m_iShotsFired);
	SHOUTMEMBERPLAYER("m_nNumFastDucks", m_nNumFastDucks);
	SHOUTMEMBERPLAYER("m_bDuckOverride", m_bDuckOverride);
	SHOUTMEMBERPLAYER("m_bNightVisionOn", m_bNightVisionOn);
	SHOUTMEMBERPLAYER("m_bHasNightVision", m_bHasNightVision);
	SHOUTMEMBERPLAYER("m_flVelocityModifier", m_flVelocityModifier);
	SHOUTMEMBERPLAYER("m_flGroundAccelLinearFracLastTime", m_flGroundAccelLinearFracLastTime);
	SHOUTMEMBERPLAYER("m_iStartAccount", m_iStartAccount);
	SHOUTMEMBERPLAYER("m_totalHitsOnServer", m_totalHitsOnServer);
	SHOUTMEMBERPLAYER("pad_0x1D88[4]", pad_0x1D88);
	SHOUTMEMBERPLAYER("m_hRagdoll", m_hRagdoll);
	SHOUTMEMBERPLAYER("m_hCarriedHostage",	m_hCarriedHostage);
	SHOUTMEMBERPLAYER("m_hCarriedHostageProp", 	m_hCarriedHostageProp);
	SHOUTMEMBERPLAYER("pad_0x1D98[0x18]",	pad_0x1D98);
	SHOUTMEMBERPLAYER("m_flFlashMaxAlpha", m_flFlashMaxAlpha);
	SHOUTMEMBERPLAYER("m_flFlashDuration", m_flFlashDuration);
	SHOUTMEMBERPLAYER("m_flLastPrimaryAttack", m_flLastPrimaryAttack);
	SHOUTMEMBERPLAYER("pad_0x1DBC[4]", pad_0x1DBC);
	SHOUTMEMBERPLAYER("m_flSomeOrigin", m_flSomeOrigin[0]);
	SHOUTMEMBERPLAYER("m_iGlowIndex", m_iGlowIndex);
	SHOUTMEMBERPLAYER("pad_0x1DD0[0x1C]", pad_0x1DD0);
	SHOUTMEMBERPLAYER("m_pAnimating", m_pAnimating);
	SHOUTMEMBERPLAYER("pad_0x1DF0[0xC]", pad_0x1DF0);
	SHOUTMEMBERPLAYER("m_iMatchStats_Kills[120]", m_iMatchStats_Kills);
	SHOUTMEMBERPLAYER("m_iMatchStats_Damage[120]", m_iMatchStats_Damage	);
	SHOUTMEMBERPLAYER("m_iMatchStats_EquipmentValue[120]", m_iMatchStats_EquipmentValue	);
	SHOUTMEMBERPLAYER("m_iMatchStats_MoneySaved[120]", m_iMatchStats_MoneySaved);
	SHOUTMEMBERPLAYER("m_iMatchStats_KillReward[120]", m_iMatchStats_KillReward);
	SHOUTMEMBERPLAYER("m_iMatchStats_LiveTime[120]", m_iMatchStats_LiveTime);
	SHOUTMEMBERPLAYER("m_iMatchStats_Deaths[120]",	m_iMatchStats_Deaths);
	SHOUTMEMBERPLAYER("m_iMatchStats_Assists[120]",	m_iMatchStats_Assists);
	SHOUTMEMBERPLAYER("m_iMatchStats_HeadShotKills[120]", m_iMatchStats_HeadShotKills);
	SHOUTMEMBERPLAYER("m_iMatchStats_Objective[120]", m_iMatchStats_Objective);
	SHOUTMEMBERPLAYER("m_iMatchStats_CashEarned[120]", m_iMatchStats_CashEarned);
	SHOUTMEMBERPLAYER("pad_0x2324[0xC]", pad_0x2324);
	SHOUTMEMBERPLAYER("m_someTimer1", m_someTimer1.m_duration);
	SHOUTMEMBERPLAYER("pad_0x233C[0x28]", pad_0x233C);
	SHOUTMEMBERPLAYER("m_flOriginOfSomeSort", m_flOriginOfSomeSort[0]);
	SHOUTMEMBERPLAYER("pad_0x2370[0x14]", pad_0x2370);
	SHOUTMEMBERPLAYER("m_unRoundStartEquipmentValue", m_unRoundStartEquipmentValue);
	SHOUTMEMBERPLAYER("m_unFreezetimeEndEquipmentValue", m_unFreezetimeEndEquipmentValue);
	SHOUTMEMBERPLAYER("m_rank_1", m_rank_1);
	SHOUTMEMBERPLAYER("m_rank_2", m_rank_2);
	SHOUTMEMBERPLAYER("m_rank_3", m_rank_3);
	SHOUTMEMBERPLAYER("m_rank_4", m_rank_4);
	SHOUTMEMBERPLAYER("m_rank_5", m_rank_5);
	SHOUTMEMBERPLAYER("m_rank_6", m_rank_6);
	SHOUTMEMBERPLAYER("m_unMusicID", m_unMusicID);
	SHOUTMEMBERPLAYER("m_iAccount",	m_iAccount);
	SHOUTMEMBERPLAYER("m_bHasHelmet", m_bHasHelmet);
	SHOUTMEMBERPLAYER("m_iClass", m_iClass);
	SHOUTMEMBERPLAYER("m_ArmorValue", m_ArmorValue);
	SHOUTMEMBERPLAYER("m_angEyeAngles", m_angEyeAngles);
	SHOUTMEMBERPLAYER("m_bHasDefuser", m_bHasDefuser);
	SHOUTMEMBERPLAYER("m_bInHostageRescueZone",	m_bInHostageRescueZone);
	SHOUTMEMBERPLAYER("pad_0x23C2[0x12]",	pad_0x23C2);
	SHOUTMEMBERPLAYER("m_bHud_RadarHidden", m_bHud_RadarHidden);
	SHOUTMEMBERPLAYER("m_nLastKillerIndex", m_nLastKillerIndex);
	SHOUTMEMBERPLAYER("m_nLastConcurrentKilled", m_nLastConcurrentKilled);
	SHOUTMEMBERPLAYER("m_nDeathCamMusic", m_nDeathCamMusic);
	SHOUTMEMBERPLAYER("m_iv_angEyeAngles", m_iv_angEyeAngles.m_pValue);
	SHOUTMEMBERPLAYER("m_iCrossHairID", m_iCrossHairID);
	SHOUTMEMBERPLAYER("m_someTimer2", m_someTimer2.m_duration);
	SHOUTMEMBERPLAYER("pad_0x2420[8]",	pad_0x2420);
	SHOUTMEMBERPLAYER("m_someTimer3", m_someTimer3.m_duration);
	SHOUTMEMBERPLAYER("pad_0x2434[8]", pad_0x2434);
	printf("\n");
}

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline C_BasePlayer* ToBasePlayer(C_BaseEntity *pEntity)
{
	if (!pEntity)
		return NULL;
	return static_cast<C_BasePlayer*>(pEntity);
}

inline const C_BasePlayer *ToBasePlayer(const C_BaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;
	return static_cast<const C_BasePlayer *>(pEntity);
}

inline IClientVehicle *C_BasePlayer::GetVehicle()
{ 
	C_BaseEntity *pVehicleEnt = m_hVehicle.Get();
	return pVehicleEnt ? pVehicleEnt->GetClientVehicle() : NULL;
}

inline bool C_BasePlayer::IsObserver() const
{
	return (GetObserverMode() != OBS_MODE_NONE);
}

inline int C_BasePlayer::GetImpulse(void) const
{
	return m_nImpulse;
}


inline C_CommandContext* C_BasePlayer::GetCommandContext()
{
	return &m_CommandContext;
}

inline int CBasePlayer::CurrentCommandNumber() const
{
	Assert(m_pCurrentCommand);
	if (!m_pCurrentCommand)
		return 0;
	return m_pCurrentCommand->command_number;
}

inline const CUserCmd *CBasePlayer::GetCurrentUserCommand() const
{
	Assert(m_pCurrentCommand);
	return m_pCurrentCommand;
}

extern bool g_bEngineIsHLTV;

inline bool C_BasePlayer::IsHLTV() const
{
	return m_bIsLocalPlayer && g_bEngineIsHLTV;
}

inline bool	C_BasePlayer::IsLocalPlayer(void) const
{
	return m_bIsLocalPlayer;
}


#endif // C_BASEPLAYER_H
