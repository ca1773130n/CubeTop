#pragma once

#include "CubeTop.h"
#include "CtObject.h"

typedef struct VSelection VSelection;

struct VSelection
{
	int vNum;
	VSelection *prev;
	VSelection *next;
};

typedef class CubeTopVertexSelection CubeTopVertexSelection;

class CubeTopVertexSelection
{
public:
	D3DXVECTOR3 m_vCenter;
	D3DXVECTOR3 m_vCenterTemp;
	int m_iNumSelection;
	VSelection *m_vselectionList;	
public:
	CubeTopVertexSelection();
	~CubeTopVertexSelection();
	void Add( int vNum );
	void Remove( int vNum );	
	void Clear( void );
	bool IsInList( int vNum );
};

