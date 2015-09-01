//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef C_BASEANIMATINGOVERLAY_H
#define C_BASEANIMATINGOVERLAY_H
#pragma once

#include "c_baseanimating.h"
#include "toolframework/itoolframework.h"

#ifndef LOG_BASEANIMATINGOVERLAY
#define LOG_BASEANIMATINGOVERLAY
#endif

#define SHOUTMEMBERANIMATINGOVERLAY(memberName, member) if (typeid(member) == typeid(float)) { LOG_BASEANIMATINGOVERLAY("0x%X | %s = %f (size 0x%X)", offsetof(class C_BaseAnimatingOverlay,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %f (size 0x%X)\n", offsetof(class C_BaseAnimatingOverlay,  member), memberName, member, sizeof(member)); } else { LOG_BASEANIMATINGOVERLAY("0x%X | %s = %d (size 0x%X)", offsetof(class C_BaseAnimatingOverlay,  member), memberName, member, sizeof(member)); printf("0x%X | %s = %d (size 0x%X)\n", offsetof(class C_BaseAnimatingOverlay,  member), memberName, member, sizeof(member)); }

// For shared code.
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay


class C_BaseAnimatingOverlay : public C_BaseAnimating
{
	DECLARE_CLASS(C_BaseAnimatingOverlay, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	// Inherited from C_BaseAnimating
public:
	virtual void	AccumulateLayers(IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime);
	virtual void	DoAnimationEvents(CStudioHdr *pStudioHdr);
	virtual void	GetRenderBounds(Vector& theMins, Vector& theMaxs);
	virtual CStudioHdr *OnNewModel();

	virtual bool	Interpolate(float flCurrentTime);

public:
	enum
	{
		MAX_OVERLAYS = 15,
	};

	C_BaseAnimatingOverlay();
	CAnimationLayer* GetAnimOverlay(int i);
	void			SetNumAnimOverlays(int num);	// This makes sure there is space for this # of layers.
	int				GetNumAnimOverlays() const;
	void			SetOverlayPrevEventCycle(int nSlot, float flValue);
	void			CheckInterpChanges(void);
	void			CheckForLayerPhysicsInvalidate(void);
	C_BaseAnimatingOverlay(const C_BaseAnimatingOverlay &); // not defined, not accessible
	friend void ResizeAnimationLayerCallback(void *pStruct, int offsetToUtlVector, int len);
	void CheckForLayerChanges(CStudioHdr *hdr, float currentTime);
	void			dump();

public: // members
	CUtlVector<CAnimationLayer>	m_AnimOverlay; // 0xD50
	CUtlVector<CInterpolatedVar<CAnimationLayer>> m_iv_AnimOverlay;
	float m_flOverlayPrevEventCycle[MAX_OVERLAYS];
	char pad_0xDB4[0x4]; // AnimatingOverlay ends BaseFlex begins at 0xDC4
};


inline void C_BaseAnimatingOverlay::dump()
{
	printf("C_BaseAnimatingOverlay -> Address = 0x%X\n", (DWORD)this);
	LOG_BASEANIMATINGOVERLAY("C_BaseAnimatingOverlay -> Address = 0x%X", (DWORD)this);
	SHOUTMEMBERANIMATINGOVERLAY("m_AnimOverlay", m_AnimOverlay.m_Memory.m_pMemory);
	SHOUTMEMBERANIMATINGOVERLAY("m_iv_AnimOverlay", m_iv_AnimOverlay.m_Memory.m_pMemory);
	SHOUTMEMBERANIMATINGOVERLAY("m_flOverlayPrevEventCycle[MAX_OVERLAYS]", m_flOverlayPrevEventCycle);
	SHOUTMEMBERANIMATINGOVERLAY("pad_0xDB4[4]", pad_0xDB4);
	printf("\n");
}


inline void C_BaseAnimatingOverlay::SetOverlayPrevEventCycle(int nSlot, float flValue)
{
	m_flOverlayPrevEventCycle[nSlot] = flValue;
}

EXTERN_RECV_TABLE(DT_BaseAnimatingOverlay);


#endif // C_BASEANIMATINGOVERLAY_H




