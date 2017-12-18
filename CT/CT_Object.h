#pragma once

#include "stdafx.h"
#include "CT_AnimInfo.h"
#include "CT_Settings.h"
#include "ZMorpher.h"

#include <deque>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 상태 플래그 및 매크로. 상태가 서로 겹칠수 있기때문에 비트 플래그로 함
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OBJECT_STATE_HIDDEN					(1 << 4)
#define OBJECT_STATE_HOVERED				(1 << 5)
#define OBJECT_STATE_SELECTED				(1 << 6)
#define OBJECT_STATE_LIFTED					(1 << 7)
#define OBJECT_STATE_SLEPT					(1 << 8)
#define OBJECT_STATE_FREEZED				(1 << 9)
#define OBJECT_STATE_HANGED					(1 << 10)
#define OBJECT_STATE_TRANSFORMED			(1 << 11)
#define OBJECT_STATE_PILED					(1 << 12)
#define OBJECT_STATE_REWINDFOWARD			(1 << 13)
#define OBJECT_STATE_FORCED					(1 << 14)
#define OBJECT_STATE_DELETED				(1 << 15)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 열거형
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum CtObjectType
{
	OBJECT_TYPE_CHIP,
	OBJECT_TYPE_CUBE,
	OBJECT_TYPE_IMAGE,
	OBJECT_TYPE_MOVIE,
	OBJECT_TYPE_DOC,
	OBJECT_TYPE_PPT,
	OBJECT_TYPE_SELECTION,
	OBJECT_TYPE_PILE,
};

enum CtMeshType
{ 
	MESH_TYPE_DXUT, 
	MESH_TYPE_ZMESH,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 파일 정보 구조체
 
