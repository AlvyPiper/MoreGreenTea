//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef C_TE_EFFECT_DISPATCH_H
#define C_TE_EFFECT_DISPATCH_H
#ifdef _WIN32
#pragma once
#endif

#include "effect_dispatch_data.h"
#include "precache_register.h"
#include "c_basetempentity.h"
#include "networkstringtable_clientdll.h"
#include "tier1/keyvalues.h"
#include "toolframework_client.h"
#include "tier0/vprof.h"
#include "particles_new.h"

typedef void (*ClientEffectCallback)( const CEffectData &data );


class CClientEffectRegistration
{
public:
	CClientEffectRegistration( const char *pEffectName, ClientEffectCallback fn )
	{
		AssertMsg1(pEffectName[0] != '\"', "Error: Effect %s. Remove quotes around the effect name in DECLARE_CLIENT_EFFECT.\n", pEffectName);
		m_pEffectName = pEffectName;
		m_pFunction = fn;
		m_pNext = s_pHead;
		s_pHead = this;
	}

public:
	const char *m_pEffectName;
	ClientEffectCallback m_pFunction;
	CClientEffectRegistration *m_pNext;

	static CClientEffectRegistration *s_pHead;
};

//
// Use this macro to register a client effect callback. 
// If you do DECLARE_CLIENT_EFFECT( MyEffectName, MyCallback ), then MyCallback will be 
// called when the server does DispatchEffect( "MyEffectName", data )
//
#define DECLARE_CLIENT_EFFECT_INTERNAL( effectName, callbackFunction ) \
	static CClientEffectRegistration ClientEffectReg_##callbackFunction( #effectName, callbackFunction );

#define DECLARE_CLIENT_EFFECT( effectName, callbackFunction )		\
	DECLARE_CLIENT_EFFECT_INTERNAL( effectName, callbackFunction )	\
	PRECACHE_REGISTER_BEGIN( DISPATCH_EFFECT, effectName )			\
	PRECACHE_REGISTER_END()

#define DECLARE_CLIENT_EFFECT_BEGIN( effectName, callbackFunction ) \
	DECLARE_CLIENT_EFFECT_INTERNAL( effectName, callbackFunction )	\
	PRECACHE_REGISTER_BEGIN( DISPATCH_EFFECT, effectName )

#define DECLARE_CLIENT_EFFECT_END()	PRECACHE_REGISTER_END()

__inline void DispatchEffectToCallback(const char *pEffectName, const CEffectData &m_EffectData, IPrecacheSystem* pPrecacheSystem)
{
	printf("entered DispatchEffectToCallback\n");
	// Built a faster lookup 
	static CUtlStringMap< CClientEffectRegistration* > map;
	static bool bInitializedMap = false;
	if (!bInitializedMap)
	{
		for (CClientEffectRegistration *pReg = CClientEffectRegistration::s_pHead; pReg; pReg = pReg->m_pNext)
		{
			// If the name matches, call it
			if (map.Defined(pReg->m_pEffectName))
			{
				Warning("Encountered multiple different effects with the same name \"%s\"!\n", pReg->m_pEffectName);
				continue;
			}

			map[pReg->m_pEffectName] = pReg;
		}
		bInitializedMap = true;
	}

	// Look through all the registered callbacks
	UtlSymId_t nSym = map.Find(pEffectName);
	if (nSym == UTL_INVAL_SYMBOL)
	{
		Warning("DispatchEffect: effect \"%s\" not found on client\n", pEffectName);
		return;
	}

	// NOTE: Here, we want to scope resource access to only be able to use
	// those resources specified as being dependencies of this effect
	pPrecacheSystem->LimitResourceAccess(DISPATCH_EFFECT, pEffectName);
	map[nSym]->m_pFunction(m_EffectData);

	// NOTE: Here, we no longer need to restrict resource access
	pPrecacheSystem->EndLimitedResourceAccess();
}


//-----------------------------------------------------------------------------
// Record effects
//-----------------------------------------------------------------------------
__inline static void RecordEffect(const char *pEffectName, const CEffectData &data)
{
	if ((data.m_fFlags & EFFECTDATA_NO_RECORD) == 0)
	{
		char pName[1024];
		Q_snprintf(pName, sizeof(pName), "TE_DispatchEffect %s", pEffectName);
		printf("> %s <\n", pName);
		printf("  time: %f\n", gpGlobals->curtime);
		printf("  originx: %f\n", data.m_vOrigin.x);
		printf("  originy: %f\n", data.m_vOrigin.y);
		printf("  originz: %f\n", data.m_vOrigin.z);
		printf("  startx: %f\n", data.m_vStart.x);
		printf("  starty: %f\n", data.m_vStart.y);
		printf("  startz: %f\n", data.m_vStart.z);
		printf("  normalx: %f\n", data.m_vNormal.x);
		printf("  normaly: %f\n", data.m_vNormal.y);
		printf("  normalz: %f\n", data.m_vNormal.z);
		printf("  anglesx: %f\n", data.m_vAngles.x);
		printf("  anglesy: %f\n", data.m_vAngles.y);
		printf("  anglesz: %f\n", data.m_vAngles.z);
		printf("  flags: %i\n", data.m_fFlags);
		printf("  scale: %f\n", data.m_flScale);
		printf("  magnitude: %f\n", data.m_flMagnitude);
		printf("  radius: %f\n", data.m_flRadius);
		printf("  color: %i\n", data.m_nColor);
		printf("  damagetype: %i\n", data.m_nDamageType);
		printf("  hitbox: %i\n", data.m_nHitBox);
		printf("  effectname: %s\n", pEffectName);
		printf("  attachmentindex: %i\n", data.m_nAttachmentIndex);
		printf("  entindex: %i\n", (void*)data.entindex()); 
	}
}

