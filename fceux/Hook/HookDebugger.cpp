/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Ben Parnell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "debugger.cpp"

#if 1

#include "MapperBase.h"
#include "../resource.h"
#include <list>

void UpdateStackList();

void ReloadSybol(HWND hwndDlg)
{
	if (!ramBankNames)
		return;
	Name* pName = ramBankNames;
	Name* pNameC;
	ramBankNames = NULL;
	HWND hList = GetDlgItem(hwndDlg, IDC_DEBUGGER_LIST_SYBOL);
	SendDlgItemMessageA(hwndDlg, IDC_DEBUGGER_LIST_SYBOL, LB_RESETCONTENT, 0, 0);
	SendDlgItemMessageA(hwndDlg, IDC_DEBUGGER_LIST_SYBOL_DATA, LB_RESETCONTENT, 0, 0);
	char szText[100];
	while (pName)
	{
		Name** pNameCur = &ramBankNames;
		if (pName->offsetNumeric >= 0x8000)
		{
			int bank = getBank(pName->offsetNumeric);
			pNameCur = &pageNames[bank];
		}
		if (!*pNameCur)
		{
			*pNameCur = pName;
		}
		else
		{
			pNameC = *pNameCur;
			while (pNameC)
			{
				if (!pNameC->next)
				{
					pNameC->next = pName;
					break;
				}
				pNameC = pNameC->next;
			}
		}
		auto pszName = pName->name;
		auto CtrlID = IDC_DEBUGGER_LIST_SYBOL;
		if (strstr(pName->name, "_D") || pName->offsetNumeric < 0x1000)
		{
			CtrlID = IDC_DEBUGGER_LIST_SYBOL_DATA;
			pszName += 2;
		}
		else if (strstr(pName->name, "_F"))
		{
			pszName += 2;
		}
		strcpy(szText, pszName);
		strcpy(pName->name, szText);
		sprintf(szText, "%s[%04X]", pName->name, pName->offsetNumeric);
		
		int i = SendDlgItemMessageA(hwndDlg, CtrlID, LB_ADDSTRING, 0, LPARAM(szText));
		SendDlgItemMessageA(hwndDlg, CtrlID, LB_SETITEMDATA, i, LPARAM(pName->offsetNumeric));
		pNameC = pName;
		pName = pName->next;
		pNameC->next = NULL;
	}
}

static HWND hwndDlg;
HHOOK HHOOKKeyboardProc;
static bool bBreak = false;
static LRESULT CALLBACK KeyboardProc(int code,
	WPARAM wParam,
	LPARAM lParam
)
{
	if(!IsWindow(hwndDlg) || code != HC_ACTION || !(0x40000000 & lParam))
		return CallNextHookEx(HHOOKKeyboardProc, code, wParam, lParam);
	switch (wParam)
	{
	case VK_F10:
	{
		if (!FCEUI_EmulationPaused())
			break;
		bBreak = true;
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_DEBUGGER_STEP_OVER, BN_CLICKED), 0);
		UpdateDebugger(true);
		break;
	}
	case VK_F9:
		break;
	case VK_F5:
	{
		if (!FCEUI_EmulationPaused())
			FCEUI_SetEmulationPaused(1);
		else
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_DEBUGGER_RUN, BN_CLICKED), 0);
		break;
	}
	case VK_F11:
	{
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_DEBUGGER_STEP_IN, BN_CLICKED), 0);
		if (FCEUI_Debugger().stepout)
			bBreak = true;
		break;
	}
	default:
		break;
	}
	return CallNextHookEx(HHOOKKeyboardProc, code, wParam, lParam);
}

static void OnSeekPC(HWND hwndDlg) {
	char str[5], strList[512];
	GetDlgItemText(hwndDlg, IDC_DEBUGGER_VAL_PCSEEK, str, 5);
	auto addr = offsetStringToInt(BT_C, str);
	if (addr == -1)
		return;
	for (int i = SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
		SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_GETTEXT, i, (LPARAM)strList);
		auto a = strtol(strList, nullptr, 16);
		if (addr == a) {
			SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_DELETESTRING, i, 0);
		}
	}
	SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_INSERTSTRING, 0, (LPARAM)str);
}

static void OnSeekPCClicked(HWND hwndDlg) {
	auto sel = SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_GETCURSEL, 0, 0);
	if (sel < 0)
		return;
	char strList[512];
	SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_PC, LB_GETTEXT, sel, (LPARAM)strList);
	auto addr = offsetStringToInt(BT_C, strList);
	if (addr == -1)
		return;
	if (FCEUI_EmulationPaused())
		UpdateRegs(hwndDlg);
	auto pc = X.PC;
	X.PC = addr;
	Disassemble(hDebug, IDC_DEBUGGER_DISASSEMBLY, IDC_DEBUGGER_DISASSEMBLY_VSCR, addr - 10 < 0 ? 0 : addr - 10);
	X.PC = pc;
}

