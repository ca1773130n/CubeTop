#pragma once

int ConvertWideCharToMultiByte(const wchar_t *pwsz, char *psz, int nLen, UINT nCodePage /*= CP_ACP*/);
int ConvertMultiByteToWideChar(const char *psz, wchar_t *pwsz, int nLen, UINT nCodePage /*= CP_ACP*/);
int ConvertTCharToMultiByte(const TCHAR *ptsz, char *psz, int nLen, UINT nCodePage /*= CP_ACP*/);
int ConvertTCharToWideChar(const TCHAR *ptsz, wchar_t *pwsz, int nLen, UINT nCodePage /*= CP_ACP*/);
int ConvertMultiByteToTChar(const char *psz, TCHAR *ptsz, int nLen, UINT nCodePage /*= CP_ACP*/);
int ConvertWideCharToTChar(const wchar_t *pwsz, TCHAR *ptsz, int nLen, UINT nCodePage /*= CP_ACP*/);
