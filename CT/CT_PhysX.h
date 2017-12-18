﻿#pragma once

#include "stdafx.h"

#undef min
#undef max
#include "NxPhysics.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PhysX 정의 및 매크로
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ACTOR_GROUP_DEFAULT					(NxActorGroup)1
#define ACTOR_GROUP_TEMPPLANE				(NxActorGroup)28
#define ACTOR_GROUP_BREAKINGCHIP			(NxActorGroup)29
#define ACTOR_GROUP_STATIC					(NxActorGroup)30
#define	ACTOR_GROUP_SELECTED				(NxActorGroup)31

#define ACTOR_MATERIAL_INDEX_DEFAULT		0
#define ACTOR_MATERIAL_INDEX_HIGHFRICTION	1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 클래스 선언
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtPhysX;
class CtPhysXUserNotify;
class CtPhysXContactReport;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief PhysX User Notify 클래스
 
 Joint가 break될때라든가 하는 이벤트가 발생할때의 핸들링을 위한 클래스이다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtPhysXUserNotify : public NxUserNotify 
{   
public:

public:  		
	CtPhysXUserNotify( void );
	~CtPhysXUserNotify( void );

	// TRUE 리턴시 조인트를 삭제, FALSE 리턴시 삭제하지 않음	
	bool onJointBreak(NxReal breakingImpulse, NxJoint & brokenJoint) { return TRUE; }
	void onWake (NxActor **actors, NxU32 count) {}
	void onSleep( NxActor **actors, NxU32 count );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief PhysX Contact Report 클래스

 특정 pair 그룹 간의 충돌이 발생하는 경우 등의 이벤트를 감지하고 핸들링하기 위한 클래스이다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtPhysXContactReport : public NxUserContactReport
{
public:

public:
	~CtPhysXContactReport( void ) {}
	void onContactNotify( NxContactPair& pair, NxU32 events );

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief PhysX 메인 클래스

 CT 프로그램과 PhysX API 사이의 중간 레이어 역할을 수행. 
 PhysX API들을 이용하여 원하는 작업을 수행하는 함수들을 구현한다.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtPhysX
{
public:
	BOOL					m_bHasHW;				///< PPU 지원 여부
	NxPhysicsSDK*			m_pPhysXSDK;			///< PhysX SDK 메인
	CtPhysXUserNotify		m_PhysXUserNotify;		///< 사용자 정의 이벤트(조인트 상태 변경 등)용 객체
	CtPhysXContactReport	m_PhysXContactReport;	///< 사용자 정의 충돌처리 이벤트용 충돌 리포트 객체

	BOOL					m_bSimulation;			///< 시뮬레이션 수행 여부

public:

	/// 초기화 함수
	BOOL			Init( VOID );

	/// 제거 함수
	VOID			Exit( VOID );

	/// PhysX 장면을 생성하는 함수
	NxScene*		CreateScene( VOID );

	/// 두 액터 사이에 fixed joint를 생성하는 함수
	NxFixedJoint*	CreateFixedJoint( NxScene* pScene, NxActor* a0, NxActor* a1 );

	/// PhysX TriangleMesh를 생성하는 함수
	VOID			GenerateTriangleMesh( VOID* pObject, bool bScaling );

	/// 시뮬레이션 수행 여부를 반환
	BOOL			OnSimulation( VOID )			{ return m_bSimulation; }

	/// 시뮬레이션 토글
	VOID			ToggleSimulation( VOID )		{ m_bSimulation = !m_bSimulation; }
};


