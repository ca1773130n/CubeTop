#pragma once

#undef min
#undef max
#include "NxPhysics.h"

#define PI 3.141592f

/*************************************************************************************************************************************************************/
/* ����, ��� ���� ����� ���� �Լ���. Direct3D ���� �Լ��� �����鼭 ���� �� �Լ����� �̰��� �ۼ�															 */
/*************************************************************************************************************************************************************/

/// 3D ���ͷκ��� 2D �������ǵ� ��ũ�� ��ǥ�� ��� �Լ�.
D3DXVECTOR3			GetProjectedPoint( D3DXVECTOR3& point );

/// ���콺 ��ǥ�κ��� unprojection�� ���� ray�� dir ���͸� ��� �Լ�
VOID				GetUnprojectedPoint( CPoint point, D3DXVECTOR3 *vPickRayDir, D3DXVECTOR3 *vPickRayOrig );

/// �� ���� ������ ���� ������ ��ȯ�ϴ� �Լ�
FLOAT				GetAngleBetweenVectors( D3DXVECTOR3 v1, D3DXVECTOR3 v2 );

/// �� 2D ���� ������ ���� ������ ��ȯ�ϴ� �Լ�
FLOAT				GetAngleBetweenVectors( CONST D3DXVECTOR2* v1, CONST D3DXVECTOR2* v2 );

/// ���͸� ������ �� ���Ϳ� ���� ������ ���� ���� ��ŭ ȸ����Ű�� �Լ�
D3DXVECTOR3			RotateVectorByAxis( D3DXVECTOR3* vOut, D3DXVECTOR3* vIn, D3DXVECTOR3 vAxis, FLOAT angle );

/// ������ NxVec3�� D3DXVECTOR3�� ��ȯ�ϴ� �Լ�
D3DXVECTOR3			NxVectorToD3DVector( NxVec3 v );

/// D3DXVECTOR3�� ������ NxVec3�� ��ȯ�ϴ� �Լ�
NxVec3				D3DXVec3ToNxVec3( D3DXVECTOR3 v );

/// ������ NxMat34 ����� D3DXMATRIX�� ��ȯ�ϴ� �Լ�
VOID				NxMat34ToD3DMATRIX( NxMat34 *inMatr, D3DXMATRIX *outMatr );

/// D3DXMATRIX�� ������ NxMat34 ��ķ�  ��ȯ�ϴ� �Լ�
VOID				D3DMATRIXToNxMat34( D3DXMATRIX *inMatr, NxMat34 *outMatr );

/// Double ���� �����Ͽ� FLOAT ������ ��ȯ�ϴ� �Լ�
FLOAT				DoubleFloor( double x );

/*************************************************************************************************************************************************************/
/* ���� �Լ���. Ű������ �ִϸ��̼� �ÿ� ���� Ÿ�Կ� ���� ���� �Լ����� ���⿡ ������ �д�.																	*/
/*************************************************************************************************************************************************************/

/// �־��� �� FLOAT ���� ������ ���� ��ȯ
FLOAT				GetInterpolatedFLOAT( FLOAT t, FLOAT f1, FLOAT f2 );

/// �־��� �� ���� ���� ������ ���� ��ȯ
D3DXVECTOR3			GetInterpolatedD3DVec3( FLOAT t, D3DXVECTOR3 v1, D3DXVECTOR3 v2 );

/// �־��� �� D3D ��� ���� ������ ���� ��ȯ
D3DXMATRIX			GetInterpolatedD3DXMATRIX( FLOAT t, D3DXMATRIX* mat1, D3DXMATRIX* mat2 );

/// �־��� �� Nx ���� ���� ������ ���� ��ȯ
NxVec3				GetInterpolatedNxVec3( FLOAT t, NxVec3 v1, NxVec3 v2 );

/// �־��� �� Nx ��� ���� ������ ���� ��ȯ
NxMat34				GetInterpolatedNxMat34( FLOAT t, NxMat34* mat1, NxMat34* mat2 );