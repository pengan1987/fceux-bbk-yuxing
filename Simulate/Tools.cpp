#include "Tools.h"
#include <windows.h>
#include <stdio.h>
#include<tchar.h>
#include"resource.h"
#include "MapperBase.h"
#ifdef _SIMULATE_FCEUX
#include "utils/unzip.h"
#endif
#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"
#include "waveout.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
const UINT nIDIdentify = 0x22334;
const UINT nIDMenuImgListMax = 20;
const UINT nIDMenuImgList = WM_USER + 100;
const UINT nIDMenuExplorImg = nIDMenuImgList + nIDMenuImgListMax + 5;
const UINT nIDMenuExplorNES = nIDMenuExplorImg + 1;
const UINT nIDMenuStart = nIDMenuExplorImg + 10;
const UINT nIDTimerLoadROM = 100, nIDTimerTitleDelay = 101, nIDTimerReleadROM = 102;

extern unsigned char chFirstData[554];
Tools* pTools = new Tools();
static char m_strPathConfig[2048];// = ".\\YXConfig.ini";
bool bCPUHookLog, bIDC_CHECK_IRQ = false, bIDC_CHECK_KEYBOARD = false, bIDC_CHECK_LogOut = false;
int nCPUHookLogIndex;
HINSTANCE Tools::hInstance;
VectorByte vByte, vByte1, vByte2;
const bool bIndex = true;
const bool bTime = false;
static bool bsThreadVoiceExit = false;
static HANDLE hEventVoice;
static int nIndexDebugString = 0;
static char szBuf[10240];

#include<time.h>
void DebugString(const char* fmt, ...)
{
	if(pTools && (!pTools->bDebugString && BST_UNCHECKED == IsDlgButtonChecked(pTools->mWndDebug, IDC_CHECK_LogOut)))
		return;
	static char buf[20480];
	time_t t;
	time(&t);
	auto tm = localtime(&t);
	if (bIndex) {
		sprintf(buf, "%04x ", nIndexDebugString++);
		OutputDebugStringA(buf);
	}
	if (bTime) {
		static int n = 0;
		sprintf(buf, "%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);
		OutputDebugStringA(buf);
	}
	printf(buf);
	buf[0] = 0;
	va_list	va;
	va_start(va, fmt);
	::vsprintf(buf, fmt, va);
	strcat(buf, "\r\n");
	OutputDebugStringA(buf);
	printf(buf);
}

#include <codecvt>
#include <locale>
std::string utf8_to_gb2312(std::string const& strUtf8)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
	std::wstring wTemp = cutf8.from_bytes(strUtf8);
#ifdef _MSC_VER
	std::locale loc("zh-CN");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	const wchar_t* pwszNext = nullptr;
	char* pszNext = nullptr;
	mbstate_t state = {};

	std::vector<char> buff(wTemp.size() * 2);
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).out(state,
			wTemp.data(), wTemp.data() + wTemp.size(), pwszNext,
			buff.data(), buff.data() + buff.size(), pszNext);

	if (std::codecvt_base::ok == res)
	{
		return std::string(buff.data(), pszNext);
	}
	return "";
}
std::string ansi2utf8(const std::string& str)
{
	typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
	static std::wstring_convert<F> strtemp(new F("zh-CN"));
	static std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(strtemp.from_bytes(str));
}

DWORD WINAPI Tools::s_WindowDialogThreadProc(__in  LPVOID lpParameter) {
	pTools->mWndDebug = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_DEBUG), NULL, [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return (INT_PTR)pTools->WindowProc(hwnd, uMsg, wParam, lParam);
		});
	ShowWindow(pTools->mWndDebug, SW_SHOW);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

Tools::~Tools()
{
	Close();
}
Tools::Tools()
{
	bCanSetTitle = bDebugString = bFocus = true;
	dwTickTitle = 0;
	char szFile[1024];
	GetPrivateProfileStringA("Config", "bScanRom", "", szFile, sizeof(szFile), m_strPathConfig);
	strImageFile = szFile;
	bMe = bScanRom = bFloppySave = false;
	if (strImageFile == "true")
		bScanRom = true;
	GetModuleFileNameA(nullptr, m_strPathConfig, sizeof(m_strPathConfig));
	LPSTR lpInsertPos = strrchr(m_strPathConfig, '\\');
	lpInsertPos[1] = 0;
	strcat(m_strPathConfig, "Config.ini");
	GetDefaultMedia();

	mWNDPROC = 0;
	lpFloppy = nullptr;
}

