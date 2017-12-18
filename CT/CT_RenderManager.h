#pragma once

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 렌더링 상태 구조체

 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtRenderState
{
public:
	/************************************************************************/
	/* BOOL 속성들                                                          */
	/************************************************************************/
	BOOL			bWireframe;			///< 와이어프레임
	BOOL			bSmooth;			///< 스무스 셰이딩
	BOOL			bAlphaBlend;		///< 알파 블렌딩
	BOOL			bLighting;			///< 라이팅
	BOOL			bSpecular;			///< 스페큘러 라이팅
	
	/************************************************************************/
	/* INT 속성들                                                           */
	/************************************************************************/

	/************************************************************************/
	/* FLOAT 속성들                                                         */
	/************************************************************************/
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 렌더매니져 클래스

 렌더링과 관련된 것들을 관리하기 위한 클래스이다. 
 Direct3D RenderState와 함께 카메라, 조명, 재질, 행렬 등의 관리를 수행한다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef class CtRenderManager
{  
public:
	LPDIRECT3DDEVICE9		m_pDev;					///< D3D 장치 포인터
	CtRenderState			m_RenderState;			///< 렌더링 상태 구조체

	LPD3DXMATRIXSTACK		m_pMatrixStack;			///< D3D 행렬 스택
	D3DXMATRIX				m_matReflect;			///< 기본 반사 행렬
	D3DXMATRIX				m_matShadow;			///< 기본 그림자 행렬
	D3DMATERIAL9			m_mtrlShadow;			///< 기본 그림자 재질

	ID3DXEffect*			m_pMainEffect;			///< 기본 셰이더 이펙트
	LPDIRECT3DTEXTURE9		m_pShadowMap;			///< 기본 셰도우 맵 텍스쳐
	LPDIRECT3DSURFACE9		m_pShadowSurface;		///< 기본 셰도우 맵 서피스
	
	LPDIRECT3DSTATEBLOCK9	m_pStateBlock;			///< D3D State Block

public:

	CtRenderManager( VOID );

	/// 기본 설정대로 초기화를 수행하는 함수
	VOID					Init( VOID ); 

	/// 행렬 스택에 현재 월드 행렬을 저장
	VOID					PushMatrix( VOID );

	/// 행렬 스택에 저장된 월드 행렬로 복귀
	VOID					PopMatrix( VOID );

	/// State Block을 얻음
	LPDIRECT3DSTATEBLOCK9	GetStateBlock( VOID )			{ return m_pStateBlock; }

	/// 기본 반사 행렬을 얻음
	D3DXMATRIX				GetReflectMatrix( VOID )				{ return m_matReflect; }

	/// 기본 반사 행렬을 지정
	VOID					SetReflectMatrix( D3DXPLANE* pPlane )	{ D3DXMatrixReflect( &m_matReflect, pPlane ); }

	/// 기본 그림자 행렬을 얻음
	D3DXMATRIX				GetShadowMatrix( VOID )					{ return m_matShadow; }

	/// 기본 그림자 재질을 얻음
	D3DMATERIAL9			GetShadowMaterial( VOID )				{ return m_mtrlShadow; }

	/// 기본 그림자 행렬을 지정
	VOID					SetShadowMatrix( D3DXMATRIX matShadow )	{ m_matShadow = m_matShadow; }

	/// 지정된 인덱스의 조명의 위치를 얻음
	D3DXVECTOR3				GetLightPosition( UINT iLightIndex );

	/// 지정된 인덱스의 조명의 방향을 얻음
	D3DXVECTOR3				GetLightDirection( UINT iLightIndex );

	VOID					ToggleLighting( VOID );
	VOID					ToggleWireframe( VOID );
	VOID					ToggleSmooth( VOID );
	VOID					ToggleAlphaBlend( VOID );
	VOID					ToggleSpecular( VOID );

	/// 장치가 파괴될 때의 핸들러 함수
	VOID					OnDestroyDevice( VOID );

} CtRenderManager;