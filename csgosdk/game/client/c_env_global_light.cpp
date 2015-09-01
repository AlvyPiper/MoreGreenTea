//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Sunlight shadow control entity.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

#include "c_baseplayer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar cl_sunlight_ortho_size;
extern ConVar cl_sunlight_depthbias;

ConVar cl_globallight_freeze( "cl_globallight_freeze", "0" );
ConVar cl_globallight_xoffset( "cl_globallight_xoffset", "-800" );
ConVar cl_globallight_yoffset( "cl_globallight_yoffset", "1600" );

//------------------------------------------------------------------------------
// Purpose : Sunlights shadow control entity
//------------------------------------------------------------------------------
class C_GlobalLight : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_GlobalLight, C_BaseEntity );

	DECLARE_CLIENTCLASS();

	virtual ~C_GlobalLight();

	void OnDataChanged( DataUpdateType_t updateType );
	void Spawn();
	bool ShouldDraw();

	void ClientThink();

private:
	Vector m_shadowDirection;
	bool m_bEnabled;
	char m_TextureName[ MAX_PATH ];
	CTextureReference m_SpotlightTexture;
	color32	m_LightColor;
	Vector m_CurrentLinearFloatLightColor;
	float m_flCurrentLinearFloatLightAlpha;
	float m_flColorTransitionTime;
	float m_flSunDistance;
	float m_flFOV;
	float m_flNearZ;
	float m_flNorthOffset;
	bool m_bEnableShadows;
	bool m_bOldEnableShadows;

	static ClientShadowHandle_t m_LocalFlashlightHandle;
};


ClientShadowHandle_t C_GlobalLight::m_LocalFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;


IMPLEMENT_CLIENTCLASS_DT(C_GlobalLight, DT_GlobalLight, CGlobalLight)
	RecvPropVector(RECVINFO(m_shadowDirection)),
	RecvPropBool(RECVINFO(m_bEnabled)),
	RecvPropString(RECVINFO(m_TextureName)),
	RecvPropInt(RECVINFO(m_LightColor), 0, RecvProxy_Int32ToColor32),
	RecvPropFloat(RECVINFO(m_flColorTransitionTime)),
	RecvPropFloat(RECVINFO(m_flSunDistance)),
	RecvPropFloat(RECVINFO(m_flFOV)),
	RecvPropFloat(RECVINFO(m_flNearZ)),
	RecvPropFloat(RECVINFO(m_flNorthOffset)),
	RecvPropBool(RECVINFO(m_bEnableShadows)),
END_RECV_TABLE()


C_GlobalLight::~C_GlobalLight()
{
	if ( m_LocalFlashlightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_LocalFlashlightHandle );
		m_LocalFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

void C_GlobalLight::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_SpotlightTexture.Init( m_TextureName, TEXTURE_GROUP_OTHER, true );
	}

	BaseClass::OnDataChanged( updateType );
}