 CT 객체가 표현하는 파일의 정보를 저장하는 구조체이다.
 윈도우 파일시스템의 기본적인 정보와 함께 직관성을 위한 추가적인 통계 정보를 저장한다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtFileInfo
{
public:
	/************************************************************************/
	/* 기본 정보                                                            */
	/************************************************************************/
	bool	bIsDir;					///< TRUE 이면 디렉토리. FALSE 이면 파일.
	CString	lpszDisplayName;		///< 실제 디스플레이 되는 이름
	CString lpszFileName;			///< 확장자를 포함한 파일명
	CString lpszExt;				///< 확장자
	CString lpszFileType;			///< 윈도우에서 파일 종류를 나타내는 문자열.
	CString lpszFilePath;			///< 파일의 전체 경로
	CString lpszMeshFilePath;		///< 객체가 갖는 메시파일의 전체 경로
	ULONG	iFileSize;				///< 파일 크기(바이트)
	
	/************************************************************************/
	/* 통계 정보                                                            */
	/************************************************************************/
	UINT	iNumOpened;				///< 프로그램 설치 후 객체가 더블클릭된 횟수
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 메시 정보 구조체

 CT 객체들은 모두 3D 메시를 갖는다. 이 메시에 대한 정보를 저장하는 구조체이다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtMeshInfo
{
public:
	CtMeshType			type;		///< 메시의 종류. 
	int					totmesh;	///< 서브메시 개수
	DWORD				totvert;	///< 버텍스 수
	DWORD				totface;	///< 페이스 수
	DWORD				totedge;	///< 엣지의 수
	int					totmat;		///< 머티리얼 수
	D3DMATERIAL9*		mat;		///< 머티리얼 배열
	LPDIRECT3DTEXTURE9	tex;		///< 텍스쳐 배열
	DWORD				vbsize;		///< 버텍스 버퍼 크기
	DWORD				ibsize;		///< 인덱스 버퍼 크기
};

// 클래스 선언
class CtObjectChip;
class CtObjectCube;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 기본 오브젝트 클래스

 CT 공간상의 객체를 표현하는 기본 클래스이다. 기본적인 3D 속성들을 갖는다.
 
 \note 
 직렬화를 위해 CObject로부터 상속받고 DECLARE_SERIAL을 선언한다. cpp에는 IMPLEMENT_SERIAL을 선언.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtObjectBase : public CObject
{
	// 직렬화 선언
	DECLARE_SERIAL( CtObjectBase );						

protected:	

	CtObjectType				m_enType;				///< 오브젝트 타입
	LONG						m_iStateFlag;			///< 상태 플래그
	bool						m_bHasCustomMesh;		///< 기본 메쉬가 아닌 커스텀 메쉬를 갖는지 여부

	CtObjectCube*				m_pParentCube;			///< 부모 디렉토리 큐브

	CDXUTMesh*					m_pDXUTMesh;			///< 메쉬 포인터
	
	/************************************************************************/
	/* 포즈, 스케일                                                         */
	/************************************************************************/
	NxActor*					m_pActor;				///< PhysX 액터
	NxTriangleMesh*				m_pActorMesh;			///< PhysX TriangleMesh 
	BOOL						m_bSleep;				///< 액터가 현재 sleep 중인지 여부
	NxMat34						m_matPose;				///< 글로벌 포즈
	NxMat34						m_matPoseTemp;			///< 이전값으로 돌아가기 위한 임시값
	deque<NxMat34>				m_matPoses;				///< 글로벌 포즈 저장 리스트(undo, redo용)
	int							m_iCurPoseIndex;		///< 현재 글로벌 포즈 인덱스 	
	D3DXVECTOR3					m_vScale;				///< x, y, z 방향 스케일 factor
	deque<D3DXVECTOR3>			m_vScales;				///< 스케일 벡터 저장 리스트(undo, redo용)
	D3DXVECTOR3					m_vScaleTemp;			///< 이전값으로 돌아가기 위한 임시값
	D3DXVECTOR3					m_vActorSize;			///< PhysX 액터의 크기		
	D3DXVECTOR3					m_vAnchor;				///< 오브젝트 중심에 대한 마우스로 집은 지점의 벡터. 드래그 모드등에 사용

	/************************************************************************/
	/* 재질 관련                                                            */
	/************************************************************************/
	FLOAT						m_fTransparency;		///< 오브젝트 투명도
	UINT						m_iNumMaterials;		///< D3D 재질 갯수
	D3DMATERIAL9*				m_pMeshMaterials;		///< 재질 배열 포인터
	LPDIRECT3DTEXTURE9*			m_pMeshTextures;		///< 텍스쳐 배열 포인터
	
	/************************************************************************/
	/* 정보 구조체들                                                        */
	/************************************************************************/
	CtFileInfo					m_stFileInfo;			///< 파일 정보 구조체
	CtMeshInfo					m_stMeshInfo;			///< 메시 정보 구조체
	CtAnimInfo<CtObjectKeyData> m_AnimInfo;				///< 키프레임 애니메이션 정보	
	
	/************************************************************************/
	/* 모핑 관련                                                            */
	/************************************************************************/
	BOOL						m_bHasMorphMesh;		///< 모핑용 메시를 가졌는지 여부
	ZObject*					m_pMorphObj;			///< 모핑용 ZObject
	LPDIRECT3DVERTEXBUFFER9		m_pMeshVB;				///< ZMesh 렌더링을 위한 버텍스 버퍼
	LPDIRECT3DINDEXBUFFER9		m_pMeshIB;				///< ZMesh 렌더링을 위한 인덱스 버퍼

	/************************************************************************/
	/* 기타                                                                 */
	/************************************************************************/

public:
						CtObjectBase( VOID );
						~CtObjectBase( VOID );

	/// 직렬화 함수
	VOID				Serialize( CArchive& ar );

	/// 객체에 대한 조작 메소드 처리용 함수. CtSelection::DoAtAllObject()로부터 call 됨       
	VOID				DoMethod( CtSelectionMethod method, va_list pArgs );

	/************************************************************************/
	/* 트랜스폼 관련 함수들                                                 */
	/************************************************************************/

	/// 객체를 지정된 벡터만큼 이동
	VOID				Move( NxVec3 vOffset );

	/// 로컬 X축에 대해 회전
	VOID				RotateLocalX( float fAmount );

	/// 로컬 Y축에 대해 회전
	VOID				RotateLocalY( float fAmount );

	/// 로컬 Z축에 대해 회전
	VOID				RotateLocalZ( float fAmount );

	/// 월드 X축에 대해 회전
	VOID				RotateGlobalX( float fAmount );

	/// 월드 Y축에 대해 회전
	VOID				RotateGlobalY( float fAmount );

	/// 월드 Z축에 대해 회전
	VOID				RotateGlobalZ( float fAmount );

	/// 객체의 종류에 맞게 크기 및 질량을 다시 계산
	VOID				Scale( VOID );

	/// 각 축에 대해 지정된 비율만큼 객체의 크기를 스케일링
	VOID				Scale(	DOUBLE fOffsetX, DOUBLE fOffsetY, DOUBLE fOffsetZ );

	VOID				ResetMass( FLOAT fDensity );

	/************************************************************************/
	/* 포즈 관련 함수들                                                     */
	/************************************************************************/

	/// 객체의 포즈를 카메라를 바라보도록 빌보딩
	VOID				Billboard( D3DXMATRIX* pBillboardMatrix );

	/// 빌보딩 브라우징을 위한 함수
	VOID				BillboardPosScale( D3DXMATRIX* pBillboardMatrix, NxVec3 vTgtPos, FLOAT fScaleFactor );

	/// 키프레임 애니메이션을 설정. 
	VOID				SetPoseTransition( NxMat34 matPose, D3DXVECTOR3 vScale, FLOAT fBias, FLOAT fTime );

	/// 현재 포즈를 저장
	VOID				SavePoseScale( VOID );

	/// 저장되었던 포즈를 얻음
	VOID				LoadPoseScale( VOID );

	/************************************************************************/
	/* 메시 관련 함수들                                                     */
	/************************************************************************/

	/// 파일로부터 메시를 읽어들이는 함수
	HRESULT				ImportMesh( LPCWSTR filepath );

	/// 칩, 큐브의 옆면 텍스쳐를 생성하는 함수
	VOID				GenerateNameTexture( VOID );

	/************************************************************************/
	/* PhysX 핸들러들                                                       */
	/************************************************************************/

	/// 객체가 Sleep 모드로 들어갈 때의 핸들러
	VOID				OnSleep( VOID );

	/************************************************************************/
	/* HUD UI 핸들러들                                                      */
	/************************************************************************/
	
	/// 객체의 포즈를 가장 최근에 정지(평형)상태였던 때로 되돌이킴
	VOID				Rewind( VOID );

	/// Rewind 했다가 다시 redo하는 함수
	VOID				Foward( VOID );

	/************************************************************************/
	/* 객체 상태 관련 함수들                                                */
	/************************************************************************/

	/// 객체를 렌더링하지 않도록 숨긴다
	VOID				Hide( VOID )						{ RaiseStateFlag( OBJECT_STATE_HIDDEN ); }

	/// 객체를 렌더링하도록 설정
	VOID				Show( VOID )						{ ClearStateFlag( OBJECT_STATE_HIDDEN ); }

	/// 객체를 Z축 방향으로 띄우는 모드
	VOID				Lift( VOID );

	/// 리프트 모드 해제
	VOID				UnLift( VOID );

	/// 객체 위에 마우스 커서가 위치하였을 경우의 핸들러
	VOID				Hover( VOID );

	/// 객체 위에서 마우스 커서가 벗어낫을 경우의 핸들러
	VOID				UnHover( VOID );

	/// 객체를 움직이지 않도록 고정
	VOID				Freeze( VOID );

	/// 객체의 고정을 해제
	VOID				UnFreeze( VOID );

	/// 객체를 선택했을때의 처리기
	VOID				Select( VOID );

	/// 객체가 선택 해제될때의 처리기
	VOID				UnSelect( VOID );	

	/// 상태 플래그를 on
	VOID				RaiseStateFlag( ULONG flag )		{ m_iStateFlag |= flag; }

	/// 상태 플래그를 off
	VOID				ClearStateFlag( ULONG flag )		{ m_iStateFlag &= ~flag; }

	/// 액터 플래그를 on
	VOID				RaiseActorFlag( NxActorFlag flag )	{ m_pActor->raiseActorFlag(flag); }

	/// 액터 플래그를 off
	VOID				ClearActorFlag( NxActorFlag flag )	{ m_pActor->clearActorFlag(flag); }

	bool				IsChip( VOID )						{ return (m_enType == OBJECT_TYPE_CHIP || m_enType == OBJECT_TYPE_IMAGE); }
	bool				IsCube( VOID )						{ return (m_enType == OBJECT_TYPE_CUBE); }
	bool				IsHidden( VOID )					{ return (m_iStateFlag & OBJECT_STATE_HIDDEN); }
	bool				IsHovered( VOID )					{ return (m_iStateFlag & OBJECT_STATE_HOVERED); }
	bool				IsSelected( VOID )					{ return (m_iStateFlag & OBJECT_STATE_SELECTED); }
	bool				IsLifted( VOID )					{ return (m_iStateFlag & OBJECT_STATE_LIFTED); }
	bool				IsSlept( VOID )						{ return (m_iStateFlag & OBJECT_STATE_SLEPT); }
	bool				IsFreezed( VOID )					{ return (m_iStateFlag & OBJECT_STATE_FREEZED); }
	bool				IsHanged( VOID )					{ return (m_iStateFlag & OBJECT_STATE_HANGED); }
	bool				IsTransformed( VOID )				{ return (m_iStateFlag & OBJECT_STATE_TRANSFORMED); }
	bool				IsPiled( VOID )						{ return (m_iStateFlag & OBJECT_STATE_PILED); }
	bool				IsForced( VOID )					{ return (m_iStateFlag & OBJECT_STATE_FORCED); }
	bool				IsDeleted( VOID )					{ return (m_iStateFlag & OBJECT_STATE_DELETED); }
	bool				NowOnRewindFoward( VOID )			{ return (m_iStateFlag & OBJECT_STATE_REWINDFOWARD); }

	/************************************************************************/
	/* Get, Set                                                             */
	/************************************************************************/

	/// 객체의 PhysX 액터를 반환
	NxActor*						GetActor( VOID )							{ return m_pActor; }

	/// 액터를 지정
	VOID							SetActor( NxActor* pActor )					{ m_pActor = pActor; }

	/// 객체의 PhysX TriangleMesh를 반환
	NxTriangleMesh*					GetActorMesh( VOID )						{ return m_pActorMesh; }

	/// TriangleMesh를 지정
	VOID							SetActorMesh( NxTriangleMesh* pTriMesh )	{ m_pActorMesh = pTriMesh; }

	/// 객체의 스케일 벡터를 반환
	D3DXVECTOR3						GetScale( VOID )							{ return m_vScale; }

	/// 스케일 벡터를 지정
	VOID							SetScale( D3DXVECTOR3 vScale )				{ m_vScale = vScale; }

	/// 객체의 임시 스케일 벡터를 반환
	D3DXVECTOR3						GetScaleTemp( VOID )						{ return m_vScaleTemp; }

	/// 임시 스케일 벡터를 지정
	VOID							SetScaleTemp( D3DXVECTOR3 vScale )			{ m_vScaleTemp = vScale; }

	/// 객체의 Anchor 벡터를 반환
	D3DXVECTOR3						GetAnchor( VOID	)							{ return m_vAnchor; }

	/// Anchor 벡터를 지정
	VOID							SetAnchor( D3DXVECTOR3 vAnchor )			{ m_vAnchor = vAnchor; }

	/// 객체의 사이즈 벡터를 반환
	D3DXVECTOR3						GetActorSize( VOID )						{ return m_vActorSize; }

	/// 사이즈 벡터를 지정
	VOID							SetActorSize( D3DXVECTOR3 vSize )			{ m_vActorSize = vSize; }
			
	/// DXUT Mesh를 반환
	CDXUTMesh*						GetDXUTMesh( VOID )							{ return m_pDXUTMesh; }

	/// DXUTMesh를 지정
	VOID							SetDXUTMesh( CDXUTMesh* pMesh )				{ m_pDXUTMesh = pMesh; }

	/// 포즈 행렬을 반환
	NxMat34							GetPose( VOID )								{ return m_matPose; }

	/// 포즈 행렬을 지정
	VOID							SetPose( NxMat34 matPose )					{ m_matPose = matPose; }

	/// 임시 포즈 행렬을 반환
	NxMat34							GetPoseTemp( VOID )							{ return m_matPoseTemp; }

	/// 임시 포즈 행렬을 지정
	VOID							SetPoseTemp( NxMat34 matPose )				{ m_matPoseTemp = matPose; }

	/// 객체의 상태 플래그를 반환
	ULONG							GetState( VOID )							{ return m_iStateFlag; }

	/// 상태를 지정
	VOID							SetState( ULONG state )						{ m_iStateFlag = state; }
			
	/// 객체의 투명도를 반환
	FLOAT							GetTransparency( VOID )						{ return m_fTransparency; }

	/// 투명도를 지정
	VOID							SetTransparency( FLOAT trans )				{ m_fTransparency = trans; }

	/// 객체의 타입을 반환
	CtObjectType					GetType( VOID )								{ return m_enType; }

	/// 타입을 지정
	VOID							SetType( CtObjectType type )				{ m_enType = type; }

	/// 메시 타입을 얻는 함수
	CtMeshType						GetMeshType( VOID )							{ return m_stMeshInfo.type; }

	/// 메시 타입을 지정하는 함수
	VOID							SetMeshType( CtMeshType type )				{ m_stMeshInfo.type = type; }

	/// 객체가 기본 메시가 아닌 다른 메시를 형태로 삼는지 여부를 반환
	BOOL							HasCustomMesh( VOID )						{ return m_bHasCustomMesh; }

	/// 객체가 기본 메시가 아닌 다른 메시를 형태로 삼는지 여부를 지정
	VOID							SetHasCustomMesh( BOOL onoff )				{ m_bHasCustomMesh = onoff; }

	/// 객체가 모핑용 메시를 갖는지 여부를 반환
	BOOL							HasMorphMesh( VOID )						{ return m_bHasMorphMesh; }

	/// 객체가 모핑용 메시를 갖는지 여부를 지정
	VOID							SetHasMorphMesh( BOOL onoff )				{ m_bHasMorphMesh = onoff; }

	/// 키프레임 애니메이션 객체를 반환
	CtAnimInfo<CtObjectKeyData>*	GetAnimInfo( VOID )							{ return &m_AnimInfo; }

	/// 부모 큐브 포인터를 반환
	CtObjectCube*					GetParentCube( VOID )						{ return m_pParentCube; }

	/// 부모 큐브 포인터를 지정
	VOID							SetParentCube( CtObjectCube* pParent )		{ m_pParentCube = pParent; }

	/// 객체의 파일 정보를 반환
	CtFileInfo*						GetFileInfo( VOID )							{ return &m_stFileInfo; }

	/// 객체가 가리키는 파일의 전체 경로를 반환
	CString							GetFilePath( VOID )							{ return CString(m_stFileInfo.lpszFilePath) + CString(m_stFileInfo.lpszFileName); }

	/// 모핑 및 타 목적용 VB 반환
	LPDIRECT3DVERTEXBUFFER9			GetVB( VOID )								{ return m_pMeshVB; }

	/// 모핑 및 타 목적용 VB 반환
	VOID							SetVB( LPDIRECT3DVERTEXBUFFER9 pVB )		{ m_pMeshVB = pVB; }

	/// 모핑 및 타 목적용 VB 반환
	LPDIRECT3DINDEXBUFFER9			GetIB( VOID )								{ return m_pMeshIB; }

	/// 모핑 및 타 목적용 VB 반환
	VOID							SetIB( LPDIRECT3DINDEXBUFFER9 pIB )			{ m_pMeshIB = pIB; }

	/// 모핑용 ZObject를 반환
	ZObject*						GetMorphObject( VOID )						{ return m_pMorphObj; }

	/// 추가 텍스쳐 배열을 반환
	LPDIRECT3DTEXTURE9*				GetTextures( VOID )							{ return m_pMeshTextures; }

	/// 모든 shape에 대해 액터그룹 지정
	VOID							SetCollisionGroup( NxCollisionGroup group );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 칩 클래스(일반 파일)

 문서 및 일반 파일들은 칩 형태를 기본으로 한다. 
 칩들은 Piling 시켜 하나의 연결된 더미로 쌓거나 벽에 걸어놓는 등 여러가지로 정리할 수 있다.
 기본 칩 형태에서 원하는 모델 메쉬로 바꾼 오브젝트의 경우는 Piling시 기본 칩 형태로 바뀌어 Piling된다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtObjectChip : public CtObjectBase
{
protected:
	/************************************************************************/
	/* Piling 관련                                                          */
	/************************************************************************/
	BOOL			m_bBoundaryOfPile;		///< Pile 상에서 맨 위 또는 아래에 위치한 칩인지 여부
	NxJoint*		m_pPileJoint1;			///< Piling 시 아래 오브젝트와의 조인트
	NxJoint*		m_pPileJoint2;			///< Piling 시 위   오브젝트와의 조인트
	NxJoint*		m_pHangJoint;			///< 벽에 걸어놓을때 쓰이는 조인트

	UINT			m_iPileOrder;			///< Pile 상에서 가장 밑바닥 오브젝트에서부터의 순번
	CtObjectChip*	m_pPrevObj;				///< Pile 상에서 이전 객체 포인터
	CtObjectChip*	m_pNextObj;				///< Pile 상에서 다음 객체 포인터

public:
					CtObjectChip( VOID );
					CtObjectChip( LPCWSTR lpszFileName, LPCWSTR lpszFilePath, LPCWSTR lpszDisplayName, LPCWSTR lpszTypeName );
					~CtObjectChip( VOID );

	/************************************************************************/
	/* Piling 관련                                                          */
	/************************************************************************/

	/// Piling용 Joint(앞)을 반환
	NxJoint*		GetPileJointPrev( VOID )					{ return m_pPileJoint1; }

	/// Piling용 Joint(뒤)을 반환
	NxJoint*		GetPileJointNext( VOID )					{ return m_pPileJoint2; }

	/// Piling용 Joint(앞)을 지정
	VOID			SetPileJointPrev( NxJoint* pJoint )			{ m_pPileJoint1 = pJoint; }

	/// Piling용 Joint(뒤)을 지정
	VOID			SetPileJointNext( NxJoint* pJoint )			{ m_pPileJoint2 = pJoint; }

	/// Piling용 이전 객체 포인터를 반환
	CtObjectChip*	GetPrev( VOID )								
	{ 
		if( m_pPileJoint1 )
		{
			NxActor *actor1, *actor2;
			m_pPileJoint1->getActors( &actor1, &actor2 );
			return (CtObjectChip*)actor1->userData;
		}
		return m_pPrevObj;		
	}
	
	/// Piling용 다음 객체 포인터를 반환
	CtObjectChip*	GetNext( VOID )								
	{
		if( m_pPileJoint2 )
		{
			NxActor *actor1, *actor2;
			m_pPileJoint2->getActors( &actor1, &actor2 );
			return (CtObjectChip*)actor2->userData;
		}
		return m_pNextObj;		
	}

	/// Piling용 이전 객체 포인터를 지정
	VOID			SetPrev( CtObjectChip* pObj )				{ m_pPrevObj = pObj; }

	/// Piling용 다음 객체 포인터를 지정
	VOID			SetNext( CtObjectChip* pObj )				{ m_pNextObj = pObj; }

	/// Pile에서의 순서(인덱스)를 반환
	UINT			GetPileOrder( VOID )						{ return m_iPileOrder; }

	/// Pile에서의 순서(인덱스)를 지정
	VOID			SetPileOrder( UINT order )					{ m_iPileOrder = order; }

	/// Pile의 양쪽 끝 경계에 위치한 객체인지 여부를 반환
	BOOL			IsBoundaryOfFile( VOID )					{ return m_bBoundaryOfPile; }

	/// Pile의 양쪽 끝 경계에 위치한 객체인지 여부를 지정
	VOID			SetBoundaryOfPile( BOOL bOnOff );

	/************************************************************************/
	/* 메시 관련                                                            */
	/************************************************************************/

	/// 윈도우 아이콘으로부터 텍스쳐를 생성하는 함수
	HRESULT			LoadIcon( HICON hIcon );	
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 큐브 클래스(디렉토리)

 모든 디렉토리는 큐브 클래스 객체로 생성된다. 칩과 달리 큐브들은 Piling 시키지 않는다.
 큐브는 각각의 씬을 가지고 있어서 큐브 내부로 이동시 해당 씬을 렌더링하게 된다.
 프로그램 시작시의 기본 큐브는 바탕화면 디렉토리에 대한 큐브이다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtObjectCube : public CtObjectBase
{
protected:
	bool			m_bMaximized;			///< 큐브가 최대화 되었는지 여부

	CGrowableArray
	<CtObjectBase*>	m_ObjectArray;			///< 오브젝트 배열	

	NxScene*		m_pPhysXScene;			///< 큐브 당 하나의 씬을 갖음

	NxCompartment*	m_pPhysXCompartment;	///< GPU PhysX를 위한 것. 사용하지 않음

	D3DMATERIAL9	m_Material;				///< 기본 큐브 형태를 가질 때의 재질
	
public:
					CtObjectCube( VOID );
					CtObjectCube( LPCWSTR lpszFileName, LPCWSTR lpszFilePath, LPCWSTR lpszDisplayName, LPCWSTR lpszTypeName );
					~CtObjectCube( VOID );

	/// 큐브 내 오브젝트들을 직렬화 하는 함수
	VOID			Serialize( BOOL bStoring );

	/// 객체를 배열에 삽입
	VOID			AddObject( CtObjectBase* pObj )	{ m_ObjectArray.Add( pObj ); }

	VOID RemoveObject( CtObjectBase* pObj, bool bDeleteActor = FALSE );

	/************************************************************************/
	/* Get, Set                                                             */
	/************************************************************************/

	/// 가지고 있는 객체들의 배열을 반환
	CtObjectBase**	GetObjects( VOID )				{ return m_ObjectArray.GetData(); }

	/// 가지고 있는 객체들의 수를 반환
	DWORD			GetNumObjects( VOID )			{ return m_ObjectArray.GetSize(); }
	
	/// 최대화 되었는지 여부를 반환
	BOOL			IsMaximized( VOID )				{ return m_bMaximized; }

	/// 최대화 되었는지 여부를 지정
	VOID			SetMaximized( BOOL bOnOff )		{ m_bMaximized = bOnOff; }

	/// 머티리얼을 반환
	D3DMATERIAL9	GetMaterial( VOID )				{ return m_Material; }

	/// 머티리얼을 지정
	VOID			SetMaterial( D3DMATERIAL9 mat )	{ m_Material = mat; }

	/// PhysX 씬 포인터를 돌려줌
	NxScene*		GetScene( VOID )				{ return m_pPhysXScene; }

	VOID			SetPhysXCompartment(NxCompartment *comp) { m_pPhysXCompartment = comp; }
};




