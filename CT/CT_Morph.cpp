#include "stdafx.h"
#include "CT_Morph.h"
#include "CT_Globals.h"
#include "Utility.h"
#include <stack>
#include <queue>

#undef min
#undef max
#include "NxPhysics.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
//
// ZVector 클래스
//
//////////////////////////////////////////////////////////////////////////
ZVector2::ZVector2( VOID )
{
	x = y = min.x = min.y = org.x = org.y = 0;
}

ZVector2
ZVector2::operator=( D3DXVECTOR2 v )
{
	x = v.x;
	y = v.y;
	return *this;
}

ZVector2
ZVector2::operator=( ZVector2 v )
{
	x = v.x;
	y = v.y;
	org = v.org;
	min = v.min;
	max = v.max;
	return *this;
}

D3DXVECTOR2
ZVector2::GetVal( VOID )
{
	return D3DXVECTOR2( x, y );
}

VOID
ZVector2::SetVal( D3DXVECTOR2 v )
{
	x = v.x;
	y = v.y;
}

VOID
ZVector2::SetValAll( D3DXVECTOR2 v )
{
	x = v.x;
	y = v.y;

	min = max = org = v;
}

VOID
ZVector2::SetMin( VOID )
{
	min.x = x;
	min.y = y;
}

VOID
ZVector2::Interpolate( FLOAT alpha )
{
	x = ( 1.f - alpha ) * min.x + alpha * max.x;
	y = ( 1.f - alpha ) * min.y + alpha * max.y;
}

VOID
ZVector2::Interpolate( D3DXVECTOR2 v, FLOAT alpha )
{
	x = ( 1.f - alpha ) * min.x + alpha * v.x;
	y = ( 1.f - alpha ) * min.y + alpha * v.y;
}

ZVector3::ZVector3( VOID )
{
	x = y = z = min.x = min.y = min.z = org.x = org.y = org.z = 0;
}

