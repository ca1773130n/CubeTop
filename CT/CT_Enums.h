///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ������ Ŭ������ ��� ���� ��ü�鿡 ���� CtObjectBase �� �Լ��� �ϰ� �����ϱ� ���� �޼ҵ�� ����
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum CtSelectionMethod
{
	SBM_HIDE,
	SBM_SHOW,
	SBM_MOVE,
	SBM_ROTLX,
	SBM_ROTLY,
	SBM_ROTLZ,
	SBM_ROTGX,
	SBM_ROTGY,
	SBM_ROTGZ,
	SBM_SCALE,
	SBM_SCALEOFFSET,
	SBM_BILLBOARD,
	SBM_SAVEPOSE,
	SBM_LOADPOSE,
	SBM_IMPORTMESH,
	SBM_REWIND,
	SBM_FOWARD,
	SBM_LIFT,
	SBM_UNLIFT,
	SBM_FREEZE,
	SBM_UNFREEZE,
	SBM_SETPOSE,
	SBM_SETPOSEBACK,
	SBM_SELECT,
	SBM_UNSELECT,
	SBM_RAISESF,
	SBM_CLEARSF,	
	SBM_RAISEAF,
	SBM_CLEARAF,
	SBM_DELETE,
};

// ���ؽ� ���� 
struct VERTEX
{
	D3DXVECTOR3 pos;		
	FLOAT		rhw;
	DWORD	    color; 
};
#define D3DFVF_VERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)