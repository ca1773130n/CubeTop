#pragma once

#include "D3DEx.h"

/// 레지스트리 변경을 통해 아이콘 사이즈 설정을 강제로 수행하는 함수. 큰 아이콘(48X48)으로 바꾸어 아이콘 해상도를 높여 아이콘을 추출하고 다시 원래대로(32X32) 돌려놓기 위해 쓴다
DWORD				SetShellLargeIconSize( DWORD dwNewSize );

/// 레지스트리 변경을 통해 아이콘들의 화살표 모양 숏컷을 제거하는 함수
bool				SetShortCutArrowDisable( VOID );

/// 숏컷을 다시 표시하도록 하는 함수
bool				SetShortCutArrowEnable( VOID );

/// HBITMAP을 bmp 파일로 만들어 주는 함수. 마스크도 지원한다.
BITMAPFILEHEADER*	hBitmapToFile( HBITMAP hBitmap, HBITMAP hMask, int BitCountPerPixel, LPCWSTR FilePath );

/// Bitmap을 bmp 파일로 만들어 주는 함수
VOID				BitmapToFile( Bitmap *bmp, LPCWSTR filepath );

/// BitmapToFile에서 사용
int					GetEncoderClsid( const WCHAR *format, CLSID *pClsid );

/// 지정한 핸들의 윈도우의 자식 윈도우 중 지정한 클래스 네임을 갖는것을 찾아 핸들을 반환
HWND				FindChildWindow( LPCTSTR psChildWndClassName, HWND hWndParent );

/// 파일 사이즈를 얻는 함수
NxU32				MyGetFileSize(const char *fileName);

/// 전체 파일경로로부터 확장자 부분을 추출하는 함수
CString				GetExtFromFilePath( LPCWSTR filepath );

