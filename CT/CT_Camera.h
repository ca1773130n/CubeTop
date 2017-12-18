#pragma once

#include "stdafx.h"
#include "CT_AnimInfo.h"

class CtFirstPersonCamera : public CFirstPersonCamera
{
protected:
	CtAnimInfo<CtCameraKeyData> m_AnimInfo;

public:
	VOID				SetPoseTransition( FLOAT fTime, FLOAT fBias, ULONG iKeyAttributesFlag, ... );

	virtual VOID		FrameMove( FLOAT fElapsedTime );	
};
