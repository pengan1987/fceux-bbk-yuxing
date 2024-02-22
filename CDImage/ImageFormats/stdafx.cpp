//This file is used to build a precompiled header
#include "stdafx.h"
#include "bzscore/Win32/win32_string.cpp"
#include "UDFAnalyzer.h"
#include "Filesystems/miniudf.h"
using namespace BazisLib;
using namespace ImageFormats;


//#define EXPORT_SYBOLS
#include "../libCDImage.h"
//static ManagedPointer<AIFile> pFile;
static ManagedPointer<AIParsedCDImage> pImage;
DLL_SYBOL bool CDImageOpen(const wchar_t* pszPath) {
	ImageFormatDatabase formats;
	ManagedPointer<AIFile> pFile = new ACFile(pszPath, FileModes::OpenReadOnly.ShareAll());
	if (!pFile->Valid())
	{
		return false;
	}
	pImage = formats.OpenCDImage(pFile, pszPath);
	return true;
}

DLL_SYBOL bool CDImageClose() {
	pImage->Release();
	return true;
}

DLL_SYBOL size_t ReadSectors(unsigned FirstSector, void* pBuffer, unsigned SectorCount) {
	return pImage->ReadSectorsBoundsChecked(FirstSector, pBuffer, SectorCount);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	wchar_t* pwszImage = L"O:\\360极速浏览器下载\\OK\\Game\\科王\\科王电脑VCD操作说明及演示.bin";
	auto d = CDImageOpen(pwszImage);
	char buf[40960];
	auto f = ReadSectors(65, buf, 10);
	d = CDImageOpen(pwszImage);
	CDImageClose();
	return 0;
}