char* Tools::GetDefaultMedia(const char* pszExt) {
	char szTemp[512];
	static char szFile[512];
	const char* ppsz[] = { "ima", "img", "imz" };
	for (int i = 0; ; i++) {
		sprintf(szTemp, "ImagePath%02d", i);
		if (GetPrivateProfileStringA("Config", szTemp, "", szFile, sizeof(szFile), m_strPathConfig) <= 0)
			break;
		auto pName = strrchr(szFile, '.');
		if (!pName)
			continue;
		pName++;
		pName = strlwr(pName);

		if (nullptr != pszExt) {
			if(strcmp(pName, pszExt) == 0)
				return szFile;
			continue;
		}

		bool bFind = false;
		for (int i = sizeof(ppsz) / sizeof(ppsz[0]) - 1; i >= 0; i--) {
			if (strcmp(pName, ppsz[i]) == 0) {
				bFind = true;
				strImageFile = szFile;
				return szFile;
			}
		}
	}
	return nullptr;
}


LPBYTE Tools::FloppyIMGSave()
{
	bFloppySave = false;
	return FloppyIMGLoad(lpFloppy);
}

//加载或保存(lpFloppyWrite非NULL)软盘镜像
LPBYTE Tools::FloppyIMGLoad(LPBYTE lpFloppyWrite , bool bChange )
{
	FILE* fp = NULL;
	static int size = 0;
	int nSizeNormal = 2 * 80 * 18 * 512;
	auto strTemp = strImageFile;
	auto pName = strlwr((char*)strTemp.c_str());
	pName = strrchr(pName, '.');
	enumTitleStatus = EnumTitleStatusWriteable;
#ifdef _SIMULATE_FCEUX
	while (pName && std::string(++pName) == "imz") {
		auto tz = unzOpen(strImageFile.c_str());
		if (!tz)
			break;
		if (unzGoToFirstFile(tz) != UNZ_OK || unzOpenCurrentFile(tz) != UNZ_OK) {
			unzClose(tz);
			break;
		}
		char tempu[512];
		unzGetCurrentFileInfo(tz, 0, tempu, sizeof(tempu), 0, 0, 0, 0);
		tempu[sizeof(tempu) - 1] = 0;
		unz_file_info ufo;
		unzGetCurrentFileInfo(tz, &ufo, 0, 0, 0, 0, 0, 0);
		nSizeNormal = ufo.uncompressed_size;
		if (lpFloppy)
			delete[] lpFloppy;
		if (!(lpFloppy = (LPBYTE)new char[nSizeNormal + 1]))
			goto CleanUNZ;

		ZeroMemory(lpFloppy, nSizeNormal);
		unzReadCurrentFile(tz, lpFloppy, ufo.uncompressed_size);
		enumTitleStatus = EnumTitleStatusReadOnly;
		unzCloseCurrentFile(tz);
		unzClose(tz);
		SetTitle();
		MenuImgListSave(strImageFile.c_str());
		return lpFloppy;

	CleanUNZ:
		unzCloseCurrentFile(tz);
		unzClose(tz);
		break;
	}
#endif
	if (!(fp = ::fopen(strImageFile.c_str(), lpFloppyWrite ? "wb" : "rb"))) {
		if (lpFloppyWrite)
			return nullptr;
		//没有指定软盘文件或打开失败,在模拟器目录生成临时软盘
		if (lpFloppy)
			delete[] lpFloppy;

		extern unsigned char ImageEmpty[2025];
		zlib_filefunc64_def def;
		memset(&def, 0, sizeof(def));
		struct Img {
			BYTE* buf;
			ZPOS64_T lPos;
		};
		static Img img;
		def.zopen64_file = [](voidpf opaque, const void* filename, int mode) {
			return (voidpf)&img;
		};
		def.zseek64_file = [](voidpf opaque, voidpf stream, ZPOS64_T offset, int origin) {
			if (origin == SEEK_SET)
				img.lPos = 0;
			else if (origin == SEEK_END)
				img.lPos = sizeof(ImageEmpty);
			img.lPos += offset;
			return 0L;
		};
		def.ztell64_file = [](voidpf opaque, voidpf stream) {
			return (ZPOS64_T)img.lPos;
		};
		def.zclose_file = [](voidpf opaque, voidpf stream) { return 0; };
		def.zread_file = [](voidpf opaque, voidpf stream, void* buf, uLong size) {
			memcpy(buf, &ImageEmpty[img.lPos], size);
			img.lPos += size;
			return size;
		};
		auto tz = unzOpen2_64(ImageEmpty, &def);
		if (!tz)
			return false;
		if (unzGoToFirstFile(tz) != UNZ_OK || unzOpenCurrentFile(tz) != UNZ_OK) {
			unzClose(tz);
			return false;
		}
		char tempu[512];
		unzGetCurrentFileInfo(tz, 0, tempu, sizeof(tempu), 0, 0, 0, 0);
		tempu[sizeof(tempu) - 1] = 0;
		unz_file_info ufo;
		unzGetCurrentFileInfo(tz, &ufo, 0, 0, 0, 0, 0, 0);
		nSizeNormal = ufo.uncompressed_size;
		if (lpFloppy)
			delete[] lpFloppy;
		lpFloppy = (LPBYTE)new char[nSizeNormal + 1];
		size = nSizeNormal;
		ZeroMemory(lpFloppy, nSizeNormal);
		unzReadCurrentFile(tz, lpFloppy, ufo.uncompressed_size);
		unzCloseCurrentFile(tz);
		unzClose(tz);
		char szPath[8192];
		strcpy(szPath, m_strPathConfig);
		auto p = strrchr(szPath, '\\');
		p[1] = 0;
		strcat(szPath, "Temp.img");
		strImageFile = szPath;
		FloppyIMGLoad(lpFloppy);
		return FloppyIMGLoad();
	}
	if (lpFloppyWrite) {
		//保存数据
		if(enumTitleStatus == EnumTitleStatusReadOnly)
			return lpFloppy;
		fwrite(lpFloppyWrite, size, 1, fp);
		fclose(fp);
		return lpFloppy;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size < nSizeNormal)
		size = nSizeNormal;
	//加载软盘
	LPBYTE lplpFloppy;
	if (!(lplpFloppy = (LPBYTE)new char[size + 1])) {
		return nullptr;
	}
	ZeroMemory(lplpFloppy, size);
	fseek(fp, 0, SEEK_SET);
	::fread(lplpFloppy, size, 1, fp);
	fclose(fp);
	MenuImgListSave(strImageFile.c_str());
	SetTitle(!bChange ? "" : "-切换新磁盘");
	if (lpFloppy)
		delete[] lpFloppy;
	lpFloppy = lplpFloppy;

	return lplpFloppy;
}

