#pragma once

#include "stdafx.h"
#include "DXUTMesh.h"

#undef min
#undef max
#include "NxPhysics.h"

struct stKeyFrame
{
	int		iNumFrame;
	NxMat34	matPose;
	NxVec3	vScale;
};

class CtObjectAnimInfo
{
protected:
	bool						m_bEnabled;
	UINT						m_iCurrentFrame;
	UINT						m_iCurrentKey;	
	CGrowableArray<stKeyFrame>	m_KeyFrameArray;

public:
	CtObjectAnimInfo( VOID );

	VOID						Enable( VOID );

	VOID						Disable( VOID );

	bool						IsEnabled( VOID );

	VOID						AddKey( UINT iNumFrame, NxMat34 matPose, NxVec3 vScale );

	UINT						GetNbKey( VOID );

	NxMat34						GetPose( VOID );
	
	NxVec3						GetScale( VOID );

	bool						FrameMove( VOID );

	VOID						Clear( VOID );
};