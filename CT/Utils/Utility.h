#pragma once

#include "D3DEx.h"

DWORD				SetShellLargeIconSize( DWORD dwNewSize );

bool				SetShortCutArrowDisable( VOID );

bool				SetShortCutArrowEnable( VOID );

BITMAPFILEHEADER*	hBitmapToFile( HBITMAP hBitmap, HBITMAP hMask, int BitCountPerPixel, LPCWSTR FilePath );

VOID				BitmapToFile( Bitmap *bmp, LPCWSTR filepath );

int					GetEncoderClsid( const WCHAR *format, CLSID *pClsid );

HWND				FindChildWindow( LPCTSTR psChildWndClassName, HWND hWndParent );

VOID				RenderText( int left, int top, int right, int bottom, D3DXCOLOR& color, DWORD dwFlags, const WCHAR* strMsg, ... );

NxU32				MyGetFileSize(const char *fileName);

CString				GetExtFromFilePath( LPCWSTR filepath );

