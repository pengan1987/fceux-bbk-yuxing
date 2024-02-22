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

#include "ppuview.cpp"
#if 0
void SetChrRamSize(int nSzie, uint8* pAddr) {}

#else

#include "../resource.h"
#include "mhook.h"

static int nChrRamSize;
static uint8* pChrRamAddr;
static HWND hWndComboxChrs;
void SetChrRamSize(int nSzie, uint8* pAddr)
{
	nChrRamSize = nSzie;
	pChrRamAddr = pAddr;
}

void SetListPages(HWND hwndDlg) {
	bool bRom = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_VROM, BM_GETCHECK, 0, 0));
	hWndComboxChrs = GetDlgItem(hwndDlg, IDC_COMBO_CHRS);
	SendMessage(hWndComboxChrs, CB_RESETCONTENT, 0, 0);
	SendMessage(hWndComboxChrs, CB_ADDSTRING, 0, (LPARAM)"PPU:0x0000-0x1FFF");
	auto nSize = (bRom ? CHRsize[0] : nChrRamSize) / 1024 / 8;
	for (int i = 0; i < nSize; i++)
	{
		TCHAR szText[250];
		sprintf(szText, "CHR:0x%04X-0x%04X", i * 8192, (i + 1) * 8192);
		SendMessage(hWndComboxChrs, CB_ADDSTRING, 0, (LPARAM)szText);
	}
	SendMessage(hWndComboxChrs, CB_SETCURSEL, 0, 0);
}

void HookFCEUD_UpdatePPUView(int scanline, int refreshchr);

static INT_PTR CALLBACK s_PPUViewCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetListPages(hwndDlg);
		break;
	}
	case  WM_COMMAND: {
		switch (HIWORD(wParam)) {
		case BN_CLICKED: {
			if (LOWORD(wParam) == IDC_CHECK_VROM)
				SetListPages(hwndDlg);
			break;
		}
		}
		break;
	}
	}

	return PPUViewCallB(hwndDlg, uMsg, wParam, lParam);
}

typedef void(*tDoPPUView)();
tDoPPUView sysDoPPUView = (tDoPPUView)DoPPUView;

void myDoPPUView()
{
	if (!GameInfo) {
		FCEUD_PrintError("You must have a game loaded before you can use the PPU Viewer.");
		return;
	}
	if (GameInfo->type == GIT_NSF) {
		FCEUD_PrintError("Sorry, you can't use the PPU Viewer with NSFs.");
		return;
	}

	if (!hPPUView)
	{
		hPPUView = CreateDialog(fceu_hInstance, MAKEINTRESOURCE(IDD_PPUVIEW), NULL, s_PPUViewCallB);
		EnsurePixelSize();
	}
	if (hPPUView)
	{
		//SetWindowPos(hPPUView,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);
		ShowWindow(hPPUView, SW_SHOWNORMAL);
		SetForegroundWindow(hPPUView);
		// redraw now
		PPUViewSkip = PPUViewRefresh;
		HookFCEUD_UpdatePPUView(-1, 1);
		PPUViewDoBlit();
	}
}
static bool bDoPPUView = Mhook_SetHook((PVOID*)&sysDoPPUView, myDoPPUView);

extern void FCEUD_UpdatePPUView(int scanline, int refreshchr);
extern uint8 chrcache0[0x1000], chrcache1[0x1000], logcache0[0x1000], logcache1[0x1000]; //cache CHR, fixes a refresh problem when right-clicking
extern unsigned int cdloggerVideoDataSize;
extern unsigned char* cdloggervdata;

typedef void(*tFCEUD_UpdatePPUView)(int scanline, int refreshchr);
tFCEUD_UpdatePPUView sysFCEUD_UpdatePPUView = (tFCEUD_UpdatePPUView)FCEUD_UpdatePPUView;

