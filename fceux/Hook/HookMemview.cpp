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

#include "memview.cpp"

#if 1

#include "TabCtrl.h"
#include "MapperBase.h"
#include "../resource.h"

static int nStartY = 0, nCols = 16;
static CTabCtrl gCTabCtrl;
static HBRUSH hHBRUSH = NULL;

const UINT nTimerAddress = 101;

typedef void(*tDoMemView)();
tDoMemView sysDoMemView = (tDoMemView)DoMemView;

INT_PTR OnCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


#define HIGHLIGHT_ACTIVITY_NUM_COLORS 16
#define PREVIOUS_VALUE_UNDEFINED -1

struct MemAddressInfo
{
	int nMemMode;
	WORD wAddr;
};
static MemAddressInfo gMemAddressInfo[10] = {};

int GetXOffset() {
	return 6 + 2 + nCols * 3 + 2 + 1;
}

typedef void(*tUpdateMemoryView)(int draw_all);
tUpdateMemoryView sysUpdateMemoryView = (tUpdateMemoryView)UpdateMemoryView;
void HookUpdateMemoryView(int draw_all)
{
	if (!hMemView) return;
	const int MemFontWidth = debugSystem->HexeditorFontWidth + HexCharSpacing;
	const int MemFontHeight = debugSystem->HexeditorFontHeight + HexRowHeightBorder;
	const char hex[] = "0123456789ABCDEF";
	const COLORREF CBackColor = RGB(HexBackColorR, HexBackColorG, HexBackColorB);
	const COLORREF CForeColor = RGB(HexForeColorR, HexForeColorG, HexForeColorB);
	int i, j;
	int byteValue;
	int byteHighlightingValue;
	char str[512];

	if (PreviousCurOffset != CurOffset)
		resetHighlightingActivityLog();
	int nOffset = GetXOffset();
	strcpy(str, "         ");
	SetTextColor(HDataDC, 0xff0000);	//addresses text color #000000 = black, #FFFFFF = white
	SetBkColor(HDataDC, CBackColor);		//addresses back color
	for (int i = 0; i < nCols; i++) {
		sprintf(str, "%02X", i);
		TextOut(HDataDC, MemFontWidth * 8 + i * MemFontWidth * 3, 0, str, strlen(str));
	}
	for (i = CurOffset; i < CurOffset + DataAmount; i += nCols)
	{
		const int MemLineRow = MemFontHeight * ((i - CurOffset) / nCols + 1);
		int MemLinePos = 8 * MemFontWidth;
		int pos = i - CurOffset;
		if ((PreviousCurOffset != CurOffset) || draw_all)
		{
			SetBkColor(HDataDC, CBackColor);		//addresses back color
			if (i < MaxSize)
				SetTextColor(HDataDC, CForeColor);	//addresses text color #000000 = black, #FFFFFF = white
			else
				SetTextColor(HDataDC, RGB(HexBoundColorR, HexBoundColorG, HexBoundColorB));	//addresses out of bounds
			sprintf(str, "%06X:  ", i);
			for (int i = 0; i <= nCols; i++) {
				strcat(str, "   ");
			}
			strcat(str, "  :  ");
			TextOut(HDataDC, 0, MemLineRow, str, strlen(str));
		}
		for (j = 0; j < nCols; j++)
		{
			bDisassembleReadMem = true;
			byteValue = GetMemViewData(i + j);
			bDisassembleReadMem = false;
			if (MemView_HighlightActivity && ((PreviousValues[pos] != byteValue) && (PreviousValues[pos] != PREVIOUS_VALUE_UNDEFINED)))
				byteHighlightingValue = HighlightedBytes[pos] = MemView_HighlightActivity_FadingPeriod;
			else
				byteHighlightingValue = HighlightedBytes[pos];

			if ((CursorEndAddy == -1) && (CursorStartAddy == i + j))
			{
				//print up single highlighted text
				if (TempData != PREVIOUS_VALUE_UNDEFINED)
				{
					// User is typing New Data
					// 1st nibble
					SetBkColor(HDataDC, RGB(255, 255, 255));
					SetTextColor(HDataDC, RGB(255, 0, 0));
					str[0] = hex[(byteValue >> 4) & 0xF];
					str[1] = 0;
					TextOut(HDataDC, MemLinePos, MemLineRow, str, 1);
					// 2nd nibble
					SetBkColor(HDataDC, CForeColor);
					SetTextColor(HDataDC, CBackColor);
					str[0] = hex[(byteValue >> 0) & 0xF];
					str[1] = 0;
					TextOut(HDataDC, MemLinePos + MemFontWidth, MemLineRow, str, 1);
				}
				else
				{
					// Single Byte highlight
					// 1st nibble
					SetBkColor(HDataDC, RGB(0, 0, 0));
					SetTextColor(HDataDC, RGB(255, 255, 255));
					str[0] = hex[(byteValue >> 4) & 0xF];
					str[1] = 0;
					TextOut(HDataDC, MemLinePos, MemLineRow, str, 1);
					// 2nd nibble
					SetBkColor(HDataDC, BGColorList[pos]);
					SetTextColor(HDataDC, TextColorList[pos]);
					str[0] = hex[(byteValue >> 0) & 0xF];
					str[1] = 0;
					TextOut(HDataDC, MemLinePos + MemFontWidth, MemLineRow, str, 1);
				}

				// single address highlight - right column
				SetBkColor(HDataDC, RGB(0, 0, 0));
				SetTextColor(HDataDC, RGB(255, 255, 255));
				str[0] = chartable[byteValue];
				if ((u8)str[0] < 0x20) str[0] = 0x2E;
				//				if ((u8)str[0] > 0x7e) str[0] = 0x2E;
				str[1] = 0;
				TextOut(HDataDC, (nOffset + j) * MemFontWidth, MemLineRow, str, 1);

				PreviousValues[pos] = PREVIOUS_VALUE_UNDEFINED; //set it to redraw this one next time
			}
			else if (draw_all || (PreviousValues[pos] != byteValue) || byteHighlightingValue)
			{
				COLORREF tmpcolor = TextColorList[pos];
				SetBkColor(HDataDC, BGColorList[pos]);
				// print up normal text
				if (byteHighlightingValue)
				{
					// fade out 1 step
					if (MemView_HighlightActivity_FadeWhenPaused || !FCEUI_EmulationPaused() || JustFrameAdvanced)
						byteHighlightingValue = (--HighlightedBytes[pos]);

					if (byteHighlightingValue > 0)
					{
						// if the byte was changed in current frame, use brightest color, even if the "fading period" demands different color
						// also use the last color if byteHighlightingValue points outside the array of predefined colors
						if (byteHighlightingValue == MemView_HighlightActivity_FadingPeriod - 1 || byteHighlightingValue >= HIGHLIGHT_ACTIVITY_NUM_COLORS)
							tmpcolor = highlightActivityColors[HIGHLIGHT_ACTIVITY_NUM_COLORS - 1];
						else
							tmpcolor = highlightActivityColors[byteHighlightingValue];
					}
				}
				SetTextColor(HDataDC, tmpcolor);
				str[0] = hex[(byteValue >> 4) & 0xF];
				str[1] = hex[(byteValue >> 0) & 0xF];
				str[2] = 0;
				TextOut(HDataDC, MemLinePos, MemLineRow, str, 2);

				str[0] = chartable[byteValue];
				if ((u8)str[0] < 0x20) str[0] = 0x2E;
				//				if ((u8)str[0] > 0x7e) str[0] = 0x2E;
				str[1] = 0;
				TextOut(HDataDC, (nOffset + j) * MemFontWidth, MemLineRow, str, 1);

				PreviousValues[pos] = byteValue;
			}
			MemLinePos += MemFontWidth * 3;
			pos++;
		}
	}

	SetTextColor(HDataDC, RGB(0, 0, 0));
	SetBkColor(HDataDC, RGB(0, 0, 0));
	MoveToEx(HDataDC, 0, 0, NULL);
	PreviousCurOffset = CurOffset;
	return;
}
static bool bUpdateMemoryView = Mhook_SetHook((PVOID*)&sysUpdateMemoryView, HookUpdateMemoryView);

