// file debug.h
//#ifdef _DEBUG
#ifdef __cplusplus
extern "C" {
#endif

void Trace(char *fmt, ...);
//#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
//#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
//#else
//#define ASSERT(x)
//#define VERIFY(x) x
//#endif
//#ifdef _DEBUG
//#define TRACE _trace
//#else
//void _trace(LPCTSTR fmt, ...);
//#define TRACE  1 ? (void)0 : _trace
//#endif
#ifdef __cplusplus
}
#endif