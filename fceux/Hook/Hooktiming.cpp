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

#include "timing.cpp"

#if 1

#include "mhook.h"

typedef INT(CALLBACK*tTimingConCallB)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
tTimingConCallB sysTimingConCallB = (tTimingConCallB)TimingConCallB;
//tTimingConCallB sysTimingConCallB =  (tTimingConCallB)GetZwdllFunction("TimingConCallB");
INT CALLBACK  myTimingConCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto r = sysTimingConCallB(hwndDlg, uMsg, wParam, lParam);
	if (uMsg == WM_INITDIALOG){
		SendDlgItemMessage(hwndDlg, IDC_EXTRA_SCANLINES, EM_SETLIMITTEXT, 6, 0);
		SendDlgItemMessage(hwndDlg, IDC_VBLANK_SCANLINES, EM_SETLIMITTEXT, 6, 0);
	}
	return r;
}
static bool bTimingConCallB = Mhook_SetHook((PVOID*)&sysTimingConCallB, myTimingConCallB);

void myCloseTimingDialog(HWND hwndDlg)
{
	char strNum[10];
	if (IsDlgButtonChecked(hwndDlg, CB_SET_HIGH_PRIORITY) == BST_CHECKED)
	{
		eoptions |= EO_HIGHPRIO;
	}
	else
	{
		eoptions &= ~EO_HIGHPRIO;
	}

	if (IsDlgButtonChecked(hwndDlg, CB_DISABLE_SPEED_THROTTLING) == BST_CHECKED)
	{
		eoptions |= EO_NOTHROTTLE;
	}
	else
	{
		eoptions &= ~EO_NOTHROTTLE;
	}

	overclock_enabled = (IsDlgButtonChecked(hwndDlg, CB_OVERCLOCKING) == BST_CHECKED);
	skip_7bit_overclocking = (IsDlgButtonChecked(hwndDlg, CB_SKIP_7BIT) == BST_CHECKED);

	GetDlgItemText(hwndDlg, IDC_EXTRA_SCANLINES, strNum, 6);
	sscanf(strNum, "%d", &postrenderscanlines);

	GetDlgItemText(hwndDlg, IDC_VBLANK_SCANLINES, strNum, 6);
	sscanf(strNum, "%d", &vblankscanlines);

	if (postrenderscanlines < 0)
	{
		postrenderscanlines = 0;
		MessageBox(hwndDlg, "Overclocking is when you speed up your CPU, not slow it down!", "Error", MB_OK | MB_ICONERROR);
		sprintf(strNum, "%d", postrenderscanlines);
		SetDlgItemText(hwndDlg, IDC_EXTRA_SCANLINES, str);
		SetFocus(GetDlgItem(hwndDlg, IDC_EXTRA_SCANLINES));
	}
	else if (vblankscanlines < 0)
	{
		vblankscanlines = 0;
		MessageBox(hwndDlg, "Overclocking is when you speed up your CPU, not slow it down!", "Error", MB_OK | MB_ICONERROR);
		sprintf(strNum, "%d", vblankscanlines);
		SetDlgItemText(hwndDlg, IDC_VBLANK_SCANLINES, str);
		SetFocus(GetDlgItem(hwndDlg, IDC_VBLANK_SCANLINES));
	}
	else if (overclock_enabled && newppu)
	{
		MessageBox(hwndDlg, "Overclocking doesn't work with new PPU!", "Error", MB_OK | MB_ICONERROR);
		SetFocus(GetDlgItem(hwndDlg, CB_OVERCLOCKING));
	}
	else
		EndDialog(hwndDlg, 0);

	totalscanlines = normalscanlines + (overclock_enabled ? postrenderscanlines : 0);
}
typedef void(*tCloseTimingDialog)(HWND hwndDlg);
tCloseTimingDialog sysCloseTimingDialog = (tCloseTimingDialog)CloseTimingDialog;
static bool bCloseTimingDialog = Mhook_SetHook((PVOID*)&sysCloseTimingDialog, myCloseTimingDialog);

#endif