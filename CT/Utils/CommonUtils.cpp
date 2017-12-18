#include "stdafx.h"
#include "CommonUtils.h"

#define PI 3.141592f

DWORD
SetShellLargeIconSize( DWORD dwNewSize )
{
#define MAX_LENGTH   512
	DWORD   dwOldSize, dwLength = MAX_LENGTH, dwType = REG_SZ;
	TCHAR   szBuffer[MAX_LENGTH];
	HKEY	hKey;

	RegOpenKey( HKEY_CURRENT_USER, L"Control Panel\\desktop\\WindowMetrics", &hKey );
	RegQueryValueEx( hKey, L"Shell Icon Size", NULL, &dwType, (LPBYTE)szBuffer, &dwLength );

	CStringA temp( szBuffer );
	char *charBuffer = temp.GetBuffer();
	temp.ReleaseBuffer();

	dwOldSize = atol( charBuffer );
	// We will allow only values >=16 and <=72
	if( (dwNewSize>=16) || (dwNewSize<=72) )
	{
		wsprintf( szBuffer, L"%d", dwNewSize );
		RegSetValueEx( hKey, L"Shell Icon Size", 0, REG_SZ, (LPBYTE)szBuffer,
			lstrlen(szBuffer) + 2 );	// 원래 + 1 임.. 유니코드때문에 꽁수
	}
	// Clean up
	RegCloseKey( hKey );
	// Let everyone know that things changed
	SendMessage( HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETICONMETRICS, (LPARAM)("WindowMetrics") );
	return dwOldSize;
#undef MAX_LENGTH
}

bool
SetShortCutArrowDisable( VOID )
{
#define MAX_LENGTH   512
	DWORD   dwLength = MAX_LENGTH, dwType = REG_SZ;
	TCHAR   szBuffer[MAX_LENGTH];
	HKEY    hKey = 0;

	RegOpenKey( HKEY_CLASSES_ROOT, L"lnkfile", &hKey);
	if( RegQueryValueEx( hKey, L"IsShortcut", NULL, &dwType, (LPBYTE)szBuffer, &dwLength ) ==  ERROR_SUCCESS )	
	{
		if( RegDeleteValue( hKey, L"IsShortcut" ) == ERROR_SUCCESS ) 
			return TRUE;
	}
	return FALSE;
#undef MAX_LENGTH
}

bool
SetShortCutArrowEnable( VOID )
{
	HKEY hKey = 0;

	RegOpenKey( HKEY_CLASSES_ROOT, L"lnkfile", &hKey);

	if( RegSetValueEx( hKey, L"IsShortcut", 0, REG_SZ, 0, 0 ) == ERROR_SUCCESS )
		return TRUE;

	return FALSE;
}

