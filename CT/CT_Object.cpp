#include "stdafx.h"
#include "CT_Object.h"

#include "CT_PhysX.h"
#include "CommonUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 기본 오브젝트 클래스
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL( CtObjectBase, CObject, 1 )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 생성자

 객체 생성시 초기 값들을 지정한다. 

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectBase::CtObjectBase( VOID )
{	
	m_iStateFlag		= 0;

	m_bSleep			= FALSE;
	m_pActor			= 0;
	m_pActorMesh		= 0;
	m_vScale			= D3DXVECTOR3( 1, 1, 1 );
	m_vScaleTemp		= D3DXVECTOR3( 1, 1, 1 );
	
	m_fTransparency		= 0;
	m_iNumMaterials		= 0;
	m_pMeshMaterials	= 0;
	m_pMeshTextures		= 0;

	m_bHasCustomMesh	= FALSE;	

	m_pDXUTMesh			= 0;
	
	m_AnimInfo.Disable();

	m_bHasMorphMesh		= FALSE;

	m_pMeshVB			= NULL;
	m_pMeshIB			= NULL;

	m_iCurPoseIndex		= -1;

	m_matPose.id();
	m_matPoseTemp.id();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 소멸자

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectBase::~CtObjectBase( VOID )
{
	if( m_pActor ) 
	{
		NxShape **shape = (NxShape**)m_pActor->getShapes();
		for( int i=0; i < m_pActor->getNbShapes(); i++ ) m_pActor->releaseShape( *shape[i] );
	}
	if( m_bHasMorphMesh ) delete m_pMorphObj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 직렬화 함수

 객체의 속성들을 파일에 직렬화 한다

 * \param ar	CArchive 객체
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::Serialize( CArchive& ar )
{
	CObject::Serialize( ar );

	NxMat34 matPose;

	// 저장하는 경우
	if( ar.IsStoring() )
	{
		// 직렬화된 객체를 유일하게 식별할 수 있는 파일 경로와 파일명을 먼저 넣는다
		ar << m_stFileInfo.lpszFilePath << m_stFileInfo.lpszFileName;

		matPose = m_pActor->getGlobalPose();
		for( int i=0; i < 3; i++ )
			ar << matPose.M.getRow( i ).x << matPose.M.getRow( i ).y << matPose.M.getRow( i ).z;

		// 빼내는 순서와 동일하게 넣는다
		ar << matPose.t.x << matPose.t.y << matPose.t.z;
	}
	// 로드하는 경우
	else
	{
		NxVec3 temp;		
		for( int i=0; i < 3; i++ )
		{
			ar >> temp.x >> temp.y >> temp.z;
			matPose.M.setRow( i, temp );
		}		
		ar >> matPose.t.x >> matPose.t.y >> matPose.t.z;
		
		SetPoseTransition( matPose, m_vScale, 0.3f, 1.5f );
	}
}

VOID
CtObjectBase::DoMethod( CtSelectionMethod method, va_list pArgs )
{
	switch( method )
	{
	case SBM_HIDE:
		{
			Hide();
			break;
		}
	case SBM_SHOW:
		{
			Show();
			break;
		}
	case SBM_MOVE:
		{
			Move( va_arg(pArgs, NxVec3) );
			break;
		}
	case SBM_ROTLX:
		{			
			RotateLocalX( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_ROTLY:
		{
			RotateGlobalY( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_ROTLZ:
		{
			RotateGlobalZ( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_ROTGX:
		{
			RotateGlobalX( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_ROTGY:
		{
			RotateGlobalY( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_ROTGZ:
		{
			RotateGlobalZ( va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_SCALE:
		{
			Scale();
			break;
		}
	case SBM_SCALEOFFSET:
		{
			Scale( va_arg(pArgs, DOUBLE), va_arg(pArgs, DOUBLE), va_arg(pArgs, DOUBLE) );
			break;
		}
	case SBM_BILLBOARD:
		{
			Billboard( va_arg(pArgs, D3DXMATRIX*) );
			break;
		}
	case SBM_SAVEPOSE:
		{
			SavePoseScale();
			break;
		}
	case SBM_LOADPOSE:
		{
			LoadPoseScale();
			break;
		}
	case SBM_IMPORTMESH:
		{
			ImportMesh( va_arg(pArgs, CString) );
			break;
		}
	case SBM_REWIND:
		{
			Rewind();
			break;
		}
	case SBM_FOWARD:
		{
			Foward();
			break;
		}
	case SBM_LIFT:
		{
			Lift();
			break;
		}
	case SBM_UNLIFT:
		{
			UnLift();
			break;
		}
	case SBM_FREEZE:
		{
			Freeze();
			break;
		}
	case SBM_UNFREEZE:
		{
			UnFreeze();
			break;
		}
	case SBM_SETPOSE:
		{
			SetPoseTransition( va_arg(pArgs, NxMat34), va_arg(pArgs, D3DXVECTOR3), va_arg(pArgs, FLOAT), va_arg(pArgs, FLOAT) );
			break;
		}
	case SBM_SETPOSEBACK:
		{
			SetCollisionGroup( ACTOR_GROUP_DEFAULT );
			Rewind();
			break;
		}
	case SBM_SELECT:
		{
			Select();
			break;
		}
	case SBM_UNSELECT:
		{
			UnSelect();
			break;
		}
	case SBM_RAISESF:
		{
			RaiseStateFlag( va_arg(pArgs, ULONG) );
			break;
		}
	case SBM_CLEARSF:
		{
			ClearStateFlag( va_arg(pArgs, ULONG) );
			break;
		}
	case SBM_RAISEAF:
		{
			m_pActor->raiseActorFlag( va_arg(pArgs, NxActorFlag) );
			break;
		}
	case SBM_CLEARAF:
		{
			m_pActor->clearActorFlag( va_arg(pArgs, NxActorFlag) );
			break;
		}
	case SBM_DELETE:
		{
			SHFILEOPSTRUCT sfos;
			sfos.hwnd = AfxGetApp()->GetMainWnd()->m_hWnd;
			sfos.wFunc = FO_DELETE;
			CString fullpath = this->GetFilePath();
			sfos.pFrom = fullpath;
			sfos.pTo=NULL;
			sfos.fFlags = FOF_ALLOWUNDO ;
			sfos.fAnyOperationsAborted = FALSE;
			sfos.hNameMappings = NULL ;
			sfos.lpszProgressTitle = L"파일 삭제 중";
			int a = SHFileOperation(&sfos);

			RaiseStateFlag( OBJECT_STATE_DELETED );
			break;
		}
	}
}

VOID	
CtObjectBase::Move( NxVec3 vOffset )
{

}

VOID
CtObjectBase::RotateLocalX( float fAmount )
{

}

VOID
CtObjectBase::RotateLocalY( float fAmount )
{

}

VOID		
CtObjectBase::RotateLocalZ( float fAmount )
{

}

VOID		
CtObjectBase::RotateGlobalX( float fAmount )
{
	NxMat33 pose = m_pActor->getGlobalOrientation();		
	NxMat33 matRotate;
	matRotate.id();
	matRotate.rotX( fAmount );
	m_pActor->setGlobalOrientation( matRotate * pose );		
}

VOID		
CtObjectBase::RotateGlobalY( float fAmount )
{
	NxMat33 pose = m_pActor->getGlobalOrientation();		
	NxMat33 matRotate;
	matRotate.id();
	matRotate.rotY( fAmount );
	m_pActor->setGlobalOrientation( matRotate * pose );		
}

VOID		
CtObjectBase::RotateGlobalZ( float fAmount )
{
	NxMat33 pose = m_pActor->getGlobalOrientation();		
	NxMat33 matRotate;
	matRotate.id();
	matRotate.rotZ( fAmount );
	m_pActor->setGlobalOrientation( matRotate * pose );		
}

VOID
CtObjectBase::ResetMass( FLOAT fDensity )
{
	FLOAT sizeX, sizeY, sizeZ;
	sizeX = m_vActorSize.x * m_vScale.x;
	sizeY = m_vActorSize.y * m_vScale.y;
	sizeZ = m_vActorSize.z * m_vScale.z;
	m_pActor->setMass( fDensity * sqrtf(D3DXVec3Length(&m_vActorSize)) );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 스케일링 함수

 인자가 없는 스케일 함수는 객체가 가진 스케일 및 액터 사이즈 벡터를 이용하여 객체의 액터 크기를 업데이트한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::Scale( VOID )
{
	NxBoxShapeDesc boxDesc;

	FLOAT sizeX, sizeY, sizeZ;
	sizeX = m_vActorSize.x * m_vScale.x;
	sizeY = m_vActorSize.y * m_vScale.y;
	sizeZ = m_vActorSize.z * m_vScale.z;

	// 객체 종류에 따라 크기를 다시 계산
	// 메쉬의 경우 버텍스마다 uniform 스케일링 적용할 것
	if( m_bHasCustomMesh == FALSE )
	{
		boxDesc.dimensions = NxVec3( sizeX / 2.f, sizeY / 2.f, sizeZ / 2.f );

		// 기존 액터를 지우고 변경된 크기의 액터를 다시 생성
		NxShape **shape = (NxShape**)m_pActor->getShapes();
		m_pActor->releaseShape( *shape[0] );
		m_pActor->createShape( boxDesc );
		//m_pActor->updateMassFromShapes( 1, sizeX * sizeY * sizeZ );
	}

	ResetMass( 1 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 스케일링 함수
 * \param fOffsetX	X축에 대한 스케일 오프셋 값
 * \param fOffsetY	Y축에 대한 스케일 오프셋 값
 * \param fOffsetZ	Z축에 대한 스케일 오프셋 값
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::Scale( DOUBLE fOffsetX, DOUBLE fOffsetY, DOUBLE fOffsetZ )
{	
	m_vScale.x *= fOffsetX;
	m_vScale.y *= fOffsetY;
	m_vScale.z *= fOffsetZ;

	Scale();	
}

/*************************************************************************************************************************************************************/
/* 객체 속성 변경 관련 함수들                                                                                                                                */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체를 띄우기 상태로 만드는 함수

 현재는 플래그 값만 변화시키나, 차후에 상태 변화시에 추가로 수행할 작업이 있으면 여기에 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::Lift( VOID )
{
	m_iStateFlag |= OBJECT_STATE_LIFTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 띄우기 상태를 해제하는 함수

 현재는 플래그 값만 변화시키나, 차후에 상태 변화시에 추가로 수행할 작업이 있으면 여기에 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::UnLift( VOID )
{
	m_iStateFlag &= ~OBJECT_STATE_LIFTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체를 롤오버 상태로 만드는 함수

 현재는 플래그 값만 변화시키나, 차후에 상태 변화시에 추가로 수행할 작업이 있으면 여기에 작성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::Hover( VOID )
{
	m_iStateFlag |= OBJECT_STATE_HOVERED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 롤오버 상태를 해제하는 함수
 
 현재는 플래그 값만 변화시키나, 차후에 상태 변화시에 추가로 수행할 작업이 있으면 여기에 작성한다.
 
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::UnHover( VOID )
{
	m_iStateFlag &= ~OBJECT_STATE_HOVERED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체를 고정 상태로 만드는 함수
 
 객체의 상태를 바꾸고 액터를 Freeze시킨다. 
 
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::Freeze( VOID )
{
	m_iStateFlag |= OBJECT_STATE_FREEZED;
	m_pActor->raiseBodyFlag( NX_BF_FROZEN );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 고정 상태를 해제하는 함수
 
 객체의 상태를 바꾸고 액터를 UnFreeze 시킨다.
 
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID	
CtObjectBase::UnFreeze( VOID )
{
	m_iStateFlag &= ~OBJECT_STATE_FREEZED;
	m_pActor->clearBodyFlag( NX_BF_FROZEN );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 선택 함수

 객체를 선택할 때 수행해야 할 작업들을 작성한다. 현재는 객체 상태를 선택상태로 바꾸고 액터 그룹을 선택물 액터 그룹으로 지정한다.
 * \param VOID 
 * \return 
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::Select( VOID )
{
	m_iStateFlag |= OBJECT_STATE_SELECTED;

	if ( m_pActor ) {
		int numShape = m_pActor->getNbShapes();
		NxShape *const* pShape = m_pActor->getShapes();

		// 액터의 모든 shape의 그룹을 selected로 지정
		for (int i = 0; i < numShape; i++)
			pShape[i]->setGroup(ACTOR_GROUP_SELECTED);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 선택 해제 함수
 
 객체를 선택 해제할 때 수행해야 할 작업들을 작성한다. 현재는 객체 상태를 선택상태로 바꾸고 액터 그룹을 기본 액터 그룹으로 지정한다.

 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID		
CtObjectBase::UnSelect( VOID )
{
	m_iStateFlag &= ~OBJECT_STATE_SELECTED;

	if ( m_pActor ) {
		int numShape = m_pActor->getNbShapes();
		NxShape *const* pShape = m_pActor->getShapes();

		// 액터의 모든 shape들의 그룹을 default로 지정
		for (int i = 0; i < numShape; i++)
			pShape[i]->setGroup(ACTOR_GROUP_DEFAULT);
	}
}

VOID 
CtObjectBase::Billboard( D3DXMATRIX* pBillboardMatrix )
{
	NxMat34 nxmatBill;
	D3DMATRIXToNxMat34( pBillboardMatrix, &nxmatBill );
}

VOID CtObjectBase::BillboardPosScale( D3DXMATRIX* pBillboardMatrix, NxVec3 vTgtPos, FLOAT fScaleFactor )
{
	NxMat34 nxmatBill;
	D3DMATRIXToNxMat34( pBillboardMatrix, &nxmatBill );
	nxmatBill.t = vTgtPos;

	m_AnimInfo.AddKey( 0, 0, CtObjectKeyData(m_matPose, m_vScale) );
	m_AnimInfo.AddKey( 30, 0, CtObjectKeyData(nxmatBill, m_vScale * fScaleFactor) );

	m_AnimInfo.Enable();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 포즈 및 스케일에 대한 키프레임 애니메이션을 설정한다

 * \see CtAnimInfo 클래스 참조

 * \param matPose	트랜지션할 포즈 행렬
 * \param vScale	트랜지션할 스케일 벡터
 * \param fBias		쏠림 값
 * \param fTime		트랜지션이 완료되기까지의 시간
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::SetPoseTransition( NxMat34 matPose, D3DXVECTOR3 vScale, FLOAT fBias, FLOAT fTime )
{
	m_AnimInfo.Enable();

	// 셋팅된 키프레임이 없는 경우 현재 포즈의 키를 추가함
	if( m_AnimInfo.GetNumKey() == 0 && m_pActor) 
		m_AnimInfo.AddKey( 0, 0, CtObjectKeyData(m_pActor->getGlobalPose(), m_vScale) );

	m_AnimInfo.AddKey( (UINT)(fTime * 60), fBias, CtObjectKeyData(matPose, vScale) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 포즈 및 스케일 벡터를 임시값에 저장하는 함수

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID				
CtObjectBase::SavePoseScale( VOID )
{
	m_matPoseTemp = m_matPose;
	m_vScaleTemp = m_vScale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 포즈 및 스케일 벡터를 임시값으로부터 로드하는 함수
 
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::LoadPoseScale( VOID )
{
	m_matPose = m_matPoseTemp;
	m_vScale = m_vScale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 메시 임포트 함수

 주어진 파일명으로부터 객체를 표현할 메시를 로드한다

 * \param filepath	메시 파일의 전체 경로
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		
CtObjectBase::ImportMesh( LPCWSTR filepath )
{
	HRESULT hr;
	m_pDXUTMesh = new CDXUTMesh();
	V_RETURN( m_pDXUTMesh->Create( DXUTGetD3DDevice(), filepath ) );

	// 모델 크기 노멀라이즈
	MESHVERTEX* pVertices = NULL;
	WORD* pIndices = NULL;

	DWORD numV = m_pDXUTMesh->m_dwNumVertices;
	DWORD numF = m_pDXUTMesh->m_dwNumFaces;

	m_pDXUTMesh->m_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (LPVOID*)&pVertices );
	m_pDXUTMesh->m_pMesh->LockIndexBuffer( D3DLOCK_READONLY, (LPVOID*)&pIndices );
	
	D3DXVECTOR3 maxV;
	FLOAT curD, maxD = 0;
	for( DWORD i=0; i < numV; i++ )
	{
		if( (curD = D3DXVec3Length(&pVertices[i].pos)) > maxD ) 
		{
			maxD = curD;
		}
	}

	for( DWORD i=0; i < numV; i++ )
		pVertices[i].pos /= maxD;

	m_pDXUTMesh->m_pMesh->UnlockVertexBuffer();
	m_pDXUTMesh->m_pMesh->UnlockIndexBuffer();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** 
 * \brief 객체의 이름 텍스쳐를 동적으로 생성하는 함수.
 
 객체의 Display name으로부터 이름 텍스쳐를 생성하는 함수이다. 칩의 경우 객체의 한쪽 옆면에, 큐브의 경우 6면에 표시될 텍스쳐이다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CtObjectBase::GenerateNameTexture( VOID )
{
	HRESULT hr;

	int				tWidth, tHeight;	
	RectF			R;
	REAL			fontSize;
	StringFormat	drawFormat;
	Color			textColor;

	LPDIRECT3DTEXTURE9* ppTexture = &m_pMeshTextures[1];

	WCHAR bmpfilepath[255];
	WCHAR currentDir[255];
	GetCurrentDirectory( 255, currentDir );
	
	switch( m_enType )
	{
	case OBJECT_TYPE_CHIP:
		{
			tWidth = CHIP_SIDE_TEXTURE_WIDTH;
			tHeight = CHIP_SIDE_TEXTURE_HEIGHT;
			drawFormat.SetLineAlignment( StringAlignmentNear );
			textColor.SetValue( CHIP_SIDE_TEXTURE_COLOR );
			fontSize = 14;
			break;
		}
	case OBJECT_TYPE_CUBE:
		{
			tWidth = CUBE_SIDE_TEXTURE_WIDTH;
			tHeight = CUBE_SIDE_TEXTURE_HEIGHT;
			drawFormat.SetLineAlignment( StringAlignmentCenter );						
			drawFormat.SetAlignment( StringAlignmentCenter );			
			textColor.SetValue( 0xffffffff );
			fontSize = 20;
		}
	}
	R.Width = tWidth;
	R.Height = tHeight;
	wsprintf( bmpfilepath, L"%s\\%s.bmp", currentDir, m_stFileInfo.lpszFileName );

	/************************************************************************/
	/* 메모리에 옆면 텍스쳐에 사용할 텍스트를 그린 뒤 BMP 파일로 저장한다   */
	/************************************************************************/
	BYTE *pBits = new BYTE[tWidth * tHeight * 4];
	Bitmap bmp(tWidth, tHeight, tWidth * 4, PixelFormat32bppARGB, pBits);
	Graphics grp(&bmp);
	grp.Clear(Color(0,0,0,0));

	// 텍스트 출력
	FontFamily FM( DEFAULT_SIDENAME_FONT );
	if ( !FM.IsAvailable() ) {
		MessageBox( 0, CString(L"GDI : 폰트 사용 불가.") + CString(DEFAULT_SIDENAME_FONT) + CString(L" 폰트를 설치해 주십시오."), 0, 0 );		
		exit(0);
	}

	Font font( &FM, fontSize );		
	SolidBrush black( textColor );		
	grp.DrawString( m_stFileInfo.lpszDisplayName, -1, &font, R, &drawFormat, &black );

	BitmapToFile( &bmp, bmpfilepath );

	/************************************************************************/
	/* 저장한 BMP 파일을 읽어 옆면 텍스쳐 생성                              */
	/************************************************************************/
	V( DXUTGetGlobalResourceCache().CreateTextureFromFileEx(DXUTGetD3DDevice(), bmpfilepath, tWidth, tHeight, 0, 0, D3DFMT_DXT5, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0, ppTexture) );

	delete[] pBits;
	DeleteFile( bmpfilepath );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체의 액터가 sleep 상태로 들어갈 때의 핸들러 함수

 객체 포즈의 undo / redo 시, 객체의 포즈 리스트를 참조하여 이전 또는 다음 포즈로 변환하게 된다.
 이를 위해 객체들은 고정 크기의 객체 리스트(큐)를 갖는데, 이 리스트에는 객체가 sleep 될때마다 포즈가 push 된다.
 즉 객체가 완전히 멈춘 상태의 포즈들만 저장하는 것이다. 
 
 * \note 
 
 SetGlobalPose() 함수가 호출되어도 해당 함수 내부에서 PutToSleep()을 호출하여 본 핸들러가 호출되게 되는데, 
 포즈 undo / redo 시에도 SetGlobalPose() 를 호출하므로 이때에는 포즈의 저장을 제외하여 undo / redo 중간에 포즈 리스트가 변화하는 것을 막는다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::OnSleep( VOID )
{
	// undo 또는 redo 중이면 해당 상태를 해제하고 리턴
	// undo 및 redo의 포즈 트랜지션이 완료된 경우임
	if( NowOnRewindFoward() ) 
	{
		m_iStateFlag &= ~OBJECT_STATE_REWINDFOWARD;
		return;
	}

	// 아닌 경우 객체의 글로벌 포즈를 포즈 리스트에 저장
	size_t qSize = m_matPoses.size();
	NxMat34 matPose = m_pActor->getGlobalPose();	

	// 리스트의 맨 마지막인 경우(일반적인 경우와 redo가 끝에 다다른 경우) 리스트 뒤에 추가
	if( m_iCurPoseIndex++ == qSize - 1 )
	{
		m_matPoses.push_back( matPose );
		m_vScales.push_back( m_vScale );
		if( qSize == UNDO_LIMIT ) 
		{
			// 제한 크기를 넘긴 경우 앞쪽의 노드를 pop 시켜 리스트 사이즈를 유지
			m_matPoses.pop_front();
			m_vScales.pop_front();
			m_iCurPoseIndex--;
		}
	}
	// 아닌 경우(undo, redo 중에 포즈를 변화시킨 경우) 리스트의 현재 노드의 포즈 값을 현재 포즈로 지정(덮어씌움)
	else 
	{
		m_matPoses[m_iCurPoseIndex] = matPose;
		m_vScales[m_iCurPoseIndex] = m_vScale;
	}

	ClearStateFlag( OBJECT_STATE_FORCED );
	m_matPoseTemp = matPose;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 포즈 undo 함수

 객체의 포즈 리스트의 현재 노드의 앞 노드로 포즈를 복귀시켜 포즈 undo를 수행한다. 포즈 변환시에 키프레임 애니메이션을 사용

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::Rewind( VOID )
{	
	if( m_iCurPoseIndex > 0 && m_matPoses.size() > 0 ) 
	{
		m_iCurPoseIndex--;
		m_iStateFlag |= OBJECT_STATE_REWINDFOWARD;
		SetPoseTransition( m_matPoses.at(m_iCurPoseIndex), m_vScales.at(m_iCurPoseIndex), 0.3f, 1.f );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 객체 포즈 redo 함수
 
 객체의 포즈 리스트의 현재 노드의 다음 노드로 포즈를 복귀시켜 포즈 redo를 수행한다. 포즈 변환시에 키프레임 애니메이션을 사용
 
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectBase::Foward( VOID )
{
	if( m_iCurPoseIndex < m_matPoses.size() - 1 ) 
	{
		m_iCurPoseIndex++;
		m_iStateFlag |= OBJECT_STATE_REWINDFOWARD;
		SetPoseTransition( m_matPoses.at(m_iCurPoseIndex), m_vScales.at(m_iCurPoseIndex), 0.3f, 1.f );
	}
}

VOID 
CtObjectBase::SetCollisionGroup( NxCollisionGroup group )
{
	NxShape *const* pShapes = m_pActor->getShapes();
	for( UINT i=0; i < m_pActor->getNbShapes(); i++ )
		pShapes[i]->setGroup( group );
}
	
/*************************************************************************************************************************************************************/
/* 칩 클래스(일반 파일)                                                                                                                                      */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 기본 생성자


 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectChip::CtObjectChip( VOID )
{	
	m_bBoundaryOfPile	= FALSE;
	m_enType			= OBJECT_TYPE_CHIP;
	m_pParentCube		= NULL;
	m_pPrevObj			= NULL;
	m_pNextObj			= NULL;
	m_pPileJoint1		= 0;
	m_pPileJoint2		= 0;
	m_pHangJoint		= 0;
	m_iPileOrder		= 0;
	m_vAnchor			= D3DXVECTOR3( 0, 0, 0 );
	m_pMeshTextures		= new LPDIRECT3DTEXTURE9[2];	
	m_pMeshTextures[0]	= NULL;
	m_pMeshTextures[1]	= NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 파일 정보로부터 생성하는 생성자

 * \param lpszFileName		파일명
 * \param lpszFilePath		파일 경로(파일명 제외)
 * \param lpszDisplayName	윈도우 아이콘에 보여지는 이름
 * \param lpszTypeName		파일 포맷을 나타내는 문자열.(확장자가 아니고, "텍스트 파일"과 같은 형태)
 * \return					없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectChip::CtObjectChip( LPCWSTR lpszFileName, LPCWSTR lpszFilePath, LPCWSTR lpszDisplayName, LPCWSTR lpszTypeName )
{
	new (this) CtObjectChip();	

	m_stFileInfo.lpszFileName.SetString( lpszFileName );
	m_stFileInfo.lpszFilePath.SetString( lpszFilePath );
	m_stFileInfo.lpszFileType.SetString( lpszTypeName );
	m_stFileInfo.lpszDisplayName.SetString( lpszDisplayName );
	
	// 파일명으로부터 확장자 분리
	int pos = 0;	
	CString strExt = lpszFileName, token, temp;
	while( (token = strExt.Tokenize(L".", pos)) != L"" ) temp = token;
	m_stFileInfo.lpszExt.SetString( temp );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 소멸자

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectChip::~CtObjectChip( VOID )
{	
	m_pDXUTMesh->Destroy();
	SAFE_RELEASE( m_pMeshTextures[0] );
	SAFE_RELEASE( m_pMeshTextures[1] );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Pile의 양 끝 가장자리의 칩을 설정하거나 해제한다.

 * \param bOnOff	TRUE면 가장자리임을 명시, FALSE이면 반대
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectChip::SetBoundaryOfPile( BOOL bOnOff )
{
	m_bBoundaryOfPile = bOnOff;

	if( bOnOff )
	{
		// 이벤트 처리를 위한 flag 설정. Pile의 바운더리 액터는 contact시 파일에 추가하는 핸들링을 수행한다.
		//m_pActor->setContactReportFlags( NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD | NX_NOTIFY_FORCES );
		//m_pActor->setContactReportThreshold( 10000.f );
	}	
	else
	{
		//m_pActor->setContactReportFlags( NX_IGNORE_PAIR );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 칩 객체의 아이콘 추출 함수

 칩 객체의 경우 기본 메시(칩)에 객체가 가리키는 파일의 아이콘 모양을 동적 텍스쳐로 생성하여 입히게 된다.
 이를 위해 아이콘을 추출하고 텍스쳐를 생성하는 함수이다.

 * \param hIcon		아이콘 핸들
 * \return			오류 값
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT 
CtObjectChip::LoadIcon( HICON hIcon )
{
	HRESULT hr;

	// 아이콘에서 비트맵 추출		
	ICONINFO pIcon;
	BITMAP bmcolor;

	V( GetIconInfo(hIcon, &pIcon) );
	V( GetObject(pIcon.hbmColor, sizeof(BITMAP), &bmcolor) );

	//////////////////////////////////////////////////////////////////////////
	// 아이콘을 BMP 파일로 저장
	//////////////////////////////////////////////////////////////////////////
	WCHAR bmpfilepath[255];
	WCHAR currentDir[255];
	GetCurrentDirectory( 255, currentDir );
	wsprintf( bmpfilepath, L"%s\\%s.bmp", currentDir, m_stFileInfo.lpszExt );
	hBitmapToFile( pIcon.hbmColor, pIcon.hbmMask, bmcolor.bmBitsPixel, bmpfilepath );
	
	LPDIRECT3DDEVICE9 pDev = DXUTGetD3DDevice();

	//////////////////////////////////////////////////////////////////////////
	// 저장한 BMP 파일을 불러들여 아이콘 텍스쳐를 생성한다.
	// 특정 확장자를 제외하고는 리소스 캐쉬를 사용하여 텍스쳐 생성. 중복 텍스쳐 생성을 막기 위함.	
	// 퀄리티를 위해 아이콘 이미지는 DXT를 사용하지 않는다. 
	//////////////////////////////////////////////////////////////////////////
	if( !lstrcmpW(L"lnk", m_stFileInfo.lpszExt) || 
		!lstrcmpW(L"LNK", m_stFileInfo.lpszExt) || 
		!lstrcmpW(L"exe", m_stFileInfo.lpszExt) || 
		!lstrcmpW(L"EXE", m_stFileInfo.lpszExt) )
	{
		V( D3DXCreateTextureFromFileEx(pDev, bmpfilepath, 64, 64, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0, &m_pMeshTextures[0]) );
	}
	else if( !lstrcmpW(L"jpg", m_stFileInfo.lpszExt) || 
			 !lstrcmpW(L"JPG", m_stFileInfo.lpszExt) || 
			 !lstrcmpW(L"bmp", m_stFileInfo.lpszExt) || 
			 !lstrcmpW(L"BMP", m_stFileInfo.lpszExt) 
			 )
	{
		m_enType = OBJECT_TYPE_IMAGE;
		CString fullpath = CString(m_stFileInfo.lpszFilePath) + CString(m_stFileInfo.lpszFileName);
		V( DXUTGetGlobalResourceCache().CreateTextureFromFile(pDev, fullpath, &m_pMeshTextures[0]) );//Ex(pDev, fullpath, img.GetWidth(), img.GetHeight(), 0, 0, d3dfmt_, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0, &m_pMeshTextures[0]) );		
	}
	else
	{	
		m_enType = OBJECT_TYPE_CHIP;
		V( DXUTGetGlobalResourceCache().CreateTextureFromFileEx(pDev, bmpfilepath, 64, 64, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0, &m_pMeshTextures[0]) );
	}
	// BMP 파일 삭제
	DeleteFile( bmpfilepath );

	DeleteObject( pIcon.hbmColor );
	DeleteObject( pIcon.hbmMask );	

	return hr;
}

/*************************************************************************************************************************************************************/
/* 큐브 클래스(디렉토리)                                                                                                                                     */
/*************************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브 객체 기본 생성자

 생성자에서 PhysX 씬을 생성한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectCube::CtObjectCube( VOID )
{	
	m_bMaximized		= FALSE;
	m_pParentCube		= NULL;
	m_enType			= OBJECT_TYPE_CUBE;	
	m_pMeshTextures		= new LPDIRECT3DTEXTURE9[2];	
	m_pPhysXCompartment = NULL;

	m_pPhysXScene		= GetPhysX()->CreateScene();	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 디렉토리 정보로부터 큐브 객체를 생성하는 생성자

 * \param lpszFileName		디렉토리 이름
 * \param lpszFilePath		디렉토리 경로(자신의 디렉토리명 제외)
 * \param lpszDisplayName	윈도우 display name
 * \param lpszTypeName		형식 문자열 = "파일 폴더"
 * \return					없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectCube::CtObjectCube( LPCWSTR lpszFileName, LPCWSTR lpszFilePath, LPCWSTR lpszDisplayName, LPCWSTR lpszTypeName )
{
	new (this) CtObjectCube();	

	m_stFileInfo.lpszFileName.SetString( lpszFileName );
	m_stFileInfo.lpszFilePath.SetString( lpszFilePath );
	m_stFileInfo.lpszFileType.SetString( lpszTypeName );
	m_stFileInfo.lpszDisplayName.SetString( lpszDisplayName );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브 객체 소멸자

 객체 소멸시에 소멸 작업과 함께 추가로 자기 자신에 대한 직렬화 함수를 호출하여 객체 속성을 파일에 저장한다.
 또 포함하고 있는 객체 배열의 객체들을 모두 해제하여 연쇄적으로 메모리를 해제하고 직렬화 시킨다. 

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CtObjectCube::~CtObjectCube( VOID )
{	
	Serialize( TRUE );

	CtObjectBase** pObj = GetObjects();
	for( DWORD i=0; i < m_ObjectArray.GetSize(); i++ )
	{
		delete pObj[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 큐브 객체 직렬화 함수

 큐브가 갖는 포함 객체들의 직렬화를 수행한다. 큐브 내부로 처음 진입할 때 객체들을 로드하고 나서 이 직렬화 함수가 호출되어
 이전에 파일에 저장해 두었던 객체들의 속성값을 복구시키게 된다. 또 큐브가 해제될때 포함 객체들의 속성을 모두 파일에 저장한다.

 * \param bStoring	TRUE면 저장, FALSE면 로드 모드
 * \return			없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtObjectCube::Serialize( BOOL bStoring )
{
	CFile				file;
	CArchive::Mode		mode;
	int					fmode;

	if( bStoring ) 
	{
		mode = CArchive::store;
		fmode = CFile::modeCreate | CFile::modeWrite;
	}
	else
	{
		mode = CArchive::load;
		fmode = CFile::modeRead;
	}

	// 직렬화 파일은 설정된 직렬화 파일 저장 경로에 객체의 fullpath + .dat 로 한다
	CString filepath = SERIALIZED_FILEPATH;
	CString dir( m_stFileInfo.lpszFilePath );
	dir.Remove( L'\\' );
	dir.Remove( L':' );
	filepath += dir + m_stFileInfo.lpszFileName + CString( L".dat" );

	CtObjectBase** pObjects = GetObjects();
	DWORD nObjects = m_ObjectArray.GetSize();
	DWORD nObjectsStored = 0;

	// 저장 모드에서 빈 큐브인 경우 그냥 리턴
	if( nObjects == 0 && bStoring ) return;

	// 파일을 여는데 실패한 경우
	if( !file.Open(filepath, fmode) ) return;	

	// 아카이브 선언
	CArchive ar( &file, mode );
	
	// 저장 모드인 경우
	if( bStoring )
	{		
		ar << nObjects;

		for( DWORD i=0; i < nObjects; i++ )
			pObjects[i]->Serialize( ar );			
	}
	// 로드 모드인 경우
	else
	{	
		CString filepath, filename;
		CtFileInfo* pFI;

		ar >> nObjectsStored;
		for( DWORD i=0; i < nObjectsStored; i++ )
		{
			ar >> filepath >> filename;
			for( DWORD j=0; j < nObjects; j++ )
			{
				pFI = pObjects[j]->GetFileInfo();
				//////////////////////////////////////////////////////////////////////////
				// < 파일명을 포함한 전체 경로 > 로 객체를 유일하게 식별한다.
				// 미리 로드한 객체들 중 현재 꺼내려고 하는 직렬화 데이터와 이것이 일치하는 것을 판별하여
				// 해당 객체의 저장 데이터를 복구한다.
				//////////////////////////////////////////////////////////////////////////
				if( !lstrcmpW(filepath + filename, pFI->lpszFilePath + pFI->lpszFileName) )
				{
					pObjects[j]->Serialize( ar );
					break;
				}
			}
		}
	}

	ar.Close();
	file.Close();
}

VOID CtObjectCube::RemoveObject( CtObjectBase* pObj, bool bDeleteActor /*= FALSE */ )
{
	CtObjectBase** pObjects = GetObjects();
	for( DWORD i=0; i < m_ObjectArray.GetSize(); i++ )
	{
		if( pObjects[i] == pObj )
		{
			if( bDeleteActor )
				m_pPhysXScene->releaseActor( *pObj->GetActor() );
			m_ObjectArray.Remove( i );			
		}
	}
}