//============ Copyright (c) Valve Corporation, All rights reserved. ============
//
// Functionality to render a glowing outline around client renderable objects.
//
//===============================================================================

#ifndef GLOW_OUTLINE_EFFECT_H
#define GLOW_OUTLINE_EFFECT_H

#if defined( COMPILER_MSVC )
#pragma once
#endif

#include "utlvector.h"
#include "mathlib/vector.h"


class CViewSetup;
class CMatRenderContextPtr;

static const int GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS = -1;

class CGlowObjectManager
{
public:
	CGlowObjectManager() : m_nFirstFreeSlot( GlowObjectDefinition_t::END_OF_FREE_LIST )
	{
	}

	struct GlowObjectDefinition_t
	{
		bool ShouldDraw() const { return m_pEntity && (m_bRenderWhenOccluded || m_bRenderWhenUnoccluded); /*&& m_pEntity->ShouldDraw();*/ }
		bool IsUnused() const { return m_nNextFreeSlot != GlowObjectDefinition_t::ENTRY_IN_USE; }
		void DrawModel();

		C_BaseEntity* m_pEntity; // 0x00
		Vector m_vGlowColor; // 0x04
		float m_flGlowAlpha; // 0x10
		int m_hSomeHandle; // 0x14
		int fill1; // 0x18
		float m_flBloomAmount; // 0x1C
		int fill2; // 0x20
		bool m_bRenderWhenOccluded; // 0x24
		bool m_bRenderWhenUnoccluded; // 0x25
		bool m_bFullBloomRender; // 0x26
		int m_nFullBloomStencilTestValue; // 0x28 only render full bloom objects if stencil is equal to this value (value of -1 implies no stencil test)
		int m_nSplitScreenSlot; // 0x2C
		// Linked list of free slots
		int m_nNextFreeSlot; // 0x30
		// Special values for GlowObjectDefinition_t::m_nNextFreeSlot
		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	}; // size = 0x34 = 52


	bool IsDuplicate(C_BaseEntity* pEntity)
	{
		for (int i = 0; i < m_GlowObjectDefinitions.Count(); i++)
		{
			if (m_GlowObjectDefinitions[i].m_pEntity == pEntity)
			{
				return true;
			}
		}
		return false;
	}

	int GetIndex(C_BaseEntity* pEntity)
	{
		if (pEntity)
		{
			for (int i = 0; i < m_GlowObjectDefinitions.Count(); i++)
			{
				if (m_GlowObjectDefinitions[i].m_pEntity == pEntity)
					return i;
			}
		}
		return 0;
	}

	int RegisterGlowObject(C_BaseEntity* pEntity, const Vector& vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded )
	{
		printf("registering entity 0x%X\n", pEntity);

		int nIndex;
		if ( m_nFirstFreeSlot == GlowObjectDefinition_t::END_OF_FREE_LIST )
		{
			//return 0;
			nIndex = m_GlowObjectDefinitions.AddToTail();
		}
		else
		{
			nIndex = m_nFirstFreeSlot;
			m_nFirstFreeSlot = m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot;
		}
		
		m_GlowObjectDefinitions[nIndex].m_pEntity = pEntity;
		m_GlowObjectDefinitions[nIndex].m_vGlowColor = vGlowColor;
		m_GlowObjectDefinitions[nIndex].m_flGlowAlpha = flGlowAlpha;
		m_GlowObjectDefinitions[nIndex].m_hSomeHandle = 0;
		m_GlowObjectDefinitions[nIndex].fill1= 0;
		m_GlowObjectDefinitions[nIndex].m_flBloomAmount = 1.0f;
		m_GlowObjectDefinitions[nIndex].fill2 = 0;
		m_GlowObjectDefinitions[nIndex].m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_GlowObjectDefinitions[nIndex].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
		m_GlowObjectDefinitions[nIndex].m_bFullBloomRender = false;
		m_GlowObjectDefinitions[nIndex].m_nFullBloomStencilTestValue = -1;
		m_GlowObjectDefinitions[nIndex].m_nSplitScreenSlot = GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS;
		m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot = GlowObjectDefinition_t::ENTRY_IN_USE;

		return nIndex;
	}