static int nPRGRamSize = 0;
void SetMemViewPRamSize(int nSize) {
	nPRGRamSize = nSize;
}
void SetPRamPages(HWND hwnd) {
	auto sel = SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_SIZE, CB_GETCURSEL, 0, 0);
	auto bRom = (BST_CHECKED == SendDlgItemMessageA(hwnd, IDC_CHECK_ROM, BM_GETCHECK, 0, 0));
	auto nSize = 8 << sel;
	auto nPages = bRom ? head.ROM_size : nPRGRamSize / (nSize * 1024);
	SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_PAGE, CB_RESETCONTENT, 0, 0);
	for (int i = 0; i < nPages; i++) {
		char szText[50];
		sprintf(szText, "%03d", i);
		SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_PAGE, CB_ADDSTRING, 0, (LPARAM)szText);
	}
	SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_PAGE, CB_SETCURSEL, 0, 0);
}

#include "MapperBase.h"
static void OnSetPRam(HWND hwnd) {
	if (nPRGRamSize <= 0)
		return;
	char szTemp[512];
	if (SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_ADDR, WM_GETTEXT, sizeof(szTemp), (LPARAM)szTemp) <= 0)
		return;
	auto nAddr = strtol(szTemp, nullptr, 16);
	auto sel = SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_SIZE, CB_GETCURSEL, 0, 0);
	auto nSize = 8 << sel;
	sel = SendDlgItemMessageA(hwnd, IDC_COMBO_BANK_PAGE, CB_GETCURSEL, 0, 0);
	auto bRom = (BST_CHECKED == SendDlgItemMessageA(hwnd, IDC_CHECK_ROM, BM_GETCHECK, 0, 0));
	switch (nSize)
	{
	case 8: {
		bRom ? setprg8(nAddr, sel) : setprg8r(PRGRAMIndex, nAddr, sel);
		break;
	}
	case 16: {
		bRom ? setprg16(nAddr, sel) : setprg16r(PRGRAMIndex, nAddr, sel);
		break;
	}
	case 32: {
		bRom ? setprg32(nAddr, sel) : setprg32r(PRGRAMIndex, nAddr, sel);
		break;
	}
	default:
		break;
	}
}

