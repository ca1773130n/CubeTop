#include "stdafx.h"
#include "CT_AnimInfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 데이터 타입 열거형의 크기를 반환한다

 * \param type	데이터 타입 열거형
 * \return		데이터 타입 크기
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT 
GetSizeOfADT( enDataType type )
{
	switch( type )
	{
	case ADT_BOOL:
		return sizeof( BOOL );
	case ADT_INT:
		return sizeof( INT );
	case ADT_LONG:
		return sizeof( LONG );
	case ADT_FLOAT:
		return sizeof( FLOAT );
	case ADT_DOUBLE:
		return sizeof( DOUBLE );
	case ADT_D3DXVECTOR3:
		return sizeof( D3DXVECTOR3 );
	case ADT_D3DXMATRIX:
		return sizeof( D3DXMATRIX );
	case ADT_NXVEC3:
		return sizeof( NxVec3 );
	case ADT_NXMAT34:
		return sizeof( NxMat34 );
	default:
		return 0;
	}	
}