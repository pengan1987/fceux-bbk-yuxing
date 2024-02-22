/*
未使用的Mapper ID
161 169 171 179
20 39 54 55 56 63 81 84 98 100 102 104 109 110 122 124 126 127 128 129 130 131 135 158   182 218 223 224 236 237 239 247 248 251 255
*/

#include "MapperBase.h"
#include <Imagehlp.h>
#pragma comment(lib,"imagehlp.lib")
MapperBase* MapperBase::pMapper = nullptr;
bool bMemViewUpdateMemory = false, bDisassembleReadMem = false;
readfunc MapperBase::AReadBack[0xffff];
writefunc MapperBase::BWriteBack[0xffff];

const int CHRRAMIndex = 5;
const int PRGRAMIndex = 6;
const int WORKRAMIndex = 7;
bool bRTIStopRun = false, bDisableSetVram = false;

#include "MapperBaseHook.inc"

bool FceuxCheckSum()
{
	HWND hWnd = GetConsoleWindow();
	if (hWnd)
		ShowWindow(hWnd, SW_HIDE);
	DWORD HeadChksum = 1, Chksum = 0;
	TCHAR text[512];
	GetModuleFileName(GetModuleHandle(NULL), text, sizeof(text));
	if (MapFileAndCheckSum(text, &HeadChksum, &Chksum) != CHECKSUM_SUCCESS)
	{
		MessageBox(NULL, "Check error!", "Error", MB_OK);
		return false;
	}
	if (HeadChksum != Chksum)
	{
		MessageBox(NULL, "执行文件被修改.", "警告", MB_OK);
	}
	return true;
}
static bool bCheckSum = FceuxCheckSum();

void setpageptr_MapperBase(int s, uint32 A, uint8* p, int ram) {
	if (nullptr == MapperBase::pMapper)
		return;
}

const char* UnicodeToAnsi(const WCHAR* pszWide)
{
	if (pszWide == NULL)
		return "";

	int nSize = ::WideCharToMultiByte(CP_ACP, 0, pszWide, -1, NULL, 0, NULL, NULL);
	CHAR* pszString = NULL;
	pszString = new CHAR[nSize + 1];
	pszString[nSize] = 0;
	WideCharToMultiByte(CP_ACP, 0, pszWide, -1, pszString, nSize, NULL, NULL);

	static std::string strRet;
	strRet = pszString;
	delete[]pszString;

	return strRet.c_str();
}

#include <Shellapi.h>
extern int main(int argc, char* argv[]);
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int /*nCmdShow*/) {
	LPWSTR* szArglist;
	int nArgs;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist) {
		char* argv[1];
		argv[0] = "";
		return main(1, argv);
	}
	//GetModuleFileNameA(NULL, )
	std::string *strArgs = new std::string[nArgs];
	char **argv = new LPSTR[nArgs];
	for (int i = 0; i < nArgs; i++) {
		strArgs[i].assign(UnicodeToAnsi(szArglist[i]));
		argv[i] = (char*)strArgs[i].c_str();
	}
	return main(nArgs, argv);
}

BMAPPINGLocal* GetBMAPPINGLocal(int num) {
	auto p = AddMapper(0, nullptr);
	auto iter = p.find(num);
	//overclock_enabled = false;
	if (iter != p.end())
		return iter->second(num);
	return nullptr;
}

MAP_Mapper& AddMapper(INT no, pCreateMapper pFun) {
	static MAP_Mapper _MAP_Mapper;
	if (nullptr == pFun)
	{
		return _MAP_Mapper;
	}
	_MAP_Mapper.insert(std::make_pair(no, pFun));
	return _MAP_Mapper;
}

