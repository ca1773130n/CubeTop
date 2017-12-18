#include "stdafx.h"
#include "CT_RenderManager.h"

CtRenderManager::CtRenderManager( VOID )
{
	m_pDev = NULL;
	ZeroMemory( &m_RenderState, sizeof(CtRenderState) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 렌더매니져 초기화 함수.

 필요한 RenderState 설정을 하고 렌더링을 위한 공용 데이터들을 생성 및 초기화 한다.

 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID 
CtRenderManager::Init( VOID )
{
	HRESULT hr;

	m_pDev = DXUTGetD3DDevice();
	m_pDev->CreateStateBlock( D3DSBT_ALL, &m_pStateBlock );

	m_pDev->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	m_pDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	m_pDev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	m_pDev->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

	// 라이팅 관련
	ToggleLighting();
	ToggleSpecular();

	// Z, 알파 관련
	m_pDev->SetRenderState( D3DRS_ZENABLE, TRUE );		
	m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );			
	m_pDev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// 텍스쳐 관련
	m_pDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
	m_pDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
	m_pDev->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, 1);			

	// 행렬 스택 생성
	D3DXCreateMatrixStack( NULL, &m_pMatrixStack);	

	// 반사, 그림자용 행렬 초기화
	D3DXPLANE plane( 0.f, 1.f, 0.f, 0.f );
	SetReflectMatrix( &plane );
	
	// 그림자용 머티리얼
	ZeroMemory( &m_mtrlShadow, sizeof(D3DMATERIAL9) );
	m_mtrlShadow.Ambient.a = m_mtrlShadow.Specular.a = 1;
	m_mtrlShadow.Diffuse.a = 0.4f; 
	m_mtrlShadow.Power = 10;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 주어진 인덱스의 조명의 포지션 벡터를 반환한다.

 * \param iLightIndex	D3D 조명의 인덱스
 * \return				조명의 포지션 벡터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3
CtRenderManager::GetLightPosition( UINT iLightIndex )
{
	D3DLIGHT9 light;
	m_pDev->GetLight( iLightIndex, &light );
	return D3DXVECTOR3( light.Position.x, light.Position.y, light.Position.z );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 주어진 인덱스의 조명의 방향 벡터를 반환한다. directional light에만 사용할것.

 * \param iLightIndex	D3D 조명의 인덱스
 * \return				조명의 방향 벡터
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DXVECTOR3
CtRenderManager::GetLightDirection( UINT iLightIndex )
{
	D3DLIGHT9 light;
	m_pDev->GetLight( iLightIndex, &light );	
	return D3DXVECTOR3( light.Direction.x, light.Direction.y, light.Direction.z );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 행렬 스택에 현재 월드 행렬을 저장
 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtRenderManager::PushMatrix( VOID )
{
	D3DXMATRIXA16 matWorld;
	m_pMatrixStack->Push();
	m_pDev->GetTransform( D3DTS_WORLD, &matWorld );
	m_pMatrixStack->LoadMatrix( &matWorld );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 행렬 스택으로부터 저장했던 월드 행렬을 복구
 * \param VOID	없음
 * \return		없음
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtRenderManager::PopMatrix( VOID )
{
	D3DXMATRIX *matWorld = m_pMatrixStack->GetTop();
	m_pDev->SetTransform( D3DTS_WORLD, matWorld );		
	m_pMatrixStack->Pop();	
}

VOID
CtRenderManager::ToggleLighting( VOID )
{
	m_pDev->SetRenderState( D3DRS_LIGHTING, !m_RenderState.bLighting );
	m_RenderState.bLighting = !m_RenderState.bLighting;
}

VOID
CtRenderManager::ToggleWireframe( VOID )
{
	if( m_RenderState.bWireframe ) 
	{
		m_pDev->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		m_RenderState.bWireframe = FALSE;
	}
	else 
	{
		m_pDev->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		m_RenderState.bWireframe = TRUE;
	}
}

VOID
CtRenderManager::ToggleSmooth( VOID )
{
	if( m_RenderState.bSmooth )
	{
		m_pDev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		m_RenderState.bSmooth = FALSE;
	}
	else
	{
		m_pDev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		m_RenderState.bSmooth = TRUE;
	}
}

VOID
CtRenderManager::ToggleAlphaBlend( VOID )
{
	m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, !m_RenderState.bAlphaBlend );
	m_RenderState.bAlphaBlend = !m_RenderState.bAlphaBlend;
}

VOID
CtRenderManager::ToggleSpecular( VOID )
{
	m_pDev->SetRenderState( D3DRS_SPECULARENABLE, !m_RenderState.bSpecular );
	m_RenderState.bSpecular = !m_RenderState.bSpecular;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * \brief 장치가 파괴될 때 렌더매니져를 통해서 추가적으로 수행할 것을 작성한다.
 * \param VOID	없음
 * \return		없음
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID
CtRenderManager::OnDestroyDevice( VOID )
{
	SAFE_RELEASE( m_pMatrixStack );
}