char* Tools::GetBitNumber(BYTE data)
{
	static char szText[1024];
	char szTemp[10];
	sprintf(szText, " %02X=", data);
	for (int i = 7; i>=0; i--)
	{
		auto d = 1<<i;
		if (!(data & d))
			continue;
		sprintf(szTemp, "%02X+", d);
		strcat(szText, szTemp);
	}
	return szText;
}

//设置窗口标题提示当前状态
void Tools::SetTitle(const std::string strSet, DWORD dwDelayHide) {
	if (!mWndTitle || !bCanSetTitle)
		return;
	if (dwDelayHide != 0) {
		SetTimer(mWndMain, nIDTimerTitleDelay, 50, nullptr);
		dwTickTitle = GetTickCount() + dwDelayHide;
	}
	static std::string strRom;
	char szTitle[10240];
	GetWindowTextA(mWndTitle, szTitle, sizeof(szTitle));
	auto pszTitle = strchr(szTitle, ':');//FCeux
	if (pszTitle)
	{
		pszTitle[0] = 0;
		pszTitle++;
		strRom = pszTitle;
	}
	else
	{
		pszTitle = szTitle;
		strcpy(pszTitle, strRom.c_str());
		strcat(pszTitle, "-");
	}
	if (strImageFile != "") {
		auto pName = strrchr(strImageFile.c_str(), '\\');
		if (pName)
		{
			pName++;
			strcat(pszTitle, " ");
			strcat(pszTitle, pName);
		}
	}
	if (enumTitleStatus != EnumTitleStatusNone) {
		strcat(pszTitle, "[");
		strcat(pszTitle, enumTitleStatus == EnumTitleStatusReadOnly ? "只读]" : "读写]");
	}
	if (strSet != "") {
		auto p = strrchr(strSet.c_str(), '\\');
		strcat(pszTitle, "-");
		strcat(pszTitle, p ? ++p : strSet.c_str());
	}
	SetWindowTextA(mWndTitle, pszTitle);
}

