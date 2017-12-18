#pragma once
#include "stdafx.h"
#include "D3DEx.h"
#include <bitset>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 생성자

 다른 클래스에서 이 객체를 포인터가 아닌 멤버 변수로 가지고 있기 때문에 인자가 없는 생성자만 사용한다.
 템플릿 타입 T 클래스의 정적 함수 TellAttributeList() 를 호출하여 T 클래스에게 멤버 변수들의 구성을 알려준다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
CtAnimInfo<T>::CtAnimInfo( VOID )
{
	m_bEnabled		= FALSE;
	m_bOnTransition	= FALSE;
	m_iCurrentFrame	= 0;
	m_iCurrentKey	= 0;

	// 키프레임 데이터가 가져야 할 멤버 변수들의 속성을 T 클래스에게 알려준다.
	T::TellAttributeList( m_arKeyAttributes );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 키프레임 추가 함수

 이 함수는 키 데이터 객체 T를 받아서 통째로 키프레임에 넣는다. 
 즉 키 데이터의 모든 변수(속성)을 보간하고 싶은 경우에 사용한다.

 * \param iNumFrame		키프레임의 프레임 번호
 * \param bias			쏠림 값
 * \param data			키 데이터 객체
 * \return				없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
VOID
CtAnimInfo<T>::AddKey( UINT iNumFrame, FLOAT bias, T data )
{
	if( m_bOnTransition ) return;
	
	CtKeyFrame<T> keyframe;

	DWORD numKeys = m_KeyFrameArray.GetSize();

	// 앞 키프레임이 있으면 일단 그대로 복사한다.
	if( numKeys > 0 )
		keyframe = m_KeyFrameArray[numKeys - 1];
	
	keyframe.iNumFrame	+= iNumFrame;
	keyframe.fBias		= bias;
	keyframe.data		= data;

	// 플래그 값을 -1로 하여 모든 비트를 1로 만든다. 모든 속성을 보간할 것임을 명시하는 것.
	keyframe.data.SetKeyAttributesFlag( -1 );

	// 배열에 키프레임 객체를 추가
	m_KeyFrameArray.Add( keyframe );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 키 데이터 객체의 특정 속성만 보간하도록 키프레임을 추가하는 함수.

 이 함수에서는 가변인자를 사용하여 지정한 값만을 보간하도록 해 준다.
 지정하지 않은 속성은 앞 키프레임에서의 값이 그대로 유지된다. 필요 없는 보간 계산을 하지 않기 위함.

 * \param iNumFrame				키프레임 번호
 * \param bias					쏠림 값
 * \param iKeyAttributesFlag	키 속성들을 전부 or 한 플래그 값
 * \param pArgs					가변 인자 리스트. 
 * \return						없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
VOID
CtAnimInfo<T>::AddKey( UINT iNumFrame, FLOAT bias, ULONG iKeyAttributesFlag, va_list pArgs )
{
	if( m_bOnTransition ) return;

	DWORD numKeys = m_KeyFrameArray.GetSize();
	ASSERT( numKeys > 0 );

	// 일단 앞 키프레임을 그대로 복사한다.
	CtKeyFrame<T> keyframe	= m_KeyFrameArray[numKeys - 1];
	
	// 프레임 번호와 쏠림값을 지정
	keyframe.iNumFrame		= iNumFrame;
	keyframe.fBias			= bias;

	// 키를 추가하고
	m_KeyFrameArray.Add( keyframe );

	// 지정한 값들만 보간하도록 설정
	SetData( numKeys, iKeyAttributesFlag, pArgs );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 프레임 이동 함수

 현재 애니메이션 상황을 업데이트한다. 프레임을 증가시키고 마지막 프레임에 도달하면 off 시킴.

 * \param VOID	없음
 * \return		프레임의 마지막에 도달했는지 여부를 반환
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
BOOL
CtAnimInfo<T>::FrameMove( VOID )
{	
	m_iCurrentFrame++;
	m_bOnTransition = TRUE;

	DWORD numKey = m_KeyFrameArray.GetSize();
	
	if( m_iCurrentKey < numKey - 1 && m_iCurrentFrame > m_KeyFrameArray[m_iCurrentKey + 1].iNumFrame ) m_iCurrentKey++;
	if( m_iCurrentKey == numKey - 1 )
	{
		// 마지막 프레임에 도달함, 애니메이션을 off시킨다		
		Clear();
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 애니메이션을 제거시키는 함수.

 지정한 키프레임의 마지막 프레임에 도달하면 이 함수를 호출하여 키프레임 데이터를 모두 지운다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
VOID
CtAnimInfo<T>::Clear( VOID )
{
	Disable();
	m_bOnTransition = FALSE;
	m_iCurrentFrame = 0;
	m_iCurrentKey = 0;
	m_KeyFrameArray.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 양 키 데이터를 보간하는 데 사용할 보간값 t를 구하여 돌려주는 함수

 양 키 데이터의 쏠림값과 구간의 프레임 수 및 현재 프레임을 통해 0~1 사이의 t값을 반환

 * \param VOID	없음
 * \return		보간에 쓸 t 값을 반환
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
FLOAT
CtAnimInfo<T>::GetT( VOID )
{
	CtKeyFrame<T>*	pKey = 0;
	UINT			frame1, frame2;
	FLOAT			bias;
	
	while( !pKey ) pKey = m_KeyFrameArray.GetData();

	frame1 = pKey[m_iCurrentKey].iNumFrame;
	frame2 = pKey[m_iCurrentKey + 1].iNumFrame;
	bias = pKey[m_iCurrentKey + 1].fBias;

	// 양 키 프레임 사이의 구간에서 현재 프레임이 어디에 위치하는지 0~1 사이의 값으로 변환
	FLOAT t = ((FLOAT)(m_iCurrentFrame - frame1)) / ((FLOAT)(frame2 - frame1));

	// 쏠림값에 따라 제곱 또는 루트값을 취한다.
	int iBias = (int)((0.5f - bias) * 10.f);
	if( iBias < 0 ) 
		t = pow( t, -iBias );
	else
		while( iBias-- > 0 ) t = sqrtl( t );

	return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 보간한 키 데이터 객체를 반환하는 함수

 양 키 데이터를 보간값 t를 구하여 그만큼 보간한 키 데이터 객체를 반환한다.

 * \param VOID	없음
 * \return		보간한 키 데이터 객체를 반환
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template < typename T >
T
CtAnimInfo<T>::GetData( VOID )
{
	T				data[3];			// 키 데이터 객체들. 0번에 반환할 객체가, 1번에 앞 키 데이터 객체, 2번에 뒤 키 데이터 객체
	CtKeyFrame<T>	*pKey1, *pKey2;		// 양족 키프레임 객체 포인터
	FLOAT			t;					// 보간값
	UINT			dataSize, chunk;	// 키 데이터 객체의 멤버 변수 크기의 합, 포인터 이동을 위한 chunk
	ULONG			address;			// 현재 포인터 값
	VOID*			ptr;				// 현재 포인터

	// 보간값 계산
	t = GetT();

	pKey1 = &m_KeyFrameArray[m_iCurrentKey];
	pKey2 = &m_KeyFrameArray[m_iCurrentKey + 1];

	data[1] = pKey1->data;
	data[2] = pKey2->data;
	data[0] = data[1];
	
	// 4는 클래스 객체의 시작 부분 포인터로부터 멤버 변수가 시작하는 위치의 offset이 4바이트임을 나타냄.
	// 앞의 4바이트는 가상 함수 테이블관련..이 잡아먹음
	chunk = 4;
	dataSize = sizeof( T );
	
	// 애니메이션하도록 지정한 키 데이터 객체의 멤버 속성 타입에 따라 해당 변수의 위치 포인터를 잡고
	// 양쪽 키프레임 데이터의 속성 값을 보간한 값으로 반환할 키 데이터 객체의 속성을 지정
	UINT numAttributes = m_arKeyAttributes.size();
	for( UINT i=0, address = (ULONG)&data + chunk; i < numAttributes; i++, address += chunk )
	{
		ptr = (VOID*)address;

		// 다음 멤버 변수 타입의 크기를 얻는다.
		chunk = GetSizeOfADT( m_arKeyAttributes[i] );

		// 뒤쪽 키프레임 데이터에서 애니메이션하도록 지정한 속성 변수만 골라서 보간한다.
		if( data[2].GetKeyAttributesFlag() & (1 << i) )
		{
			switch( m_arKeyAttributes[i] )
			{
			case ADT_FLOAT:
				{
					*((FLOAT*)ptr) = GetInterpolatedFLOAT( t, *((FLOAT*)((ULONG)ptr + dataSize)), *((FLOAT*)((ULONG)ptr + 2 * dataSize)) );
					break;
				}
			case ADT_D3DXVECTOR3:
				{
					*((D3DXVECTOR3*)ptr) = GetInterpolatedD3DVec3( t, *((D3DXVECTOR3*)((ULONG)ptr + dataSize)), *((D3DXVECTOR3*)((ULONG)ptr + 2 * dataSize)) );
					break;
				}
			case ADT_D3DXMATRIX:
				{
					*((D3DXMATRIX*)ptr) = GetInterpolatedD3DXMATRIX( t, (D3DXMATRIX*)((ULONG)ptr + dataSize), (D3DXMATRIX*)((ULONG)ptr + 2 * dataSize) );
					break;
				}
			case ADT_NXVEC3:
				{
					*((NxVec3*)ptr) = GetInterpolatedNxVec3( t, *((NxVec3*)((ULONG)ptr + dataSize)), *((NxVec3*)((ULONG)ptr + 2 * dataSize)) );
					break;
				}
			case ADT_NXMAT34:
				{
					*((NxMat34*)ptr) = GetInterpolatedNxMat34( t, (NxMat34*)((ULONG)ptr + dataSize), (NxMat34*)((ULONG)ptr + 2 * dataSize) );
					break;
				}
			}
		}		
	}

	return data[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 추가된 키프레임 데이터에 대해, 지정한 속성들만을 보간하도록 설정

 AddKey() 함수에서 미리 추가된 키프레임 객체에 대해 가변인자로 받은 속성들만 보간하도록 설정한다.

 * \param iKeyIndex				추가한 키프레임의 인덱스 번호
 * \param iKeyAttributesFlag	보간할 키 데이터 객체의 속성들의 플래그를 모두 or 한 값
 * \param pArgs					보간할 속성들 및 속성값들 리스트
 * \return						없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
VOID
CtAnimInfo<T>::SetData( UINT iKeyIndex, ULONG iKeyAttributesFlag, va_list pArgs )
{
	// GetData 참조
	UINT			numAttsOffset, chunk;
	ULONG			iAttribute;
	CtKeyFrame<T>	*pKey;
	T*				pData;
	VOID*			ptr;

	pKey = &m_KeyFrameArray[iKeyIndex];
	pData = &pKey->data;

	// 보간할 속성들이 어떤것들인지 명시
	pData->SetKeyAttributesFlag( iKeyAttributesFlag );

	// 인자로 받은 보간할 속성들의 flag 합에서 1로 셋트된 것이 몇개인지 얻는다.
	// 즉 보간할 속성의 개수를 셈.
	bitset<32> bits( iKeyAttributesFlag );
	UINT iNumArgs = bits.count();

	// 키 데이터 객체의 애니메이트할 멤버 변수 부분을 포인터로 가리키면서 
	// 전달받은 값들로 설정한다
	UINT numAttributes = m_arKeyAttributes.size();
	for( UINT i=0, address = (ULONG)pData + 4; i < iNumArgs; i++ )
	{
		// 가변 인자 리스트에서 다음 속성 타입을 얻음
		iAttribute = va_arg( pArgs, ULONG );

		numAttsOffset = 0;
		for( UINT j=iAttribute; j > 0; j /= 2, numAttsOffset++ ); 

		// chunk 사이즈를 계산
		chunk = 0;
		for( int j=numAttsOffset - 1; j > 0; j-- )
			chunk += GetSizeOfADT( m_arKeyAttributes[j - 1] );

		ptr = (VOID*)(address + chunk);

		// 해당 변수를 가리키는 포인터에 전달받은 속성 값을 지정
		switch( m_arKeyAttributes[numAttsOffset - 1] )
		{
		case ADT_FLOAT:
			{					
				*((FLOAT*)ptr) = (FLOAT)va_arg( pArgs, DOUBLE );
				break;
			}
		case ADT_D3DXVECTOR3:
			{
				*((D3DXVECTOR3*)ptr) = va_arg( pArgs, D3DXVECTOR3 );
				break;
			}
		case ADT_D3DXMATRIX:
			{
				*((D3DXMATRIX*)ptr) = va_arg( pArgs, D3DXMATRIX );
				break;
			}
		}
	}
}