﻿#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
//
// Global 변수들
//
//////////////////////////////////////////////////////////////////////////

extern CtMain* g_pCTMain;

//////////////////////////////////////////////////////////////////////////
//
// Global Set / Get 함수들
//
//////////////////////////////////////////////////////////////////////////

CtMain*
GetCTmain( VOID )
{
	return g_pCTMain; 
}

VOID
SetCTmain( CtMain* pMain )
{
	g_pCTMain = pMain;
}

CtPhysX*
GetPhysX( VOID )
{
	return &GetCTmain()->m_PhysX;
}