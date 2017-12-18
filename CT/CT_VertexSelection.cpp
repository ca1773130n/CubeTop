#include "stdafx.h"
#include "CtVertexSelection.h"
#include "CubeTopDlg.h"

CubeTopVertexSelection::CubeTopVertexSelection( void )
{
	m_iNumSelection = 0;
	m_vselectionList = new VSelection;
	m_vselectionList->next = m_vselectionList->prev = NULL;
}

CubeTopVertexSelection::~CubeTopVertexSelection( void )
{

}

void
CubeTopVertexSelection::Add(int vNum)
{
	VSelection *newSel = new VSelection;
	newSel->vNum = vNum;

	VSelection *temp = m_vselectionList->next;
	m_vselectionList->next = newSel;
	newSel->prev = m_vselectionList;
	newSel->next= temp;

	if( temp ) temp->prev = newSel;
	m_iNumSelection++;
}

void
CubeTopVertexSelection::Remove(int vNum)
{
	for( VSelection *ptr = m_vselectionList->next; ptr; ptr = ptr->next )
	{
		if( ptr->vNum == vNum )
		{
			ptr->prev->next = ptr->next;
			if( ptr->next ) ptr->next->prev = ptr->prev;
			delete( ptr );
			m_iNumSelection--;
			break;
		}
	}
}

bool
CubeTopVertexSelection::IsInList( int vNum )
{
	for( VSelection *ptr = m_vselectionList->next; ptr; ptr = ptr->next )
	{
		if( ptr->vNum == vNum )
		{
			return true;
		}
	}
	return false;
}

void
CubeTopVertexSelection::Clear( void )
{
	m_iNumSelection = 0;
	VSelection *temp = m_vselectionList->next;
	while( temp )
	{
		m_vselectionList->next = temp->next;
		if( temp->next ) temp->next->prev = m_vselectionList;
		delete( temp );
		temp = m_vselectionList->next;
	}
}
/*
void
CubeTopVertexSelection::Move( float offsetX, float offsetY, float offsetZ )
{
	for( VSelection *ptr = m_vselectionList->next; ptr; ptr = ptr->next )
	{
		ptr->
	}
}
*/
