#include "stdafx.h"
#include "CT_Selection.h"

#include "D3DEx.h"
#include <set>

using namespace std;

/*************************************************************************************************************************************************************/
/* 셀렉션 클래스																																			 */
/*************************************************************************************************************************************************************/

CtObjectSelection::CtObjectSelection()
{
	m_vPos = D3DXVECTOR3( 0, 0, 0 );	
	m_Array.SetSize( 100 );
	m_iPilingNow = 0;
}

CtObjectSelection::~CtObjectSelection()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 셀렉션에 객체를 추가하는 함수

 객체를 추가할 때마다 셀렉션의 중심 벡터를 재 계산한다. 셀렉션에 추가할때 추가적으로 수행할 작업들을 이곳에 작성한다.

 * \param *pObject	객체 포인터
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::Add( CtObjectBase *pObject )
{
	pObject->Select();

	m_Array.Add( pObject );

	CalcCenter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 셀렉션에서 특정 객체를 제거한다.

 셀렉션의 포함 객체들이 현재 CGrowableArray로 되어 있어 검색이 비효율적이다. 여러 객체를 제거하는데에도 좋지 못함.

 * \param *pObject	객체 포인터
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::Remove( CtObjectBase *pObject )
{
	CtObjectBase **ptr = m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		if( ptr[i] == pObject )
		{
			m_Array.Remove( i );
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 셀렉션 클리어 함수

 선택된 객체들의 UnSelect() 함수를 수행하여 선택 해제 핸들링을 하도록 하고 객체 배열을 비운다

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::Clear( VOID )
{
	CtObjectBase **pObj = m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		pObj[i]->UnSelect();
	}
	m_Array.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 주어진 포인터의 객체가 선택물에 포함되어 있는지 여부를 반환

 * \param *pObject	객체 포인터
 * \return			TRUE면 있음, FALSE면 없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CtObjectSelection::IsInSelection( CtObjectBase *pObject )
{
	return m_Array.Contains( pObject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 셀렉션의 모든 객체들의 포지션을 평균한 셀렉션의 중심 벡터를 반환

 * \param VOID	없음
 * \return		셀렉션 중심 벡터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 
CtObjectSelection::CalcCenter( VOID )
{
	int size = m_Array.GetSize();
	if( size == 0 ) return D3DXVECTOR3(0,0,0);

	// 위치를 평균
	NxVec3 vSum = NxVec3( 0, 0, 0 );
	CtObjectBase **pObjArray = m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		vSum += pObjArray[i]->GetPose().t;	
	}
	vSum /= size;

	m_vPos = NxVectorToD3DVector( vSum );
	return m_vPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일링 시작 함수

 칩 객체들의 파일링을 시작하는 함수이다. 파일링 시작 시에 수행할 작업을 작성한다.
 셀렉션의 중심 벡터를 계산하는 것만을 수행한다. 셀렉션 중심 좌표는 파일링 시에 객체들이 쌓인 Pile이 최종적으로 위치할 지점의 벡터이다.
 또 파일링 될 후보들인 셀렉션 포함 객체들을 전부 충돌처리하지 않도록 설정한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtObjectSelection::StartPilingMode( VOID )
{
	CalcCenter();
	
	CtObjectBase** pObjects = m_Array.GetData();
	//for( DWORD i=0; i < m_Array.GetSize(); i++ )
		//if( pObjects[i]->IsChip() && !pObjects[i]->IsPiled() ) pObjects[i]->RaiseActorFlag( NX_AF_DISABLE_COLLISION );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일링 진행 함수

 칩 객체를 쌓는 파일링은 선택 영역을 회전시키면서 서서히 객체들을 모으게 된다. 이때 주어진 값 만큼 파일링을 진행하는 함수

 * \param amount	객체들을 모으는 정도
 * \return			TRUE면 파일링 완료상태로 진입을 허용, FALSE면 반대
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CtObjectSelection::ProcessPiling( float amount )
{
	BOOL bDone = false;

	if( m_Array.GetSize() == 1 )
	{
		return false;
	}
	else 
	{			
		NxVec3 vCenter = D3DXVec3ToNxVec3( m_vPos );	
		vCenter.y = 0;

		// 중간 위치로 모음
		CtObjectBase** pObjects = m_Array.GetData();
		for( DWORD i=0; i < m_Array.GetSize(); i++ )
		{			
			pObjects[i]->GetActor()->setLinearVelocity( vCenter - pObjects[i]->GetPose().t );
		}
		
		// 중점에 모였음, 칩을 쌓는다
		if( bDone )
		{
			LinkByFixedJoint();			
		}		
	}

	return bDone;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일링 모드를 취소하는 함수

 Process Piling 진행을 하면서 객체들을 모으다가 마우스 클릭을 해제할 경우 파일링을 취소하는 이 함수를 호출한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::CancelPilingMode( VOID )
{
	CtObjectBase **pObjArray = m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일링 완료 함수

 객체들이 충분히 모이면 파일링을 완료하여 pile을 형성한다. 객체들을 Y축 방향으로 쌓고 객체끼리 고정 조인트로 결속시킨다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtObjectSelection::LinkByFixedJoint( BOOL bPiling /*= FALSE */ )
{	
	UINT			nbElements, nbContinue = 0;		// 객체 수, 건너뛴 객체 수(파일링 시 큐브 및 커스텀 메시를 갖는 객체는 파일링하지 않도록 건너뛰는데, 그것을 세어두어야 함)
	CtObjectChip	*pObj = 0, *pPrevObj = 0;		// 현재 검사중인 객체 포인터와 그것의 앞(아래) 객체 포인터. 둘 사이를 연결시키게 됨.
	D3DXVECTOR3		vSize, vScale;					// 객체의 사이즈 및 스케일 벡터를 저장할 값
	NxMat34			mPoseBase, mPose;				// 객체의 포즈 행렬을 저장할 값
	D3DXMATRIX		matRotate;						// 회전시키기 위한 임시 행렬
	NxActor*		pActor;							// 객체의 액터 포인터를 저장할 값
	NxScene*		pCurScene;						// 현재 씬
		
	// 객체 배열과 수, 현재 씬을 얻음
	CtObjectBase** pObjArray = (CtObjectBase**)m_Array.GetData();
	nbElements = m_Array.GetSize();
	pCurScene = pObjArray[0]->GetParentCube()->GetScene();

	// 사이즈 순서대로 셀렉션을 소트한다
	SortBy( 0, pObjArray, 0, nbElements - 1 );

	// 베이스 포즈 행렬(Pile 중심으로 모이는 지점의 포즈) 초기화
	mPoseBase.id();		
	D3DXMatrixRotationAxis( &matRotate, &D3DXVECTOR3(0,1,0), PI / 2.f );	
	D3DMATRIXToNxMat34( &matRotate, &mPose );
	mPoseBase.multiply( mPoseBase, mPose );
	mPoseBase.t = NxVec3( m_vPos.x, 0, m_vPos.z );

	// 타겟 포즈 행렬 초기화
	mPose.id();
	mPose.t = mPoseBase.t;

	int j = 0;
	for( int i=0; i < nbElements; i++ )
	{
		pActor = pObjArray[i]->GetActor();
		vScale = pObjArray[i]->GetScale();
		vSize = pObjArray[i]->GetActorSize();

		//////////////////////////////////////////////////////////////////////////
		// 큐브 및 custom mesh를 갖는 칩, Pile을 이루는 칩은 파일링 대상에서 제외한다
		// 오로지 기본 모양을 갖는 단일 칩만 파일링 대상이다.
		//////////////////////////////////////////////////////////////////////////
		if( pObjArray[i]->IsCube() || pObjArray[i]->IsPiled() || pObjArray[i]->HasCustomMesh() )
		{
			nbContinue++;
			continue;
		}		

		pObj = (CtObjectChip*)pObjArray[i];		

		// 칩 객체의 상태를 파일링 된 상태로 만든다
		pObj->RaiseStateFlag( OBJECT_STATE_PILED );

		// Pile에서의 인덱스 순서를 셋트한다		
		pObj->SetPileOrder( j++ );							

		// 칩 객체의 Y축 방향 사이즈만큼 목적 Y 포지션을 설정한다
		if( i == 0 ) 
		{
			mPoseBase.t.y = (vSize.y * vScale.y) / 2.f;						
			pObj->SetPrev( NULL );			
			pObj->GetActor()->setMass( 10 );
		}		

		if( i > 0 ) 
		{
			pObj->GetActor()->setMass( 1.f / j );
			if( i == nbElements - 1 )
			{
				pObj->SetNext( NULL );				
			}
		}		
		
		// 객체의 포즈를 객체가 위치할 타겟 포즈로 트랜지션하도록 키프레임 애니메이션 설정
		if( bPiling )
		{
			pObj->SetPoseTransition( mPoseBase, pObj->GetScale(), 0.6f, 0.6f );

			mPose.t.y += (vSize.y * vScale.y) / 2.f;				
			pObj->SetPoseTransition( mPose, pObj->GetScale(), 0.6f, (i - nbContinue + 1) * 0.05f );
			mPose.t.y += (vSize.y * vScale.y) / 2.f;	

			// 충돌 처리 설정 : 충돌에 의한 파일링 추가 기능
			pActor->setContactReportFlags( NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD | NX_NOTIFY_FORCES );
			pActor->setContactReportThreshold( CONTACT_FORCE_THRESHOLD_ADDCHIP_TO_PILE );

			// Pile의 양 끝 경계에 있는 객체는 바운더리로 지정한다
			if( bPiling && i - nbContinue == 0 ) 
			{
				pObj->SetBoundaryOfPile( TRUE );			
			}
		}
		
		if( (i - nbContinue) > 0 )
		{
			pPrevObj = (CtObjectChip*)pObjArray[i - 1 - nbContinue];	
						
			// 객체의 prev, next 포인터로 앞뒤 객체를 연결한다
			pObj->SetPrev( pPrevObj );
			pPrevObj->SetNext( pObj );			

			if( !bPiling )
			{
				NxActor *pActor1, *pActor2;
				NxJoint *pJoint;

				pObj->SetState( OBJECT_STATE_PILED );

				if( pPrevObj )
				{		
					pActor1 = pPrevObj->GetActor();
					pActor2 = pObj->GetActor();

					// 앞위 액터 사이에 고정 조인트를 생성하여 연결한다
					pJoint = GetCTmain()->m_PhysX.CreateFixedJoint( GetCTmain()->m_pCurrentCube->GetScene(), pActor1, pActor2 );
					pObj->SetPileJointPrev( pJoint );
					pPrevObj->SetPileJointNext( pJoint );					
				}
			}
		}		
		
		nbContinue = 0;
	}
	if( bPiling && pObj ) pObj->SetBoundaryOfPile( TRUE );

	// 파일링이 완료하면 셀렉션을 클리어한다
	Clear();

	m_iPilingNow++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일의 타입(확장자) 별로 분류하여 Pile들을 생성하는 함수

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::PileByFileTypes( VOID )
{
	UINT			iNumTypes = 0;		// 선택물들이 몇가지 파일 타입으로 구성되었는지의 수
	CtFileInfo*		pFI;				// 파일 정보 객체 포인터
	set<CString>	strTypes;			// 파일 타입 문자열들의 셋

	// 선택물의 모든 객체들의 파일 정보 객체를 검사하여 파일 타입 문자열들을 집합에 추가 
	CtObjectBase **pObjArray = m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		pFI = pObjArray[i]->GetFileInfo();
		if( strTypes.find(pFI->lpszFileType) == strTypes.end() )
		{
			strTypes.insert( pFI->lpszFileType );
			iNumTypes++;
		}
	}

	CtObjectSelection* selTypes = new CtObjectSelection[iNumTypes];
	
	int index;
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		pFI = pObjArray[i]->GetFileInfo();

		// set에서 찾은 iterator의 인덱스를 얻음
		index = std::distance( strTypes.begin(), strTypes.find(pFI->lpszFileType) );

		// 해당 인덱스의 셀렉션에 추가		
		selTypes[index].Add( pObjArray[i] );			
		selTypes[index].m_lpszTypeName = pFI->lpszFileType;
	}

	for( int i=0; i < iNumTypes; i++ )
		// 1개 이상인 셀렉션만 파일링한다
		if( selTypes[i].GetNumObjects() > 1 ) 
		{
			m_iPilingNow++;
			selTypes[i].CalcCenter();
			selTypes[i].StartPilingMode();
			selTypes[i].LinkByFixedJoint(TRUE);		
		}

	delete[] selTypes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Pile에서 선택된 칩 객체 들을 떼어내는 함수

 선택물에 존재하는 객체들을 Pile 로부터 결속을 해제하고 Z축 방향으로 이동시켜 떼어낸다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CtObjectSelection::PopChipFromPile( const D3DXVECTOR3* vEyePos )
{
	D3DXVECTOR3		vScale1, vScale2;
	NxActor			*pActor1, *pActor2, *pActorTmp, *pTempActor1, *pTempActor2;
	NxJoint			*pJoint1, *pJoint2, *pJointTmp;
	CtObjectChip	*pObj1 = NULL, *pObj2 = NULL, *pTempChip = NULL;

	CtObjectBase **pObjArray = (CtObjectBase**)m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{	
		CtObjectChip *pObj = (CtObjectChip*)pObjArray[i];		

		// 떼어지는 칩의 그룹을 설정하여 충돌감지 하지 않도록 한다. 
		// 칩이 떼어지면서 Pile의 칩들과 마찰되지 않도록 하기 위함.
		pObj->SetCollisionGroup( ACTOR_GROUP_BREAKINGCHIP );

		// 위아래 칩에 대한 양쪽 조인트 끊음		
		pJoint1 = pObj->GetPileJointPrev();
		pJoint2 = pObj->GetPileJointNext();

		// 앞쪽 조인트가 있으면 해제
		if( pJoint1 ) 
		{
			pJoint1->getActors( &pActor1, &pActorTmp );
			pObj1 = (CtObjectChip*)pActor1->userData;
			pObj->GetParentCube()->GetScene()->releaseJoint( *pJoint1 );
			pObj->SetPileJointPrev( NULL );
			pObj1->SetPileJointNext( NULL );
		}
		// 뒤쪽 조인트가 있으면 해제
		if( pJoint2 ) 
		{
			pJoint2->getActors( &pActorTmp, &pActor2 );
			pObj2 = (CtObjectChip*)pActor2->userData;
			pObj->GetParentCube()->GetScene()->releaseJoint( *pJoint2 );
			pObj2->SetPileJointPrev( NULL );
			pObj->SetPileJointNext( NULL );

			pTempChip = pObj2;

			// 뒤쪽 객체들을 전부 검사하여 PileOrder를 업데이트시킨다.
			// 선택 칩이 떼어져 Pile에서 없어지므로 Pile에 존재하는 칩들의 PileOrder가 바뀌어야 함
			while( pJointTmp = pTempChip->GetPileJointNext() )
			{
				pJointTmp->getActors( &pTempActor1, &pTempActor2 );
				pTempChip->SetPileOrder( pTempChip->GetPileOrder() - 1 );
				pTempChip = (CtObjectChip*)pTempActor2->userData;
			}			
		}

		// 떼어지는 칩 속도 설정		
		NxVec3 vP = NxVec3(vEyePos->x, vEyePos->y, vEyePos->z) - pObj->GetPose().t;
		vP.normalize();
		pObj->UnFreeze();
		pObj->GetActor()->setLinearVelocity( vP * 16 );

		// 위아래 칩 연결. 위아래 칩이 모두 있는 경우만 해당
		if( pObj1 != NULL && pObj2 != NULL )
		{
			vScale1 = pObj1->GetScale();
			vScale2 = pObj2->GetScale();

			NxMat34 matPose = pActor1->getGlobalPose();
			NxVec3 vUpvec = matPose.M.getColumn( 1 );
			vUpvec.normalize();
			vUpvec *= ( (CHIP_THICKNESS * vScale1.z) / 2.f ) + ( (CHIP_THICKNESS * vScale2.z) / 2.f);

			// 떼어지는 칩의 shape들을 모두 BREAKINGCHIP 액터 그룹 설정한다. 
			// 이 액터 그룹으로 설정되어 있는 동안은 기본 액터 그룹과 충돌하지 않는다.
			pObj2->SetCollisionGroup( ACTOR_GROUP_BREAKINGCHIP );

			pActor1->setLinearVelocity( NxVec3(0,0,0) );
			pActor2->setLinearVelocity( NxVec3(0,0,0) );

			matPose.t += vUpvec;
			pActor2->setGlobalPose( matPose );				

			pActor1->putToSleep();
			pActor2->putToSleep();
			
			// 떼어지는 칩의 위아래 칩을 조인트로 연결
			NxJoint* newJoint = GetPhysX()->CreateFixedJoint( pObj->GetParentCube()->GetScene(), pActor1, pActor2 );
			pObj1->SetPileJointNext( newJoint );
			pObj2->SetPileJointPrev( newJoint );

			// 칩이 완전히 떼어지고 나서는 다시 원래의 기본 액터 그룹으로 복원시켜 충돌처리가 되도록 한다.
			pObj2->SetCollisionGroup( ACTOR_GROUP_DEFAULT );
		}		
		
		// 떼어진 칩의 PILED 상태를 해제시킨다
		pObj->ClearStateFlag( OBJECT_STATE_PILED );

		pObj->SetPrev( NULL );
		pObj->SetNext( NULL );

		pObj->ResetMass( 1 );		
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 선택된 객체들 중 Pile에 속한 것들에 대해 해당 Pile들을 모두 끊는 함수

 예를 들어 3개의 Pile이 있고 선택물 중 2개의 칩이 2개의 Pile에 각각 속한 것이면 그 2개의 Pile을 모두 break 한다

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::BreakPiles( VOID )
{
	CtObjectBase **pObjArray = (CtObjectBase**)m_Array.GetData();
	for( int i=0; i < m_Array.GetSize(); i++ )
	{
		if( pObjArray[i]->IsPiled() ) BreakPile( pObjArray[i] );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 지정한 객체가 속한 Pile을 끊는 함수

 * \param pObj	(칩) 객체 포인터
 * \return		(끊어진) Pile에 있었던 객체들의 수
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int 
CtObjectSelection::BreakPile( CtObjectBase* pObj )
{	
	CtObjectChip*	pTemp = (CtObjectChip*)pObj;
	CtObjectChip*	pPrev;
	NxActor			*actor1, *actor2;
	NxJoint*		pJoint1;
	NxJoint*		pJoint2;
	NxScene*		pScene = pTemp->GetParentCube()->GetScene();

	// Pile의 맨 아래 칩을 찾아간다. pTemp가 맨 아래 칩을 가리키게 됨.
	while( pPrev = pTemp->GetPrev() ) pTemp = pPrev;

	// 다시 올라가면서 조인트들을 끊는다	
	int numObjects = 0;
	while( 1 )
	{	
		pTemp->ResetMass( 1 );
		//pTemp->GetActor()->setMassSpaceInertiaTensor( NxVec3(0,0,0) );
		pTemp->SetPrev( NULL );
		pTemp->SetNext( NULL );

		pTemp->ClearStateFlag( OBJECT_STATE_PILED );
		pTemp->SetPileOrder( 0 );
		
		pJoint1 = pTemp->GetPileJointPrev();
		if( pJoint1 != NULL ) pScene->releaseJoint( *pJoint1 );
		pTemp->SetPileJointPrev( NULL );

		pJoint2 = pTemp->GetPileJointNext();
		if( !pJoint2 ) break;

		pJoint2->getActors( &actor1, &actor2 );		

		pPrev = pTemp;
		pTemp = (CtObjectChip*)actor2->userData;
		pPrev->SetPileJointNext( NULL );
		
		numObjects++;
	}

	return numObjects;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 배열을 받아서 지정한 메소드로 배열을 소트하는 함수

 현재는 객체의 사이즈를 통한 단순 퀵소팅을 하고 있으나 priority queue를 사용하고 여러 속성별로 소트할 수 있도록 개선하는 것이 좋을 듯함

 * \param iMethod	소팅 방법
 * \param **pArray	객체 배열 포인터
 * \param top		소트할 배열 부분의 마지막 인덱스
 * \param bottom	소트할 배열 부분의 첫 인덱스
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectSelection::SortBy( int iMethod, CtObjectBase **pArray, int top, int bottom )
{
	int middle;
	if( top < bottom )
	{
		middle = SortBy_partition( iMethod, pArray, top, bottom );
		SortBy( iMethod, pArray, top, middle );
		SortBy( iMethod, pArray, middle+1, bottom );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 소트 함수의 partition 수행 함수

 * \param iMethod	소팅 방법
 * \param **pArray	객체 배열 포인터
 * \param top		소트할 부분의 마지막 인덱스
 * \param bottom	소트할 부분의 첫 인덱스
 * \return			middle 인덱스를 반환
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT
CtObjectSelection::SortBy_partition( int iMethod, CtObjectBase **pArray, int top, int bottom )
{	
	FLOAT x = pArray[top]->GetScale().x;
	int i = top - 1;
	int j = bottom + 1;
	CtObjectBase *temp;

	do {
		do {
			j--;
		} while( x > pArray[j]->GetScale().x );

		do 
		{
			i++;
		} while( x < pArray[i]->GetScale().x );

		if( i < j )
		{
			temp = pArray[j];
			m_Array.SetAt( j, pArray[i] );
			m_Array.SetAt( i, temp );			
		}
	} while( i < j );
	
	return j;	
}

VOID
CtObjectSelection::DoAtAllObject( CtSelectionMethod method, ... )
{
	va_list pArgs;
	va_start( pArgs, method );

	CtObjectBase** pObjects = m_Array.GetData();
	for( DWORD i=0; i < m_Array.GetSize(); i++ )
	{
		pObjects[i]->DoMethod( method, pArgs );
	}	

	va_end( pArgs );
}

//////////////////////////////////////////////////////////////////////////
// 
// Lasso Tool
//
//////////////////////////////////////////////////////////////////////////
CtToolSelectFan::CtToolSelectFan( VOID )
{
	m_bDirection			= TRUE;
	m_bReverse				= FALSE;
	m_fTheta				= 0;
}

CtToolSelectFan::~CtToolSelectFan( VOID )
{

}

VOID
CtToolSelectFan::Init( UINT iNumVBSize )
{
	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();
	pDev->CreateVertexBuffer( iNumVBSize, D3DUSAGE_WRITEONLY, D3DFVF_VERTEX, D3DPOOL_SYSTEMMEM, &m_pVB, NULL );
	
	m_iMaxSize = iNumVBSize;
	m_Vertices.SetSize( iNumVBSize );
}

VOID
CtToolSelectFan::Draw( VOID )
{
	DWORD nVerts = m_Vertices.GetSize();

	if( nVerts < 3 ) return;

	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();

	VERTEX* pV = m_Vertices.GetData();

	// 영역 그리기
	pDev->SetFVF( D3DFVF_VERTEX );
	pDev->SetTexture( 0, 0 );

	pDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );	

	pDev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, nVerts - 2, (VOID*)pV, sizeof(VERTEX) );
	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );	
	pDev->DrawPrimitiveUP( D3DPT_LINESTRIP, nVerts - 1, (VOID*)pV, sizeof(VERTEX) );

	pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );	
	pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

VOID 
CtToolSelectFan::Ready( D3DXVECTOR3 v0, CPoint pt )
{
	m_fTheta = 0;
	m_bDirection = TRUE;
	m_bReverse = FALSE;

	m_vStartLassoPos = v0;

	VERTEX pV = { D3DXVECTOR3(pt.x, pt.y, 0.f), 1.f, DEFAULT_LASSOVERTEX_COLOR };
	m_Vertices.RemoveAll();
	m_Vertices.SetSize( DEFAULT_LASSOVERTEX_SIZE );
	m_Vertices.Add( pV );
}

VOID 
CtToolSelectFan::Begin( CPoint pt )
{	
	VERTEX pV = { D3DXVECTOR3(pt.x, pt.y, 0.f), 1.f, DEFAULT_LASSOVERTEX_COLOR };
	m_Vertices.Add( pV );
}

VOID
CtToolSelectFan::Process( D3DXVECTOR3 v1, D3DXVECTOR3 v2, CPoint pt )
{
	D3DXVECTOR3 v0 = m_vStartLassoPos;			
	D3DXVECTOR3 e1 = v1 - v0;
	D3DXVECTOR3 e2 = v2 - v0;

	FLOAT e1Distance = D3DXVec3Length( &e1 );
	FLOAT e2Distance = D3DXVec3Length( &e2 );

	// 내적을 구해 각도 계산
	FLOAT degree = GetAngleBetweenVectors( v1 - v0, v2 - v0 );
	if( degree > PI || degree < -PI ) return;			

	// 외적을 구해 방향을 파악
	D3DXVECTOR3 vCross;

	D3DXVec3Cross( &vCross, &v1, &v2 );
	if( vCross.y > 0 ) {
		m_bDirection = TRUE;
	}
	else 
	{
		m_bDirection = FALSE;
		degree = -degree;
	}

	m_fTheta += degree;

	DWORD numV = m_Vertices.GetSize();
	VERTEX pV = { D3DXVECTOR3(pt.x, pt.y, 0), 1, DEFAULT_LASSOVERTEX_COLOR };
	
	m_Vertices.Add( pV );	
}

VOID
CtToolSelectFan::End( VOID )
{
}