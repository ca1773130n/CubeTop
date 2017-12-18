//file debug.cpp
#include <stdio.h>
#include <stdarg.h>
//#include <windows.h>
#include "stdafx.h"
#include "debug.h"
/*
void _trace(char *fmt, ...)
{
	char out[1024];
	va_list body;
	va_start(body, fmt);
	vsprintf(out, fmt, body);
	va_end(body);
	OutputDebugStringA(out);
}
*/
void Trace(char *fmt, ...)
{
	char out[1024];
	va_list body;
	va_start(body, fmt);
	vsprintf(out, fmt, body);
	va_end(body);
	OutputDebugStringA(out);
}