LRESULT APIENTRY FilterEditCtrlProcCheatText(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP)
{
	bool through = true;
	INT_PTR result = 0;
	switch (msg)
	{
	case WM_PASTE:
	{
		if (!OpenClipboard(hwnd))
			break;

		HANDLE handle = GetClipboardData(CF_TEXT);
		if (!handle)
			break;

		// get the original clipboard string
		char* clipStr = (char*)GlobalLock(handle);

		// check if the text in clipboard has illegal characters
		int len = strlen(clipStr);
		auto pfind = strstr(clipStr, "-");
		while (pfind)
		{
			auto psecond = strstr(pfind + 1, "-");
			if (psecond)
			{
				pfind[0] = ':';
				strcpy(&pfind[1], &psecond[1]);
				auto hMen = GlobalAlloc(GMEM_MOVEABLE, len);
				strcpy((LPSTR)GlobalLock(hMen), clipStr);
				GlobalUnlock(handle);
				SetClipboardData(CF_TEXT, hMen);
				GlobalUnlock(hMen);
				break;
			}
			GlobalUnlock(handle);
			break;
		}
		CloseClipboard();
		break;
	}
	//extern WNDPROC FilterEditCtrlProc;
	}
	extern LRESULT APIENTRY FilterEditCtrlProc(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP);
	return CallWindowProc(FilterEditCtrlProc, hwnd, msg, wP, lP);
}

typedef BOOL(WINAPI* tSetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
tSetWindowPos sysSetWindowPos = (tSetWindowPos)SetWindowPos;
BOOL WINAPI mySetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	if (pTools->mWndMain == hWnd) {
		//MessageBox(NULL, "", "WindowProc", MB_OK);
		if (X < 0 || Y < 0)
			return TRUE;
		uFlags = uFlags | SWP_NOZORDER | SWP_NOOWNERZORDER;
	}
	//if (pTools->mWndMain == hWnd && bIDC_CHECK_LogOut) {
	//	//MessageBox(NULL, "", "WindowProc", MB_OK);
	//	uFlags = uFlags | SWP_NOZORDER | SWP_NOOWNERZORDER;
	//}
	return sysSetWindowPos(hWnd, NULL, X, Y, cx, cy, uFlags);// | SWP_NOACTIVATE));
}
//static bool bSetWindowPos = Mhook_SetHook((PVOID*)&sysSetWindowPos, mySetWindowPos);

extern int CreateMainWindow();
typedef BOOL(WINAPI* tCreateMainWindow)();
tCreateMainWindow sysCreateMainWindow = (tCreateMainWindow)CreateMainWindow;
int HookCreateMainWindow() {
	auto i = sysCreateMainWindow();
	pTools->HookWindow(hAppWnd, NULL);

	//extern void LoadConfig(const char* filename);
	//LoadConfig("Y:\\Temp\\FCEUX2.3\\bin\\RDebug\\fceux.cfg");

	//auto menu = GetMenu(hAppWnd);
	//TCHAR strName[256];
	//for (int i = 0; i < GetMenuItemCount(menu); i++) {
	//	auto sub = GetSubMenu(menu, i);
	//	GetMenuString(menu, i, strName, sizeof(strName) / sizeof(strName[0]), MF_BYPOSITION);
	//	DebugString("%d:%s", GetMenuItemID(menu, i), strName);
	//	for (int j = 0; j < GetMenuItemCount(sub); j++) {
	//		GetMenuString(sub, j, strName, sizeof(strName) / sizeof(strName[0]), MF_BYPOSITION);
	//		DebugString("%d:%s", GetMenuItemID(sub, j), strName);
	//	}
	//}

	extern BMAPPINGLocal bmap[];
	auto p = AddMapper(0, nullptr);
	bool bFind;
	for(int i=0; i<256; i++){
		auto tmp = bmap;
		//if (p.find(i) != p.end()) {
		//	continue;
		//}
		bFind = false;
		while (tmp->init){
			if (tmp->number == i) {
				bFind = true;
				break;
			}
			tmp++;
		}
		//if(!bFind)
		//	DebugString("%d", i);
	}
	return i;
}
static bool bCreateMainWindow = Mhook_SetHook((PVOID*)&sysCreateMainWindow, HookCreateMainWindow);

