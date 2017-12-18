#pragma once

#include "stdafx.h"

#undef min
#undef max
#include "NxPhysics.h"
#include <vector>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 키 데이터 멤버 변수 타입들을 정의한 열거형

 애니메이션을 원하는 속성들을 모아서 CtKeyDataBase 클래스로부터 상속받는 클래스를 정의하여 사용하게 되는데,
 이때 정의하는 클래스의 멤버 변수들의 타입을 TellAttributeList 함수를 통해 알려주어야 한다. 
 이때 이 열거형에 정의된 것들을 사용하여 멤버 선언된 순서대로 arrTypes에 넣으면 된다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum enDataType
{
	ADT_BOOL,
	ADT_INT,
	ADT_LONG,
	ADT_FLOAT,
	ADT_DOUBLE,
	ADT_D3DXVECTOR3,
	ADT_D3DXMATRIX,
	ADT_NXVEC3,
	ADT_NXMAT34,
};
/// 열거형의 타입 크기(sizeof)를 반환하는 함수
UINT GetSizeOfADT( enDataType type );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 키 데이터 기본 클래스

 키프레임 애니메이션을 위한 키 데이터를 정의하는 클래스이다.
 이 클래스로부터 상속받아 각각의 목적에 맞는(예를들어 카메라의 경우 eye, lookat, fov값등을 키프레임 속성으로 하는 등) 키 데이터 클래스를 만들어 사용한다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CtKeyDataBase
{
protected:		
	ULONG					m_iKeyAttributes;			///< 이전 키와 이 키 사이 구간에서 애니메이션되는 속성들을 정의.

public:
	/// 생성자
	CtKeyDataBase( VOID )	{ m_iKeyAttributes = 0; }

	/// 키 데이터의 멤버 변수들의 타입들을 나타내는 플래그를 반환
	ULONG					GetKeyAttributesFlag( VOID )		{ return m_iKeyAttributes; }

	/// 키 데이터의 멤버 변수들의 타입들을 나타내는 플래그를 지정
	VOID					SetKeyAttributesFlag( ULONG flag )	{ m_iKeyAttributes = flag; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 오브젝트 키 데이터 클래스

 객체의 키프레임 애니메이션을 위한 키 데이터를 정의하는 클래스이다.
 기본적으로 포즈 및 스케일 값을 애니메이션하도록 되어 있다. 
 
 @note 
 다른 속성을 애니메이션하고 싶을 경우 클래스 멤버 변수를 추가하고, 아래 CTKA_ 플래그 값을 선언하여 사용한다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 객체 키 데이터 속성의 종류를 정의. 아래 CtObjectKeyData의 멤버 변수와 맞춰 준다.
// <주의!> : 이 비트플래그들은 클래스에 선언된 **멤버변수의 순서와 일치** 해야하며, 1을 0~N까지 쉬프트한 값으로만 지정해야 한다.
#define CTKA_OBJ_POSE		(1 << 0)
#define CTKA_OBJ_SCALE		(1 << 1)

class CtObjectKeyData : public CtKeyDataBase
{
public:
	NxMat34					matPose;					///< 객체 포즈 행렬
	D3DXVECTOR3				vScale;						///< 객체의 사이즈 벡터

	/// 생성자
	CtObjectKeyData( VOID ) {}

	/// 생성자
	CtObjectKeyData( NxMat34 pose, D3DXVECTOR3 scale )	
	{ 
		matPose = pose; 
		vScale = scale; 
	}

	/// 속성 정보를 작성해서 CtAnimInfo 객체에 알려주는 함수 구현
	static VOID				TellAttributeList( vector<enDataType>& arrTypes )
	{
		arrTypes.push_back( ADT_NXMAT34 );
		arrTypes.push_back( ADT_D3DXVECTOR3 );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 카메라 키 데이터 클래스

 카메라의 키프레임 애니메이션을 위한 키 데이터를 정의하는 클래스이다.
 eye, lookat, fov 등을 애니메이션하도록 되어 있다. 
 
 @note 
 다른 속성을 애니메이션하고 싶을 경우 클래스 멤버 변수를 추가하고, 아래 CTKA_ 플래그 값을 선언하여 사용한다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 카메라 키 데이터 속성의 종류를 정의. 아래 CtCameraKeyData의 멤버 변수와 맞춰 준다.
// <주의!> : 이 비트플래그들은 클래스에 선언된 **멤버변수의 순서와 일치** 해야하며, 1을 0~N까지 쉬프트한 값으로만 지정해야 한다.
#define CTKA_CAM_EYE		(1 << 0)
#define CTKA_CAM_LOOKAT		(1 << 1)
#define CTKA_CAM_FOV		(1 << 2)

class CtCameraKeyData : public CtKeyDataBase
{
public:
	D3DXVECTOR3				vEye;						///< 카메라 위치 벡터
	D3DXVECTOR3				vLookAt;					///< 카메라가 보는 지점
	FLOAT					fFOV;						///< FOV 값

	/// 생성자
	CtCameraKeyData( VOID ) {}

	/// 생성자
	CtCameraKeyData( D3DXVECTOR3 eye, D3DXVECTOR3 lookat, FLOAT fov ) { vEye = eye; vLookAt = lookat; fFOV = fov; }

	/// 속성 정보를 작성해서 CtAnimInfo 객체에 알려주는 함수 구현
	static VOID				TellAttributeList( vector<enDataType>& arrTypes )
	{
		arrTypes.push_back( ADT_D3DXVECTOR3 );
		arrTypes.push_back( ADT_D3DXVECTOR3 );
		arrTypes.push_back( ADT_FLOAT );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 키프레임 클래스

 템플릿에 의해 해당 키 데이터 클래스에 대한 키프레임을 정의하는 클래스이다.
 멤버 함수는 없고 속성만을 갖는다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class CtKeyFrame
{
public:
	FLOAT					fBias;						///< 키 사이에서 어느 키 쪽에 더 쏠리게 애니메이션하는지의 값
	UINT					iNumFrame;					///< 키 프레임이 위치하는 프레임 값. 
	T						data;						///< 키 데이터 객체. 

public:
	CtKeyFrame<T>( VOID ) 
	{
		iNumFrame = 0;
		fBias = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 @brief 키프레임 애니메이션 정보 클래스

 키프레임 애니메이션 정보를 가지며 그것들을 관리하는 클래스이다. 
 키프레임 애니메이션 하기를 원하는 객체가 있는 경우 [애니메이션하려는 속성들을 정의하는 키 데이터 클래스를 정의]하고
 해당 객체에 그 키 데이터 클래스를 템플릿 인자로 하는 이 CtAnimInfo 클래스를 멤버로 포함시켜 사용하면 된다.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class CtAnimInfo
{
protected:
	BOOL							m_bEnabled;			///< 키프레임 애니메이션 On, Off 상태
	BOOL							m_bOnTransition;	///< 현재 애니메이션 보간 중인지 여부. 현재 TRUE인 상태에서 새로운 키의 추가는 제한된다.
	UINT							m_iCurrentFrame;	///< 애니메이션 중인 현재 프레임
	UINT							m_iCurrentKey;		///< 현재 키의 인덱스. 0번 키 ~ 1번 키 사이를 애니메이션 중이라면 0을 갖는다
	CGrowableArray< CtKeyFrame<T> >	m_KeyFrameArray;	///< 키프레임 배열
	vector<enDataType>				m_arKeyAttributes;	///< 키 데이터의 변수 타입들을 순서대로 나열한 배열

public:
	/// 생성자
	CtAnimInfo( VOID );									

	/// 키프레임 애니메이션 사용
	VOID					Enable( VOID )				{ m_bEnabled = TRUE; }

	/// 키프레임 애니메이션을 끈다
	VOID					Disable( VOID )				{ m_bEnabled = FALSE; }

	/// 키프레임 애니메이션 사용 여부
	BOOL					IsEnabled( VOID )			{ return m_bEnabled; }

	/// 키프레임 수 반환
	UINT					GetNumKey( VOID )			{ return m_KeyFrameArray.GetSize(); }

	/// 키프레임 배열 포인터 반환
	CtKeyFrame<T>*			GetKeyArray( VOID )			{ return m_KeyFrameArray.GetData(); }

	/// 현재 구간에 대해, 다음 키의 bias 값 및 구간의 프레임 수 등을 가지고 0~1 사이의 값을 갖는 보간값 t를 반환 
	FLOAT					GetT( VOID );

	/// 키프레임을 모두 삭제 및 애니메이션 Off
	VOID					Clear( VOID );

	/// 프레임 이동 함수. 
	BOOL					FrameMove( VOID );

	/// 키프레임을 추가하는 함수. 
	VOID					AddKey( UINT iNumFrame, FLOAT bias, T data );

	/// 가변 인자를 통해 키프레임을 추가하는 함수. 키 데이터 속성들 중 일부만을 애니메이션 하고 싶을때를 위함.
	VOID					AddKey( UINT iNumFrame, FLOAT bias, ULONG iKeyAttributesFlag, va_list pArgs );

	/// 현재 프레임의 데이터를 돌려주는 함수.
	T						GetData( VOID );

	/// 키의 값들을 가변 인자에 의해 필요한 것들만 자동으로 셋트해 주는 함수. AddKey 함수에서 호출됨.
	VOID					SetData( UINT iKeyIndex, ULONG iKeyAttributesFlag, va_list pArgs );
};

// 템플릿 함수들은 관례상 hpp에 정의한다.
#include "CT_AnimInfo.hpp"	