//扫描拖入的目录下所有文件,保存到vectorPath
void Tools::ScanFolder(const std::string& strFile, std::vector<std::string>& vectorPath) {
	auto strDir = strFile;
	strDir += "\\*.*";
	WIN32_FIND_DATAA ffd;
	auto hFind = FindFirstFileA(strDir.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
		return;
	do
	{
		strDir = ffd.cFileName;
		if (strDir == "." || strDir == "..")
			continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			strDir = strFile + "\\";
			ScanFolder(strDir + ffd.cFileName, vectorPath);
			continue;
		}

		strDir = strFile + "\\";
		vectorPath.push_back(strDir + ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);
}

//扫描vectorPath中的文件,把有效的ROM(压缩文件会被解压)并在模拟器RomsOrder目录下有 mapperNumber-文件名.扩展名 保存ROM
void Tools::ScanRoms(std::vector<std::string>& vectorPath) {
	SetTitle("转换完成...");
}

bool Tools::OnDropFiles(const char* file) {
	char *ftmp = new char[strlen(file)+1];
	std::string strFile = file;
	strcpy(ftmp, strFile.c_str());
	auto attr = GetFileAttributesA(strFile.c_str());
	if (INVALID_FILE_ATTRIBUTES == attr)
		return false;
	if (FILE_ATTRIBUTE_DIRECTORY & attr) {
		if (!bScanRom)
			return false;
		std::vector<std::string> vectorPath;
		ScanFolder(ftmp, vectorPath);
		ScanRoms(vectorPath);
		return false;
	}
	strlwr(ftmp);
	auto pName = strrchr(ftmp, '.');
	pName[0] = 0;
	pName++;
	std::string strTemp = pName;
	delete[] ftmp;
	if (strTemp != "ima" && strTemp != "img" && strTemp != "imz")
		return false;
	if (bFloppySave) {
		FloppyIMGSave();
	}
	strImageFile = strFile;
	return FloppyIMGLoad(nullptr, true);
}

//Hook主窗口过程
LRESULT Tools::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto c = GetDlgItem(hwnd, nIDIdentify);
	auto mWNDPROC = (WNDPROC)GetWindowLongPtr(c, GWLP_USERDATA);
	if (nullptr != MapperBase::pMapper && MapperBase::pMapper->WindowProc(hwnd, uMsg, wParam, lParam)){
		return 0;
	}
	switch (uMsg) {
	case WM_KILLFOCUS: {
		bFocus = false;
		break;
	}
	case WM_SETFOCUS: {
		bFocus = true;
		break;
	}
	case WM_TIMER: {
		if (nIDTimerTitleDelay == wParam) {
			if (GetTickCount() >= dwTickTitle) {
				dwTickTitle = 0;
				SetTitle("");
				KillTimer(hwnd, wParam);
			}
			return 0;
		}
		if (wParam == nIDTimerLoadROM || nIDTimerReleadROM == wParam) {
			KillTimer(hwnd, wParam);
			char szFile[1024];
			do {
				if (wParam == nIDTimerLoadROM) {
					if (GetPrivateProfileStringA("Config", "AutoLoad", "", szFile, sizeof(szFile), m_strPathConfig) < 0)
						break;
					if (strcmpi(szFile, "true") != 0)
						break;
				}
				if (GetPrivateProfileStringA("Config", "ROM", "", szFile, sizeof(szFile), m_strPathConfig) > 0) {
					extern bool ALoad(const char* nameo, char* innerFilename = 0, bool silent = false);
					auto attr = GetFileAttributesA(szFile);
					if (INVALID_FILE_ATTRIBUTES != attr)
						ALoad(szFile);
				}
			} while (false);
			return 0;
		}
		break;
	}
	case WM_CLOSE: {
		if (mWndDebug == hwnd)
			ShowWindow(mWndDebug, SW_HIDE);
		else if (hwnd == mWndMain)
			Close();
		break;
	}
	case WM_DROPFILES: {
		//拖入文件/目录
		auto len = DragQueryFileA((HDROP)wParam, 0, 0, 0) + 1;
		char* ftmp = new char[len];
		DragQueryFileA((HDROP)wParam, 0, ftmp, len);
		std::string strFie = ftmp;
		delete ftmp;
		auto p = strrchr(strFie.c_str(), '.');
		if (p) {
			if (strcmpi(p, ".nes") == 0 || strcmpi(p, ".rom") == 0 || strcmpi(p, ".zip") == 0) {
				break;
			}
		}
		if (nullptr != MapperBase::pMapper && MapperBase::pMapper->OnDropFiles(strFie)) {
			MenuImgListSave(strFie.c_str());
			SetForegroundWindow(mWndMain);
			return 0;
		}
		if (OnDropFiles(strFie.c_str())) {
			SetForegroundWindow(mWndMain);
			return 0;
		}
		if (strcmpi(p, ".nes") == 0 || strcmpi(p, ".rom") == 0) {
			break;
		}
		return 0;
	}
	case WM_MBUTTONUP: {
		bIDC_CHECK_IRQ = !bIDC_CHECK_IRQ;
		break;
	}
	case WM_SYSKEYDOWN:
	{
		if (VK_F4 == wParam)
			return 0;
		break;
	}
	//按下捕获鼠标,驱动ROM鼠标接口
	case WM_LBUTTONDOWN:
	{
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		//ClientToScreen(hwnd, &pt);
		auto h = ChildWindowFromPoint(mWndMain, pt);
		if (NULL != mWndView && h != mWndView)
			break;
		if (!bCaptureMouse)
			break;
		bLButtonDown = true;
		if (!bCapture) {
			bCapture = true;
			ShowCursor(FALSE);
		}
		else if (bRButtonDown) {
			bCapture = false;
			ShowCursor(TRUE);
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		bLButtonDown = false;
		break;
	}
	case WM_RBUTTONDOWN: {
		bRButtonDown = true;
		break;
	}
	case WM_RBUTTONDBLCLK: {
		if (!IsWindow(mWndMain))
			break;
		//弹出调试菜单
		auto hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_STAR));
		if (!hMenu)
			break;
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		ClientToScreen(mWndMain, &pt);
		TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, mWndMain, nullptr);
		DestroyMenu(hMenu);
		return 1;
		break;
	}
	case WM_ACTIVATE: {
		//窗口进入未激活状态,释放鼠标捕捉
		if (WA_INACTIVE == wParam) {
			bCapture = false;
			ShowCursor(TRUE);
		}
		return 0;
		break;
	}
	case WM_RBUTTONUP:
	{
		//按下鼠标左键再按右键释放鼠标捕捉
		bRButtonDown = false;
		if (bLButtonDown) {
			bCapture = false;
			ShowCursor(TRUE);
			return 0;
		}
		if (!bCapture)
			break;
#ifndef _DEBUG
		return 0;
#endif
		if (!IsWindow(mWndMain))
			break;
		return 1;
	}
	case WM_COMMAND: {
		if (WindowProcCommand(hwnd, uMsg, wParam, lParam))
			return 1;
		break;
	}
	}
	if (mWNDPROC == LONG_PTR(0))
		return 0;
	return CallWindowProc(mWNDPROC, hwnd, uMsg, wParam, lParam);
}

