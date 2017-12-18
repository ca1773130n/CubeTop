#include "stdafx.h"
#include "CT_Camera.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라의 포즈 트랜지션을 설정하는 함수.

 가변 인자로 전달받은 속성 타입 및 속성값들의 리스트를 통해 지정한 속성들만 지정한 값들로 보간하는 키프레임 애니메이션을 셋팅한다.

 * \param fTime					애니메이트할 구간의 시간
 * \param fBias					쏠림 값
 * \param iKeyAttributesFlag	애니메이트할 속성들의 타입 flag를 모두 or한 값
 * \param ...					가변 인자 리스트. 보간할 객체 속성 뒤에 지정할 값이 반복됨.
 * \return						없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtFirstPersonCamera::SetPoseTransition( FLOAT fTime, FLOAT fBias, ULONG iKeyAttributesFlag, ... )
{
	va_list args;
	va_start( args, iKeyAttributesFlag );

	// 애니메이션을 켠다
	m_AnimInfo.Enable();

	// 키프레임 배열이 비어있으면 현재 속성값들로 시작 키를 추가한다.
	DWORD numKeys = m_AnimInfo.GetNumKey();
	if( numKeys == 0 ) 
	{
		m_AnimInfo.AddKey( 0, 0, CtCameraKeyData(m_vEye, m_vLookAt, m_fFOV) );
		numKeys++;
	}

	// 목적 키를 추가
	m_AnimInfo.AddKey( (UINT)(fTime * 60), fBias, iKeyAttributesFlag, args );

	// 현재 카메라 포즈를 저장
	SavePoseToDefault();

	va_end( args );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 카메라 프레임 이동 함수

 키프레임 애니메이션 설정되어 있는 경우 보간값을 얻어 뷰 행렬을 설정하고, 
 그렇지 않으면 CFirstPersonCamera::FrameMove()를 호출하여 마우스 입력에 대한 카메라 회전을 수행

 * \param fElapsedTime	경과 시간
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtFirstPersonCamera::FrameMove( FLOAT fElapsedTime )
{
	
	if( m_AnimInfo.IsEnabled() )
	{
		CtCameraKeyData keyData = m_AnimInfo.GetData();
		if( m_vEye != keyData.vEye || m_vLookAt != keyData.vLookAt ) 
			SetViewParams( &keyData.vEye, &keyData.vLookAt );
		if( m_fFOV != keyData.fFOV )
			SetFOV( keyData.fFOV );	

			m_AnimInfo.FrameMove();
		
	}
	else 
	{
		CFirstPersonCamera::FrameMove( fElapsedTime );		
	}
}
