#ifndef HEADER_VNES
#define HEADER_VNES

extern void DebugString(const char* fmt, ...);
#ifdef _RELEASE
#define DEBUGOUTFLOPPY //DebugString
#define DEBUGOUTROM //DebugString
#define DEBUGOUTIO //DebugString
#define DEBUGOUTCPU //DebugString
#define DEBUGOUTVOICE //DebugString
#define DEBUGOUTBUFF //DebugString
#define DEBUGOUTTEST //DebugString
#define DEBUGOUTIODetail //DebugString
#else
#define DEBUGOUTFLOPPY //DebugString
#define DEBUGOUTROM //DebugString
#define DEBUGOUTIO DebugString
#define DEBUGOUTIODetail //DebugString
#define DEBUGOUTCPU //DebugString
#define DEBUGOUTVOICE //DebugString
#define DEBUGOUTBUFF //DebugString
#define DEBUGOUTTEST //DebugString
#endif

#ifndef u16
typedef unsigned short u16;
typedef unsigned short uint16;
typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned int u32;
typedef unsigned int uint32;
#endif

#endif