SHORT Tools::ShowCursor(bool bShow) {
	static bool bShowPrev = true;
	if (bShowPrev == bShow)
		return 0;
	::ShowCursor(bShow);
	bShowPrev = bShow;
	return 0;
}
void Tools::Close(void)
{
	if (hEventVoice) {
		bsThreadVoiceExit = true;
	}
	if (mWNDPROC != 0)
	{
		SetWindowLongPtr(mWndMain, GWLP_WNDPROC, mWNDPROC);
		mWNDPROC = 0;
		mWndMain = 0;
		mWndTitle = 0;
	}
	if (mWNDPROCView != 0)
	{
		SetWindowLongPtr(mWndView, GWLP_WNDPROC, mWNDPROCView);
		mWNDPROCView = 0;
		mWndView = 0;
	}
	if (lpFloppy) {
		if (bFloppySave)
			FloppyIMGSave();
		delete[] lpFloppy;
		lpFloppy = nullptr;
	}
}
UINT Tools::NewIDMenu(int nMax) {
	static UINT id = nIDMenuStart;
	auto d = id;
	id += nMax;
	return d;
}

void Tools::HookWindow(HWND hMain, HWND hView)
{
	if (mWNDPROC != 0)
		return;

	mWndMain = hMain;
	if (mWndMain)
	{
		auto c = CreateWindow(_T("STATIC"), _T(""), WS_CHILD, 0, 0, 0, 0, mWndMain, (HMENU)nIDIdentify, (HINSTANCE)GetWindowLong(mWndMain, GWL_HINSTANCE), nullptr);
		mWNDPROC = SetWindowLongPtr(mWndMain, GWLP_WNDPROC, LONG_PTR((WNDPROC)[](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			return pTools->WindowProc(hwnd, uMsg, wParam, lParam);
			}));
		SetWindowLongPtr(c, GWLP_USERDATA, (LONG_PTR)mWNDPROC);
		mWndTitle = mWndMain;
		auto menu = GetMenu(mWndMain);
		//
		auto sub = GetSubMenu(menu, GetMenuItemCount(menu) - 1);
		InsertMenuA(sub, 0, MF_STRING | MF_BYPOSITION, UINT_PTR(nIDMenuExplorNES), "打开ROM目录");
		InsertMenuA(sub, 0, MF_STRING | MF_BYPOSITION, UINT_PTR(nIDMenuExplorImg), "打开IMG目录");
		//加载过的镜像列表
		mMenuImgList = CreateMenu();
		AppendMenu(menu, MF_POPUP | MF_STRING, (UINT_PTR)mMenuImgList, _T("ImgList"));
		MenuImgListUpdate();

		SetTimer(mWndMain, nIDTimerLoadROM, 500, NULL);
	}
	mWndView = hView;
	if (mWndView)
	{
		mWNDPROCView = SetWindowLongPtr(mWndView, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)[](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			return pTools->WindowProc(hwnd, uMsg, wParam, lParam);
			});
		SetWindowLongPtr(mWndView, GWLP_USERDATA, (LONG_PTR)mWNDPROCView);
	}
}
void Tools::MenuImgListCommand(UINT nID) {
	if (!MapperBase::pMapper)
		return;
	char strName[256];
	auto j = nID - nIDMenuImgList;
	GetMenuStringA(mMenuImgList, j, strName, sizeof(strName) / sizeof(strName[0]), MF_BYPOSITION);
	if (MapperBase::pMapper->OnDropFiles(strName))
		MenuImgListSave(strName);
	else
		OnDropFiles(strName);
}