static bool OnCommandBefore(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto CtrlID = LOWORD(wParam);
	switch (HIWORD(wParam))
	{
	case BN_CLICKED: {
		if (DEBUGCONSOLEWINDOW == CtrlID) {
			int s = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED ? SW_SHOW : SW_HIDE;
			HWND hWnd = GetConsoleWindow();
			if (hWnd)
				ShowWindow(hWnd, s);
			break;
		}
		if (CtrlID == IDC_DEBUGGER_SEEK_TO) {
			OnSeekPC(hwndDlg);
			break;
		}
		if (IDC_DEBUGGER_STEP_OVER == CtrlID || IDC_DEBUGGER_STEP_IN == CtrlID)
			UpdateStackList();
		break;
	}
	case LBN_SELCHANGE: {
		if (CtrlID == LIST_DEBUGGER_PC) {
			OnSeekPCClicked(hwndDlg);
			break;
		}
		break;
	}
	case LBN_DBLCLK:
	{
		if (CtrlID != IDC_DEBUGGER_LIST_SYBOL && CtrlID != IDC_DEBUGGER_LIST_SYBOL_DATA)
			break;
		int i = SendDlgItemMessage(hwndDlg, CtrlID, LB_GETCURSEL, 0, 0);
		int nAddr = SendDlgItemMessage(hwndDlg, CtrlID, LB_GETITEMDATA, i, 0);
		char szText[10];
		sprintf(szText, "%04X", nAddr);
		HWND hEdit = GetDlgItem(hwndDlg, IDC_DEBUGGER_VAL_PCSEEK);
		SetWindowTextA(hEdit, szText);
		SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_DEBUGGER_SEEK_TO, BN_CLICKED), 0);
		//Disassemble(hDebug, IDC_DEBUGGER_DISASSEMBLY, IDC_DEBUGGER_DISASSEMBLY_VSCR, tmp);
	}
	}
	return false;
}

static bool OnCommandAfter(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto CtrlID = LOWORD(wParam);
	switch (HIWORD(wParam))
	{
	case BN_CLICKED: {
		if (CtrlID == IDC_DEBUGGER_RELOAD_SYMS) {
			ReloadSybol(hwndDlg);
			break;
		}
	}
	}
	return false;
}

static INT_PTR CALLBACK s_DebuggerCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		::hwndDlg = hwndDlg;
		SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, (HINSTANCE)GetWindowLongPtr(hwndDlg, GWLP_HINSTANCE), GetCurrentThreadId());
		if(!ramBankNames)
			loadNameFiles();
		ReloadSybol(hwndDlg);
		break;
	}
	case WM_COMMAND:
	{
		if (OnCommandBefore(hwndDlg, uMsg, wParam, lParam))
			return 1;
		break;
	}
	case WM_SIZE:
	{
		if (SIZE_MAXIMIZED == wParam)
			wParam = SIZE_RESTORED;
		break;
	}
	case WM_CLOSE:
	case WM_QUIT: {
		extern char LoadedRomFName[2048];
		extern int storePreferences(const char* romname);
		break_on_cycles = false;
		storePreferences(mass_replace(LoadedRomFName, "|", ".").c_str());
		break;
	}
	}
	auto res = DebuggerCallB(hwndDlg, uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		if (OnCommandAfter(hwndDlg, uMsg, wParam, lParam))
			return 1;
		break;
	}
	default:
		break;
	}
	return res;
}

void UpdateStackList()
{
	if (!IsWindow(hDebug) || nullptr == MapperBase::pMapper)
		return;

	auto p = MapperBase::pMapper;
	p->CleanListStackInfo();
	extern std::vector <std::pair<unsigned int, std::string>> bookmarks;
	bookmarks.clear();
	SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_BOOKMARKS, LB_RESETCONTENT, 0, 0);
	char szText[100];
	DWORD n;
	for (auto it = p->mListStackInfo.begin(); it != p->mListStackInfo.end(); ++it)
	{
		int wAddr = it->wAddr - 2;
		sprintf(szText, "%04X", wAddr - 10);
		bookmarks.push_back(std::make_pair(wAddr - 10, ""));
		sprintf(szText, "%04X", wAddr);
		if(it->bNMI)
			strcat(szText, "[NMI]");
		else if (it->bIRQ)
			strcat(szText, "[IRQ]");
		SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_BOOKMARKS, LB_ADDSTRING, 0, LPARAM(szText));
	}
	auto &listLatest10 = MapperBase::pMapper->listLatest10;
	if (listLatest10.size() > 0) {
		SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_BOOKMARKS, LB_ADDSTRING, 0, LPARAM("以下最近执行语句"));
		bookmarks.push_back(std::make_pair(0x8000, ""));
	}
	for (auto it = listLatest10.begin(); it != listLatest10.end(); ++it) {
		auto addr = it->wAddr & 0xffff;
		sprintf(szText, "%04X", addr);
		if (it->bNMI)
			strcat(szText, "[NMI]");
		else if (it->bIRQ)
			strcat(szText, "[IRQ]");
		bookmarks.push_back(std::make_pair(addr - 10, ""));
		SendDlgItemMessageA(hwndDlg, LIST_DEBUGGER_BOOKMARKS, LB_ADDSTRING, 0, LPARAM(szText));
	}
}

