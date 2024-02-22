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

#include "win/cheat.cpp"

#if 1

#include "mhook.h"

typedef INT(CALLBACK* tCheatConsoleCallB)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
tCheatConsoleCallB sysCheatConsoleCallB = (tCheatConsoleCallB)CheatConsoleCallB;
INT CALLBACK  myCheatConsoleCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        extern LRESULT APIENTRY FilterEditCtrlProcCheatText(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP);
        SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHEAT_TEXT), GWLP_WNDPROC, (LONG_PTR)FilterEditCtrlProcCheatText);
        break;
    }
    }
    return sysCheatConsoleCallB(hwndDlg, uMsg, wParam, lParam);
}
static bool bCheatConsoleCallB = Mhook_SetHook((PVOID*)&sysCheatConsoleCallB, myCheatConsoleCallB);

#endif