static INT_PTR OnCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto cmd = HIWORD(wParam); 
	auto id = LOWORD(wParam);

	switch (id) {
	case IDC_CHECK_ROM: {
		if (cmd == BN_CLICKED)
			SetPRamPages(hwnd);
		break;
	}
	case IDC_BUTTON_SetBank: {
		if (cmd == BN_CLICKED)
			OnSetPRam(hwnd);
		break;
	}
	case IDC_COMBO_BANK_SIZE: {
		if (cmd == CBN_SELCHANGE)
			SetPRamPages(hwnd);
		break;
	}
	case IDC_COMBO_GoToAddress: {
		if (cmd == CBN_SELCHANGE) {
			SetTimer(hwnd, nTimerAddress, 10, NULL);
			break;
		}
		if (cmd != CBN_EDITCHANGE)
			break;
		SetTimer(hwnd, nTimerAddress, 500, NULL);
		break;
	}
	case MENU_MV_VIEW_RAM:
	case MENU_MV_VIEW_PPU:
	case MENU_MV_VIEW_OAM:
	case MENU_MV_VIEW_ROM:
	{
		const int MemFontHeight = debugSystem->HexeditorFontHeight + HexRowHeightBorder;
		int _EditingMode = wParam - MENU_MV_VIEW_RAM;
		// Leave NES Memory
		if (_EditingMode == MODE_NES_MEMORY && EditingMode != MODE_NES_MEMORY)
			CreateCheatMap();
		// Enter NES Memory
		if (_EditingMode != MODE_NES_MEMORY && EditingMode == MODE_NES_MEMORY)
			ReleaseCheatMap();
		EditingMode = _EditingMode;
		for (int i = MODE_NES_MEMORY; i <= MODE_NES_FILE; i++)
			if (EditingMode == i)
			{
				CheckMenuRadioItem(GetMenu(hMemView), MENU_MV_VIEW_RAM, MENU_MV_VIEW_ROM, MENU_MV_VIEW_RAM + i, MF_BYCOMMAND);
				break;
			}

		MaxSize = GetMaxSize(EditingMode);

		if (CurOffset >= MaxSize - DataAmount) CurOffset = MaxSize - DataAmount;
		if (CurOffset < 0) CurOffset = 0;
		if (CursorEndAddy >= MaxSize) CursorEndAddy = -1;
		if (CursorStartAddy >= MaxSize) CursorStartAddy = MaxSize - 1;

		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = (SIF_RANGE | SIF_PAGE);
		si.nMin = 0;
		si.nMax = MaxSize / nCols;
		si.nPage = ClientHeight / MemFontHeight;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		resetHighlightingActivityLog();
		UpdateColorTable();
		UpdateCaption();
		return 1;
	}
	}
	if (CBN_SELCHANGE == cmd && IDC_COMBO_COLS == id)
	{
		HWND hDlg = (HWND)lParam;
		auto cur = SendMessage(hDlg, CB_GETCURSEL, 0, 0);
		if (CB_ERR == cur)
			return 0;
		TCHAR szText[30] = { 0 };
		SendMessage(hDlg, CB_GETLBTEXT, cur, (LPARAM)szText);
		nCols = strtol(szText, nullptr, 10);
		SendMessage(hwnd, WM_TIMER, 0x100, 0);
		return 1;
	}
	return 0;
}

