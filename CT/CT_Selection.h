#pragma once

#include "stdafx.h"
#include "CT_Object.h"
#include "CT_PhysX.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 셀렉션 클래스

 객체들의 선택물 그룹을 표현하는 클래스이다. 선택 객체들에 대한 각종 처리를 수행한다.
 객체들에 대해 수행 가능한 모든 기능들은 (선택 객체들이 동일 타입 객체들인 경우) 모두 이 클래스를 통해 일괄 처리가 가능해야 한다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtObjectSelection
{
protected:	
	D3DXVECTOR3						m_vPos;						///< 선택 객체들의 전체 포지션의 평균값
	CGrowableArray<CtObjectBase*>	m_Array;					///< 선택 객체들의 포인터 배열
	CString							m_lpszTypeName;				///< 선택 객체들의 종류를 나타내는 문자열
	UINT							m_iPilingNow;				///< 현재 파일링 중인 Pile의 수. 파일 종류별 파일링 시에 필요

public:
	CtObjectSelection();
	~CtObjectSelection();

	/// 선택물 수를 반환
	UINT							GetNumObjects( VOID	)				{ return m_Array.GetSize(); }

	/// 선택 영역의 중심을 반환
	D3DXVECTOR3						GetCenter( VOID )					{ CalcCenter(); return m_vPos; }		

	/// 선택 영역의 중심을 지정
	VOID							SetCenter( D3DXVECTOR3 vCenter )	{ m_vPos = vCenter; }		

	/// 선택물 이름을 반환
	CString							GetName( VOID )						{ return m_lpszTypeName; }

	/// 선택 객체의 (포인터) 배열 반환
	CtObjectBase**					GetObjects( VOID )					{ return m_Array.GetData(); }

	/// 현재 파일링 중인 Pile들의 수를 반환
	UINT							GetPilingNow( VOID )				{ return m_iPilingNow; }

	/// 현재 파일링 중인 Pile들의 수를 반환
	VOID							SetPilingNow( UINT iNum )			{ m_iPilingNow = iNum; }

	/// 이름을 지정
	VOID							SetName( CString name )				{ m_lpszTypeName = name; }	

	/// 선택 그룹에 객체를 추가
	VOID							Add( CtObjectBase *pObject );
	
	/// 선택 그룹에서 객체를 제거
	VOID							Remove( CtObjectBase *pObject );

	/// 선택물을 비움
	VOID							Clear( VOID );

	/// 선택된 객체 배열을 정렬하는 함수
	VOID							SortBy( int iMethod, CtObjectBase **pArray, int top, int bottom );
	
	/// 선택된 객체 배열을 정렬하는 함수
	UINT							SortBy_partition( int iMethod, CtObjectBase **pArray, int top, int bottom );

	/// 선택 그룹에 객체가 있는지 검사
	BOOL							IsInSelection( CtObjectBase *pObject );	

	/// 선택물의 중점 벡터를 계산
	D3DXVECTOR3						CalcCenter( VOID );

	/************************************************************************/
	/* 일괄 처리 함수들. 선택 객체들의 해당 함수를 단순히 일괄 호출         */
	/************************************************************************/

	// 일괄 처리 함수. 메소드에 해당하는 선택 객체들의 해당 함수를 단순히 일괄 호출
	VOID							DoAtAllObject( CtSelectionMethod method, ... );

	/************************************************************************/
	/* Piling 관련                                                          */
	/************************************************************************/

	/// 파일링 모드를 시작
	VOID							StartPilingMode( VOID );	
	
	/// 파일링 모드 진행
	BOOL							ProcessPiling( float amount );
	
	/// 파일링 모드 취소
	VOID							CancelPilingMode( VOID );
		
	/// 파일링 모드 완료
	VOID							LinkByFixedJoint( BOOL bPiling = FALSE );	

	/// 확장자별 파일링 모드 수행
	VOID							PileByFileTypes( VOID );

	/// 파일에서 칩을 떼어냄
	VOID							PopChipFromPile( const D3DXVECTOR3* vEyePos );

	/// 파일들의 연결을 모두 끊음
	VOID							BreakPiles( VOID );

	/// 파일의 연결을 모두 끊음
	int								BreakPile( CtObjectBase* pObj );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 올가미 선택 툴 클래스

 객체들을 선택하기 위한 올가미 툴 클래스이다.
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtToolSelectFan
{
protected:
	BOOL						m_bDirection;					///< 호를 그리는 방향
	BOOL						m_bReverse;						///< 방향이 바뀐 상태
	UINT						m_iMaxSize;						///< 버퍼 최대 크기
	FLOAT						m_fTheta;						///< 현재 삼각형을 이루는 각도
	
	D3DXVECTOR3					m_vStartLassoPos;				///< 시작점의 3D 좌표

	LPDIRECT3DVERTEXBUFFER9		m_pVB;							///< 선택 영역 렌더링을 위한 버텍스 버퍼
	CGrowableArray<VERTEX>		m_Vertices;						///< 버텍스 동적 배열(DrawPrimitiveUP을 위함)

	LPDIRECT3DTEXTURE9			m_pTexture;						///< 선택 영역 렌더링을 위한 텍스쳐 배열

public:
	CtToolSelectFan( VOID );
	~CtToolSelectFan( VOID );

	/// 초기화 함수
	VOID						Init( UINT iNumVBSize );

	/// 렌더링 함수
	VOID						Draw( VOID );

	/// 툴 모드 준비 함수
	VOID						Ready( D3DXVECTOR3 v0, CPoint pt );
	
	/// 툴 모드 시작 함수
	VOID						Begin( CPoint pt );

	/// 툴 모드 진행 함수
	VOID						Process( D3DXVECTOR3 v1, D3DXVECTOR3 v2, CPoint pt );

	/// 툴 모드 완료 함수
	VOID						End( VOID );

	/// 호를 이루는 버텍스 수를 얻음
	DWORD						GetNumVertex( VOID )			{ return m_Vertices.GetSize(); }

	/// 시작점의 3D 좌표를 얻음
	D3DXVECTOR3					GetStartVertex( VOID )			{ return m_vStartLassoPos; }
};