void C_GlobalLight::Spawn()
{
	BaseClass::Spawn();

	m_bOldEnableShadows = m_bEnableShadows;

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//------------------------------------------------------------------------------
// We don't draw...
//------------------------------------------------------------------------------
bool C_GlobalLight::ShouldDraw()
{
	return false;
}

void C_GlobalLight::ClientThink()
{
	VPROF("C_GlobalLight::ClientThink");

	bool bSupressWorldLights = false;

	if ( cl_globallight_freeze.GetBool() == true )
	{
		return;
	}

	if ( m_bEnabled )
	{
		Vector vLinearFloatLightColor( m_LightColor.r, m_LightColor.g, m_LightColor.b );
		float flLinearFloatLightAlpha = m_LightColor.a;

		if ( m_CurrentLinearFloatLightColor != vLinearFloatLightColor || m_flCurrentLinearFloatLightAlpha != flLinearFloatLightAlpha )
		{
			float flColorTransitionSpeed = gpGlobals->frametime * m_flColorTransitionTime * 255.0f;

			m_CurrentLinearFloatLightColor.x = Approach( vLinearFloatLightColor.x, m_CurrentLinearFloatLightColor.x, flColorTransitionSpeed );
			m_CurrentLinearFloatLightColor.y = Approach( vLinearFloatLightColor.y, m_CurrentLinearFloatLightColor.y, flColorTransitionSpeed );
			m_CurrentLinearFloatLightColor.z = Approach( vLinearFloatLightColor.z, m_CurrentLinearFloatLightColor.z, flColorTransitionSpeed );
			m_flCurrentLinearFloatLightAlpha = Approach( flLinearFloatLightAlpha, m_flCurrentLinearFloatLightAlpha, flColorTransitionSpeed );
		}

		FlashlightState_t state;

		Vector vDirection = m_shadowDirection;
		VectorNormalize( vDirection );

		//Vector vViewUp = Vector( 0.0f, 1.0f, 0.0f );
		Vector vSunDirection2D = vDirection;
		vSunDirection2D.z = 0.0f;

		HACK_GETLOCALPLAYER_GUARD( "C_GlobalLight::ClientThink" );

		if ( !C_BasePlayer::GetLocalPlayer() )
			return;

		Vector vPos;
		QAngle EyeAngles;
		float flZNear, flZFar, flFov;

		C_BasePlayer::GetLocalPlayer()->CalcView( vPos, EyeAngles, flZNear, flZFar, flFov );
//		Vector vPos = C_BasePlayer::GetLocalPlayer()->GetAbsOrigin();
		
//		vPos = Vector( 0.0f, 0.0f, 500.0f );
		vPos = ( vPos + vSunDirection2D * m_flNorthOffset ) - vDirection * m_flSunDistance;
		vPos += Vector( cl_globallight_xoffset.GetFloat(), cl_globallight_yoffset.GetFloat(), 0.0f );

		QAngle angAngles;
		VectorAngles( vDirection, angAngles );

		Vector vForward, vRight, vUp;
		AngleVectors( angAngles, &vForward, &vRight, &vUp );

		state.m_fHorizontalFOVDegrees = m_flFOV;
		state.m_fVerticalFOVDegrees = m_flFOV;

		state.m_vecLightOrigin = vPos;
		BasisToQuaternion( vForward, vRight, vUp, state.m_quatOrientation );

		state.m_fQuadraticAtten = 0.0f;
		state.m_fLinearAtten = m_flSunDistance * 2.0f;
		state.m_fConstantAtten = 0.0f;
		state.m_FarZAtten = m_flSunDistance * 2.0f;
		state.m_Color[0] = m_CurrentLinearFloatLightColor.x * ( 1.0f / 255.0f ) * m_flCurrentLinearFloatLightAlpha;
		state.m_Color[1] = m_CurrentLinearFloatLightColor.y * ( 1.0f / 255.0f ) * m_flCurrentLinearFloatLightAlpha;
		state.m_Color[2] = m_CurrentLinearFloatLightColor.z * ( 1.0f / 255.0f ) * m_flCurrentLinearFloatLightAlpha;
		state.m_Color[3] = 0.0f; // fixme: need to make ambient work m_flAmbient;
		state.m_NearZ = 4.0f;
		state.m_FarZ = m_flSunDistance * 2.0f;
		state.m_fBrightnessScale = 2.0f;
		state.m_bGlobalLight = true;

		float flOrthoSize = 1000.0f;

		if ( flOrthoSize > 0 )
		{
			state.m_bOrtho = true;
			state.m_fOrthoLeft = -flOrthoSize;
			state.m_fOrthoTop = -flOrthoSize;
			state.m_fOrthoRight = flOrthoSize;
			state.m_fOrthoBottom = flOrthoSize;
		}
		else
		{
			state.m_bOrtho = false;
		}

		state.m_bDrawShadowFrustum = true;
		state.m_flShadowSlopeScaleDepthBias = g_pMaterialSystemHardwareConfig->GetShadowSlopeScaleDepthBias();;
		state.m_flShadowDepthBias = g_pMaterialSystemHardwareConfig->GetShadowDepthBias();
		state.m_bEnableShadows = m_bEnableShadows;
		state.m_pSpotlightTexture = m_SpotlightTexture;
		state.m_pProjectedMaterial = NULL; // don't complain cause we aren't using simple projection in this class
		state.m_nSpotlightTextureFrame = 0;

		state.m_nShadowQuality = 1; // Allow entity to affect shadow quality
//		state.m_bShadowHighRes = true;

		if ( m_bOldEnableShadows != m_bEnableShadows )
		{
			// If they change the shadow enable/disable, we need to make a new handle
			if ( m_LocalFlashlightHandle != CLIENTSHADOW_INVALID_HANDLE )
			{
				g_pClientShadowMgr->DestroyFlashlight( m_LocalFlashlightHandle );
				m_LocalFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
			}

			m_bOldEnableShadows = m_bEnableShadows;
		}

		if( m_LocalFlashlightHandle == CLIENTSHADOW_INVALID_HANDLE )
		{
			m_LocalFlashlightHandle = g_pClientShadowMgr->CreateFlashlight( state );
		}
		else
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_LocalFlashlightHandle, state );
			g_pClientShadowMgr->UpdateProjectedTexture( m_LocalFlashlightHandle, true );
		}

		bSupressWorldLights = m_bEnableShadows;
	}
	else if ( m_LocalFlashlightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_LocalFlashlightHandle );
		m_LocalFlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}

	g_pClientShadowMgr->SetShadowFromWorldLightsEnabled( !bSupressWorldLights );

	BaseClass::ClientThink();
}