static INT_PTR OnNotifyTab(HWND hwndDlg, NMHDR * pNMHDR)
{
	switch (pNMHDR->code)
	{
	case TCN_SELCHANGE: {
		TCITEM ti;
		ti.mask = TCIF_PARAM;
		auto h = GetDlgItem(hwndDlg, IDC_TAB_MEM_VIEW);
		auto sel = SendMessage(h, TCM_GETCURSEL, 0, 0);
		SendMessage(h, TCM_GETITEM, sel, (LPARAM)&ti);
		auto mod = ti.lParam >> 16;
		if (mod > 3)
			mod = 0;
		OnCommand(hMemView, WM_COMMAND, MENU_MV_VIEW_RAM+ mod, NULL);
		SetHexEditorAddress((int)(ti.lParam & 0xffff));
		break;
	}
	case NM_RCLICK:
		break;
	}
	return TRUE;
}

LRESULT CALLBACK s_StaticViewProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	auto proc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_SETTEXT:
		return SendMessage(GetParent(hwnd), uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, hHBRUSH);
		EndPaint(hwnd, &ps);
		HookUpdateMemoryView(1);
		return 1;
	}
	case WM_MOUSEWHEEL:
	{
		auto i = (short)HIWORD(wParam);///WHEEL_DELTA;
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.fMask = SIF_ALL;
		si.cbSize = sizeof(SCROLLINFO);
		GetScrollInfo(hwnd, SB_VERT, &si);
		if (i < 0)si.nPos += 2;
		if (i > 0)si.nPos -= 2;
		if (si.nPos < si.nMin) si.nPos = si.nMin;
		if ((si.nPos + (int)si.nPage) > si.nMax) si.nPos = si.nMax - si.nPage; //added cast
		CurOffset = si.nPos * nCols;
		if (CurOffset >= MaxSize - DataAmount) CurOffset = MaxSize - DataAmount;
		if (CurOffset < 0) CurOffset = 0;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		UpdateColorTable();
		return 0;
	}
	case WM_VSCROLL: {
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.fMask = SIF_ALL;
		si.cbSize = sizeof(SCROLLINFO);
		GetScrollInfo(hwnd, SB_VERT, &si);
		switch (LOWORD(wParam)) {
		case SB_ENDSCROLL:
		case SB_TOP:
		case SB_BOTTOM: break;
		case SB_LINEUP: si.nPos--; break;
		case SB_LINEDOWN:si.nPos++; break;
		case SB_PAGEUP: si.nPos -= si.nPage; break;
		case SB_PAGEDOWN: si.nPos += si.nPage; break;
		case SB_THUMBPOSITION: //break;
		case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
		}
		if (si.nPos < si.nMin) si.nPos = si.nMin;
		if ((si.nPos + (int)si.nPage) > si.nMax) si.nPos = si.nMax - si.nPage; //mbg merge 7/18/06 added cast
		CurOffset = si.nPos * nCols;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		UpdateColorTable();
		return 0;
	}
	case WM_SIZE:
	{
		const int MemFontWidth = debugSystem->HexeditorFontWidth;
		const int MemFontHeight = debugSystem->HexeditorFontHeight + HexRowHeightBorder;
		if (wParam == SIZE_RESTORED)										//If dialog was resized
		{
			RECT newMemViewRect;
			GetWindowRect(hwnd, &newMemViewRect);						//Get new size
			MemViewSizeX = newMemViewRect.right - newMemViewRect.left;	//Store new size (this will be used to store in the .cfg file)	
			MemViewSizeY = newMemViewRect.bottom - newMemViewRect.top;
		}
		ClientHeight = HIWORD(lParam) - MemFontHeight;//排除第一行标题头
		if (DataAmount != ((ClientHeight / MemFontHeight) * nCols))
		{
			DataAmount = ((ClientHeight / MemFontHeight) * nCols);
			if (CurOffset >= MaxSize - DataAmount) CurOffset = MaxSize - DataAmount;
			if (CurOffset < 0) CurOffset = 0;
			//mbg merge 7/18/06 added casts:
			TextColorList = (COLORREF*)realloc(TextColorList, DataAmount * sizeof(COLORREF));
			BGColorList = (COLORREF*)realloc(BGColorList, DataAmount * sizeof(COLORREF));
			PreviousValues = (int*)realloc(PreviousValues, (DataAmount) * sizeof(int));
			HighlightedBytes = (unsigned int*)realloc(HighlightedBytes, (DataAmount) * sizeof(unsigned int));
			resetHighlightingActivityLog();
		}
		//Set vertical scroll bar range and page size
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = (SIF_RANGE | SIF_PAGE);
		si.nMin = 0;
		si.nMax = MaxSize / nCols;
		si.nPage = ClientHeight / MemFontHeight;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		UpdateColorTable();
		return 0;
	}
	}
	auto r = MemViewCallB(hwnd, uMsg, wParam, lParam);
	if (!r)
		return r;

	return CallWindowProc(proc, hwnd, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK s_MemViewCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		uMsg = WM_CREATE;
		SendDlgItemMessageA(hwndDlg, IDC_COMBO_BANK_ADDR, CB_LIMITTEXT, 4, 0);
		for (int i = 8; i <= 0xf; i++) {
			char szText[50];
			sprintf(szText, "%04X", 0x1000 * i);
			SendDlgItemMessageA(hwndDlg, IDC_COMBO_BANK_ADDR, CB_ADDSTRING, 0, (LPARAM)szText);
			if (i > 0xa)
				continue;
			sprintf(szText, "%dK", 8<<(i - 8));
			SendDlgItemMessageA(hwndDlg, IDC_COMBO_BANK_SIZE, CB_ADDSTRING, 0, (LPARAM)szText);
		}
		SendDlgItemMessageA(hwndDlg, IDC_COMBO_BANK_ADDR, CB_SETCURSEL, 0, 0);
		SendDlgItemMessageA(hwndDlg, IDC_COMBO_BANK_SIZE, CB_SETCURSEL, 0, 0);
		SetPRamPages(hwndDlg);

		SendDlgItemMessageA(hwndDlg, IDC_COMBO_GoToAddress, CB_LIMITTEXT, 4, 0);
		TCITEM tc;
		tc.mask = TCIF_TEXT | TCIF_PARAM;
		char szText[50];
		tc.pszText = szText;
		for (int i = 0; i < 5; i++)
		{
			sprintf(szText, "Mem%d", i + 1);
			tc.lParam = LPARAM(&gMemAddressInfo[i]);
			SendDlgItemMessageA(hwndDlg, IDC_TAB_MEM_VIEW, TCM_INSERTITEM, i, (LPARAM)&tc);
		}
		const int cols[] = { 16,32,48 };
		for (int i = 0; i < sizeof(cols)/sizeof(int); i++)
		{
			sprintf(szText, "%d", cols[i]);
			SendDlgItemMessageA(hwndDlg, IDC_COMBO_COLS, CB_ADDSTRING, 0, (LPARAM)szText);
		}
		SendDlgItemMessageA(hwndDlg, IDC_COMBO_COLS, CB_SETCURSEL, 0, 0);
		break;
	}
	case WM_TIMER:
	{
		KillTimer(hwndDlg, wParam);
		if (nTimerAddress == wParam) {
			auto h = GetDlgItem(hwndDlg, IDC_COMBO_GoToAddress);
			TCHAR szText[128] = { 0 }, szIndex[128];
			GetWindowText(h, szText, sizeof(szText));
			auto addr = _tcstol(szText, nullptr, 16);
			SetHexEditorAddress(addr);
			auto cnt = SendMessage(h, CB_GETCOUNT, 0, 0);
			for (int i = cnt - 1; i >= 0; i--) {
				SendMessage(h, CB_GETLBTEXT, i, (LPARAM)szIndex);
				auto ad = _tcstol(szIndex, nullptr, 16);
				if (ad != addr)
					continue;
				SendMessage(h, CB_DELETESTRING, i, 0);
				break;
			}
			SendMessage(h, CB_INSERTSTRING, 0, (LPARAM)szText);

			h = GetDlgItem(hwndDlg, IDC_TAB_MEM_VIEW);
			auto sel = SendMessage(h, TCM_GETCURSEL, 0, 0);
			TCITEM ti;
			ti.mask = TCIF_PARAM | TCIF_TEXT;
			ti.lParam = addr | (EditingMode<<16);
			ti.pszText = szText;
			ti.cchTextMax = sizeof(szText) / sizeof(szText[0]);
			SendMessage(h, TCM_SETITEM, sel, (LPARAM)&ti);
			break;
		}
		if (wParam != 0x100)
			break;
		RECT rc;
		HWND hTab = GetDlgItem(hwndDlg, IDC_TAB_MEM_VIEW);
		GetWindowRect(hTab, &rc);
		int nHeight = rc.bottom - rc.top;
		POINT pt;
		pt.x = rc.left;
		pt.y = rc.top;
		const int MemFontHeight = debugSystem->HexeditorFontHeight + HexRowHeightBorder;
		ScreenToClient(hwndDlg, &pt);
		GetClientRect(hwndDlg, &rc);
		MoveWindow(hTab, pt.x, pt.y, rc.right, nHeight, TRUE);
		nStartY = nHeight + pt.y;
		auto tp = nStartY + 5;
		auto bt = rc.bottom - nStartY;
		bt = bt - bt % MemFontHeight + 3 - MemFontHeight;
		MoveWindow(hMemView, pt.x, tp, rc.right, bt, TRUE);
		InvalidateRect(hMemView, &rc, TRUE);
		s_StaticViewProc(hMemView, WM_SIZE, 0, MAKELPARAM(rc.right, bt));
		HookUpdateMemoryView(1);
		return 1;
	}
	case WM_SIZE:
	{
		SetTimer(hwndDlg, 0x100, 0, NULL);
		return 1;
	}
	case WM_NOTIFY:
	{
		auto pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->idFrom == IDC_TAB_MEM_VIEW)
			return OnNotifyTab(hwndDlg, pNMHDR);
		break;
	}
	case WM_CLOSE:
	{
		ShowWindow(hwndDlg, SW_HIDE);
		break;
	}
	case WM_COMMAND:
	{
		auto cmd = OnCommand(hwndDlg, uMsg, wParam, lParam);
		if (cmd)
			return cmd;
		auto b = hMemView;
		hMemView = hwndDlg;
		auto r = MemViewCallB(hMemView, uMsg, wParam, lParam);
		hMemView = b;
		if (!r)
			return r;
		break;
	}
	}
	return 0;
}