void Tools::MenuImgListUpdate() {
	char szFile[1024], szTemp[512];
	for (int j = GetMenuItemCount(mMenuImgList) - 1; j >= 0; j--) {
		DeleteMenu(mMenuImgList, 0, MF_BYPOSITION);
	}
	for (int i = 0; ; i++) {
		sprintf(szTemp, "ImagePath%02d", i);
		if (i >= nIDMenuImgListMax || GetPrivateProfileStringA("Config", szTemp, "", szFile, sizeof(szFile), m_strPathConfig) <= 0)
			break;
		auto attr = GetFileAttributesA(szFile);
		if (INVALID_FILE_ATTRIBUTES == attr)
			continue;
		AppendMenuA(mMenuImgList, MF_STRING, (UINT_PTR)(nIDMenuImgList+i), szFile);
	}
}

void Tools::MenuImgListSave(const char* pszPath) {
	int nStart = 0;
	if (nullptr != pszPath) {
		nStart++;
		WritePrivateProfileString("Config", "ImagePath00", pszPath, m_strPathConfig);
	}
	char strName[256], szTemp[512];;
	for (int j = 0; j < GetMenuItemCount(mMenuImgList); j++) {
		GetMenuStringA(mMenuImgList, j, strName, sizeof(strName) / sizeof(strName[0]), MF_BYPOSITION);
		if (nullptr != pszPath && 0 == lstrcmpiA(pszPath, strName))
			continue;
		sprintf(szTemp, "ImagePath%02d", nStart++);
		WritePrivateProfileStringA("Config", szTemp, strName, m_strPathConfig);
	}
	sprintf(szTemp, "ImagePath%02d", nStart++);
	WritePrivateProfileStringA("Config", szTemp, nullptr, m_strPathConfig);
	if (nullptr != pszPath) {
		MenuImgListUpdate();
	}
}