typedef BOOL(WINAPI* tTrackPopupMenu)(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT* prcRect);
tTrackPopupMenu sysTrackPopupMenu = (tTrackPopupMenu)TrackPopupMenu;
BOOL WINAPI myTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT* prcRect)
{
	//if (!bWndMainTrackPopupMenu)
	//	return sysTrackPopupMenu(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
	//bWndMainTrackPopupMenu = false;
	return sysTrackPopupMenu(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
}
//static bool bTrackPopupMenu = Mhook_SetHook((PVOID*)&sysTrackPopupMenu, myTrackPopupMenu);

extern void SetMainWindowText();
typedef void(*tSetMainWindowText)();
tSetMainWindowText sysSetMainWindowText = (tSetMainWindowText)SetMainWindowText;
void  mySetMainWindowText()
{
	sysSetMainWindowText();
	if (nullptr != MapperBase::pMapper)
		pTools->SetTitle("");
}
static bool bSetMainWindowText = Mhook_SetHook((PVOID*)&sysSetMainWindowText, mySetMainWindowText);

void MapperBase::OnRTS() {
}
void MapperBase::OnRTI() {
	if (bNMI)
		bNMI = false;
	if (bIRQ)
		bIRQ = false;
}
void MapperBase::OnNMI(WORD addr) {
	bNMI = true;
}
void MapperBase::OnIRQ(WORD addr) {
	bIRQ = true;
}

void MapperBase::OnRUNBefore(WORD addr) {
	auto b1 = RdMem(addr);
	extern HWND hDebug;
	switch (b1) {
	case 0x0: {  /* BRK */
		if (!IsWindow(hDebug))
			return;
		extern void UpdateOtherDebuggingDialogs();
		FCEUI_Debugger().step = true;
		FCEUI_SetEmulationPaused(0);
		UpdateOtherDebuggingDialogs();
		return;
	}
	}
}
void MapperBase::CleanListStackInfo() {
	auto sp = 0x100 + X.S + 1;
	static WORD wSP[0x100];
	memset(wSP, 0, sizeof(wSP));
	for (auto it = mListStackInfo.begin(); it != mListStackInfo.end();) {
		if (wSP[it->wSP - 0x100] == 1) {
			it = mListStackInfo.erase(it);
			continue;
		}
		wSP[it->wSP - 0x100] = 1;
		if (it->wSP < sp) {
			it = mListStackInfo.erase(it);
			continue;
		}
		WORD ad = ReadRam(it->wSP);
		ad |= ReadRam(it->wSP + 1) << 8;
		if (ad != it->wAddr) {
			it = mListStackInfo.erase(it);
			continue;
		}
		++it;
	}
}

void MapperBase::OnRUN(WORD addr, BYTE value) {
	extern HWND hDebug;
	if (IsWindow(hDebug)) {
		StackInfo s;
		s.wAddr = addr;
		s.bIRQ = bIRQ;
		s.bNMI = bNMI;
		listLatest10.push_front(s);
		if (listLatest10.size() > 100)
			listLatest10.pop_back();
	}
	switch (value) {
	case 0x40: {  /* RTI */
		pMapper->OnRTI();
		return;
	}
	case 0x60: { /* RTS */
		pMapper->OnRTS();
		return;
	}
	case 0x20: { /* JSR */
		WORD wValid = 0x100 + X.S + 1;
		StackInfo s;
		s.wSP = wValid;
		WORD ad = ReadRam(wValid++);
		ad |= ReadRam(wValid) << 8;
		s.wAddr = ad;
		s.bIRQ = bIRQ;
		s.bNMI = bNMI;
		mListStackInfo.push_front(s);
		if (mListStackInfo.size() % 10 == 0)
			CleanListStackInfo();
		return;
	}
	}
}

void MapperBase::LogCallStack() {
	char szText[1024] = { "MapperBase::LogCallStack: " };
	char szTemp[64];
	for (auto it = mListStackInfo.begin(); it != mListStackInfo.end(); ++it)
	{
		int wAddr = it->wAddr - 2;
		sprintf(szTemp, "%04X ", wAddr);
		strcat(szText, szTemp);
	}
	DebugString(szText);
}

bool MapperBase::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg != WM_COMMAND)
		return false;
	auto cmd = HIWORD(wParam);
	if (cmd != 0)
		return false;
	auto id = LOWORD(wParam);
	if(id == nIDMenu_Joy) {
		SetReadHandler(0x4016, 0x4016, AReadBack[0x4016]);
		SetReadHandler(0x4017, 0x4017, AReadBack[0x4017]);
		SetWriteHandler(0x4016, 0x4016, BWriteBack[0x4016]);
		SetWriteHandler(0x4017, 0x4017, BWriteBack[0x4017]);
		mTools.bCaptureMouse = false;
		return true;
	}
	if (id == nIDMenu_Mouse) {
		OnMenuCommandMouse(id);
		return true;
	}
	if (id == nIDMenu_KeyBoard) {
		OnMenuCommandKeyBoard(id);
		return true;
	}
	return false;
}

