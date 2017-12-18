#include "stdafx.h"
#include "Unicode.h"

#pragma warning(disable:4996) 

int ConvertWideCharToMultiByte(const wchar_t *pwsz, char *psz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
	int nLenWideChar = wcslen(pwsz);
	int nLenMultiByte = ::WideCharToMultiByte(nCodePage, 0, pwsz, nLenWideChar+1, psz, nLen, 0, 0);
	psz[nLenMultiByte] = '\0';
	return nLenMultiByte;
} 

int ConvertMultiByteToWideChar(const char *psz, wchar_t *pwsz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
	int nLenMultiByte = strlen(psz);
	int nLenWideChar = ::MultiByteToWideChar(nCodePage, 0, psz, nLenMultiByte+1, pwsz, nLen);
	pwsz[nLenWideChar] = L'\0';
	return nLenWideChar;
} 

int ConvertTCharToMultiByte(const TCHAR *ptsz, char *psz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
#ifdef _UNICODE
	return ConvertWideCharToMultiByte(ptsz, psz, nLen, nCodePage);
#else
	if(nCodePage == CP_ACP)
	{
		strncpy(psz, ptsz, nLen-1);
		psz[nLen-1] = '\0';
		return strlen(psz);
	} 
	wchar_t wszTmp[1024];
	ASSERT(_tcslen(ptsz) < countof(wszTmp));
	ConvertMultiByteToWideChar(ptsz, wszTmp, countof(wszTmp), CP_ACP);
	return ConvertWideCharToMultiByte(wszTmp, psz, nLen, nCodePage);
#endif //_UNICODE
} 

int ConvertTCharToWideChar(const TCHAR *ptsz, wchar_t *pwsz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
#ifdef _UNICODE
	wcsncpy(pwsz, ptsz, nLen-1);
	pwsz[nLen-1] = '\0';
	return wcslen(pwsz);
#else
	return ConvertMultiByteToWideChar(ptsz, pwsz, nLen, nCodePage);
#endif //_UNICODE
} 

int ConvertMultiByteToTChar(const char *psz, TCHAR *ptsz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
#ifdef _UNICODE
	return ConvertMultiByteToWideChar(psz, ptsz, nLen, nCodePage);
#else
	if(nCodePage == CP_ACP)
	{
		strncpy(ptsz, psz, nLen-1);
		ptsz[nLen-1] = '\0';
		return _tcslen(ptsz);
	} 
	wchar_t wszTmp[1024];
	ASSERT(strlen(psz) < countof(wszTmp));
	ConvertMultiByteToWideChar(psz, wszTmp, countof(wszTmp), CP_ACP);
	return ConvertWideCharToMultiByte(wszTmp, ptsz, nLen, nCodePage);
#endif // _UNICODE
} 

int ConvertWideCharToTChar(const wchar_t *pwsz, TCHAR *ptsz, int nLen, UINT nCodePage /*= CP_ACP*/)
{
#ifdef _UNICODE
	wcsncpy(ptsz, pwsz, nLen-1);
	ptsz[nLen-1] = '\0';
	return _tcslen(ptsz);
#else
	return ConvertWideCharToMultiByte(pwsz, ptsz, nLen, nCodePage);
#endif // _UNICODE
} 

#pragma warning(default:4996) 