#include <shlobj.h>
bool Tools::ExplorerFolder(const char* pszPath) {
	auto b = false;
	auto path = ILCreateFromPathA(pszPath);
	if (path) {
		auto hr = SHOpenFolderAndSelectItems(path, 0, 0, 0);
		b = SUCCEEDED(hr);
	}
	ILFree(path);
	return b;
}


bool Tools::WindowProcCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto cmd = HIWORD(wParam);
	if (cmd != 0)
		return false;
	auto id = LOWORD(wParam);
	if (id >= nIDMenuImgList && id <= nIDMenuImgList + nIDMenuImgListMax) {
		MenuImgListCommand(id);
		return false;
	}
	switch (id)
	{
	case nIDMenuExplorImg: {
		ExplorerFolder(strImageFile.c_str());
		break;
	}
	case nIDMenuExplorNES: {
		ExplorerFolder(strNesFile.c_str());
		break;
	}
	default:
		break;
	}
	switch (cmd) {
	case BN_CLICKED: {
		if (id == IDC_CHECK_IRQ)
			bIDC_CHECK_IRQ = (BST_CHECKED == IsDlgButtonChecked(mWndDebug, IDC_CHECK_IRQ));
		else if (id == IDC_CHECK_KEYBOARD)
			bIDC_CHECK_KEYBOARD = (BST_CHECKED == IsDlgButtonChecked(mWndDebug, IDC_CHECK_KEYBOARD));
		else if (id == IDC_CHECK_LogOut)
			bIDC_CHECK_LogOut = (BST_CHECKED == IsDlgButtonChecked(mWndDebug, IDC_CHECK_LogOut));
		break;
	}
	}
	switch (id) {
	case ID_Menu_DebugDialog:
	{
		if (IsWindow(mWndDebug)) {
			ShowWindow(mWndDebug, SW_SHOW);
			return 1;
		}
		DWORD dwID;
		CreateThread(nullptr, 0, s_WindowDialogThreadProc, 0, 0, &dwID);
		return 1;
	}
	break;
	}
	return false;
}


void Tools::Reset()
{
	if (bFloppySave)
		FloppyIMGSave();
	bCaptureMouse = false;
	bLButtonDown = bRButtonDown = bCapture = false;
	nIndexDebugString = 0;
}
char* Tools::LogMAPData(MAPData& mapData)
{
	if (mapData.size() <= 0)
		return "";
	sprintf(szBuf, "size=%d ", mapData.size());
	char szTemp[5];
	for (MAPData::iterator c=mapData.begin(); c!=mapData.end();++c) {
		sprintf(szTemp, "%04X ", c->first);
		strcat(szBuf, szTemp);
	}
	strcat(szBuf, "\r\n");
	//DEBUGOUTBUFF(szBuf);
	return szBuf;
}
char* Tools::WriteBits(DWORD dwData, int nBits) {
	char szNum[50];
	strcpy(szBuf, "000000000000000000000000000000000000000000000000");
	itoa(dwData, szNum, 2);
	strcpy(&szBuf[nBits - strlen(szNum)], szNum);
	DEBUGOUTBUFF(szBuf);
	return szBuf;
}

char * Tools::WriteVByte(VectorByte &vByte, bool bClear) {
	if (vByte.size() <= 0)
		return "";
	sprintf(szBuf, "%04d buff ", vByte.size());
	char szTemp[5];
	for (auto i = 0; i < vByte.size(); i++) {
		sprintf(szTemp, "%02X ", vByte[i]);
		strcat(szBuf, szTemp);
	}
	DEBUGOUTBUFF(szBuf);
	if(bClear)
		vByte.clear();
	return szBuf;
}
bool Tools::isDisableIRQ()
{
	return BST_CHECKED == IsDlgButtonChecked(mWndDebug, IDC_CHECK_IRQ);
}

static VectorByte vByteVoice;

