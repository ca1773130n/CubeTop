#include "stdafx.h"
#include "CT_PhysX.h"
#include "CT_Object.h"
#include "NxCooking.h"
#include "Stream.h"
#include <vector>

/*************************************************************************************************************************************************************/
/* 피직스 클래스                                                                                                                                             */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메인 초기화 함수

 PhysX 환경을 초기화 하는 함수이다. SDK를 초기화 하고 기본 파라미터를 설정한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL 
CtPhysX::Init( VOID )
{
	NxPhysicsSDKDesc desc;
	NxSDKCreateError errorCode = NXCE_NO_ERROR;

	m_pPhysXSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, NULL, desc, &errorCode );

	if( !m_pPhysXSDK ) 
	{
		MessageBox( 0, L"PhysX SDK를 초기화하지 못했습니다.", 0, 0 );
		return false;
	}

	NxHWVersion hwCheck = m_pPhysXSDK->getHWVersion();
	m_bHasHW = (hwCheck != NX_HW_VERSION_NONE);

	// PhysX 파라미터 설정
	m_pPhysXSDK->setParameter( NX_SKIN_WIDTH, 0.005f );
	m_pPhysXSDK->setParameter( NX_ASYNCHRONOUS_MESH_CREATION, 1.0f );	

	m_bSimulation = TRUE;

	// 쿠킹 초기화
	if( !NxInitCooking() )
	{
		MessageBox(0,L"NxInitCooking 실패!",0,0);
		exit(0);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief PhysX 씬 생성 함수

 큐브 마다 할당되는 기본 씬 객체를 생성하는 함수이다. 이 함수는 한개의 ground plane static actor를 갖는 기본 씬을 생성한다.
 또 이 기본 씬은 기본 및 고마찰력 피직스 재질 및 기본 collision group들을 갖는다.

 * \note

 씬을 생성할 때 NxSceneDesc 작성시 flags 값에 유의할 것. 멀티쓰레딩 시 원하지 않는 결과를 얻을 수 있다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NxScene*
CtPhysX::CreateScene( VOID )
{
	NxBounds3 bounds;
	bounds.max = NxVec3( -32, -32, -32 );
	bounds.min = NxVec3( 32, 32, 32 );

	NxSceneDesc sceneDesc;
	sceneDesc.simType			= ( m_bHasHW ? NX_SIMULATION_HW : NX_SIMULATION_SW );
	sceneDesc.gravity			= NxVec3( 0.0f, -20.0f, 0.f );
	sceneDesc.maxBounds			= &bounds;
	sceneDesc.flags				|= NX_SF_RESTRICTED_SCENE;
	sceneDesc.flags				|= NX_SF_ENABLE_MULTITHREAD;
	//sceneDesc.flags				&= ~NX_SF_SIMULATE_SEPARATE_THREAD;

	NxScene *scene = m_pPhysXSDK->createScene( sceneDesc );
	if( !scene ) 
	{		
		MessageBox( 0, L"PhysX Scene 생성 에러", 0, 0 );
		return NULL;
	}

	// 기본 재질 설정
	NxMaterial* defaultMaterial = scene->getMaterialFromIndex( 0 );
	defaultMaterial->setRestitution( 0.0f );
	defaultMaterial->setStaticFriction( 0.4f );
	defaultMaterial->setDynamicFriction( 0.2f );

	NxMaterialDesc mDesc;
	mDesc.staticFriction = 1000.0f;
	mDesc.dynamicFriction = 1000.0f;
	mDesc.restitution = 0.f;	
	scene->createMaterial( mDesc );

	scene->setGroupCollisionFlag( ACTOR_GROUP_DEFAULT, ACTOR_GROUP_TEMPPLANE, FALSE );
	scene->setGroupCollisionFlag( ACTOR_GROUP_DEFAULT, ACTOR_GROUP_BREAKINGCHIP, FALSE );
	scene->setGroupCollisionFlag( ACTOR_GROUP_BREAKINGCHIP, ACTOR_GROUP_BREAKINGCHIP, FALSE );

	// 이벤트 처리를 위한 user notify, contact report 등록
	scene->setUserNotify( &m_PhysXUserNotify );
	scene->setUserContactReport( &m_PhysXContactReport );

	// 큐브 내부 6면 액터 생성	
	NxPlaneShapeDesc planeDesc;		
	NxActorDesc actorDesc;		

	planeDesc.normal = NxVec3( 0, 1, 0 );
	planeDesc.d = 0;
	actorDesc.shapes.pushBack( &planeDesc );
	scene->createActor( actorDesc );	

	return scene;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 정리 함수

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtPhysX::Exit( VOID )
{
	if(m_pPhysXSDK != NULL)
	{
		NxReleasePhysicsSDK(m_pPhysXSDK);
		m_pPhysXSDK = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 지정한 씬의 두 액터 사이에 고정 조인트를 생성하는 함수

 * \param pScene	씬 포인터
 * \param a0		액터1 포인터
 * \param a1		액터2 포인터
 * \return			생성한 조인트 포인터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NxFixedJoint* 
CtPhysX::CreateFixedJoint( NxScene* pScene, NxActor* a0, NxActor* a1 )
{	
	NxFixedJointDesc jointDesc;
	jointDesc.jointFlags |= NX_JPM_LINEAR_MINDIST;
	jointDesc.actor[0] = a0;
	jointDesc.actor[1] = a1;		
	NxFixedJoint *joint = (NxFixedJoint*)pScene->createJoint(jointDesc);

	return joint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 박스 등 기본도형이 아닌 실제 메시의 액터를 생성하는 함수이다.

 * @param bScaling 액터를 생성할때 객체의 스케일 정보를 사용할 것인지 여부

 * @warning

 PhysX 기본 도형 간 및 기본 도형과 TriangleMesh 액터 간의 충돌처리는 제대로 되나
 TriangleMesh 간의 충돌처리는 완전하지 않다. 이것을 위해서는 Pmap을 사용하여야 한다.

 Pmap를 사용하려면 최초 1번은 메시 파일에 대한 Pmap 파일을 작성하여야 하며, 비용이 크다.
 한번 Pmap 파일을 작성하였으면 다음부터는 그것을 불러들여 사용하면 되고 비용은 거의 들지 않는다.

 Pmap을 사용하더라도 TriangleMesh간의 충돌처리는 완전하지 않다. TriangleMesh가 실제 메시를
 많이 단순화시키기 때문이다. Pmap의 하드웨어 지원은 아직 되지 않고 있다.

 * \param pObject	CT객체 포인터
 * \param bScaling	TRUE이면 객체의 스케일 벡터에 따라 스케일링된 피직스 메시 생성, FALSE면 그대로 생성
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtPhysX::GenerateTriangleMesh( VOID* pObject, bool bScaling )
{
	CtObjectBase* pObj	= (CtObjectBase*)pObject;
	LPD3DXMESH pMesh	= pObj->GetDXUTMesh()->m_pMesh;

	DWORD NumVerticies	= pMesh->GetNumVertices();
	DWORD NumTriangles	= pMesh->GetNumFaces();

	NxVec3* verts		= new NxVec3[NumVerticies];
	WORD*pIndices		= new WORD[NumTriangles*3];

	FLOAT				maxX = 0, maxY = 0, maxZ = 0;		// 부피 추정을 위한 값들
	BYTE*				DXMeshPtr;

	pMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&DXMeshPtr);

	//////////////////////////////////////////////////////////////////////////
	// 버텍스 버퍼 채움
	//////////////////////////////////////////////////////////////////////////
	NxVec3 vScale = pObj->GetScale();
	DWORD perByte = pMesh->GetNumBytesPerVertex();
	for(DWORD i = 0; i < NumVerticies; i++)
	{
		MESHVERTEX *DXMeshFVF = (MESHVERTEX*)DXMeshPtr;
		verts[i] = NxVec3(DXMeshFVF->pos.x * vScale.x, DXMeshFVF->pos.y * vScale.y, DXMeshFVF->pos.z * vScale.z);
		if( maxX < verts[i].x ) maxX = verts[i].x;
		if( maxY < verts[i].y ) maxY = verts[i].y;
		if( maxZ < verts[i].z ) maxZ = verts[i].z;
		DXMeshPtr += perByte;
	}
	pMesh->UnlockVertexBuffer();

	//////////////////////////////////////////////////////////////////////////
	// 인덱스 버퍼 채움
	//////////////////////////////////////////////////////////////////////////
	WORD *pIB = 0;
	pMesh->LockIndexBuffer(D3DLOCK_READONLY, (VOID**)&pIB);
	memcpy( pIndices, pIB, sizeof(WORD)*NumTriangles*3 );	
	pMesh->UnlockIndexBuffer();

	NxTriangleMeshDesc TriMeshDesc;
	TriMeshDesc.numVertices				= NumVerticies;
	TriMeshDesc.numTriangles			= NumTriangles;
	TriMeshDesc.pointStrideBytes		= 3 * sizeof(float);
	TriMeshDesc.triangleStrideBytes		= 3 * sizeof(WORD);
	TriMeshDesc.points					= verts;
	TriMeshDesc.triangles				= pIndices;
	TriMeshDesc.flags					= NX_MF_16_BIT_INDICES;

	// 메모리 버퍼에 메시를 쿠킹
	MemoryWriteBuffer buf;	
	bool status = NxCookTriangleMesh(TriMeshDesc, buf);
	if( !status )
	{
		MessageBox( 0, L"PhysX cooking failed!", 0, 0 );
		if( verts )
			delete[] verts;
		if( pIndices )
			delete[] pIndices;
		return;
	}	

	// 기존 메쉬와 액터를 삭제한다
	NxActor* pActor = pObj->GetActor();
	NxTriangleMesh* pActorMesh = pObj->GetActorMesh();

	if( pActor ) pObj->GetParentCube()->GetScene()->releaseActor( *pActor );
	if( pActorMesh ) m_pPhysXSDK->releaseTriangleMesh( *pActorMesh );

	pActorMesh = m_pPhysXSDK->createTriangleMesh(MemoryReadBuffer(buf.data));

	NxTriangleMeshShapeDesc ShapeDesc;	

	// 바디 설정
	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= 0.5f;		
	bodyDesc.maxAngularVelocity = 1.f;
	bodyDesc.linearVelocity = NxVec3(0,0,0);

	// 액터 설정
	NxActorDesc actorDesc;
	ShapeDesc.meshData = pActorMesh;
	actorDesc.shapes.pushBack(&ShapeDesc);
	actorDesc.body				= &bodyDesc;
	actorDesc.density			= 50.0f;

	if( bScaling ) 
		actorDesc.globalPose	= pObj->GetPose();	
	else 
		actorDesc.globalPose.t	= NxVec3( pObj->GetPose().t.x, pObj->GetPose().t.y, 0 );

	// TriangleMesh 액터 생성
	pObj->SetActor( pActor = pObj->GetParentCube()->GetScene()->createActor(actorDesc) );
	pActor->userData = (VOID*)this;
	pActor->setMass( maxX * maxY * maxZ );	

	if( verts ) delete[] verts;
	if( pIndices ) delete[] pIndices;
	buf.clear();
}

/*************************************************************************************************************************************************************/
/* 피직스 UserNotify를 위한 클래스                                                                                                                           */
/*************************************************************************************************************************************************************/

CtPhysXUserNotify::CtPhysXUserNotify( void )
{
}

CtPhysXUserNotify::~CtPhysXUserNotify( void )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** 
 * \brief PhysX 객체 sleep 이벤트 핸들러

 sleep 모드로 진입하는 객체가 있을 경우 발생하는 이벤트 핸들러 함수이다. CtObjectBase::OnSleep 함수로 연결시킨다

 * \param **actors	sleep 되는 액터들의 배열
 * \param count		액터들의 수(배열 크기)
 * \return			없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtPhysXUserNotify::onSleep( NxActor **actors, NxU32 count )
{
	CtObjectBase* pObj;
	for( int i=0; i < count; i++ ) 
		((CtObjectBase*)actors[i]->userData)->OnSleep();		
}

/*************************************************************************************************************************************************************/
/* 피직스 UserContactReport를 위한 클래스                                                                                                                    */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 간 충돌 이벤트 발생시 호출되는 함수이다.

 이 함수를 사용하여 다양한 충돌 이벤트 발생시에 UI의 기능으로 활용할 수 있다.(ex: 클립보드에 충돌->클립보드 추가, Pile에 충돌->Pile에 추가 등)

 * \param pair		contact pair 객체. 충돌한 액터 및 기타 정보를 가진다
 * \param events	발생 이벤트 정보. 터치 시작인지 중인지 끝인지 여부와 threshold 값등을 알려줌. SDK참고.
 * \return			없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtPhysXContactReport::onContactNotify( NxContactPair& pair, NxU32 events )
{
	CtObjectBase* pObj1 = (CtObjectBase*)pair.actors[0]->userData;	
	CtObjectBase* pObj2 = (CtObjectBase*)pair.actors[1]->userData;

	//////////////////////////////////////////////////////////////////////////
	// Pile과 칩 객체의 충돌시
	//////////////////////////////////////////////////////////////////////////
	if( !pObj1 || !pObj2 ) return;//|| pair.sumNormalForce.magnitude() < CONTACT_FORCE_THRESHOLD_ADDCHIP_TO_PILE ) return; 	

	bool bPile1, bPile2;
	bPile1 = pObj1->IsPiled();
	bPile2 = pObj2->IsPiled();
	
	// 둘중 하나만 piled인 경우만 해당
	if( pObj1->IsChip() && pObj2->IsChip() && (bPile1 ^ bPile2) )
	{		
		CtObjectChip* pPile = bPile1 ? (CtObjectChip*)pObj1 : (CtObjectChip*)pObj2;
		CtObjectChip* pChip = bPile1 ? (CtObjectChip*)pObj2 : (CtObjectChip*)pObj1;

		if( !pChip->IsForced() ) return;
		NxMat34 matPose;

		NxActor* pActor1;
		NxActor* pActor2 = pChip->GetActor();

		while( pPile->GetNext() )
			pPile = pPile->GetNext();

		pActor1 = pPile->GetActor();
		
		// 포인터 연결 및 기타 설정
		pChip->SetPileJointNext( NULL );
		pPile->SetBoundaryOfPile( FALSE );
		pChip->SetBoundaryOfPile( TRUE );
		pChip->SetPileOrder( pPile->GetPileOrder() + 1 );

		//////////////////////////////////////////////////////////////////////////
		// 타겟 포즈를 구한다
		//////////////////////////////////////////////////////////////////////////
		matPose = pActor1->getGlobalPose();
		NxVec3 vUp = matPose.M.getColumn( 1 );
		vUp.normalize();
		D3DXVECTOR3 vScale1 = pPile->GetScale();
		D3DXVECTOR3 vScale2 = pChip->GetScale();

		vUp *= ( (CHIP_THICKNESS * vScale1.z) / 2.f ) + ( (CHIP_THICKNESS * vScale2.z) / 2.f);
		matPose.t += vUp;

		pChip->SetCollisionGroup( ACTOR_GROUP_BREAKINGCHIP );
		pActor2->raiseActorFlag( NX_AF_DISABLE_COLLISION );
		pChip->RaiseStateFlag( OBJECT_STATE_PILED );

		pChip->SetPoseTransition( matPose, pChip->GetScale(), 0.6f, 0.8f );		

		pChip->SetPrev( pPile );
		pChip->SetNext( NULL );
		pPile->SetNext( pChip );

		pPile->GetActor()->setLinearVelocity( NxVec3(0,0,0) );

		GetCTmain()->m_stStateFlags.bPilingAddModeDone = TRUE;
	}
	// 큐브에 충돌시
	bool bCube1 = pObj1->IsCube();
	bool bCube2 = pObj2->IsCube();
	if( bCube1 || bCube2 )
	{		
		CtObjectCube* pCube = bCube1 ? (CtObjectCube*)pObj1 : (CtObjectCube*)pObj2;
		CtObjectChip* pChip = bCube1 ? (CtObjectChip*)pObj2 : (CtObjectChip*)pObj1;
		
		if( !pChip->IsForced() ) return;

		// 파일을 이동
		CString newPath = pCube->GetFilePath();
		newPath += L"\\" + pChip->GetFileInfo()->lpszFileName;

		MoveFile( pChip->GetFilePath(), newPath );

		// 현재 큐브에서 해당 큐브로 객체 이동
		GetCTmain()->MoveObject( pChip, GetCTmain()->m_pCurrentCube, pCube );

		pCube->GetActor()->setLinearVelocity( NxVec3(0,0,0) );
	}
}