	void UnregisterGlowObject( int nGlowObjectHandle )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );

		m_GlowObjectDefinitions[nGlowObjectHandle].m_nNextFreeSlot = m_nFirstFreeSlot;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = NULL;
		m_nFirstFreeSlot = nGlowObjectHandle;
	}

	void SetEntity( int nGlowObjectHandle, C_BaseEntity *pEntity )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = pEntity;
	}

	void SetColor( int nGlowObjectHandle, const Vector &vGlowColor ) 
	{ 
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_vGlowColor = vGlowColor;
	}

	void SetAlpha( int nGlowObjectHandle, float flAlpha ) 
	{ 
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_flGlowAlpha = flAlpha;
	}

	void SetBloom(int nGlowObjectHandle, float flBloom)
	{
		Assert(!m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused());
		m_GlowObjectDefinitions[nGlowObjectHandle].m_flBloomAmount = flBloom;
	}

	void SetRenderFlags( int nGlowObjectHandle, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
	}

	void SetFullBloomRender( int nGlowObjectHandle, bool bFullBloomRender, int nStencilTestValue )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bFullBloomRender = bFullBloomRender;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_nFullBloomStencilTestValue = nStencilTestValue;
	}

	bool IsRenderingWhenOccluded( int nGlowObjectHandle ) const
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded;
	}
	
	bool IsRenderingWhenUnoccluded( int nGlowObjectHandle ) const
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded;
	}

	void RenderGlowEffects(const CViewSetup* pSetup);

public:

	void RenderGlowModels(const CViewSetup* pSetup, CMatRenderContextPtr &pRenderContext);
	void ApplyEntityGlowEffects(const CViewSetup* pSetup, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h);

	CUtlVector<GlowObjectDefinition_t> m_GlowObjectDefinitions;
	int m_nFirstFreeSlot;
};

extern CGlowObjectManager* g_pGlowObjectManager;

class CGlowObject
{
public:
	CGlowObject( C_BaseEntity *pEntity, const Vector &vGlowColor = Vector( 1.0f, 1.0f, 1.0f ), float flGlowAlpha = 1.0f, bool bRenderWhenOccluded = false, bool bRenderWhenUnoccluded = true)
	{
		m_nGlowObjectHandle = g_pGlowObjectManager->RegisterGlowObject( pEntity, vGlowColor, flGlowAlpha, bRenderWhenOccluded, bRenderWhenUnoccluded );
	}

	~CGlowObject()
	{
		g_pGlowObjectManager->UnregisterGlowObject(m_nGlowObjectHandle);
	}

	void SetEntity( C_BaseEntity *pEntity )
	{
		g_pGlowObjectManager->SetEntity(m_nGlowObjectHandle, pEntity);
	}

	void SetColor( const Vector &vGlowColor )
	{
		g_pGlowObjectManager->SetColor(m_nGlowObjectHandle, vGlowColor);
	}

	void SetAlpha( float flAlpha )
	{
		g_pGlowObjectManager->SetAlpha(m_nGlowObjectHandle, flAlpha);
	}

	void SetRenderFlags( bool bRenderWhenOccluded, bool bRenderWhenUnoccluded )
	{
		g_pGlowObjectManager->SetRenderFlags(m_nGlowObjectHandle, bRenderWhenOccluded, bRenderWhenUnoccluded);
	}

	void SetFullBloomRender( bool bFullBloomRender, int nStencilTestValue = -1 )
	{
		return g_pGlowObjectManager->SetFullBloomRender(m_nGlowObjectHandle, bFullBloomRender, nStencilTestValue);
	}

	bool IsRenderingWhenOccluded() const
	{
		return g_pGlowObjectManager->IsRenderingWhenOccluded(m_nGlowObjectHandle);
	}

	bool IsRenderingWhenUnoccluded() const
	{
		return g_pGlowObjectManager->IsRenderingWhenUnoccluded(m_nGlowObjectHandle);
	}

	bool IsRendering() const
	{
		return IsRenderingWhenOccluded() || IsRenderingWhenUnoccluded();
	}

	// Add more accessors/mutators here as needed

private:
	int m_nGlowObjectHandle;

	// Assignment & copy-construction disallowed
	CGlowObject( const CGlowObject &other );
	CGlowObject& operator=( const CGlowObject &other );
};

#endif // GLOW_OUTLINE_EFFECT_H