//-----------------------------------------------------------------------------
// Client version of dispatch effect, for predicted weapons
//-----------------------------------------------------------------------------
__inline void DispatchEffect(IRecipientFilter& filter, float delay, const char *pName, const CEffectData &data, IPrecacheSystem* pPrecacheSystem)
{
	//if (!te->SuppressTE(filter))
	//{
		DispatchEffectToCallback(pName, data, pPrecacheSystem);
		//RecordEffect(pName, data);
	//}
}

__inline void DispatchEffect(const char *pName, const CEffectData &data, IPrecacheSystem* pPrecacheSystem)
{
	//CPASFilter filter(data.m_vOrigin);
	//if (!te->SuppressTE(filter))
	//{
		DispatchEffectToCallback(pName, data, pPrecacheSystem);
		//RecordEffect(pName, data);
	//}
}


//-----------------------------------------------------------------------------
// Playback
//-----------------------------------------------------------------------------
__inline void DispatchEffect(IRecipientFilter& filter, float delay, KeyValues *pKeyValues, IPrecacheSystem* pPrecacheSystem)
{
	CEffectData data;
	data.m_nMaterial = 0;

	data.m_vOrigin.x = pKeyValues->GetFloat("originx");
	data.m_vOrigin.y = pKeyValues->GetFloat("originy");
	data.m_vOrigin.z = pKeyValues->GetFloat("originz");
	data.m_vStart.x = pKeyValues->GetFloat("startx");
	data.m_vStart.y = pKeyValues->GetFloat("starty");
	data.m_vStart.z = pKeyValues->GetFloat("startz");
	data.m_vNormal.x = pKeyValues->GetFloat("normalx");
	data.m_vNormal.y = pKeyValues->GetFloat("normaly");
	data.m_vNormal.z = pKeyValues->GetFloat("normalz");
	data.m_vAngles.x = pKeyValues->GetFloat("anglesx");
	data.m_vAngles.y = pKeyValues->GetFloat("anglesy");
	data.m_vAngles.z = pKeyValues->GetFloat("anglesz");
	data.m_fFlags = pKeyValues->GetInt("flags");
	data.m_flScale = pKeyValues->GetFloat("scale");
	data.m_flMagnitude = pKeyValues->GetFloat("magnitude");
	data.m_flRadius = pKeyValues->GetFloat("radius");
	const char *pSurfaceProp = pKeyValues->GetString("surfaceprop");
	data.m_nSurfaceProp = physprops->GetSurfaceIndex(pSurfaceProp);
	data.m_nDamageType = pKeyValues->GetInt("damagetype");
	data.m_nHitBox = pKeyValues->GetInt("hitbox");
	data.m_nColor = pKeyValues->GetInt("color");
	data.m_nAttachmentIndex = pKeyValues->GetInt("attachmentindex");

	// NOTE: Ptrs are our way of indicating it's an entindex
	ClientEntityHandle_t hWorld = ClientEntityList().EntIndexToHandle(0);
	data.m_hEntity = (int)pKeyValues->GetPtr("entindex", (void*)hWorld.ToInt());

	const char *pEffectName = pKeyValues->GetString("effectname");

	DispatchEffect(filter, 0.0f, pEffectName, data, pPrecacheSystem);
}

//-----------------------------------------------------------------------------
// Purpose: Displays an error effect in case of missing precache
//-----------------------------------------------------------------------------
__inline void ErrorEffectCallback(const CEffectData &data)
{
	CSmartPtr<CNewParticleEffect> pEffect = CNewParticleEffect::Create(NULL, "error");
	if (pEffect->IsValid())
	{
		pEffect->SetSortOrigin(data.m_vOrigin);
		pEffect->SetControlPoint(0, data.m_vOrigin);
		pEffect->SetControlPoint(1, data.m_vStart);
		Vector vecForward, vecRight, vecUp;
		AngleVectors(data.m_vAngles, &vecForward, &vecRight, &vecUp);
		pEffect->SetControlPointOrientation(0, vecForward, vecRight, vecUp);
	}
}


#endif // C_TE_EFFECT_DISPATCH_H