typedef void(*tGoToDebuggerBookmark)(HWND hwnd);
tGoToDebuggerBookmark sysGoToDebuggerBookmark = (tGoToDebuggerBookmark)GoToDebuggerBookmark;
void  myGoToDebuggerBookmark(HWND hwnd)
{
	auto pc = X.PC;
	int selectedItem = SendDlgItemMessage(hwnd, LIST_DEBUGGER_BOOKMARKS, LB_GETCURSEL, 0, 0);
	if (selectedItem == LB_ERR) return;
	extern unsigned int getBookmarkAddress(unsigned int index);
	unsigned int n = getBookmarkAddress(selectedItem);
	X.PC = n + 10;
	Disassemble(hwnd, IDC_DEBUGGER_DISASSEMBLY, IDC_DEBUGGER_DISASSEMBLY_VSCR, n);
	X.PC = pc;
}
static bool bGoToDebuggerBookmark = Mhook_SetHook((PVOID*)&sysGoToDebuggerBookmark, myGoToDebuggerBookmark);

typedef void(* tDoDebug)(uint8 halt);
tDoDebug sysDoDebug = (tDoDebug)DoDebug;
void HookDoDebug(uint8 halt)
{
	if (!debugger_open)
	{
		//pUpdateStackList = UpdateStackList;
		hDebug = CreateDialog(fceu_hInstance, MAKEINTRESOURCE(IDD_DEBUGGER), NULL, s_DebuggerCallB);
		if (DbgSizeX != -1 && DbgSizeY != -1)
			SetWindowPos(hDebug, 0, 0, 0, DbgSizeX, DbgSizeY, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}
	sysDoDebug(halt);
}
static bool bDoDebug = Mhook_SetHook((PVOID*)&sysDoDebug, HookDoDebug);

typedef void(*tDisassemble)(HWND hWnd, int id, int scrollid, unsigned int addr);
tDisassemble sysDisassemble = (tDisassemble)Disassemble;
void HookDisassemble(HWND hWnd, int id, int scrollid, unsigned int addr)
{
	bDisassembleReadMem = true;
	//if (!bBreak)
	{
		sysDisassemble(hWnd, id, scrollid, addr);
		bDisassembleReadMem = false;
		return;
	}
	bBreak = false;
	static int addrPrev;

	//extern DebugSystem* debugSystem;
	extern unsigned int PC_pointerOffset;
	extern int InstructionUp(int from);
	RECT rect;
	GetClientRect(GetDlgItem(hDebug, IDC_DEBUGGER_DISASSEMBLY), &rect);
	unsigned int lines = (rect.bottom - rect.top) / debugSystem->disasmFontHeight;
	if (PC_pointerOffset > lines - 3)
		PC_pointerOffset = lines - 3;

	// keep the relative position of the ">" pointer inside the Disassembly window
	int addrTest = X.PC;
	for (int i = PC_pointerOffset; i > 0; i--)
	{
		addrTest = InstructionUp(addrTest);
		if (abs(addrPrev - int(addrTest)) < 5)
		{
			addr = addrPrev;
			break;
		}
	}
	addrPrev = addr;
	sysDisassemble(hWnd, id, scrollid, addr);
	bDisassembleReadMem = false;
}
static bool bDisassemble = Mhook_SetHook((PVOID*)&sysDisassemble, HookDisassemble);

typedef void(*tBreakHit)(int bp_num);
tBreakHit sysBreakHit = (tBreakHit)BreakHit;
void  myBreakHit(int bp_num)
{
	UpdateStackList();
	FCEUI_SetEmulationPaused(EMULATIONPAUSED_PAUSED); //mbg merge 7/19/06 changed to use EmulationPaused()
	FCEUD_DebugBreakpoint(bp_num);
	//return sysBreakHit(bp_num);
}
static bool bBreakHit = Mhook_SetHook((PVOID*)&sysBreakHit, myBreakHit);

typedef void(*tFCEUI_SetEmulationPaused)(int val);
tFCEUI_SetEmulationPaused sysFCEUI_SetEmulationPaused = (tFCEUI_SetEmulationPaused)FCEUI_SetEmulationPaused;
void  myFCEUI_SetEmulationPaused(int val)
{
	UpdateStackList();
	return sysFCEUI_SetEmulationPaused(val);
}
//static bool bFCEUI_SetEmulationPaused = Mhook_SetHook((PVOID*)&sysFCEUI_SetEmulationPaused, myFCEUI_SetEmulationPaused);

#endif