void MapperBase::BaseReset() {
	pMapper->Reset(true);
	mListStackInfo.clear();
	listLatest10.clear();
	bIRQ = bNMI = false;
}

void MapperBase::BaseClose() {
	FCEU_MemDelete(pCHRRAM);
	FCEU_MemDelete(pPRGRAM);
	FCEU_MemDelete(pWORKRAM);
	Close();
}
extern uint8 PRGIsRAM[32];  /* This page is/is not PRG RAM. */

BYTE MapperBase::ReadRam(WORD A)
{
	if (PRGIsRAM[A >> 11] && Page[A >> 11])
		return Page[A >> 11][A];
	else
		return RAM[A];
}
void MapperBase::WriteRam(WORD A, BYTE V)
{
	if (PRGIsRAM[A >> 11] && Page[A >> 11])
		Page[A >> 11][A] = V;
	else
		RAM[A] = V;
}
void MapperBase::Write(uint32 addr, uint8 data)
{
	if (bMemViewUpdateMemory || bDisassembleReadMem || !pMapper->OnWriteRam(addr, data))
		CartBW(addr, data);
}
BYTE MapperBase::Read(uint32 addr)
{
	BYTE data;
	if (bMemViewUpdateMemory || bDisassembleReadMem || !pMapper->OnReadRam(addr, data))
		return CartBR(addr);
	return data;
}

void MapperBase::BasePower() {
	CopyMemory(AReadBack, ARead, sizeof(AReadBack));
	CopyMemory(BWriteBack, BWrite, sizeof(BWriteBack));
	hWndMain = hAppWnd;
	pTools->bCaptureMouse = false;
	memset(pCHRRAM, 0xff, GetCHRRamSize());
	memset(pPRGRAM, 0xff, GetPRGRamSize());
	memset(pWORKRAM, 0xff, GetWorkRamSize());
	AddCustomMenu(AddCustomMenu_Joy);
	Power();
}
void MapperBase::AddCustomMenu(UINT e) {
	auto menu = GetMenu(mTools.mWndMain);
	auto sub = GetSubMenu(menu, GetMenuItemCount(menu) - 2);

	if (e & AddCustomMenu_Joy && -1 == GetMenuState(sub, nIDMenu_Joy, MF_BYCOMMAND)) {
		nIDMenu_Joy = mTools.NewIDMenu();
		AppendMenuA(sub, MF_STRING | MF_BYPOSITION, UINT_PTR(nIDMenu_Joy), "FC手柄接口-连接手柄");
	}
	if (e & AddCustomMenu_KeyBoard && -1 == GetMenuState(sub, nIDMenu_KeyBoard, MF_BYCOMMAND)) {
		nIDMenu_KeyBoard = mTools.NewIDMenu();
		AppendMenuA(sub, MF_STRING | MF_BYPOSITION, UINT_PTR(nIDMenu_KeyBoard), "FC手柄接口-连接键盘");
	}
	if (e & AddCustomMenu_Mouse && -1 == GetMenuState(sub, nIDMenu_Mouse, MF_BYCOMMAND)) {
		nIDMenu_Mouse = mTools.NewIDMenu();
		AppendMenuA(sub, MF_STRING | MF_BYPOSITION, UINT_PTR(nIDMenu_Mouse), "FC手柄接口-连接鼠标");
	}
}

MapperBase::MapperBase() : mTools(*pTools){
	pMapper = this;
	nIDMenu_Mouse = nIDMenu_KeyBoard = nIDMenu_Joy = 0xffffff;
}

MapperBase::~MapperBase() {
	auto menu = GetMenu(mTools.mWndMain);
	auto sub = GetSubMenu(menu, GetMenuItemCount(menu) - 2);
	if (nIDMenu_Joy != 0xffffff) {
		DeleteMenu(sub, nIDMenu_Joy, MF_BYCOMMAND);
	}
	if (nIDMenu_KeyBoard != 0xffffff) {
		DeleteMenu(sub, nIDMenu_KeyBoard, MF_BYCOMMAND);
	}
	if (nIDMenu_Mouse != 0xffffff) {
		DeleteMenu(sub, nIDMenu_Mouse, MF_BYCOMMAND);
	}
	nIDMenu_Mouse = nIDMenu_KeyBoard = nIDMenu_Joy = 0xffffff;
}