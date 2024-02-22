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

#include "debug.cpp"

#if 1

#include "mhook.h"

typedef uint8(*tGetMem)(uint16 A);
tGetMem sysGetMem = (tGetMem)GetMem;
uint8 myGetMem(uint16 A) {
	if ((A >= 0x4018) && (A < 0x5000) && GameInfo) {							//adelikat: 11/17/09: Prevent crash if this is called with no game loaded.
		uint32 ret;
		fceuindbg = 1;
		ret = ARead[A](A);
		fceuindbg = 0;
		return ret;
	}
    return sysGetMem(A);
}
static bool bGetMem = Mhook_SetHook((PVOID*)&sysGetMem, myGetMem);

#endif