void HookFCEUD_UpdatePPUView(int scanline, int refreshchr)
{
	PPUViewScanline = 239;
	if (!PPUViewer) return;
	if (scanline != -1 && scanline != PPUViewScanline) return;

	if (!hPPUView) return;
	if (PPUViewSkip < PPUViewRefresh) return;

	if (!refreshchr)
	{
		sysFCEUD_UpdatePPUView(scanline, 0);
		return;
	}

	int x, y, i;
	auto nSel = SendMessage(hWndComboxChrs, CB_GETCURSEL, 0, 0);
	auto hwnd = GetDlgItem(hPPUView, IDC_CHECK_SETVRAM);
	extern bool bDisableSetVram;
	extern uint8* VROM;
	bool bRom = (BST_CHECKED == SendDlgItemMessage(hPPUView, IDC_CHECK_VROM, BM_GETCHECK, 0, 0));
	uint8* pChr = bRom ? VROM : pChrRamAddr;

	if (BST_CHECKED == SendMessage(hwnd, BM_GETCHECK, 0, 0)) {
		extern const int CHRRAMIndex;
		static int a;
		if (a != nSel && nSel > 0) {
			if (bRom) {
				setchr8(nSel - 1);
			}
			else {
				setchr8r(CHRRAMIndex, nSel - 1);
			}
		}
		a = nSel;
		bDisableSetVram = true;
	}
	else {
		bDisableSetVram = false;
	}
	
	for (i = 0, x = 0x1000; i < 0x1000; i++, x++)
	{
		if (nSel == 0 || !pChr)
		{
			chrcache0[i] = VPage[i >> 10][i];
			chrcache1[i] = VPage[x >> 10][x];
		}
		else
		{
			chrcache0[i] = pChr[(nSel - 1) * 8192 + i];
			chrcache1[i] = pChr[(nSel - 1) * 8192 + x];
		}
		if (!debug_loggingCD)
			continue;

		if (cdloggerVideoDataSize)
		{
			int addr;
			addr = &VPage[i >> 10][i] - CHRptr[0];
			if ((addr >= 0) && (addr < (int)cdloggerVideoDataSize))
				logcache0[i] = cdloggervdata[addr];
			addr = &VPage[x >> 10][x] - CHRptr[0];
			if ((addr >= 0) && (addr < (int)cdloggerVideoDataSize))
				logcache1[i] = cdloggervdata[addr];
		}
		else {
			logcache0[i] = cdloggervdata[i];
			logcache1[i] = cdloggervdata[x];
		}
	}
	sysFCEUD_UpdatePPUView(scanline, 0);
}
static bool bFCEUD_UpdatePPUView = Mhook_SetHook((PVOID*)&sysFCEUD_UpdatePPUView, HookFCEUD_UpdatePPUView);

typedef void(*tDrawPatternTable)(uint8* bitmap, uint8* table, uint8* log, uint8 pal);
tDrawPatternTable sysDrawPatternTable = (tDrawPatternTable)DrawPatternTable;
void myDrawPatternTable(uint8* bitmap, uint8* table, uint8* log, uint8 pal)
{
	if (BST_UNCHECKED == IsDlgButtonChecked(hPPUView, IDC_SPRITE16_16_MODE)) {
		sysDrawPatternTable(bitmap, table, log, pal);
		return;
	}
	int i, j, k, x, y, index = 0;
	int p = 0, tmp;
	uint8 chr0, chr1, logs, shift;
	uint8* pbitmap = bitmap;

	pal <<= 2;
	for (i = 0; i < (16 >> PPUView_sprite16Mode); i++)		//Columns
	{
		for (j = 0; j < 16; j++)	//Rows
		{
			//-----------------------------------------------
			for (k = 0; k < (PPUView_sprite16Mode + 1); k++) {
				for (y = 0; y < 8; y++)
				{
					chr0 = table[index];
					chr1 = table[index + 8];
					logs = log[index] & log[index + 8];
					tmp = 7;
					shift = (PPUView_maskUnusedGraphics && debug_loggingCD && (((logs & 3) != 0) == PPUView_invertTheMask)) ? 3 : 0;
					for (x = 0; x < 8; x++)
					{
						p = (chr0 >> tmp) & 1;
						p |= ((chr1 >> tmp) & 1) << 1;
						p = palcache[p | pal];
						tmp--;
						*(uint8*)(pbitmap++) = palo[p].b >> shift;
						*(uint8*)(pbitmap++) = palo[p].g >> shift;
						*(uint8*)(pbitmap++) = palo[p].r >> shift;
					}
					index++;
					pbitmap += (PATTERNBITWIDTH - 24);
				}
				index += 8;
			}
			pbitmap -= ((PATTERNBITWIDTH << (3 + PPUView_sprite16Mode)) - 24);
			//------------------------------------------------
		}
		pbitmap += (PATTERNBITWIDTH * ((8 << PPUView_sprite16Mode) - 1));
	}
}
static bool bDrawPatternTable = Mhook_SetHook((PVOID*)&sysDrawPatternTable, myDrawPatternTable);

#endif