ZVector3
ZVector3::operator=( D3DXVECTOR3 v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

ZVector3
ZVector3::operator=( ZVector3 v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	org = v.org;
	min = v.min;
	max = v.max;
	return *this;
}

D3DXVECTOR3 
ZVector3::GetVal( VOID )
{
	return D3DXVECTOR3( x, y, z );
}

VOID
ZVector3::SetVal( D3DXVECTOR3 v )
{
	x = v.x;
	y = v.y;
	z = v.z;
}

VOID
ZVector3::SetValAll( D3DXVECTOR3 v )
{
	x = v.x;
	y = v.y;
	z = v.z;

	min = max = org = v;
}

VOID
ZVector3::SetMin( VOID )
{
	min.x = x;
	min.y = y;
	min.z = z;
}

VOID
ZVector3::Interpolate( FLOAT alpha )
{
	x = ( 1.f - alpha ) * min.x + alpha * max.x;
	y = ( 1.f - alpha ) * min.y + alpha * max.y;
	z = ( 1.f - alpha ) * min.z + alpha * max.z;
}

VOID
ZVector3::Interpolate( D3DXVECTOR3 v, FLOAT alpha )
{
	x = ( 1.f - alpha ) * min.x + alpha * v.x;
	y = ( 1.f - alpha ) * min.y + alpha * v.y;
	z = ( 1.f - alpha ) * min.z + alpha * v.z;
}

//////////////////////////////////////////////////////////////////////////
//
// ZMorpher 클래스
//
//////////////////////////////////////////////////////////////////////////
ZMorpher::ZMorpher( VOID )
{
	m_dwNumESwap = m_dwNumESwapped = 0;
	m_fAlpha = 0;
}

VOID
ZMorpher::Initialize( VOID )
{
	m_dwNumESwap = m_dwNumESwapped = 0;

	m_SrcMesh.Destroy();
	m_TgtMesh.Destroy();
	m_SrcMeshClone.Destroy();	
	m_TgtMeshClone.Destroy();

	while( !m_ESSequenceQueue.empty() ) m_ESSequenceQueue.pop();
	while( !m_ESSequenceStack.empty() ) m_ESSequenceStack.pop();
}

VOID
ZMorpher::PrepareForMorph( CDXUTMesh* pSrcMesh, CDXUTMesh* pTgtMesh )
{
	m_SrcMesh.Create( pSrcMesh, Z_TYPE_SRC );
	m_TgtMesh.Create( pTgtMesh, Z_TYPE_TGT );
	m_SrcMeshClone.Create( pSrcMesh, Z_TYPE_SRC );
	m_TgtMeshClone.Create( pTgtMesh, Z_TYPE_TGT );

	// Sphere에 Embed
	m_SrcMesh.EmbedToSphere();
	m_TgtMesh.EmbedToSphere();
	
	// 카피본에 정보를 복사
	//CopyVertexInfo( m_SrcMesh, m_SrcMeshClone );
	//CopyVertexInfo( m_TgtMesh, m_TgtMeshClone );

	// 양쪽으로 Divide 한다
	DivideMeshesEachOther( m_SrcMesh, m_TgtMesh );

	//DivideMeshesEachOther( m_SrcMeshClone, m_TgtMeshClone );

	//m_SrcMesh.m_dwNumVertDummies += m_TgtMesh.m_dwNumVertDummies;
	
}

VOID
ZMorpher::CopyVertexInfo( ZMesh& srcMesh, ZMesh& tgtMesh )
{
	ZVert **pV1, **pV2;
	pV1 = srcMesh.m_verts.GetData();
	pV2 = tgtMesh.m_verts.GetData();
	for( DWORD i=0; i < srcMesh.m_verts.GetSize(); i++ )
	{
		pV2[i]->pos = pV1[i]->pos;
	}
}

VOID
ZMorpher::DivideMeshesEachOther( ZMesh& pMesh1, ZMesh& pMesh2 )
{
	ZEdge** pE;
	ZVert** pV;
	ZFace* pF;



	pV = pMesh2.m_verts.GetData();
	for( DWORD i=0; i < pMesh2.m_verts.GetSize(); i++ )
	{
		pMesh1.DivideTri( pV[i]->FindContainFace(pMesh1.m_faces, Z_TYPE_ANY), pMesh2.m_faces, pV[i] );		
	}
	pV = pMesh1.m_verts.GetData();
	for( DWORD i=0;i < pMesh1.m_verts.GetSize(); i++ ) 
	{
		if( pV[i]->correspond == NULL ) pF = pV[i]->FindContainFace( pMesh2.m_faces, Z_TYPE_ANY );		
		//if( pV[i]->correspond == NULL ) pMesh2.DivideTri(pV[i]->FindContainFace(pMesh2.m_faces, Z_TYPE_ANY), pV[i] );
	}

	
}

VOID	
ZMorpher::MakeESwapSequences( VOID )
{
	// Edge들을 error순으로 정렬할 Priority Queue
	pq_pState pMainPQ	= new pq__State;
	pMainPQ->size		= m_SrcMesh.m_edges.GetSize() + m_TgtMesh.m_edges.GetSize();
	pMainPQ->nobject	= 0;
	pMainPQ->lessThen	= lessThen;
	LONG* pHeap			= new LONG[pMainPQ->size];
	pMainPQ->heap		= (VOID**)pHeap;

	// 상대 메쉬에 Correspond가 없는 엣지들을 큐에 넣는다.
	m_SrcMeshClone.EnqueueUniqueEdges( pMainPQ, m_TgtMeshClone );
	m_TgtMeshClone.EnqueueUniqueEdges( pMainPQ, m_SrcMeshClone );
	
	ZEdge diagonalE;
	ZEdge *pPopped, *tempE, *pEorigin;
	FLOAT error, maxError = 0;
	UINT count;

	ZVert **pVsrc, **pVtgt;
	ZEdge **pEsrc, **pEtgt;
	pVsrc = m_SrcMesh.m_verts.GetData();
	pVtgt = m_TgtMesh.m_verts.GetData();
	pEsrc = m_SrcMesh.m_edges.GetData();
	pEtgt = m_TgtMesh.m_edges.GetData();

	// ESwap 시퀀스를 만든다
	while( pMainPQ->nobject > 0 )
	{
		//GetCTmain()->m_pMorphingSrcObj->ResetBuffersFromMorphMesh( &m_SrcMeshClone );
		//GetCTmain()->m_pMorphingTgtObj->ResetBuffersFromMorphMesh( &m_TgtMeshClone );
		//break;
		pPopped = (ZEdge*)pq_pop( pMainPQ );

		diagonalE.v1 = pPopped->next->v2;
		diagonalE.v2 = pPopped->twin->next->v2;
		diagonalE.error = 0;
		count = pPopped->CountIntersects( pPopped->v1->correspond );
	
		// 디버그
		/*
		GetCTmain()->m_pFeatures.RemoveAll();
		for( DWORD i=1; i <= pMainPQ->nobject; i++ )
		{
			tempE = (ZEdge*)pMainPQ->heap[i];
			//if(tempE->waitforswap)
			//{
				ZFeature fa;
				fa.vf[0] = tempE->v1;
				fa.vf[1] = tempE->v2;				
				GetCTmain()->m_pFeatures.Add(fa);
			//}
		}
		*/
		//GetCTmain()->m_pMorphingSrcObj->ResetBuffersFromMorphMesh( &m_SrcMeshClone );
		//GetCTmain()->m_pMorphingTgtObj->ResetBuffersFromMorphMesh( &m_TgtMeshClone );
		//GetCTmain()->Render();


		/*
		if( (tempE = diagonalE.GetCorrespondEdge()) != NULL )
		{
			if( pPopped->Swap() )
			{
				m_dwNumESwap++;
				if( pPopped->type == Z_TYPE_SRC ) 
				{
					pEorigin = pEsrc[pPopped->index];
					m_ESSequenceQueue.push( pEorigin );
					pEsrc[pPopped->index]->correspond = pEtgt[tempE->index];
					pEtgt[tempE->index]->correspond = pEsrc[pPopped->index];
				}
				else
				{
					pEorigin = pEtgt[pPopped->index];
					m_ESSequenceStack.push( pEorigin );
					pEtgt[pPopped->index]->correspond = pEsrc[tempE->index];
					pEsrc[tempE->index]->correspond = pEtgt[pPopped->index];
				}
				pq_removebypointer( pMainPQ, tempE );
				pq_removebypointer( pMainPQ, tempE->twin );
				
				continue;
			}			
		}
		*/
		if( count == 0 || pPopped->fixed ) 
		{
			if( (tempE = pPopped->GetCorrespondEdge()) != NULL )
			{
				pq_removebypointer( pMainPQ, tempE );
				pq_removebypointer( pMainPQ, tempE->twin );
			}
			continue;
		}
		if( pPopped->waitforswap )
		{	

			//if( count > 0 )
			if( (tempE = diagonalE.GetCorrespondEdge()) != NULL )
			{
				pPopped->Swap();
				pq_removebypointer( pMainPQ, tempE );
				pq_removebypointer( pMainPQ, tempE->twin );
			}
			else
			{
				pPopped->error = ((ZEdge*)pMainPQ->heap[pMainPQ->nobject])->error + 1;
				pq_push( pMainPQ, pPopped );
			}
			//}

			// 디버그
			/*
			BOOL bf = TRUE;
			for( DWORD i=1; i <= pMainPQ->nobject; i++ )
			{
				tempE = (ZEdge*)pMainPQ->heap[i];
				if( tempE->GetCorrespondEdge() == NULL || !tempE->waitforswap ) bf = FALSE;
			}

			if( bf ) 
			{
				for( DWORD i=1; i <= pMainPQ->nobject; i++ )
				{
					tempE = (ZEdge*)pMainPQ->heap[i];
					//tempE->GetCorrespondEdge();
					if( tempE->type == Z_TYPE_SRC )
					{
						pEsrc[tempE->index]->correspond = pEtgt[tempE->correspond->index];
						pEtgt[tempE->correspond->index]->correspond = pEsrc[tempE->index];
					}
					else
					{
						pEtgt[tempE->index]->correspond = pEsrc[tempE->correspond->index];
						pEsrc[tempE->correspond->index]->correspond = pEtgt[tempE->index];
					}
				}

				break; 
			}
			*/
			continue;
		}

		// 디버그
		
		//{
			//if( pPopped->GetCorrespondEdge() ) continue;
		//}
		
		if( !pPopped->swapped ) error = pPopped->error;
		else
		{
			if( pPopped->type == Z_TYPE_SRC ) error = pPopped->ComputeCorrespondError2( pVsrc[pPopped->v1->index] );
			if( pPopped->type == Z_TYPE_TGT ) error = pPopped->ComputeCorrespondError2( pVtgt[pPopped->v1->index] );
		}

		

		BOOL swapped = TRUE;
		if( diagonalE.CountIntersects( diagonalE.v1->correspond ) < count && (swapped = pPopped->Swap()) == TRUE ) 
		{		
			m_dwNumESwap++;
			
			if( pPopped->type == Z_TYPE_SRC ) 
			{
				pEorigin = pEsrc[pPopped->index];
				m_ESSequenceQueue.push( pEorigin );
			}
			else
			{
				pEorigin = pEtgt[pPopped->index];
				m_ESSequenceStack.push( pEorigin );
				//m_ESSequenceStack.push( pPopped );
				//pEorigin->Swap();
			}

			bool bFound = FALSE;		
			for( DWORD i=1; i <= pMainPQ->nobject; i++ )
			{
				tempE = (ZEdge*)pMainPQ->heap[i];
				if( (tempE->v1->pos == pPopped->v1->pos && tempE->v2->pos == pPopped->v2->pos) || (tempE->v1->pos == pPopped->v2->pos && tempE->v2->pos == pPopped->v1->pos) )
				{
					pq_remove( pMainPQ, i );
					// pop된 엣지의 v가 타겟 더미
					if( pPopped->type == Z_TYPE_SRC )
					{			
						if( tempE->v1->pos == pPopped->v1->pos && tempE->v2->pos == pPopped->v2->pos ) 
						{							
							pEsrc[pPopped->index]->correspond = pEtgt[tempE->index];
							pEtgt[tempE->index]->correspond = pEsrc[pPopped->index];
							pEsrc[pPopped->index]->twin->correspond = pEtgt[tempE->index]->twin;
							pEtgt[tempE->index]->twin->correspond = pEsrc[pPopped->index]->twin;
							
							if( tempE->r ) 
							{
								pEsrc[pPopped->index]->v1->kkk = TRUE;
								//pEsrc[pPopped->index]->v1->texcoord.max = pEtgt[tempE->index]->v1->texcoord.org;
							}
							if( tempE->twin->r ) 
							{
								pEsrc[pPopped->index]->twin->v1->kkk = TRUE;
								//pEsrc[pPopped->index]->twin->v1->texcoord.max = pEtgt[tempE->index]->twin->v1->texcoord.org;
							}
						}					
						else 
						{							
							pEsrc[pPopped->index]->correspond = pEtgt[tempE->index]->twin;
							pEtgt[tempE->index]->twin->correspond = pEsrc[pPopped->index];
							pEsrc[pPopped->index]->twin->correspond = pEtgt[tempE->index];
							pEtgt[tempE->index]->correspond = pEsrc[pPopped->index]->twin;
							if( tempE->r ) 
							{
								pEsrc[pPopped->index]->twin->v1->kkk = TRUE;
								//pEsrc[pPopped->index]->twin->v1->texcoord.max = pEtgt[tempE->index]->twin->v1->texcoord.org;
							}		
							if( tempE->twin->r ) 
							{
								pEsrc[pPopped->index]->v1->kkk = TRUE;
								//pEsrc[pPopped->index]->v1->texcoord.max = pEtgt[tempE->index]->twin->v1->texcoord.org;
							}		
						}						
					}
					else
					{				
						if( tempE->v2->pos == pPopped->v1->pos && tempE->v1->pos == pPopped->v2->pos )
						{							
							pEtgt[pPopped->index]->correspond = pEsrc[tempE->index];
							pEsrc[tempE->index]->correspond = pEtgt[pPopped->index];
							pEtgt[pPopped->index]->twin->correspond = pEsrc[tempE->index]->twin;
							pEsrc[tempE->index]->twin->correspond = pEtgt[pPopped->index]->twin;
							if( pPopped->r )
							{
								pEsrc[tempE->index]->v1->kkk = TRUE;
								//pEsrc[tempE->index]->v1->texcoord.max = pEtgt[pPopped->index]->v1->texcoord.org;
							}
							if( pPopped->twin->r )
							{
								pEsrc[tempE->index]->twin->v1->kkk = TRUE;
								//pEsrc[tempE->index]->twin->v1->texcoord.max = pEtgt[pPopped->index]->twin->v1->texcoord.org;
							}
						}
						else
						{
							pEtgt[pPopped->index]->correspond = pEsrc[tempE->index]->twin;
							pEsrc[tempE->index]->twin->correspond = pEtgt[pPopped->index];
							pEtgt[pPopped->index]->twin->correspond = pEsrc[tempE->index];
							pEsrc[tempE->index]->correspond = pEtgt[pPopped->index]->twin;
							if( pPopped->r )
							{
								pEsrc[tempE->index]->twin->v1->kkk = TRUE;
								//pEsrc[tempE->index]->twin->v1->texcoord.max = pEtgt[pPopped->index]->v1->texcoord.org;
							}
							if( pPopped->twin->r )
							{
								pEsrc[tempE->index]->v1->kkk = TRUE;
								//pEsrc[tempE->index]->v1->texcoord.max = pEtgt[pPopped->index]->twin->v1->texcoord.org;
							}
						}
					}
					pPopped->correspond = tempE;
					tempE->correspond = pPopped;


					bFound = TRUE;
					break;
				}
			}

			if( !bFound && (tempE = pPopped->GetCorrespondEdge()) == NULL ) 
			{
				if( pPopped->type == Z_TYPE_SRC ) diagonalE.error = diagonalE.ComputeCorrespondError2( pVsrc[diagonalE.v1->index] );
				if( pPopped->type == Z_TYPE_TGT ) diagonalE.error = diagonalE.ComputeCorrespondError2( pVtgt[diagonalE.v1->index] );
				pPopped->error = fabs(error - diagonalE.error) > 0 ? fabs(error - diagonalE.error) : error;
				pq_push( pMainPQ, (VOID*)pPopped );
			}
			else pPopped->fixed = tempE->fixed = TRUE;
		}
		else 
		{
			if( (tempE = pPopped->GetCorrespondEdge()) == NULL ) 
			{
				pPopped->error = ((ZEdge*)pMainPQ->heap[pMainPQ->nobject])->error + 1;
				//pPopped->waitforswap = TRUE;
				pq_push( pMainPQ, pPopped );
			}
			else pPopped->fixed = tempE->fixed = TRUE;
		}
	}
	GetCTmain()->m_pMorphingSrcObj->ResetBuffersFromMorphMesh( &m_SrcMeshClone );
	GetCTmain()->m_pMorphingTgtObj->ResetBuffersFromMorphMesh( &m_TgtMeshClone );
}

VOID
ZMorpher::MergeEmbeddings( ZMesh* pEmb1, ZMesh* pEmb2 )
{
	//////////////////////////////////////////////////////////////////////////
	//
	// intersection find 일주
	//
	//////////////////////////////////////////////////////////////////////////

	stack<ZEdge*> EdgeStack;

	ZVert **pV, *pNewVert;
	ZEdge *pE, **pEdges, *pBoundEdge;
	ZEdge *pLastIntersectedTgtEdge = NULL;
	ZFace *pF, *pStartFace, *pNewFaces;

	// 스택에 검사 edge와 face들을 push (너비우선탐색) : 첫번째 v에 대해서 시작
	pV = pEmb2->m_verts.GetData();
	for( DWORD i=0; i < pEmb2->m_verts.GetSize(); i++ )
	{
		pEdges = pV[i]->edges.GetData();
		if( pV[i]->correspond->FindEdgeIntersect( pEdges[0] ) != NULL )
		{
			pEdges[0]->marked = pEdges[0]->twin->marked = TRUE;
			EdgeStack.push( pEdges[0] );
			break;
		}
	}

	while( EdgeStack.empty() == FALSE )
	{
		// 스택에서 검사 edge, face를 pop
		pE = EdgeStack.top();
		EdgeStack.pop();

		// 디버깅
		ZFeature* feature = new ZFeature;
		feature->vf[0] = pE->v1;
		feature->vf[1] = pE->v2;

		// 검사 엣지의 시작점으로부터 처음 교차하는 엣지를 구함 
		pBoundEdge = pE->v1->correspond->FindEdgeIntersect( pE );
		if( !pBoundEdge ) 
		{
			TRACE(" NULL!!!!!!!!!!!!!!!!\n" );
			continue;
		}

		// 엣지의 끝점에 이를때까지 교차하는 엣지들을 구해 컷트한다
		while( 1 )
		{
			if( CheckIntersectEdges( pE, pBoundEdge, &pNewVert ) == TRUE )
			{
				// 디버깅
				GetCTmain()->m_pMorphingSrcObj->m_pFeatures.Add( pNewVert );

				// 교점을 저장한다		
				pEmb1->m_verts.Add( pNewVert );		

				// 교차하는 엣지를 컷트한다
				pBoundEdge->Cut( pEmb1, pNewVert );

				// 계속해서 다음 교차하는 엣지를 구한다
				if( CheckIntersectEdges( pE, pBoundEdge->twin->next, NULL ) == TRUE )
					pBoundEdge = pBoundEdge->twin->next;
				else if( CheckIntersectEdges( pE, pBoundEdge->twin->next->next->twin->next, NULL ) == TRUE )
					pBoundEdge = pBoundEdge->twin->next->next->twin->next;
				else
				{
					pBoundEdge = NULL;
					break;
					if( pBoundEdge->twin->next->v2->correspond == pE->v2 || D3DXVec3Length( &(pBoundEdge->twin->next->v2->pos - pNewVert->pos) ) > D3DXVec3Length( &(pE->v2->pos - pNewVert->pos) ) ) break;
					break;
					/*
					pBoundEdge->twin->next->v2->type = Z_TYPE_NEW;
					pBoundEdge->twin->next->v2->intersectVerts[0] = pE->v1;
					pBoundEdge->twin->next->v2->intersectVerts[1] = pE->v2;
					pBoundEdge->twin->next->v2->barycentricU = D3DXVec3Length( &(pBoundEdge->twin->next->v2->pos - pE->v1->pos) ) / D3DXVec3Length( &(pE->v2->pos - pE->v1->pos) );
					*/
					pBoundEdge = pBoundEdge->twin->next->v2->FindEdgeIntersect( pE );
				}

				if( !pBoundEdge ) break;
			}
			else break;
		}

		// 이제 검사 face는 검사 edge의 끝점을 포함한다. edge의 끝점으로부터 시작하는 표시 안된 엣지들과 그것들의 시작점을
		// 포함하는 face들을 스택에 넣는다
		pEdges = pE->v2->edges.GetData();
		for( DWORD i=0; i < pE->v2->edges.GetSize(); i++ )
		{			
			if( pEdges[i]->marked == FALSE )
			{
				// 검사 엣지 e와 그 twin에 표시를 해둠
				pEdges[i]->marked = pEdges[i]->twin->marked = TRUE;
				EdgeStack.push( pEdges[i] );				
			}
		}		
	}
}

VOID
ZMorpher::TransformBackToSrcMesh( VOID )
{
	//////////////////////////////////////////////////////////////////////////
	//
	// 병합된 메타메쉬에서 소스 메쉬로 트랜스폼한다
	//
	//////////////////////////////////////////////////////////////////////////
	ZVert **pV, **pDummies;
	ZEdge** pE;
	ZFace* pF;

	pV = m_SrcMesh.m_verts.GetData();
	for( DWORD i=0; i < m_SrcMesh.m_verts.GetSize(); i++ )
		pV[i]->pos = pV[i]->pos.min = pV[i]->pos.org;

	pE = m_SrcMesh.m_edges.GetData();
	for( DWORD i=0; i < m_SrcMesh.m_edges.GetSize(); i++ )
	{
		pE[i]->normal = pE[i]->normal.org;
		pE[i]->texcoord = pE[i]->texcoord.org;
		pE[i]->SetMin();
	}

	/*
	pV = m_SrcMesh.m_verts.GetData();
	for( DWORD i=0; i < m_SrcMesh.m_verts.GetSize(); i++ )
	{
		pV[i]->pos = pV[i]->pos.org;
		pV[i]->SetMin();
		
		pDummies = pV[i]->dummies.GetData();
		for( DWORD j=0; j < pV[i]->dummies.GetSize(); j++ )
		{
			pDummies[j]->pos = pV[i]->pos;			
			pDummies[j]->SetMin();
		}
	}	

	
	pV = m_TgtMesh.m_verts.GetData();
	for( DWORD i=0; i < m_TgtMesh.m_verts.GetSize(); i++ )
	{
		pV[i]->pos = pV[i]->pos.org;		
		pV[i]->SetMin();

		pDummies = pV[i]->dummies.GetData();
		for( DWORD j=0; j < pV[i]->dummies.GetSize(); j++ )
		{
			pDummies[j]->pos = pV[i]->pos;			
			pDummies[j]->SetMin();
		}
	}
	*/
}

VOID
ZMorpher::SetInterpolatedMesh( FLOAT alpha )
{	
	DWORD numSwap			= m_dwNumESwap;
	DWORD numSwapped		= m_dwNumESwapped;
	DWORD numToTargetSwap	= (DWORD)(numSwap * alpha);
	DWORD numToSwap			= numToTargetSwap - numSwapped;
	D3DXVECTOR3 vEnd;

	//////////////////////////////////////////////////////////////////////////
	// 
	// MakeESwapSequence() 함수로 미리 생성해 둔 ESwap 시퀀스 순서대로 ESwap 수행
	// Swap할 Src 엣지는 큐에, Tgt 엣지는 스택에 저장되어 있다.
	//
	//////////////////////////////////////////////////////////////////////////
	ZEdge* pEdge;
	for( DWORD i=0; i < numToSwap; i++ )
	{
		if( !m_ESSequenceQueue.empty() )
		{
			pEdge = m_ESSequenceQueue.front();
			m_ESSequenceQueue.pop();
			pEdge->Swap();
		}
		else
		{
			//pE = m_ESSequenceStack.top();
			//m_ESSequenceStack.pop();
			//pE->Swap();
			pEdge = m_ESSequenceStack.top();
			m_ESSequenceStack.pop();
			pEdge->correspond->Swap();
			
		}
		m_dwNumESwapped++;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// Interpolation 부분
	//
	//////////////////////////////////////////////////////////////////////////

	ZEdge** pE = m_SrcMesh.m_edges.GetData();
	for( DWORD i=0; i < m_SrcMesh.m_edges.GetSize(); i++ )
	{	
		pE[i]->v1->pos.Interpolate( alpha );
		pE[i]->normal.Interpolate( alpha );
		pE[i]->texcoord.Interpolate( alpha );
	}
	
	//////////////////////////////////////////////////////////////////////////
	//
	// 머티리얼 보간
	//
	//////////////////////////////////////////////////////////////////////////
	D3DMATERIAL9 *m0, *m1, *m2;
	m0 = &m_mat;
	m1 = &m_SrcMesh.m_material;
	m2 = &m_TgtMesh.m_material;

	m0->Ambient		= ( 1.f - alpha ) * m1->Ambient + alpha * m2->Ambient;
	m0->Diffuse		= ( 1.f - alpha ) * m1->Diffuse + alpha * m2->Diffuse;
	m0->Specular	= ( 1.f - alpha ) * m1->Specular + alpha * m2->Specular;
	m0->Power		= ( 1.f - alpha ) * m1->Power + alpha * m2->Power;
	m0->Emissive	= ( 1.f - alpha ) * m1->Emissive + alpha * m2->Emissive;

	m_fAlpha = alpha;
}

//////////////////////////////////////////////////////////////////////////
//
// ZMesh 클래스
//
//////////////////////////////////////////////////////////////////////////
ZMesh::ZMesh( VOID )
{
	m_dwNumVerts = m_dwNumVertDummies = m_dwNumEdges = m_dwNumVertDummies = m_dwNumFaces = 0;
}

VOID
ZMesh::Create( CDXUTMesh* pDXUTMesh, ZType type )
{
	MESHVERTEX* pVertices = 0;
	WORD* pIndices = 0;

	pDXUTMesh->m_pVB->Lock( 0, 0, (VOID**)&pVertices, D3DLOCK_READONLY );
	pDXUTMesh->m_pIB->Lock( 0, 0, (VOID**)&pIndices, D3DLOCK_READONLY );

	// 정점 생성은 그대로 한다
	ZVert **pVerts, *newvert;
	BOOL bFound;
	for( DWORD i=0; i < pDXUTMesh->m_dwNumVertices; i++ )
	{
		newvert					= new ZVert( type, pVertices[i].pos );
		newvert->index			= m_dwNumVerts++;

		bFound = FALSE;		
		pVerts = m_verts.GetData();
		for( DWORD j=0; j < m_verts.GetSize(); j++ )
			if( pVerts[j]->pos == newvert->pos )
			{
				pVerts[j]->dummies.Add( newvert );
				newvert->dummy_parent = pVerts[j];
				m_dwNumVertDummies++;
				newvert->fixed = TRUE;
				bFound = TRUE;
				break;
			}
		m_verts.Add( newvert );
	}

	// face 정보로부터 face와 엣지들을 생성한다
	ZVert* v[4];
	ZVert* r[4];
	ZEdge* e[3];	
	ZEdge* twin;		
	ZFace* newface;
	ZVert** pDum;
	DWORD mvIndex[4];

	pVerts = m_verts.GetData();
	for( DWORD i=0; i < pDXUTMesh->m_dwNumFaces * 3; i += 3 )
	{
		mvIndex[0] = pIndices[i];
		mvIndex[1] = pIndices[i+1];
		mvIndex[2] = pIndices[i+2];
		mvIndex[3] = pIndices[i];

		// 일단 새 face를 만들어둔다
		newface = new ZFace( type );		

		// 배열로 처리하기 위한 인덱스 설정
		for( int j=0; j < 3; j++ ) 
			v[j] = r[j] = pVerts[mvIndex[j]];		
		v[3] = pVerts[mvIndex[0]];
		r[3] = pVerts[mvIndex[0]];

		for( int m=0; m < 4; m++ ) 
		{
			bFound = FALSE;
			for( int j=0; j < m_verts.GetSize() && bFound == FALSE; j++ )
			{
				pDum = pVerts[j]->dummies.GetData();
				for( int k=0; k < pVerts[j]->dummies.GetSize(); k++ )
					if( mvIndex[m] == pDum[k]->index ) 
					{
						r[m] = pDum[k];
						v[m] = pVerts[j];						
						bFound = TRUE;
						break;
					}
			}
		}

		// face의 세 정점으로부터 edge 생성
		for( int j=0; j < 3; j++ )
		{	
			e[j] = new ZEdge( v[j], v[j+1], newface, pVertices[r[j]->index].normal, pVertices[r[j]->index].texcoord, type );
			
			if( r[j]->fixed ) e[j]->r = r[j];

			e[j]->f = newface;	
			e[j]->index = m_dwNumEdges++;

			v[j]->edges.Add( e[j] );			
			m_edges.Add( e[j] );			
		}
		e[0]->next = e[1];
		e[1]->next = e[2];
		e[2]->next = e[0];

		newface->edge = e[0];

		m_faces.Add( newface );
		m_dwNumFaces++;
	}

	// dummy들을 제거
	DWORD index = 0;
	while( index < m_verts.GetSize() )
	{
		while( pVerts[index]->fixed ) 
		{
			if( index < m_verts.GetSize() )
			{
				m_verts.Remove( index );
			}
			else break; 
		}
		index++;
	}

	// edge들의 twin 설정
	ZEdge** pE = m_edges.GetData();
	for( DWORD i=0; i < m_edges.GetSize(); i++ )
	{
		if( pE[i]->twin == NULL )
		{
			pE[i]->twin = pE[i]->v2->GetEdgeToVertex( pE[i]->v1 );
			if( pE[i]->twin ) pE[i]->twin->twin = pE[i];
			else MessageBox(0,L"twin not found",0,0);
		}
	}

	pDXUTMesh->m_pVB->Unlock();
	pDXUTMesh->m_pIB->Unlock();
}

VOID
ZMesh::Destroy( VOID )
{
	m_verts.RemoveAll();
	m_edges.RemoveAll();
	m_faces.RemoveAll();
	m_dwNumVerts = m_dwNumEdges = m_dwNumFaces = 0;
}

VOID 
ZMesh::DivideTri( ZFace* f, CGrowableArray<ZFace*>& pFaces, ZVert* pSrcV )
{
	ZEdge* e = f->edge;

	ZEdge* ne = new ZEdge[6];
	ZFace* nf = new ZFace[2];
	ZVert* v = new ZVert( pSrcV->type, pSrcV->pos );

	ZVert *v1, *v2, *v3;
	v1 = e->v1;
	v2 = e->next->v1;
	v3 = e->next->next->v1;

	ne[0].v2 = ne[1].v1 = v1;
	ne[0].v1 = ne[1].v2 = v;
	ne[2].v2 = ne[3].v1 = v2;
	ne[2].v1 = ne[3].v2 = v;
	ne[4].v2 = ne[5].v1 = v3;
	ne[4].v1 = ne[5].v2 = v;

	ne[0].f = ne[3].f = e->f = f;
	ne[1].f = ne[4].f = e->next->next->f = &nf[1];
	ne[2].f = ne[5].f = e->next->f = &nf[0];

	for( int i=0; i < 5; i += 2 ) 
	{
		ne[i].twin = &ne[i+1];
		ne[i+1].twin = &ne[i];
	}	

	ne[0].next = e;
	ne[1].next = &ne[4];
	ne[2].next = e->next;
	ne[3].next = &ne[0];
	ne[4].next = e->next->next;
	ne[5].next = &ne[2];

	e->next->next->next = &ne[1];
	e->next->next = &ne[5];
	e->next = &ne[3];

	v->edges.Add( &ne[0] );
	v->edges.Add( &ne[2] );
	v->edges.Add( &ne[4] );
	v1->edges.Add( &ne[1] );
	v2->edges.Add( &ne[3] );
	v3->edges.Add( &ne[5] );

	m_faces.Add( &nf[0] );
	m_faces.Add( &nf[1] );
	m_verts.Add( v );

	for( int i=0; i < 6; i++ ) 
	{
		ne[i].type = pSrcV->type == Z_TYPE_SRC ? Z_TYPE_TGT : Z_TYPE_SRC;		
		ne[i].index = m_dwNumEdges++;
		m_edges.Add( &ne[i] );
	}

	f->edge = e;
	nf[0].edge = ne[2].next;
	nf[1].edge = ne[4].next;

	pSrcV->correspond = v;
	v->correspond = pSrcV;

	// UV 보간을 위한 사전 작업
	ZEdge* boundE[3];
	boundE[0] = e;
	boundE[1] = e->next->twin->next;
	boundE[2] = e->next->twin->next->next->twin->next;

	FLOAT baryU, baryV, d;	
	D3DXIntersectTri( &v1->pos, &v2->pos, &v3->pos, &D3DXVECTOR3(0,0,0), &v->pos, &baryU, &baryV, &d );
	
	D3DXVECTOR3 pos;
	D3DXVec3BaryCentric( &pos, &v1->pos.org, &v2->pos.org, &v3->pos.org, baryU, baryV );
	v->pos.org = pos;
	v->pos.max = pSrcV->pos.org;	
	pSrcV->pos.max = pos;	

	D3DXVECTOR2 uv;
	D3DXVECTOR3 normal;
	D3DXVec2BaryCentric( &uv, &boundE[0]->texcoord.org, &boundE[1]->texcoord.org, &boundE[2]->texcoord.org, baryU, baryV );	
	D3DXVec3BaryCentric( &normal, &boundE[0]->normal.org, &boundE[1]->normal.org, &boundE[2]->normal.org, baryU, baryV );
	for( int i=0; i < 6; i += 2 )
	{
		ne[i].texcoord.org = uv;
		ne[i].normal.org = normal;
		ne[i+1].texcoord.org = boundE[i/2]->texcoord.org;
		ne[i+1].normal.org = boundE[i/2]->normal.org;
	}	

	ZEdge *pCCW1, *pCCW2, *pCCW3, *pCCW4, *pCCW5, *pCCW6;
	pCCW1 = ne[0].FindClosestCCWEdge( &ne[4], pSrcV );
	pCCW2 = ne[2].FindClosestCCWEdge( &ne[0], pSrcV );
	pCCW3 = ne[4].FindClosestCCWEdge( &ne[2], pSrcV );
	ne[0].normal.max = pCCW1 ? pCCW1->normal.org : pCCW3->normal.org;
	ne[2].normal.max = pCCW2 ? pCCW2->normal.org : pCCW1->normal.org;
	ne[4].normal.max = pCCW3 ? pCCW3->normal.org : pCCW2->normal.org;
	ne[0].texcoord.max = pCCW1 ? pCCW1->texcoord.org : pCCW3->texcoord.org;
	ne[2].texcoord.max = pCCW2 ? pCCW2->texcoord.org : pCCW1->texcoord.org;
	ne[4].texcoord.max = pCCW3 ? pCCW3->texcoord.org : pCCW2->texcoord.org;
	
	/*
	pCCW4 = ne[1].FindClosestCCWEdge( &ne[5], v1 );
	pCCW5 = ne[3].FindClosestCCWEdge( &ne[1], v2 );
	pCCW6 = ne[5].FindClosestCCWEdge( &ne[3], v3 );
	ne[1].normal.max = pCCW4 ? pCCW4->normal.org : pCCW6->normal.org;
	ne[3].normal.max = pCCW5 ? pCCW5->normal.org : pCCW4->normal.org;
	ne[5].normal.max = pCCW6 ? pCCW6->normal.org : pCCW5->normal.org;
	ne[1].texcoord.max = pCCW4 ? pCCW4->texcoord.org : pCCW6->texcoord.org;
	ne[3].texcoord.max = pCCW5 ? pCCW5->texcoord.org : pCCW4->texcoord.org;
	ne[5].texcoord.max = pCCW6 ? pCCW6->texcoord.org : pCCW5->texcoord.org;
	*/
	
	ZEdge* pEdge = NULL;
	ZFace* pFace = NULL;
	ZVert vTemp;
	if( (pEdge = pSrcV->GetEdgeToVertex(v1)) != NULL ) 
	{
		ne[0].normal.max = pEdge->normal.org;
		ne[0].texcoord.max = pEdge->texcoord.org;
		ne[1].normal.max = pEdge->twin->normal.org;
		ne[1].texcoord.max = pEdge->twin->texcoord.org;
	}
	else
	{		
		vTemp.pos = v->pos + 0.99 * (v1->pos - v->pos);
		pFace = vTemp.FindContainFace( pFaces, Z_TYPE_ANY );
		pEdge = pFace->edge;
		for( int i=0; i < 3; i++, pEdge = pEdge->next )
		{
			if( pEdge->v1->pos == v1->pos )
			{
				ne[1].normal.max = pEdge->normal.org;
				ne[1].texcoord.max = pEdge->texcoord.org;
			}
		}		
	}
	if( (pEdge = pSrcV->GetEdgeToVertex(v2)) != NULL ) 
	{
		ne[2].normal.max = pEdge->normal.org;
		ne[2].texcoord.max = pEdge->texcoord.org;
		ne[3].normal.max = pEdge->twin->normal.org;
		ne[3].texcoord.max = pEdge->twin->texcoord.org;
	}
	else
	{
		vTemp.pos = v->pos + 0.99 * (v2->pos - v->pos);
		pFace = vTemp.FindContainFace( pFaces, Z_TYPE_ANY );
		pEdge = pFace->edge;
		for( int i=0; i < 3; i++, pEdge = pEdge->next )
		{
			if( pEdge->v1->pos == v2->pos )
			{
				ne[3].normal.max = pEdge->normal.org;
				ne[3].texcoord.max = pEdge->texcoord.org;
			}
		}		
		//ne[3].normal.max = v2->containFace->GetBarycentricNormal( Z_ORG, v2->pos );
		//ne[3].texcoord.max = v2->containFace->GetBarycentricUV( Z_ORG, v2->pos );
	}
	if( (pEdge = pSrcV->GetEdgeToVertex(v3)) != NULL ) 
	{
		ne[4].normal.max = pEdge->normal.org;
		ne[4].texcoord.max = pEdge->texcoord.org;
		ne[5].normal.max = pEdge->twin->normal.org;
		ne[5].texcoord.max = pEdge->twin->texcoord.org;
	}
	else
	{
		vTemp.pos = v->pos + 0.99 * (v3->pos - v->pos);
		pFace = vTemp.FindContainFace( pFaces, Z_TYPE_ANY );
		pEdge = pFace->edge;
		for( int i=0; i < 3; i++, pEdge = pEdge->next )
		{
			if( pEdge->v1->pos == v3->pos )
			{
				ne[5].normal.max = pEdge->normal.org;
				ne[5].texcoord.max = pEdge->texcoord.org;
			}
		}		
	}
}

D3DXVECTOR3
ZMesh::GetCenterOfMass( VOID )
{
	ZVert** pV = m_verts.GetData();
	float minx = 0, miny = 0, minz = 0, maxx = 0, maxy = 0, maxz = 0;

	for( DWORD i=0; i < m_verts.GetSize(); i++ )
	{		
		if( pV[i]->pos.x < minx ) minx = pV[i]->pos.x;
		if( pV[i]->pos.x > maxx ) maxx = pV[i]->pos.x;
		if( pV[i]->pos.y < miny ) miny = pV[i]->pos.y;
		if( pV[i]->pos.y > maxy ) maxy = pV[i]->pos.y;
		if( pV[i]->pos.z < minz ) minz = pV[i]->pos.z;
		if( pV[i]->pos.z > maxz ) maxz = pV[i]->pos.z;
	}

	return D3DXVECTOR3( (minx + maxx) / 2.f, (miny + maxy) / 2.f, (minz + maxz) / 2.f );
}

FLOAT
ZMesh::ChangeCoordsByCMass( D3DXVECTOR3 vCMass )
{
	FLOAT radius = 0, temp = 0;
	
	ZVert** pV = m_verts.GetData();
	for( DWORD i=0; i < m_verts.GetSize(); i++ )
	{	
		pV[i]->pos -= vCMass;
		pV[i]->pos.org -= vCMass;
		D3DXVec3Normalize( &pV[i]->pos, &pV[i]->pos );
		pV[i]->pos.min = pV[i]->pos;
		temp = D3DXVec3Length( &pV[i]->pos );
		if( temp > radius ) radius = temp;
	}	
	return radius;
}

VOID
ZMesh::FixVertices( D3DXVECTOR3 vTetraVerts[4], CGrowableArray<ZVert*>& Anchors )
{
	FLOAT fMin, fDistance = 0;
	DWORD minIndex = 0;	

	ZVert** pV = m_verts.GetData();
	for( int i=0; i < 4; i++ )
	{
		fMin = 1000000;
		for( DWORD j=0; j < m_verts.GetSize(); j++ )
		{
			if( (fDistance = D3DXVec3Length( &(vTetraVerts[i] - pV[j]->pos) )) < fMin ) 
			{
				fMin = fDistance;
				minIndex = j;
			}
		}			
		pV[minIndex]->fixed = TRUE;
		Anchors.Add( pV[minIndex] );		
	}
}

BOOL
ZMesh::CheckCollapsed( D3DXVECTOR3 vTetraVerts[4], FLOAT fBetweenAnchors )
{
	FLOAT fMin;
	FLOAT fDistance = 0;
	DWORD minIndex = 0;

	ZEdge** pEdges;	
	ZVert** pV = m_verts.GetData();

	for( int i=0; i < 4; i ++ )
	{
		fMin = 1000000;		
		for( DWORD j=0; j < m_verts.GetSize(); j++ )
		{		
			if( pV[j]->fixed == FALSE && (fDistance = D3DXVec3Length( &(-vTetraVerts[i] - pV[j]->pos))) < fMin )
			{
				fMin = fDistance;
				minIndex = j;
			}
		}

		// diametric vertex의 인접 정점들을 검사하여 거리가 충분히 짧은지 확인
		pEdges = pV[minIndex]->edges.GetData();
		for( DWORD j=0; j < pV[minIndex]->edges.GetSize(); j++ )
		{
			if( D3DXVec3Length( &(pEdges[j]->v2->pos - pEdges[j]->v1->pos) ) >= fBetweenAnchors / 2.f ) return TRUE;			
		}
	}
	return FALSE;
}

DOUBLE
ZMesh::RelaxVertices( DOUBLE fEpsilon )
{
	ZVert** pV = m_verts.GetData();
	ZEdge** pE;
	
	DOUBLE fMaxMovement = 0, fMovement;
	int numNeighbors = 0;	
	int limit = 0;
	D3DXVECTOR3 vTemp;

	do
	{
		fMaxMovement = 0;
		for( DWORD i=0; i < m_verts.GetSize(); i++ )
		{
			vTemp.x = vTemp.y = vTemp.z = 0;
			numNeighbors = pV[i]->edges.GetSize();
			pE = pV[i]->edges.GetData();
			for( int j=0; j < numNeighbors; j++ )
			{
				vTemp += pE[j]->v2->pos;
			}					
			D3DXVec3Normalize( &vTemp, &vTemp );

			if( pV[i]->fixed == FALSE )
			{
				fMovement = D3DXVec3Length( &(vTemp - pV[i]->pos) );
				if( fMovement > fMaxMovement ) fMaxMovement = fMovement; 
				pV[i]->pos = vTemp;			
			}					
		}	
		if( ++limit > 10000 ) return -1;

	} while( fMaxMovement > fEpsilon );

	return fMaxMovement;
}

BOOL
ZMesh::CheckVertexOrientation( VOID )
{
	ZFace** pF = m_faces.GetData();	
	ZEdge** pE;
	ZEdge* pEdge;
	
	D3DXVECTOR3 vFaceVerts[3], vTemp;
	int sgn = 0, sgnFirst = 0, sgnSum = 0;
	FLOAT fTemp = 0;
	BOOL result = TRUE;
	
	for( DWORD i=0; i < m_faces.GetSize(); i++ )
	{
		pEdge = pF[i]->edge;
		for( int j=0; j < 3; j++, pEdge = pEdge->next )
			vFaceVerts[j] = pEdge->v1->pos;
		
		D3DXVec3Cross( &vTemp, &vFaceVerts[0], &vFaceVerts[1] );
		fTemp = D3DXVec3Dot( &vTemp, &vFaceVerts[2] );

		if( fTemp > 0 ) sgn = 1;
		else if( fTemp == 0 ) sgn = 0;
		else sgn = -1;				

		sgnSum += sgn;

		if( i == 0 ) sgnFirst = sgn;
		else if( sgn != sgnFirst ) result = FALSE;
	}

	return result;
}

VOID
ZMesh::EmbedToSphere( VOID )
{
	//////////////////////////////////////////////////////////////////////////
	//
	// * Alexa et al. Sphere parameterization 알고리즘 *
	//
	// 1. 바운더리 스피어를 구한다
	// 2. 질량중심을 중심으로 모든 정점 코디를 재조정한 뒤 
	// 3. 바운더리 스피어가 유닛 스피어가 되도록 하는만큼 모든 정점을 스케일링한다
	// 4. 스피어상에 랜덤 레귤러 테트라히드론을 만들고 그것과 가장 가까운 정점들을 고정한다
	// 5. 메쉬의 모든 정점들을 정점의 이웃정점들을 평균한 값으로 릴랙스시킨다
	//    릴랙스 과정은 정점들의 이동거리의 중 최대치가 입실론보다 작아질때까지 한다
	// 6. 만약 임베딩이 콜랩스되면 4번과정으로 돌아가는데 콜랩스 여부는 테트라히드론을 잇는 다이아메트릭 모델까지의
	//    가장 가까운 거리를 재서 이것이 앵커들 사이의 거리의 반보다 작은 경우 콜랩스되었다고 판단
	// 7. 4번과정에서와 같이 테트라히드론의 다이아메트릭 까지 가장 가까운 정점들을 고정한다
	// 8. 5번과정을 반복하여 릴랙스.
	// 9. 모든 face의 세 정점들을 검사해서 sgn(v0 외적 v1 을 v2와 내적한 값)이 모두 같지 않으면 입실론을 줄이고 
	//    8번과정을 반복한다. 모두 같아질때까지 계속한다.
	//
	//////////////////////////////////////////////////////////////////////////

	srand( time(NULL) );

	//////////////////////////////////////////////////////////////////////////
	//
	// Step 1, 2, 3
	// 
	// 메쉬를 단위 스피어 내부에 내접시킨다.
	// 바운더리 스피어를 구하고 메쉬의 중심을 원점으로 모든 정점 위치를 재조정한 뒤
	// 바운더리 스피어가 유닛 스피어가 되도록 하는 만큼 모든 정점을 uniform 스케일링한다
	// 
	//////////////////////////////////////////////////////////////////////////
	CGrowableArray<ZVert*> Anchors;

	D3DXVECTOR3 vCMass = GetCenterOfMass();
	FLOAT fRadius = ChangeCoordsByCMass( vCMass );	

	//////////////////////////////////////////////////////////////////////////
	//
	// Step 4, 5, 6
	// 
	// 바운더리 스피어상에 랜덤 레귤러 테트라히드론을 만들고 그것과 가장 가까운 정점들을 고정한다
	//
	// 메쉬의 모든 정점들을 정점의 이웃정점들을 평균한 값으로 릴랙스시킨다
	// 릴랙스 과정은 정점들의 이동거리의 중 최대치가 입실론보다 작아질때까지 한다
	//
	// 만약 임베딩이 콜랩스되면		4번과정으로 돌아가는데, 콜랩스 여부는 테트라히드론을 잇는 다이아메트릭 모델까지의
	// 가장 가까운 거리를 재	서 이것이 앵커들 사이의 거리의 반보다 작은 경우 콜랩스되었다고 판단한다
	//
	//////////////////////////////////////////////////////////////////////////

	// 정사면체를 스피어상에 만든다
	D3DXVECTOR3 vTetraVerts[4], vTetraInverse[4];
	DOUBLE fEpsilon = EPSILON;	
	DOUBLE fPrevEpsilon;
	FLOAT fBetweenAnchors = 0;

	ZVert** pV = m_verts.GetData();
	while( 1 )
	{
		MakeRandomRegularTetraHedron( vTetraVerts );
		for( int i=0; i < 4; i++ ) vTetraInverse[i] = -vTetraVerts[i];

		// 정사면체와 가까운 정점을 고정시킨다	
		ResetFixedVertices( Anchors );
		FixVertices( vTetraVerts, Anchors );	

		// Anchor들간의 거리중 가장 짧은것을 구해둔다		
		fBetweenAnchors = GetMinDistanceOfVertexPairs( Anchors );

		RelaxVertices( fEpsilon ); 

		if( CheckCollapsed( vTetraVerts, fBetweenAnchors ) == TRUE )
		{
			for( DWORD i=0; i < m_verts.GetSize(); i++ ) 
				pV[i]->pos = pV[i]->pos.min;						
		}
		else break;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// Step 7
	//
	// 4번 과정에서의 정사면체의 정점들의 diametric에 가까운 정점들을 고정시킨다
	//
	//////////////////////////////////////////////////////////////////////////

	ResetFixedVertices( Anchors );
	FixVertices( vTetraInverse, Anchors );
	fBetweenAnchors = GetMinDistanceOfVertexPairs( Anchors );

	//////////////////////////////////////////////////////////////////////////
	//
	// Step 8, 9
	//
	// 릴랙스를 한 뒤 
	// 모든 face의 세 정점들을 검사해서 sgn(v0 외적 v1 을 v2와 내적한 값)이 모두 같지 않으면 입실론을 줄이고 
	// 반복한다. 모두 같아질때까지 계속한다.
	//
	//////////////////////////////////////////////////////////////////////////

	do {		
		fPrevEpsilon = fEpsilon;
		fEpsilon = RelaxVertices( fEpsilon ); 		
	} while( CheckVertexOrientation() == FALSE && fEpsilon > 0 && fEpsilon < fPrevEpsilon );


	while( CheckVertexOrientation() == FALSE )
	{
		fEpsilon = EPSILON;
		while( 1 )
		{
			MakeRandomRegularTetraHedron( vTetraVerts );
			for( int i=0; i < 4; i++ ) vTetraInverse[i] = -vTetraVerts[i];

			// 정사면체와 가까운 정점을 고정시킨다	
			ResetFixedVertices( Anchors );
			FixVertices( vTetraVerts, Anchors );	

			// Anchor들간의 거리중 가장 짧은것을 구해둔다		
			fBetweenAnchors = GetMinDistanceOfVertexPairs( Anchors );

			RelaxVertices( fEpsilon );

			if( CheckCollapsed( vTetraVerts, fBetweenAnchors ) == TRUE )
			{
				for( DWORD i=0; i < m_verts.GetSize(); i++ ) 
					pV[i]->pos = pV[i]->pos.min;						
			}
			else break;
		}
		ResetFixedVertices( Anchors );
		FixVertices( vTetraInverse, Anchors );
		fBetweenAnchors = GetMinDistanceOfVertexPairs( Anchors );

		do {		
			if( CheckVertexOrientation() == TRUE ) break;
			fPrevEpsilon = fEpsilon;
			fEpsilon = RelaxVertices( fEpsilon ); 		
		} while( fEpsilon > 0 && fEpsilon < fPrevEpsilon );

		MakeRandomRegularTetraHedron( vTetraVerts );
		ResetFixedVertices( Anchors );
		FixVertices( vTetraVerts, Anchors );

		fEpsilon = EPSILON;
		do {
			if( CheckVertexOrientation() == TRUE ) break;
			fPrevEpsilon = fEpsilon;
			fEpsilon = RelaxVertices( fEpsilon ); 		
		} while( fEpsilon > 0 && fEpsilon < fPrevEpsilon );
	}
}

VOID
ZMesh::EnqueueUniqueEdges( pq_pState pq, ZMesh& oppositeMesh )
{
	ZEdge** pE = m_edges.GetData();
	for( DWORD i=0; i < m_edges.GetSize(); i++ )
	{
		if( pE[i]->marked == FALSE && pE[i]->GetCorrespondEdge() == NULL )
		{
			if( pE[i]->twin->marked == FALSE && pE[i]->twin->r != NULL )
			{
				pE[i]->marked = pE[i]->twin->marked = TRUE;
				pE[i]->twin->error = pE[i]->twin->ComputeCorrespondError();
				pq_push( pq, pE[i]->twin );
			}
			else
			{
				pE[i]->marked = pE[i]->twin->marked = TRUE;
				pE[i]->error = pE[i]->ComputeCorrespondError();
				pq_push( pq, pE[i] );
			}			
		}
	}
}

VOID
ZMesh::CalcNormals( VOID )
{
	D3DXVECTOR3 e1, e2, vNormal( 0,0,0 ), vTemp;
	DWORD numEdges = 0;

	ZEdge** pE;
	ZVert** pV = m_verts.GetData();
	for( DWORD i=0; i < m_verts.GetSize(); i++ )
	{
		pE = pV[i]->edges.GetData();
		numEdges = pV[i]->edges.GetSize();
		for( DWORD j=0; j < numEdges; j++ )
		{
			e1 = pE[j]->v2->pos - pE[j]->v1->pos;
			if( j == numEdges - 1 ) 
				e2 = pE[0]->v2->pos - pE[0]->v1->pos;
			else 
				e2 = pE[j+1]->v2->pos - pE[j+1]->v1->pos;
			D3DXVec3Cross( &vTemp, &e1, &e2 );
			D3DXVec3Normalize( &vTemp, &vTemp );
			vNormal += vTemp;
		}
		vNormal /= numEdges;
		D3DXVec3Normalize( &vNormal, &vNormal );
		//pV[i]->normal = vNormal;
		for( DWORD j=0; j < numEdges; j++ ) pE[j]->normal = vNormal;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// ZVert 클래스
//
//////////////////////////////////////////////////////////////////////////

ZVert::ZVert( VOID )
{
	type			= Z_TYPE_ANY;
	index			= 0;
	fixed			= FALSE;

	correspond		= NULL;
	interedges[0]	= NULL;
	interedges[1]	= NULL;
	containFace		= NULL;

	dummy_parent	= this;

	kkk=FALSE;	///////
}

ZVert::ZVert( ZType typecode, D3DXVECTOR3 vPos )
{
	new (this)ZVert();
	type = typecode;
	pos.SetValAll( vPos );
}

ZVert&
ZVert::operator=( ZVert& v )
{
	type = v.type;
	pos = v.pos;
	correspond = v.correspond;

	return *this;
}

ZEdge* ZVert::GetEdgeToVertex( ZVert* v2 )
{
	ZEdge** pEtest = this->edges.GetData();
	for( DWORD i=0; i < this->edges.GetSize(); i++ )
		if( pEtest[i]->v2->pos == v2->pos ) 
			return pEtest[i];

	return NULL;
}

ZFace*
ZVert::FindContainFace( CGrowableArray<ZFace*>& faces, ZType type )
{	
	D3DXVECTOR3 v1, v2, v3, cross1, cross2, cross3;
	D3DXVECTOR3 vO( 0,0,0 );
	FLOAT fd;

	ZFace** pF = faces.GetData();
	ZEdge** pE;
	ZEdge *e1, *e2, *e3;

	for( DWORD i=0; i < faces.GetSize(); i++ )
	{
		if( type != Z_TYPE_ANY && pF[i]->type != type ) continue;

		// v1, v2, v3를 구한다
		e1 = pF[i]->edge;
		e2 = e1->next;
		e3 = e2->next;

		v1 = e1->v1->pos;
		v2 = e2->v1->pos;
		v3 = e3->v1->pos;

		// face가 p를 포함하는지 검사		
		D3DXVec3Cross( &cross1, &v1, &v2 );
		D3DXVec3Cross( &cross2, &v2, &v3 );
		D3DXVec3Cross( &cross3, &v3, &v1 );

		if( D3DXVec3Dot( &cross1, &pos ) >= 0 && D3DXVec3Dot( &cross2, &pos ) >= 0 && D3DXVec3Dot( &cross3, &pos ) >= 0 )
		{
			if( D3DXIntersectTri(&v1, &v2, &v3, &vO, &pos, &this->barycentricUV.x, &this->barycentricUV.y, &fd) == TRUE )
			{
				D3DXVec3BaryCentric( &pos.max, &e1->v1->pos.org, &e2->v1->pos.org, &e3->v1->pos.org, this->barycentricUV.x, this->barycentricUV.y );
				D3DXVECTOR3 no;
				D3DXVECTOR2 co;
				D3DXVec3BaryCentric( &no, &e1->normal.org, &e2->normal.org, &e3->normal.org, this->barycentricUV.x, this->barycentricUV.y );
				D3DXVec2BaryCentric( &co, &e1->texcoord.org, &e2->texcoord.org, &e3->texcoord.org, this->barycentricUV.x, this->barycentricUV.y );

				pE = this->edges.GetData();
				for( DWORD j=0; j < this->edges.GetSize(); j++ )
				{
					pE[j]->normal.max = no;
					pE[j]->texcoord.max = co;
				}
				this->containFace = pF[i];
				return pF[i];
			}					
		}
	}

	return NULL;
}

D3DXVECTOR2
ZFace::GetBarycentricUV( ZValue valType, D3DXVECTOR3 vPos )
{
	D3DXVECTOR2 vResult;
	ZEdge *e1, *e2, *e3;
	D3DXVECTOR2 v1, v2, v3;
	
	e1 = this->edge;
	e2 = e1->next;
	e3 = e2->next;

	switch( valType )
	{
	case Z_ORG:
		{
			v1 = e1->texcoord.org;
			v2 = e2->texcoord.org;
			v3 = e3->texcoord.org;
			break;
		}
	case Z_CUR:
		{
			v1 = e1->texcoord;
			v2 = e2->texcoord;
			v3 = e3->texcoord;
			break;
		}
	case Z_MIN:
		{
			v1 = e1->texcoord.min;
			v2 = e2->texcoord.min;
			v3 = e3->texcoord.min;
			break;
		}
	case Z_MAX:
		{
			v1 = e1->texcoord.max;
			v2 = e2->texcoord.max;
			v3 = e3->texcoord.max;
		}
	}
	
	FLOAT fu, fv, fd;
	D3DXIntersectTri( &e1->v1->pos, &e2->v1->pos, &e3->v1->pos, &D3DXVECTOR3(0,0,0), &vPos, &fu, &fv, &fd );
	D3DXVec2BaryCentric( &vResult, &v1, &v2, &v3, fu, fv );
	return vResult;
}

D3DXVECTOR3
ZFace::GetBarycentricNormal( ZValue valType, D3DXVECTOR3 vPos )
{
	D3DXVECTOR3 vResult;
	ZEdge *e1, *e2, *e3;
	D3DXVECTOR3 v1, v2, v3;

	e1 = this->edge;
	e2 = e1->next;
	e3 = e2->next;

	switch( valType )
	{
	case Z_ORG:
		{
			v1 = e1->normal.org;
			v2 = e2->normal.org;
			v3 = e3->normal.org;
			break;
		}
	case Z_CUR:
		{
			v1 = e1->normal;
			v2 = e2->normal;
			v3 = e3->normal;
			break;
		}
	case Z_MIN:
		{
			v1 = e1->normal.min;
			v2 = e2->normal.min;
			v3 = e3->normal.min;
			break;
		}
	case Z_MAX:
		{
			v1 = e1->normal.max;
			v2 = e2->normal.max;
			v3 = e3->normal.max;
		}
	}

	FLOAT fu, fv, fd;
	D3DXIntersectTri( &e1->v1->pos, &e2->v1->pos, &e3->v1->pos, &D3DXVECTOR3(0,0,0), &vPos, &fu, &fv, &fd );
	D3DXVec3BaryCentric( &vResult, &v1, &v2, &v3, fu, fv );
	return vResult;
}

ZEdge* 
ZVert::FindEdgeIntersect( ZEdge* pE )
{
	ZEdge* result = NULL;

	ZEdge** pTempE = edges.GetData();
	for( DWORD i=0; i < edges.GetSize(); i++ )
	{
		if( CheckIntersectEdges( pE, pTempE[i]->next, NULL ) == TRUE )
		{
			result = pTempE[i]->next;
			break;
		}		
	}
	/*
	if( !result )
	{
	ZEdge* pMinEdge = NULL;
	ZEdge pCrossEdge;

	FLOAT fMinAngle = 1000000;
	FLOAT fAngle;
	for( DWORD i=0; i < pV->edges.GetSize(); i++ )
	{
	if( pTempE[i]->v2->correspond == pE->v2 ) return NULL;

	pCrossEdge.v1 = pTempE[i]->twin->next->v2;
	pCrossEdge.v2 = pTempE[i]->next->v2;
	if( (fAngle = FindIntersectPoint( pV->pos, pTempE[i]->v2->pos, pCrossEdge.v1->pos, pCrossEdge.v2->pos, pE )) < fMinAngle )
	{
	fMinAngle = fAngle;
	pMinEdge = pTempE[i];

	}
	}

	if( fMinAngle == -1 ) return NULL;
	if( !pMinEdge ) return NULL;

	FLOAT bu = D3DXVec3Length( &(pMinEdge->v2->pos - pE->v1->pos) ) / D3DXVec3Length( &(pE->v2->pos - pE->v1->pos) );
	if( bu > 1 || bu < 0 ) return NULL;

	pMinEdge->v2->type = Z_TYPE_NEW;
	pMinEdge->v2->intersectVerts[0] = pE->v1;
	pMinEdge->v2->intersectVerts[1] = pE->v2;
	//pMinEdge->v2->intersectVerts[2] = pMinEdge->v2;
	//pMinEdge->v2->intersectVerts[3] = pMinEdge->next->v2;
	//pMinEdge->v2->barycentricV = 1;
	pMinEdge->v2->barycentricU = bu;

	//pE->v1 = pMinEdge->v2;

	result = FindEdgeIntersect( pMinEdge->v2, pE );
	}
	*/
	return result;
}

BOOL
ZVert::RemoveEdge( ZEdge* pE )
{
	ZEdge** pEdges = edges.GetData();
	for( DWORD i=0; i < edges.GetSize(); i++ )
		if( pEdges[i] == pE ) 
		{
			edges.Remove(i);
			return TRUE;
		}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
// ZEdge 클래스
//
//////////////////////////////////////////////////////////////////////////

ZEdge::ZEdge( VOID )
{
	marked		= FALSE;
	v1 = v2		= NULL;
	twin = next = correspond = NULL;
	
	error		= 0;
	index		= 0;
	swapped		= FALSE;
	waitforswap	= FALSE;

	fixed		= FALSE;
	r			= NULL;

	//normal.SetValAll( D3DXVECTOR3(0,0,0) );
	//texcoord.SetValAll( D3DXVECTOR2(0,0) );
}

ZEdge::ZEdge( ZVert* pV1, ZVert* pV2, ZFace* pF, D3DXVECTOR3 no, D3DXVECTOR2 co, ZType typecode )
{	
	new (this) ZEdge();
	v1 = pV1;
	v2 = pV2;
	type = typecode;	

	normal.SetValAll( no );
	texcoord.SetValAll( co );
}

VOID
ZEdge::SetMin( VOID )
{
	normal.min = normal;
	texcoord.min = texcoord;
}

BOOL
ZEdge::Swap( VOID )
{
	/*
	ZEdge *e1, *e2, *e3, *e4, *e5, *e6;
	ZVert *v_1, *v_2, *v_3, *v_4;
	ZFace *f1, *f2;

	e1 = this;
	e2 = next;
	e3 = e2->next;
	e4 = twin;
	e5 = twin->next;
	e6 = e5->next;

	
	// 랑데뷰 엣지의 v가 더미이고 두 엣지의 v1 v2가 일치할 경우
	// 랑데뷰 엣지의 v를 취한다
	
	if( e1->type == Z_TYPE_SRC && e1->v1->kkk )// e1->correspond && e1->correspond->r )
	{
		e1->v1->pos = e6->v1->pos;
		e4->v1->pos = e3->v1->pos;
		//e1->v1->dummy_parent = e6->v;
		//e1->v1->pos.tmp = e6->v1->pos.tmp;
		//e1->v1->pos.org = e6->v1->pos.org;
		//e1->v1->pos.tmp = e1->correspond->r->pos.tmp;
		//e1->v1->pos.org = e1->correspond->r->pos.org;
		//e1->v1->texcoord.tmp = e1->correspond->r->texcoord.tmp;
		//e1->v1->texcoord.org = e1->correspond->r->texcoord.org;
		//e1->v1->normal.tmp = e1->correspond->r->normal.tmp;
		//e1->v1->normal.org = e1->correspond->r->normal.org;
		//e1->v1->kkk = TRUE;
	}
	else if( e4->type == Z_TYPE_SRC && e4->v1->kkk )//e4->correspond && e4->correspond->r )
	{
		e1->v1->pos = e6->v1->pos;
		e4->v1->pos = e3->v1->pos;
		//e4->v1->dummy_parent = e3->v;
		//e4->v1->pos.tmp = e3->v1->pos.tmp;
		//e4->v1->pos.org = e3->v1->pos.org;
		//e4->v1->pos.tmp = e4->correspond->r->pos.tmp;
		//e4->v1->pos.org = e4->correspond->r->pos.org;
		//e4->v1->texcoord.tmp = e4->correspond->r->texcoord.tmp;
		//e4->v1->texcoord.org = e4->correspond->r->texcoord.org;
		//e4->v1->normal.tmp = e4->correspond->r->normal.tmp;
		//e4->v1->normal.org = e4->correspond->r->normal.org;
		//e4->v1->kkk = TRUE;
	}
	else
	{
		*(e1->v1) = *(e6->v1);
		*(e4->v1) = *(e3->v1);
	}
	
	
	v_1 = e1->v1;
	v_2 = e1->v2;
	v_3 = e2->v2;
	v_4 = e5->v2;

	// 4개 정점을 가로지르는 두 엣지가 교차하는 경우만 swap한다 
	// 즉 triangle flip을 발생하는 eswap은 하지 않는다
	ZEdge te1, te2;
	te1.v1 = v_1;
	te1.v2 = v_2;
	te2.v1 = v_3;
	te2.v2 = v_4;
	if( marked && CheckIntersectEdges( &te1, &te2, NULL ) == FALSE ) return FALSE;

	// v1, v2의 edge list에서 e1, e4를 각각 삭제
	if( v_1->RemoveEdge(e1) == FALSE ) MessageBox( 0, L"remove edge fail", 0,0 );
	if( v_2->RemoveEdge(e4) == FALSE ) MessageBox( 0, L"remove edge fail", 0,0 );

	// v3, v4의 edge list에 e1, e4를 추가
	v_3->edges.Add( e4 );
	v_4->edges.Add( e1 );

	e1->v1 = e4->v2 = v_4;
	e1->v2 = e4->v1 = v_3;

	e3->next = e5;
	e5->next = e1;
	e1->next = e3;

	e4->next = e6;
	e6->next = e2;
	e2->next = e4;

	f1->edge = e5;
	f2->edge = e2;

	swapped = TRUE;
	twin->swapped = TRUE;
	waitforswap = FALSE;
	twin->waitforswap = FALSE;

	next->waitforswap = next->next->waitforswap = twin->next->waitforswap = twin->next->next->waitforswap = FALSE;
	next->twin->waitforswap = next->next->twin->waitforswap = twin->next->twin->waitforswap = twin->next->next->twin->waitforswap = FALSE;
	*/
	return TRUE;

}

VOID
ZEdge::SimpleSwap( VOID )
{
	/*
	ZEdge *e1, *e2, *e3, *e4, *e5, *e6;
	ZFace *f1, *f2;

	e1 = this;
	e2 = next;
	e3 = e2->next;
	e4 = twin;
	e5 = twin->next;
	e6 = e5->next;


	// 랑데뷰 엣지의 v가 더미이고 두 엣지의 v1 v2가 일치할 경우
	// 랑데뷰 엣지의 v를 취한다
	if( e1->type == Z_TYPE_SRC && e1->v1->kkk )
	{
		e1->v1->pos = e3->v1->pos;
		e4->v1->pos = e6->v1->pos;
	}
	else if( e4->type == Z_TYPE_SRC && e4->v1->kkk )
	{
		e1->v1->pos = e3->v1->pos;
		e4->v1->pos = e6->v1->pos;
	}
	else
	{
		*(e1->v) = *(e3->v);
		*(e4->v) = *(e6->v);
	}
	e3->next = e5;
	e5->next = e4;
	e1->next = e6;

	e4->next = e3;
	e6->next = e2;
	e2->next = e1;

	f1 = f;
	f2 = twin->f;

	f1->edge = e5;
	f2->edge = e2;
	*/
}

VOID 
ZEdge::Cut( ZMesh* pMesh, ZVert* v )
{
	//////////////////////////////////////////////////////////////////////////
	//
	// * 교차 엣지를 컷트하는 알고리즘 *
	//
	// 여기서는 컷트와 함께 triangular 과정도 수행한다.
	// 타겟 face와 그것의 반대쪽 face, 소스 엣지에 바운드되는 두 face 모두 고려해서
	// 엣지를 생성하고 주변 정보를 업데이트한다.
	//
	//////////////////////////////////////////////////////////////////////////

	ZEdge *ne = new ZEdge[6];
	ZFace *nf = new ZFace[2];

	FLOAT sp = D3DXVec3Length( &D3DXVECTOR3(this->next->v2->pos - v->interedges[1]->v1->pos) ) / D3DXVec3Length( &D3DXVECTOR3(v->interedges[1]->v2->pos - v->interedges[1]->v1->pos) );
	FLOAT sq = D3DXVec3Length( &D3DXVECTOR3(twin->next->v2->pos - v->interedges[1]->v2->pos) ) / D3DXVec3Length( &D3DXVECTOR3(v->interedges[1]->v2->pos - v->interedges[1]->v1->pos) );

	ne[3].normal.org = twin->normal.org;
	ne[4].normal.org = next->next->normal.org;
	ne[5].normal.org = twin->next->next->normal.org;
	ne[0].normal.org = ne[1].normal.org = normal.org + v->barycentricUV.y * ( next->normal.org - normal.org );
	ne[2].normal.org = twin->normal.org = twin->normal.org + ( 1 - v->barycentricUV.y ) * ( twin->next->normal.org - twin->normal.org );

	ne[3].texcoord.org = twin->texcoord.org;
	ne[4].texcoord.org = next->next->texcoord.org;
	ne[5].texcoord.org = twin->next->next->texcoord.org;
	ne[0].texcoord.org = ne[1].texcoord.org = texcoord.org + v->barycentricUV.y * ( next->texcoord.org - texcoord.org );
	ne[2].texcoord.org = twin->texcoord.org = twin->texcoord.org + ( 1 - v->barycentricUV.y ) * ( twin->next->texcoord.org - twin->texcoord.org );

	ne[3].normal.max = twin->normal.max;
	ne[4].normal.max = sp * v->interedges[1]->normal.org + (1 - sp) * v->interedges[1]->next->normal.org;
	ne[5].normal.max = sq * v->interedges[1]->twin->normal.org + (1 - sq) * v->interedges[1]->twin->next->normal.org;
	ne[0].normal.max = ne[2].normal.max = v->barycentricUV.x * v->interedges[1]->normal.org + (1 - v->barycentricUV.x) * v->interedges[1]->next->normal.org;
	ne[1].normal.max = twin->normal.max = (1 - v->barycentricUV.x) * v->interedges[1]->twin->normal.org + v->barycentricUV.x * v->interedges[1]->twin->next->normal.org;
	
	ne[3].texcoord.max = twin->texcoord.max;
	ne[4].texcoord.max = v->interedges[1]->texcoord.org + sp * ( v->interedges[1]->next->texcoord.org - v->interedges[1]->texcoord.org );
	ne[5].texcoord.max = v->interedges[1]->twin->texcoord.org + sq * ( v->interedges[1]->twin->next->texcoord.org - v->interedges[1]->texcoord.org );
	ne[0].texcoord.max = ne[2].texcoord.max = v->interedges[1]->texcoord.org + v->barycentricUV.x * ( v->interedges[1]->next->texcoord.org - v->interedges[1]->texcoord.org );
	ne[1].texcoord.max = twin->texcoord.max = v->interedges[1]->twin->texcoord.org + ( 1 - v->barycentricUV.x ) * ( v->interedges[1]->twin->next->texcoord.org - v->interedges[1]->twin->texcoord.org );

	// 대각 정점에 엣지 할당
	next->v2->edges.Add( &ne[4] );
	twin->next->v2->edges.Add( &ne[5] );

	// twin의 시작점이 컷트의 중점 v로 바뀌어야 하므로 e-v2에서 twin을 제거
	v2->RemoveEdge( twin );
	v2->edges.Add( &ne[3] );

	for( int i=0; i < 3; i++ ) 
	{
		ne[i].v1 = ne[i+3].v2 = v;		
		ne[i].twin = &ne[i+3];
		ne[i+3].twin = &ne[i];
	}
	ne[0].v2 = ne[3].v1 = v2;
	ne[1].v2 = ne[4].v1 = next->v2;
	ne[2].v2 = ne[5].v1 = twin->next->v2;
	v2 = twin->v1 = v;

	ne[0].f = &nf[0];
	ne[1].f = f;
	ne[2].f = twin->f;
	ne[3].f = twin->f;
	ne[4].f = &nf[0];
	ne[5].f = &nf[1];
	twin->next->f = &nf[1];
	next->f = &nf[0];

	ne[0].next = next;
	ne[1].next = next->next;	
	ne[2].next = twin->next->next;
	ne[3].next = &ne[2];
	ne[4].next = &ne[0];
	ne[5].next = twin;

	next->next = &ne[4];
	next = &ne[1];	
	twin->next->next->next = &ne[3];
	twin->next->next = &ne[5];

	nf[0].edge = &ne[0];
	nf[1].edge = twin;
	twin->f->edge = &ne[3];
	twin->f = &nf[1];
	f->edge = this;

	v->edges.Add( twin );
	v->edges.Add( &ne[0] );
	v->edges.Add( &ne[1] );
	v->edges.Add( &ne[2] );

	for( int i=0; i < 6; i++ ) pMesh->m_edges.Add( &ne[i] );
	for( int i=0; i < 2; i++ ) pMesh->m_faces.Add( &nf[i] );	

	v->interverts[2] = this->v1;
	v->interverts[3] = ne[0].v2;		
}

FLOAT 
ZEdge::ComputeCorrespondError( VOID )
{
	D3DXVECTOR3 v_1, v_2;
	v_1 = ( v1->pos + v2->pos ) / 2.f;
	v_2 = ( next->v2->pos + twin->next->v2->pos ) / 2.f;

	return D3DXVec3Length( &(v_2 - v_1) );
}

FLOAT 
ZEdge::ComputeCorrespondError2( ZVert* pV )
{
	if( !pV ) return 0;

	D3DXVECTOR3 v_1, v_2;
	v_1 = ( v1->pos + v2->pos ) / 2.f;
	FLOAT error = 0, maxError = 0;

	ZEdge* pBoundEdge = pV->FindEdgeIntersect( this );
	if( !pBoundEdge ) return 0;

	while( 1 )
	{
		if( CheckIntersectEdges( this, pBoundEdge, NULL ) == TRUE )
		{
			v_2 = ( pBoundEdge->v1->pos + pBoundEdge->v2->pos ) / 2.f;
			error = D3DXVec3Length( &(v_2 - v_1) );
			if( error > maxError ) maxError = error;			

			if( CheckIntersectEdges( this, pBoundEdge->twin->next, NULL ) == TRUE )
			{
				pBoundEdge = pBoundEdge->twin->next;
			}
			else if( CheckIntersectEdges( this, pBoundEdge->twin->next->next, NULL ) == TRUE )
			{
				pBoundEdge = pBoundEdge->twin->next->next;
			}
			else
			{
				break;
			}

			if( !pBoundEdge ) break;
		}
		else break;
	}

	return maxError;
}

ZEdge* 
ZEdge::GetCorrespondEdge( VOID )
{
	ZEdge** pEdges = v1->correspond->edges.GetData();
	for( DWORD i=0; i < v1->correspond->edges.GetSize(); i++ )
	{
		if( pEdges[i]->v2->pos == v2->correspond->pos ) 
		{
			pEdges[i]->fixed = fixed = TRUE;
			correspond = pEdges[i];
			pEdges[i]->correspond = this;
			return pEdges[i];
		}
		
	}
	return NULL;
}

UINT 
ZEdge::CountIntersects( ZVert* pV )
{	
	if( !pV ) return 0;

	ZEdge* pBoundEdge = pV->FindEdgeIntersect( this );
	if( !pBoundEdge ) return 0;

	int count = 0;
	while( 1 )
	{
		if( CheckIntersectEdges( this, pBoundEdge, NULL ) == TRUE )
		{
			count++;

			if( CheckIntersectEdges( this, pBoundEdge->twin->next, NULL ) == TRUE )
			{
				pBoundEdge = pBoundEdge->twin->next;
			}
			else if( CheckIntersectEdges( this, pBoundEdge->twin->next->next, NULL ) == TRUE )
			{
				pBoundEdge = pBoundEdge->twin->next->next;
			}
			else break;

			if( !pBoundEdge ) break;
		}
		else break;
	}

	TRACE("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ %d\n", count );
	return count;
}

BOOL
ZEdge::IsBetween( ZEdge* e1, ZEdge* e2 )
{
	D3DXVECTOR3 vec0 = this->v2->pos - this->v1->pos;
	D3DXVECTOR3 vec1 = e1->v2->pos - e1->v1->pos;
	D3DXVECTOR3 vec2 = e2->v2->pos - e2->v1->pos;
	FLOAT angle = GetAngleBetweenVectors( &vec1, &vec2 );
	return ( GetAngleBetweenVectors(&vec1, &vec0) < angle && GetAngleBetweenVectors(&vec2, &vec0) < angle );
}

ZEdge* 
ZEdge::FindClosestCCWEdge( ZEdge* pEdge, ZVert* pV )
{
	FLOAT angle, minangle = 1000000;
	ZEdge* pMinEdge = 0;

	ZEdge** pE = pV->edges.GetData();
	for( DWORD i=0; i < pV->edges.GetSize(); i++ )
	{
		if( pE[i]->IsBetween(pEdge, this) == TRUE )
		{
			angle = GetAngleBetweenVectors( &D3DXVECTOR3(this->v2->pos - this->v1->pos), &D3DXVECTOR3(pE[i]->v2->pos - pE[i]->v1->pos) );
			if( angle < minangle ) 
			{
				minangle = angle;
				pMinEdge = pE[i];
			}
		}
	}

	return pMinEdge;
}

//////////////////////////////////////////////////////////////////////////
//
// ZFace 클래스
//
//////////////////////////////////////////////////////////////////////////

ZFace::ZFace( VOID )
{

}

ZFace::ZFace( ZType typecode )
{
	type = typecode;
	edge = NULL;
}

//////////////////////////////////////////////////////////////////////////
// 엣지를 생성하기 전에 만들려는 엣지가 이미 v에 있는지 확인
// 엣지가 있으면 포인터를 돌려준다
//////////////////////////////////////////////////////////////////////////
VOID MakeRandomRegularTetraHedron( D3DXVECTOR3 vArray[4] )
{
	ZMesh* mesh = new ZMesh;
	
	// 가장 간단한 정사면체
	D3DXVECTOR3 vTetra[4];
	vTetra[0] = D3DXVECTOR3( 0,0,0 );
	vTetra[1] = D3DXVECTOR3( 0,1,1 );
	vTetra[2] = D3DXVECTOR3( 1,0,1 );
	vTetra[3] = D3DXVECTOR3( 1,1,0 );

	// 랜덤하게 회전한다	
	D3DXVECTOR3 vAxis[4];
	FLOAT fRandAngle;

	for( int i=0; i < 4; i++ )
	{
		vTetra[i] -= D3DXVECTOR3( 0.5f, 0.5f, 0.5f );		
		D3DXVec3Normalize( &vTetra[i], &vTetra[i] );		
		vAxis[i] = vTetra[i];
	}

	for( int i=0; i < 4; i++ )
	{
		fRandAngle = DEGREE_TO_RADIAN( rand() % 360 );
		for( int j=0; j < 4; j++ ) RotateVectorByAxis( &vTetra[j], &vTetra[j], vAxis[i], fRandAngle );		
	}
	
	for( int i=0; i < 4; i++ ) vArray[i] = vTetra[i];
}

VOID ResetFixedVertices( CGrowableArray<ZVert*>& Anchors )
{
	ZVert** pV = Anchors.GetData();
	for( DWORD i=0; i < Anchors.GetSize(); i++ )
	{
		pV[i]->fixed = FALSE;
	}
	Anchors.RemoveAll();
}

FLOAT GetMinDistanceOfVertexPairs( CGrowableArray<ZVert*>& verts )
{
	ZVert** pAnchors = verts.GetData();
	FLOAT fMin = 1000000;
	FLOAT fDistance = 0;
	for( DWORD i=0; i < verts.GetSize(); i++ )
	{
		for( DWORD j=i+1; j < verts.GetSize() - 1; j++ )
		{
			if( (fDistance = D3DXVec3Length( &(pAnchors[i]->pos - pAnchors[j]->pos) )) < fMin )
			{
				fMin = fDistance;
			}
		}
	}
	return fMin;
}

FLOAT CalcT( D3DXVECTOR3* ev1, D3DXVECTOR3* ev2, D3DXVECTOR3* vGamma )
{
	D3DXVECTOR3 vResult;
	D3DXVECTOR3 vTemp = (*ev1 + *ev2) / 2.f;
	
	D3DXVECTOR3 vNormal = -vTemp;
	D3DXVec3Normalize( &vNormal, &vNormal );
	
	D3DXPLANE plane;
	D3DXPlaneFromPointNormal( &plane, &vTemp, &vNormal );

	D3DXPlaneIntersectLine( &vResult, &plane, &D3DXVECTOR3(0,0,0), vGamma );

	if( D3DXVec3Length( &(*vGamma - vResult) ) > 50000.f )
	{
		*vGamma = -(*vGamma);
	}
	
	FLOAT fResult = D3DXVec3Length( &vResult ) / 100000.f;
	//if( fResult < D3DXVec3Length( &vTemp ) ) return -1;
	//else 
	return fResult;
	/*
	D3DXVECTOR3 v1, v2, vTemp1, vTemp2;
	FLOAT fAngle, fTwinAngle, fTanAngle, fBottom, fTemp, fTemp2;

	if( D3DXVec3Length( &(*vGamma - *ev1) ) < D3DXVec3Length( &(*vGamma - *ev2) ) ) 
	{
		v1 = *ev1;
		v2 = *ev2;
	}
	else 
	{
		v1 = *ev2;
		v2 = *ev1;
	}
	

	vTemp1 = *vGamma - v1;
	vTemp2 = v2 - v1;
	fAngle = GetAngleBetweenVectors( &vTemp1, &vTemp2 );
	fBottom = D3DXVec3Length( &vTemp1 );
	
	fTwinAngle = GetAngleBetweenVectors( &(-v1), &vTemp1 );
	fTanAngle = fTwinAngle - ( PI / 2.f - fAngle );
	if( fTanAngle < 0 ) fTanAngle = -fTanAngle;

	fTemp = fBottom * sin( fAngle ) * tan( fTanAngle );
	fTemp2 = fBottom * cos( fAngle );
	D3DXVec3Normalize( &vTemp2, &vTemp2 );
	vTemp2 *= ( fTemp + fTemp2 );
	
	
	return D3DXVec3Length( &(v1 + vTemp2) );
	*/
}

BOOL CheckIntersectEdges( ZEdge* e1, ZEdge* e2, ZVert** ppNewVert )
{
	if( e1->v1->pos == e2->v1->pos || e1->v2->pos == e2->v1->pos || e1->v1->pos == e2->v2->pos || e1->v2->pos == e2->v2->pos ) return FALSE;
	if( e1->v1 == e2->v1 || 
		e1->v2 == e2->v1 || 
		e1->v1 == e2->v2 || 
		e1->v2 == e2->v2 ||
		e1->v1->correspond == e2->v1 || 
		e1->v2->correspond == e2->v1 || 
		e1->v1->correspond == e2->v2 || 
		e1->v2->correspond == e2->v2 
		) return FALSE;

	D3DXVECTOR3 v0(0,0,0), v1, v2, v3, v4, vGamma;
	FLOAT sp, sq, sr, ss;
	D3DXPLANE plane;

	v1 = e1->v1->pos;
	v2 = e1->v2->pos;
	v3 = e2->v1->pos;
	v4 = e2->v2->pos;

	D3DXPlaneFromPoints( &plane, &v0, &v3, &v4 );
	if( D3DXPlaneIntersectLine( &vGamma, &plane, &v1, &v2 ) == NULL ) return FALSE;	
	
	sp = D3DXVec3Length( &(vGamma - v1) ) / D3DXVec3Length( &(v2 - v1) );
	sq = D3DXVec3Length( &(vGamma - v2) ) / D3DXVec3Length( &(v1 - v2) );
	
	D3DXPlaneFromPoints( &plane, &v0, &v1, &v2 );
	if( D3DXPlaneIntersectLine( &vGamma, &plane, &v3, &v4 ) == NULL ) return FALSE;
	
	sr = D3DXVec3Length( &(vGamma - v3) ) / D3DXVec3Length( &(v4 - v3) );
	ss = D3DXVec3Length( &(vGamma - v4) ) / D3DXVec3Length( &(v3 - v4) );

	if( sp < 1 && sq < 1 && sr < 1 && ss < 1 )
	{		
		if( ppNewVert != NULL )
		{
			D3DXVec3Normalize( &vGamma, &vGamma );
			ZVert* newVert = new ZVert( Z_TYPE_CUT, vGamma );
	
			// 여기서 포지션 min, max 정함
			newVert->pos.org = e2->v1->pos.org + sr * ( e2->v2->pos.org - e2->v1->pos.org );
			newVert->pos.max = e1->v1->pos.org + sp * ( e1->v2->pos.org - e1->v1->pos.org );
			newVert->barycentricUV.x = sp;
			newVert->barycentricUV.y = sr;
			newVert->interedges[0] = e2;
			newVert->interedges[1] = e1;

			newVert->interverts[0] = e1->v1->correspond ? e1->v1->correspond : e1->v1;
			newVert->interverts[1] = e1->v2->correspond ? e1->v2->correspond : e1->v2;
			newVert->interverts[2] = e2->v1;
			newVert->interverts[3] = e2->v2;

			*ppNewVert = newVert;
		}

		return TRUE;
	}
	return FALSE;
}