// 알파가 빠지지 않은 아이콘 비트맵에서 마스크를 알파로 빼서 BMP로 저장. 느림
BITMAPFILEHEADER *
hBitmapToFile( HBITMAP hBitmap, HBITMAP hMask, int BitCountPerPixel, LPCWSTR FilePath )
{
	HDC hdc = GetDC( 0 );	
	BITMAP bmp = { 0 };	
	GetObject( hBitmap, sizeof(BITMAP), &bmp );	

	// 출력 해상도
	if( BitCountPerPixel == 0 ) 
		BitCountPerPixel = bmp.bmPlanes * bmp.bmBitsPixel; 

	int RawDataSize = ((((bmp.bmWidth * BitCountPerPixel) + 31) & ~31) >> 3) * bmp.bmHeight;

	int PalSize = (BitCountPerPixel > 8 ? 0 :1 << BitCountPerPixel) << 2; 

	int AllSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PalSize + RawDataSize;

	BITMAPFILEHEADER *pbfh = (BITMAPFILEHEADER*)VirtualAlloc( 0, AllSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	
	if( !pbfh )
	{
		MessageBox(0, L"메모리 할당에 실패했습니다.", L"에러", 0); 
		return 0;
	}

	pbfh->bfType = 0x4d42;
	pbfh->bfSize = AllSize;
	pbfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PalSize;

	BITMAPINFO *pbi = (BITMAPINFO*)((byte*)pbfh + sizeof(BITMAPFILEHEADER));
	pbi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	pbi->bmiHeader.biWidth			= bmp.bmWidth;
	pbi->bmiHeader.biHeight			= bmp.bmHeight;
	pbi->bmiHeader.biPlanes			= 1;
	pbi->bmiHeader.biBitCount		= BitCountPerPixel;
	pbi->bmiHeader.biCompression	= BI_RGB;
	pbi->bmiHeader.biSizeImage		= RawDataSize;

	BYTE* temp = new BYTE[bmp.bmWidth*bmp.bmWidth*4];
	BYTE* temp2 = new BYTE[bmp.bmWidth*bmp.bmWidth*4];
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, temp, pbi, DIB_RGB_COLORS);
	GetDIBits(hdc, hMask, 0, bmp.bmHeight, temp2, pbi, DIB_RGB_COLORS);

	for( int i=0; i < bmp.bmHeight; i++ ) 
		for( int j=0; j < bmp.bmWidth; j++ )
		{
			BYTE *alphaVal = temp2 + i*bmp.bmWidth*4 + j*4;
			BYTE *colorVal = temp + i*bmp.bmWidth*4 + j*4;
			if( alphaVal[0] == 0x0 ) 
			{
				if( colorVal[3] == 0 ) colorVal[3] = 255;
			}
		}

	memcpy( (byte*)pbi + sizeof(BITMAPINFOHEADER) + PalSize, temp, bmp.bmWidth*bmp.bmWidth*4 );

	HANDLE hFile = CreateFile( FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	DWORD dwWritten;
	WriteFile( hFile, pbfh, AllSize, &dwWritten, 0 );
	CloseHandle( hFile );
	VirtualFree( pbfh, 0, MEM_RELEASE );

	ReleaseDC( 0, hdc );
	delete[] temp;
	delete[] temp2;

	return pbfh;
}

VOID
BitmapToFile( Bitmap *bmp, LPCWSTR filepath )
{
	CLSID encoderClsid;
	EncoderParameters encoderParameters;
	ULONG quality;

	GetEncoderClsid( L"image/png", &encoderClsid );

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	quality = 100;
	encoderParameters.Parameter[0].Value = &quality;

	bmp->Save( filepath, &encoderClsid, &encoderParameters );
}

int 
GetEncoderClsid( const WCHAR *format, CLSID *pClsid )
{
	UINT num = 0;
	UINT size = 0;

	ImageCodecInfo *pImageCodecInfo = NULL;

	GetImageEncodersSize( &num, &size );
	if( size == 0 )
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)malloc(size);
	if( pImageCodecInfo == NULL )
		return -1;

	GetImageEncoders( num, size, pImageCodecInfo );

	for( UINT i=0; i < num; i++ )
	{
		if( wcscmp(pImageCodecInfo[i].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			free( pImageCodecInfo );
			return i;
		}
	}

	free( pImageCodecInfo );
	return -1;
}

HWND
FindChildWindow( LPCTSTR psChildWndClassName, HWND hWndParent )
{ 
	CString sClassName; 

	if ( hWndParent == NULL ) 
		hWndParent = ::GetDesktopWindow(); 

	HWND hWndT = ::GetWindow( hWndParent , GW_CHILD); 
	while ( hWndT != NULL ) 
	{ 
		GetClassName(hWndT, sClassName.GetBuffer(MAX_PATH), MAX_PATH ); 
		sClassName.ReleaseBuffer(); 

		if ( sClassName == psChildWndClassName ) 
		{ 
			return hWndT ; 
		} 
		hWndT = ::GetWindow(hWndT, GW_HWNDNEXT); 
	} 
	return NULL; 
} 


NxU32
MyGetFileSize(const char *fileName)
{
#ifndef SEEK_END
#define SEEK_END 2
#endif

	FILE* File = fopen(fileName, "rb");
	if(!File)
		return 0;

	fseek(File, 0, SEEK_END);
	NxU32 eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}

CString
GetExtFromFilePath( LPCWSTR filepath )
{
	CString temp( filepath ), ext;	
	int dotlocate = temp.ReverseFind( L'.' );
	ext = temp.Right( temp.GetLength() - dotlocate - 1 );
	return ext;
}

