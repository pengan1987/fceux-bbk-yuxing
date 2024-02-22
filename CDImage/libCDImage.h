#ifndef EXPORT_SYBOLS

#ifdef EXPORT_SYBOLS_DLL
#define DLL_SYBOL extern "C" __declspec(dllimport)
#else
#define DLL_SYBOL
#endif
#else
#define DLL_SYBOL extern "C" __declspec(dllexport)
#endif

DLL_SYBOL bool CDImageOpen(const wchar_t* pszPath);
DLL_SYBOL bool CDImageClose();
DLL_SYBOL size_t ReadSectors(unsigned FirstSector, void* pBuffer, unsigned SectorCount);
