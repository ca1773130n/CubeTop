#pragma once

#include "D3DEx.h"

/// ������Ʈ�� ������ ���� ������ ������ ������ ������ �����ϴ� �Լ�. ū ������(48X48)���� �ٲپ� ������ �ػ󵵸� ���� �������� �����ϰ� �ٽ� �������(32X32) �������� ���� ����
DWORD				SetShellLargeIconSize( DWORD dwNewSize );

/// ������Ʈ�� ������ ���� �����ܵ��� ȭ��ǥ ��� ������ �����ϴ� �Լ�
bool				SetShortCutArrowDisable( VOID );

/// ������ �ٽ� ǥ���ϵ��� �ϴ� �Լ�
bool				SetShortCutArrowEnable( VOID );

/// HBITMAP�� bmp ���Ϸ� ����� �ִ� �Լ�. ����ũ�� �����Ѵ�.
BITMAPFILEHEADER*	hBitmapToFile( HBITMAP hBitmap, HBITMAP hMask, int BitCountPerPixel, LPCWSTR FilePath );

/// Bitmap�� bmp ���Ϸ� ����� �ִ� �Լ�
VOID				BitmapToFile( Bitmap *bmp, LPCWSTR filepath );

/// BitmapToFile���� ���
int					GetEncoderClsid( const WCHAR *format, CLSID *pClsid );

/// ������ �ڵ��� �������� �ڽ� ������ �� ������ Ŭ���� ������ ���°��� ã�� �ڵ��� ��ȯ
HWND				FindChildWindow( LPCTSTR psChildWndClassName, HWND hWndParent );

/// ���� ����� ��� �Լ�
NxU32				MyGetFileSize(const char *fileName);

/// ��ü ���ϰ�ηκ��� Ȯ���� �κ��� �����ϴ� �Լ�
CString				GetExtFromFilePath( LPCWSTR filepath );