void HookDoMemView()
{
	static HWND hWnd;
	if (IsWindow(hMemView))
	{
		ShowWindow(hWnd, SW_SHOW);
		sysDoMemView();
		return;
	}
	if (!hHBRUSH)
	{
		hHBRUSH = CreateBrushIndirect(&LOGBRUSH({ BS_SOLID, RGB(255, 255, 255), DIB_RGB_COLORS }));
	}
	FCEUI_CreateCheatMap();
	hWnd = CreateDialogA(fceu_hInstance, MAKEINTRESOURCE(IDD_DIALOG_MEMView), NULL, s_MemViewCallB);
	HMENU hMenu = LoadMenuA(fceu_hInstance, "MEMVIEWMENU");
	SetMenu(hWnd, hMenu);
	hMemView = GetDlgItem(hWnd, IDC_STATIC_MEM_VIEW);
	auto proc = SetWindowLongPtr(hMemView, GWLP_WNDPROC, (LONG_PTR)s_StaticViewProc);
	SetWindowLongPtr(hMemView, GWLP_USERDATA, (LONG_PTR)proc);
	SetWindowText(hMemView, "");
	MemViewCallB(hMemView, WM_CREATE, 0, 0);
	ShowWindow(hWnd, SW_SHOW);
	sysDoMemView();
}
static bool bDoMemView = Mhook_SetHook((PVOID*)&sysDoMemView, HookDoMemView);

