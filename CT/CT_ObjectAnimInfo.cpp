#include "stdafx.h"
#include "CT_ObjectAnimInfo.h"
#include "DXUT.h"

CtObjectAnimInfo::CtObjectAnimInfo( VOID )
{
	m_bEnabled		= FALSE;
	m_iCurrentFrame	= 0;
	m_iCurrentKey	= 0;
}

VOID
CtObjectAnimInfo::Enable( VOID )
{
	m_bEnabled = TRUE;
}

VOID
CtObjectAnimInfo::Disable( VOID )
{
	m_bEnabled = FALSE;
}

bool
CtObjectAnimInfo::IsEnabled( VOID )
{
	return m_bEnabled;
}

VOID		
CtObjectAnimInfo::AddKey( UINT iNumFrame, NxMat34 matPose, NxVec3 vScale )
{
	stKeyFrame keyframe;
	keyframe.iNumFrame = iNumFrame;
	keyframe.matPose = matPose;
	keyframe.vScale = vScale;

	m_KeyFrameArray.Add( keyframe );
}

NxMat34
CtObjectAnimInfo::GetPose( VOID )
{
	stKeyFrame *pKey = m_KeyFrameArray.GetData();

	if( pKey[m_iCurrentKey].iNumFrame == m_iCurrentFrame )
	{
		return pKey[m_iCurrentKey].matPose;
	}
	else
	{
		// 양 키프레임 사이를 선형보간
		NxMat33 matDiff = pKey[m_iCurrentKey+1].matPose.M - pKey[m_iCurrentKey].matPose.M;
		matDiff /= (FLOAT)( pKey[m_iCurrentKey+1].iNumFrame - pKey[m_iCurrentKey].iNumFrame );		
		matDiff /= (1 / (FLOAT)( pKey[m_iCurrentKey+1].iNumFrame - m_iCurrentFrame ));

		NxVec3 posDiff = pKey[m_iCurrentKey+1].matPose.t - pKey[m_iCurrentKey].matPose.t;
		posDiff /= (FLOAT)( pKey[m_iCurrentKey+1].iNumFrame - pKey[m_iCurrentKey].iNumFrame );
		posDiff *= (FLOAT)( m_iCurrentFrame - pKey[m_iCurrentKey].iNumFrame );

		NxMat34 matResult;
		matResult.M = pKey[m_iCurrentKey].matPose.M + matDiff;
		matResult.t = pKey[m_iCurrentKey].matPose.t + posDiff;

		return matResult;
	}
}

NxVec3
CtObjectAnimInfo::GetScale( VOID )
{
	stKeyFrame *pKey = m_KeyFrameArray.GetData();

	if( pKey[m_iCurrentKey].iNumFrame == m_iCurrentFrame )
	{
		return pKey[m_iCurrentKey].vScale;
	}
	else
	{
		// 다음 키프레임이 이전 키프레임과 같은 스케일인 경우 보간할 필요 없이 값 리턴
		if( pKey[m_iCurrentKey+1].vScale == pKey[m_iCurrentKey].vScale )
			return pKey[m_iCurrentKey+1].vScale;

		// 아닌 경우 양 키프레임 사이를 선형보간		
		NxVec3 vDiff( pKey[m_iCurrentKey+1].vScale.x / pKey[m_iCurrentKey].vScale.x, pKey[m_iCurrentKey+1].vScale.y / pKey[m_iCurrentKey].vScale.y, pKey[m_iCurrentKey+1].vScale.z / pKey[m_iCurrentKey].vScale.z );
		if( vDiff.magnitude() > sqrt(3.f) )
		{			
			vDiff /= (FLOAT)( pKey[m_iCurrentKey+1].iNumFrame - pKey[m_iCurrentKey].iNumFrame );
			vDiff *= (FLOAT)( m_iCurrentFrame - pKey[m_iCurrentKey].iNumFrame );
		}
		else
		{
			vDiff = pKey[m_iCurrentKey].vScale - pKey[m_iCurrentKey+1].vScale;			
			vDiff /= (FLOAT)( pKey[m_iCurrentKey+1].iNumFrame - pKey[m_iCurrentKey].iNumFrame );
			vDiff = pKey[m_iCurrentKey].vScale - (FLOAT)( m_iCurrentFrame - pKey[m_iCurrentKey].iNumFrame ) * vDiff;
		}		
		
		return vDiff;
	}
}

UINT
CtObjectAnimInfo::GetNbKey( VOID )
{
	return m_KeyFrameArray.GetSize();
}

bool
CtObjectAnimInfo::FrameMove( VOID )
{	
	m_iCurrentFrame++;
	
	stKeyFrame *pKey = m_KeyFrameArray.GetData();
	if( m_iCurrentKey < m_KeyFrameArray.GetSize() - 1 )
	{
		if ( m_iCurrentFrame >= pKey[m_iCurrentKey+1].iNumFrame )
		{
			m_iCurrentKey++;
		}
	}
	else
	{
		// 마지막 프레임에 도달함, 애니메이션을 off시킨다
		Clear();
		return TRUE;
	}
	return FALSE;
}

VOID
CtObjectAnimInfo::Clear( VOID )
{
	Disable();
	m_iCurrentFrame = 0;
	m_iCurrentKey = 0;
	m_KeyFrameArray.RemoveAll();
}