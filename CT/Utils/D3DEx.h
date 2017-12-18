#pragma once

#undef min
#undef max
#include "NxPhysics.h"

#define PI 3.141592f

/*************************************************************************************************************************************************************/
/* 벡터, 행렬 등의 계산을 돕는 함수들. Direct3D 내장 함수가 없으면서 자주 쓸 함수들을 이곳에 작성															 */
/*************************************************************************************************************************************************************/

/// 3D 벡터로부터 2D 프로젝션된 스크린 좌표를 얻는 함수.
D3DXVECTOR3			GetProjectedPoint( D3DXVECTOR3& point );

/// 마우스 좌표로부터 unprojection을 위한 ray와 dir 벡터를 얻는 함수
VOID				GetUnprojectedPoint( CPoint point, D3DXVECTOR3 *vPickRayDir, D3DXVECTOR3 *vPickRayOrig );

/// 두 벡터 사이의 라디안 각도를 반환하는 함수
FLOAT				GetAngleBetweenVectors( D3DXVECTOR3 v1, D3DXVECTOR3 v2 );

/// 두 2D 벡터 사이의 라디안 각도를 반환하는 함수
FLOAT				GetAngleBetweenVectors( CONST D3DXVECTOR2* v1, CONST D3DXVECTOR2* v2 );

/// 벡터를 지정한 축 벡터에 대해 지정한 라디안 각도 만큼 회전시키는 함수
D3DXVECTOR3			RotateVectorByAxis( D3DXVECTOR3* vOut, D3DXVECTOR3* vIn, D3DXVECTOR3 vAxis, FLOAT angle );

/// 피직스 NxVec3를 D3DXVECTOR3로 변환하는 함수
D3DXVECTOR3			NxVectorToD3DVector( NxVec3 v );

/// D3DXVECTOR3를 피직스 NxVec3로 변환하는 함수
NxVec3				D3DXVec3ToNxVec3( D3DXVECTOR3 v );

/// 피직스 NxMat34 행렬을 D3DXMATRIX로 변환하는 함수
VOID				NxMat34ToD3DMATRIX( NxMat34 *inMatr, D3DXMATRIX *outMatr );

/// D3DXMATRIX를 피직스 NxMat34 행렬로  변환하는 함수
VOID				D3DMATRIXToNxMat34( D3DXMATRIX *inMatr, NxMat34 *outMatr );

/// Double 값을 버림하여 FLOAT 값으로 반환하는 함수
FLOAT				DoubleFloor( double x );

/*************************************************************************************************************************************************************/
/* 보간 함수들. 키프레임 애니메이션 시에 변수 타입에 따라 보간 함수들을 여기에 마련해 둔다.																	*/
/*************************************************************************************************************************************************************/

/// 주어진 두 FLOAT 값을 보간한 값을 반환
FLOAT				GetInterpolatedFLOAT( FLOAT t, FLOAT f1, FLOAT f2 );

/// 주어진 두 벡터 값을 보간한 값을 반환
D3DXVECTOR3			GetInterpolatedD3DVec3( FLOAT t, D3DXVECTOR3 v1, D3DXVECTOR3 v2 );

/// 주어진 두 D3D 행렬 값을 보간한 값을 반환
D3DXMATRIX			GetInterpolatedD3DXMATRIX( FLOAT t, D3DXMATRIX* mat1, D3DXMATRIX* mat2 );

/// 주어진 두 Nx 벡터 값을 보간한 값을 반환
NxVec3				GetInterpolatedNxVec3( FLOAT t, NxVec3 v1, NxVec3 v2 );

/// 주어진 두 Nx 행렬 값을 보간한 값을 반환
NxMat34				GetInterpolatedNxMat34( FLOAT t, NxMat34* mat1, NxMat34* mat2 );