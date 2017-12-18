#include "stdafx.h"
#include "D3DEx.h"

/*************************************************************************************************************************************************************/
/* 벡터, 행렬 등의 계산을 돕는 함수들. Direct3D 내장 함수가 없으면서 자주 쓸 함수들을 이곳에 작성															 */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 내장 D3DXVec3Project() 함수를 사용하려면 world, projection, view 행렬 및 viewport를 얻는 등 번거로우므로 이 함수를 사용

 * \param point	프로젝션하려는 3D 벡터
 * \return		프로젝션 된 스크린 좌표 벡터(z값 무시)
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3
GetProjectedPoint( D3DXVECTOR3& point )
{
	D3DXVECTOR3 vScreen;
	D3DVIEWPORT9 vp;	
	D3DXMATRIX matProjection, matView, matWorld;

	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();
	pDev->GetViewport( &vp );
	pDev->GetTransform( D3DTS_PROJECTION, &matProjection );
	pDev->GetTransform( D3DTS_VIEW, &matView );
	pDev->GetTransform( D3DTS_WORLD, &matWorld );

	D3DXVec3Project( &vScreen, &point, &vp, &matProjection, &matView, &matWorld );

	return vScreen;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 내장 D3DXVec3Unproject() 함수를 사용하려면 world, projection, view 행렬 및 viewport를 얻는 등 번거로우므로 이 함수를 사용

 * \param point				언프로젝션할 2D 좌표
 * \param *vPickRayDir		카메라로부터 피킹 지점으로 향하는 방향 단위 벡터 포인터
 * \param *vPickRayOrig		카메라 시점의 eye 벡터 포인터
 * \return					없음. 포인터로 전달받은 두 벡터의 값을 채움.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
GetUnprojectedPoint( CPoint point, D3DXVECTOR3 *vPickRayDir, D3DXVECTOR3 *vPickRayOrig )
{		
	D3DVIEWPORT9 vp;
	D3DXVECTOR3 v;
	D3DXMATRIX m, matProjection, matView;
	
	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();

	pDev->GetViewport( &vp );
	pDev->GetTransform( D3DTS_PROJECTION, &matProjection );

	v.x = ((  (((point.x-vp.X)*2.0f/vp.Width ) - 1.0f)) - matProjection._31 ) / matProjection._11;
	v.y = ((- (((point.y-vp.Y)*2.0f/vp.Height) - 1.0f)) - matProjection._32 ) / matProjection._22;
	v.z =  1.0f;

	pDev->GetTransform( D3DTS_VIEW, &matView );
	D3DXMatrixInverse( &m, NULL, &matView );
	vPickRayDir->x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
	vPickRayDir->y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
	vPickRayDir->z  = v.x*m._13 + v.y*m._23 + v.z*m._33;
	D3DXVec3Normalize( vPickRayDir, vPickRayDir );
	vPickRayOrig->x = m._41;
	vPickRayOrig->y = m._42;
	vPickRayOrig->z = m._43;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 쿼터니온을 사용하여 주어진 벡터를 축 벡터에 대해 회전시킨다.

 * \param vOut		결과 벡터의 포인터
 * \param vIn		입력 벡터의 포인터
 * \param vAxis		입력 축 벡터의 포인터
 * \param angle		입력 라디안 각도 값
 * \return			결과 벡터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3 
RotateVectorByAxis( D3DXVECTOR3* vOut, D3DXVECTOR3* vIn, D3DXVECTOR3 vAxis, FLOAT angle )
{
	D3DXVECTOR3 vResult;
	D3DXQUATERNION q;
	D3DXMATRIXA16 matRotate;
	q.x = q.y = q.z = 0;
	q.w = 1;

	D3DXQuaternionRotationAxis( &q, &vAxis, angle );
	D3DXMatrixRotationQuaternion( &matRotate, &q );
	D3DXVec3TransformCoord( &vResult, vIn, &matRotate );

	if( vOut ) *vOut = vResult;

	return vResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 내적을 사용하는 일반적인 방식으로 두 3D 벡터 사이의 라디안 각도를 구한다.

 * \param v1	1번 벡터
 * \param v2	2번 벡터
 * \return		사이 각(라디안)
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT 
GetAngleBetweenVectors( CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* v2 )
{
	D3DXVECTOR3 vec1, vec2;
	D3DXVec3Normalize( &vec1, v1 );
	D3DXVec3Normalize( &vec2, v2 );

	FLOAT result = (FLOAT)acos( D3DXVec3Dot( &vec1, &vec2 ) );
	if( result > PI ) result = 2 * PI - result;
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 두 2D 벡터 사이의 각도를 반환한다.

 * \param v1	1번 벡터
 * \param v2	2번 벡터
 * \return		사이 각(라디안)
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT 
GetAngleBetweenVectors( CONST D3DXVECTOR2* v1, CONST D3DXVECTOR2* v2 )
{
	D3DXVECTOR2 temp, vec1, vec2;
	D3DXVec2Normalize( &vec1, v1 );
	D3DXVec2Normalize( &vec2, v2 );

	temp.x = v1->x * v2->x + v1->y * v2->y;
	temp.y = v2->x * v1->y - v2->y * v1->x;

	return atanf( temp.y / temp.x );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief NxVec3를 D3DXVECTOR3로 변환

 * \param v		NxVec3
 * \return		D3DXVECTOR3
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3
NxVectorToD3DVector( NxVec3 v )
{
	return D3DXVECTOR3( v.x, v.y, v.z );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief D3DXVECTOR3를 NxVec3로 변환
 
 * \param v		D3DXVECTOR3
 * \return		NxVec3
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NxVec3
D3DXVec3ToNxVec3( D3DXVECTOR3 v )
{
	return NxVec3( v.x, v.y, v.z );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief NxMat34 행렬을 D3DXMATRIX로 변환

 * \param *inMatr	NxMat34 행렬 포인터
 * \param *outMatr	D3DXMATRIX 행렬 포인터
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
NxMat34ToD3DMATRIX( NxMat34 *inMatr, D3DXMATRIX *outMatr )
{
	outMatr->_11 = inMatr->M(0, 0);
	outMatr->_12 = inMatr->M(0, 1);
	outMatr->_13 = inMatr->M(0, 2);
	outMatr->_21 = inMatr->M(1, 0);
	outMatr->_22 = inMatr->M(1, 1);
	outMatr->_23 = inMatr->M(1, 2);
	outMatr->_31 = inMatr->M(2, 0);
	outMatr->_32 = inMatr->M(2, 1);
	outMatr->_33 = inMatr->M(2, 2);
	outMatr->_14 = inMatr->t[0];
	outMatr->_24 = inMatr->t[1];
	outMatr->_34 = inMatr->t[2];
	outMatr->_41 = 0;
	outMatr->_42 = 0;
	outMatr->_43 = 0;
	outMatr->_44 = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief D3DXMATRIX 행렬을 NxMat34로 변환

 * \param *inMatr	D3DXMATRIX 포인터
 * \param *outMatr	NxMat34 포인터
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
D3DMATRIXToNxMat34( D3DXMATRIX *inMatr, NxMat34 *outMatr )
{
	outMatr->id();
	for( int i=0; i < 3; i++ ) outMatr->M.setRow( i, NxVec3(inMatr->m[i][0], inMatr->m[i][1], inMatr->m[i][2]) );
	outMatr->t = NxVec3( inMatr->_41, inMatr->_42, inMatr->_43 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 두 3D 벡터 사이의 각도를 반환한다

 * \param v1	벡터 1
 * \param v2	벡터 2	
 * \return		사이 각(라디안)
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT
GetAngleBetweenVectors( D3DXVECTOR3 v1, D3DXVECTOR3 v2 )
{
	D3DXVec3Normalize( &v1, &v1 );
	D3DXVec3Normalize( &v2, &v2 );

	return ((FLOAT)acos( D3DXVec3Dot(&v1, &v2) ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief double 값을 일정 자리 이하를 버림하여 float으로 반환한다

 이 함수는 double과 혼용을 하게 될 경우 float 정밀도 부족으로 형변환시 반올림이 되게 되는데 그것을 막고 아예 소수점 이하 n자리의 값을 버려 버린다
 현재는 쓰이는 곳은 없으나 메시 관련 프로세싱을 하다 보면 반드시 정밀도 문제에 부딪치게 된다. high precision 쪽을 알아보는게 좋다.

 * \param x		double 값
 * \return		버림한 float 값
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FLOAT 
DoubleFloor( double x )
{
	char str[255];
	sprintf( str, "%3.20f", x );

	int index;
	for( int i=0; i < 255; i++ ) 
	{
		if( str[i] == '.' ) index = i;
		if( i > index + 6 ) str[i] = '0';
	}
	return (FLOAT)atof(str);
}

/*************************************************************************************************************************************************************/
/* 보간 함수들. 키프레임 애니메이션 시에 변수 타입에 따라 보간 함수들을 여기에 마련해 둔다.																	*/
/*************************************************************************************************************************************************************/

/// 주어진 두 FLOAT 값을 보간한 값을 반환
FLOAT
GetInterpolatedFLOAT( FLOAT t, FLOAT f1, FLOAT f2 )
{
	return (1.f - t) * f1 + t * f2;
}

/// 주어진 두 D3DXVECTOR3 값을 보간한 값을 반환
D3DXVECTOR3 
GetInterpolatedD3DVec3( FLOAT t, D3DXVECTOR3 v1, D3DXVECTOR3 v2 )
{
	if( v1 == v2 ) t = 0;
	return D3DXVECTOR3( (1.f - t) * v1 + t * v2 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 주어진 두 D3DXMATRIX 값을 보간한 값을 반환

 이 함수는 D3DXMATRIX를 보간한다. 보간 방법은, 두 행렬의 쿼터니온을 구하고 quaternionSlerp 함수를 사용하여 보간 쿼터니온을 구한다.
 구한 쿼터니온은 두 행렬의 회전 값의 보간 값을 갖는다. 이것은 회전에 대한 값만 나타내므로 translation 도 따로 보간해준다.
 
 회전 및 이동 보간값을 얻은 뒤에 아핀 변환 행렬 함수로 행렬을 만들면 보간 완료

 * \param t		보간 값
 * \param mat1	행렬1 포인터
 * \param mat2	행렬2 포인터
 * \return		보간한 행렬
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXMATRIX 
GetInterpolatedD3DXMATRIX( FLOAT t, D3DXMATRIX* mat1, D3DXMATRIX* mat2 )
{
	D3DXMATRIX		mat;	
	D3DXQUATERNION	q1, q2, q3;
	D3DXVECTOR3		pos1, pos2, posDiff;

	D3DXQuaternionIdentity( &q1 );			
	D3DXQuaternionIdentity( &q2 );			
	D3DXQuaternionRotationMatrix( &q1, mat1 );
	D3DXQuaternionRotationMatrix( &q2, mat2 );

	D3DXQuaternionSlerp( &q3, &q1, &q2, t );
	
	pos1 = D3DXVECTOR3( mat1->_14, mat1->_24, mat1->_34 );
	pos2 = D3DXVECTOR3( mat2->_14, mat2->_24, mat2->_34 );

	posDiff = pos2 - pos1;
	posDiff *= t;

	D3DXMatrixAffineTransformation( &mat, 1, &D3DXVECTOR3(0,0,0), &q3, &D3DXVECTOR3(pos1 + posDiff) ); 
	return mat;
}

/// 주어진 두 Nx 벡터 값을 보간한 값을 반환
NxVec3				
GetInterpolatedNxVec3( FLOAT t, NxVec3 v1, NxVec3 v2 )
{	
	D3DXVECTOR3 dv1, dv2;
	dv1 = NxVectorToD3DVector( v1 );
	dv2 = NxVectorToD3DVector( v2 );
	return D3DXVec3ToNxVec3( GetInterpolatedD3DVec3(t, dv1, dv2) );
}

/// 주어진 두 Nx 행렬 값을 보간한 값을 반환
NxMat34 GetInterpolatedNxMat34( FLOAT t, NxMat34* mat1, NxMat34* mat2 )
{
	NxMat34 mat;
	D3DXMATRIX dmat1, dmat2;
	NxMat34ToD3DMATRIX( mat1, &dmat1 );
	NxMat34ToD3DMATRIX( mat2, &dmat2 );
	D3DMATRIXToNxMat34( &GetInterpolatedD3DXMATRIX(t, &dmat1, &dmat2), &mat );
	return mat;
}