int myGetAddyFromCoord(int x, int y)
{
	int MemFontWidth = debugSystem->HexeditorFontWidth + HexCharSpacing;
	int MemFontHeight = debugSystem->HexeditorFontHeight + HexRowHeightBorder;
	int nOffset = GetXOffset();

	if (y < MemFontHeight)
		return -1;
	y -= MemFontHeight;
	if (y < 0)y = 0;
	if (x < 8 * MemFontWidth)x = 8 * MemFontWidth + 1;

	if (y > DataAmount * MemFontHeight) return -1;

	if (x < (nOffset - 4) * MemFontWidth) {
		AddyWasText = 0;
		return ((y / MemFontHeight) * nCols) + ((x - (8 * MemFontWidth)) / (3 * MemFontWidth)) + CurOffset;
	}

	if ((x > nOffset * MemFontWidth) && (x < (nOffset + nCols) * MemFontWidth)) {
		AddyWasText = 1;
		return ((y / MemFontHeight) * nCols) + ((x - (nOffset * MemFontWidth)) / (MemFontWidth)) + CurOffset;
	}

	return -1;
}
typedef int(*tGetAddyFromCoord)(int x, int y);
tGetAddyFromCoord sysGetAddyFromCoord = (tGetAddyFromCoord)GetAddyFromCoord;
static bool bGetAddyFromCoord = Mhook_SetHook((PVOID*)&sysGetAddyFromCoord, myGetAddyFromCoord);


