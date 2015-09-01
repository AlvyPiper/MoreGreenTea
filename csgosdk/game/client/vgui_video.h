//====== Copyright � 1996-2007, Valve Corporation, All rights reserved. =======
//
// Purpose: VGUI panel which can play back video, in-engine
//
//=============================================================================

#ifndef VGUI_VIDEO_H
#define VGUI_VIDEO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/panel.h>
#include <vgui_controls/editablepanel.h>
#include "avi/ibik.h"


class VideoPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( VideoPanel, vgui::EditablePanel );
public:

	VideoPanel( unsigned int nXPos, unsigned int nYPos, unsigned int nHeight, unsigned int nWidth );

	~VideoPanel( void );

	virtual void Activate( void );
	virtual void Paint( void );
	virtual void DoModal( void );
	virtual void OnKeyCodeTyped( vgui::KeyCode code );
	virtual void OnKeyCodePressed( vgui::KeyCode code );
	virtual void OnClose( void );
	virtual void GetPanelPos( int &xpos, int &ypos );

	void SetExitCommand( const char *pExitCommand )
	{
		if ( pExitCommand && pExitCommand[0] )
		{
			Q_strncpy( m_szExitCommand, pExitCommand, MAX_PATH );
		}
	}

	bool BeginPlayback( const char *pFilename );
	void StopPlayback( void );

	void SetBlackBackground( bool bBlack ){ m_bBlackBackground = bBlack; }
	void SetAllowInterrupt( bool bAllowInterrupt ) { m_bAllowInterruption = bAllowInterrupt; }

protected:

	virtual void OnTick( void );
	virtual void OnCommand( const char *pcCommand ) { BaseClass::OnCommand( pcCommand ); }
	virtual void OnVideoOver();

protected:
	BIKMaterial_t	m_BIKHandle;
	IMaterial		*m_pMaterial;
	int				m_nPlaybackHeight;			// Calculated to address ratio changes
	int				m_nPlaybackWidth;
	char			m_szExitCommand[MAX_PATH];	// This call is fired at the engine when the video finishes or is interrupted

	float			m_flU;	// U,V ranges for video on its sheet
	float			m_flV;

	bool			m_bLooping;
	bool			m_bStopAllSounds;
	bool			m_bAllowInterruption;
	bool			m_bBlackBackground;
	int				m_nShutdownCount;

	bool			m_bStarted;
};


// Creates a VGUI panel which plays a video and executes a client command at its finish (if specified)
extern bool VideoPanel_Create( unsigned int nXPos, unsigned int nYPos, 
							   unsigned int nWidth, unsigned int nHeight, 
							   const char *pVideoFilename, 
							   const char *pExitCommand = NULL );

#endif // VGUI_VIDEO_H