DWORD WINAPI Tools::sThreadVoice(LPVOID lParam)
{
	auto &bFirst = *(bool*)lParam;
	bFirst = false;
	while (TRUE)
	{
		WaitForSingleObject(hEventVoice, INFINITE);
		if (bsThreadVoiceExit) {
			CloseHandle(hEventVoice);
			bsThreadVoiceExit = false;
			break;
		}
		auto vByte = vByteVoice;
		vByteVoice.clear();
		ResetEvent(hEventVoice);
		WORD wSize = vByte.size();
		if (wSize < 20)
			continue;
		const int nBase = 1024 * 1024;
		static short pcm_out[nBase];
		static unsigned char pVoice[128 * 1024];
		int pcm_size;
		//wSize--;
		pVoice[0] = wSize >> 8;
		pVoice[1] = wSize;
		wSize = *(WORD*)pVoice;
		memset(pVoice, 0xff, sizeof(pVoice));
		//*(WORD*)pVoice = wSize;
		DWORD dwT = GetTickCount();
		wo_reset();
		for (int i = 1; i < vByte.size(); i++)
			pVoice[i - 1] = vByte[i];
		//mTools.WriteVByte(vByte, false);
		lpc10_d6_synth(pcm_out, &pcm_size, pVoice, wSize);
		wo_write(pcm_out, pcm_size);
		//DebugString("GetTickCount=%d pcm_size=%d", GetTickCount() - dwT, pcm_size);
		vByte.clear();
	}
	return 0;
}

void Tools::OnLoadROM(const char* pszPath) {
	if (strNesFile != pszPath) {
		WritePrivateProfileStringA("Config", "ROM", pszPath, m_strPathConfig);
		strNesFile = pszPath;
	}
	if (nullptr != MapperBase::pMapper)
		MapperBase::pMapper->OnLoadROM(pszPath);
}

void Tools::ReleadROM() {
	SetTimer(mWndMain, nIDTimerReleadROM, 500, nullptr);
}

void Tools::PlayVoice(VectorByte& vbByte) {

	static bool bFirst = true;
	if (bFirst)
	{
		//bFirst = false;
		DWORD dwID;
		CreateThread(NULL, 0, sThreadVoice, &bFirst, 0, &dwID);
		hEventVoice = CreateEvent(NULL, TRUE, FALSE, _T("BBKVOICE"));
		if (0 == wo_open(10000)) {
		}
		while (bFirst);
	}
	vByteVoice = vbByte;
	vbByte.clear();
	SetEvent(hEventVoice);
}

bool BaseKeyInput::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!bUseKeyBoard)
		return false;
	if (uMsg == WM_KEYUP || uMsg == WM_KEYDOWN) {
		KeyInfo ki;
		ki.cbKey = MapVirtualKey(wParam, 0);
		ki.bPress = (uMsg == WM_KEYDOWN);
		listKeyInfo.push_back(ki);
		//if (uMsg == WM_KEYUP) {
		//	DebugString("BaseKeyInput::WindowProc KeyUp");
		//}
		return false;
	}
	return false;
}

BYTE* BaseKeyInput::GetKeyboard(bool bTestRelease) {
	memset(keys, 0, sizeof(keys));
	if (!bTestRelease && !listKeyInfo.empty()) {
		auto& i = listKeyInfo.front();
		keys[i.cbKey] = i.bPress;
		listKeyInfo.pop_front();
		DebugString("BaseKeyInput::GetPressKey key=%02X bPress=%d", i.cbKey, i.bPress);
		return keys;
	}
	if (!pTools->bFocus)
		return keys;
	if (bIDC_CHECK_KEYBOARD)
		return keys;
	HRESULT hr = lpdid->GetDeviceState(sizeof(keys), keys);
	return keys;
}
bool BaseKeyInput::GetReleaseKey(BYTE& key) {
	GetKeyboard(true);
	for (int i = 0; i < sizeof(keysPress); i++) {
		if (keysPress[i] && !keys[i]) {
			keysPress[i] = false;
			key = i;
			//DebugString("BaseKeyInput::GetReleaseKey key=%02X", key);
			return true;
		}
	}
	return false;
}
bool BaseKeyInput::GetPressKey(BYTE& key) {
	GetKeyboard();
	for (int i = 0; i < sizeof(keys); i++) {
		if (keys[i] && !keysPress[i]) {
			key = i;
			keysPress[i] = true;
			return true;
		}
	}
	return false;
}

PInitFunction _pPInitFunction[200];
static int nPInitFunction = 0;
void AddInitFunction(PInitFunction pPInitFunction)
{
	_pPInitFunction[nPInitFunction++] = pPInitFunction;
}