typedef void(*tChangeMemViewFocus)(int newEditingMode, int StartOffset, int EndOffset);
tChangeMemViewFocus sysChangeMemViewFocus = (tChangeMemViewFocus)ChangeMemViewFocus;
void  myChangeMemViewFocus(int newEditingMode, int StartOffset, int EndOffset)
{
	//return sysChangeMemViewFocus(newEditingMode, StartOffset, EndOffset);
	SCROLLINFO si;

	if (!hMemView)DoMemView();
	if (EditingMode != newEditingMode)
		MemViewCallB(hMemView, WM_COMMAND, MENU_MV_VIEW_RAM + newEditingMode, 0); //let the window handler change this for us

	if ((EndOffset == StartOffset) || (EndOffset == -1)) {
		CursorEndAddy = -1;
		CursorStartAddy = StartOffset;
	}
	else {
		CursorStartAddy = std::min(StartOffset, EndOffset);
		CursorEndAddy = std::max(StartOffset, EndOffset);
	}
	CursorDragPoint = -1;


	if (std::min(StartOffset, EndOffset) >= MaxSize)return; //this should never happen

	//if (StartOffset < CurOffset) {
	//	CurOffset = (StartOffset / nCols) * nCols;
	//}

	//if (StartOffset >= CurOffset + DataAmount) {
	//	CurOffset = ((StartOffset / nCols) * nCols) - DataAmount + nCols;
	//	if (CurOffset < 0)CurOffset = 0;
	//}
	CurOffset = (StartOffset / nCols) * nCols;
	if (CurOffset < 0)CurOffset = 0;

	SetFocus(hMemView);
	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.fMask = SIF_POS;
	si.cbSize = sizeof(SCROLLINFO);
	si.nPos = CurOffset / nCols;
	SetScrollInfo(hMemView, SB_VERT, &si, TRUE);
	UpdateCaption();
	UpdateColorTable();
	return;
}
static bool bChangeMemViewFocus = Mhook_SetHook((PVOID*)&sysChangeMemViewFocus, myChangeMemViewFocus);

#endif