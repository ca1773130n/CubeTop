#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 환경 설정용 정의 : 차후 옵션 파일로 대체
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define DEBUG_DIR L"Debug\\"
#else
#define DEBUG_DIR L".\\"
#endif

#define DEFAULT_SIDENAME_FONT		L"HY견고딕"											///< 옆면 텍스트용 폰트명

#define SERIALIZED_FILEPATH			CString(DEBUG_DIR) + CString(L"data\\serialized\\")	///< 직렬화 데이터 저장 경로

#define DEFAULT_MESHFILE_CHIP		CString(DEBUG_DIR) + CString(L"data\\common\\chip.x")///< 기본 칩 메시 파일명
#define DEFAULT_MESHFILE_CUBE		CString(DEBUG_DIR) + CString(L"data\\common\\cube.x")///< 기본 큐브 메시 파일명
#define DEFAULT_MESHFILE_IMAGE		CString(DEBUG_DIR) + CString(L"data\\common\\image.x")///< 이미지 객체 메시 파일명
#define DEFAULT_MESHFILE_WORLD		CString(DEBUG_DIR) + CString(L"data\\common\\world.x")///< 기본 배경 메시 파일명

#define DEFAULT_LASSOVERTEX_SIZE	512													///< 올가미 선택툴 버퍼 크기
#define DEFAULT_LASSOVERTEX_COLOR	0x22EE0000											///< 올가미 선택 폴리곤 색상

#define DESK_WIDTH					32													///< 큐브 내부 공간 가로 크기. 충돌처리에 적용.
#define DESK_HEIGHT					32													///< 큐브 내부 공간 세로 크기. 충돌처리에 적용.
#define DESK_DEPTH					32													///< 큐브 내부 공간 높이 크기. 충돌처리에 적용.
#define MAIN_VERTEXBUFFER_SIZE		256													///< 기본 버텍스버퍼 크기.
#define MAIN_INDEXBUFFER_SIZE		512													///< 기본 인덱스버퍼 크기.
#define DEFAULT_FOV					(PI / 4.0f)											///< 기본 카메라 FOV값
#define MAX_FOV						0.0000000001f										///< 최대 FOV값
#define CENTER_OF_DESKTOP_BOTTOM	D3DXVECTOR3( DESK_WIDTH/2, DESK_HEIGHT/2, 0 )		///< 큐브 내부 바닥 중앙 좌표 벡터

#define CHIP_SIDE_TEXTURE_WIDTH		328													///< 칩 옆면 텍스쳐 가로 사이즈
#define CHIP_SIDE_TEXTURE_HEIGHT	40													///< 칩 옆면 텍스쳐 세로 사이즈
#define CUBE_SIDE_TEXTURE_WIDTH		320													///< 큐브 옆면 텍스쳐 가로 사이즈
#define CUBE_SIDE_TEXTURE_HEIGHT	320													///< 큐브 옆면 텍스쳐 세로 사이즈
#define CHIP_SIDE_TEXTURE_COLOR		0xeeffffff											///< 칩 옆면 텍스쳐 글자 색상

#define UNDO_LIMIT					20													///< 메인 UNDO 가능 횟수

#define MYMAX(x,y)					(x>y?x:y)											///< 기본 min(), max() 함수 사용 불가로 인한 정의
#define MYMIN(x,y)					(x<y?x:y)											///< 기본 min(), max() 함수 사용 불가로 인한 정의

#define DEFAULT_ICONNAME_FONT					L"Arial"
#define DEFAULT_CONTROLTEXTURE_FILENAME			CString(DEBUG_DIR) + CString(L"data\\common\\dxutcontrols.dds")

#define CHIP_THICKNESS				0.4f
#define CHIP_WIDTH					2
#define CHIP_HEIGHT					2
#define CUBE_WIDTH					1
#define IMAGE_HEIGHT				4
#define IMAGE_THICKNESS				0.2f

#define CONTACT_FORCE_THRESHOLD_ADDCHIP_TO_PILE		1.f

#define DEFAULT_SELECTION_TEXTURE_FILEPATH		CString(DEBUG_DIR) + CString(L"data\\common\\selectionfan.dds")