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

#include "ppu.cpp"

#if 1

#include "MapperBase.h"

typedef void(* tDoLine)();
tDoLine sysDoLine = (tDoLine)DoLine;
void CALLBACK  myDoLine()
{
    //    sysDoLine();
    //    return;
    //if (PPU[0] & 0x80) {
    //    sysDoLine();
    //    return;
    //}
    //scanline++;
    //X6502_Run(256 + 69 + 16);
    if(nullptr != MapperBase::pMapper)
        MapperBase::pMapper->PPUhook(scanline, 0);
    sysDoLine();

}
//static bool bDoLine = Mhook_SetHook((PVOID*)&sysDoLine, myDoLine);
typedef void(*tMMC5_hb)(int scanline);
tMMC5_hb sysMMC5_hb = (tMMC5_hb)MMC5_hb;
void  myMMC5_hb(int scanline)
{
    if (nullptr != MapperBase::pMapper)
        MapperBase::pMapper->MMC5_hb(scanline);
    return sysMMC5_hb(scanline);
}
static bool bMMC5_hb = Mhook_SetHook((PVOID*)&sysMMC5_hb, myMMC5_hb);

#endif