/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2002 Xodnizel
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
#include "ines.cpp"
#include "MapperBase.h"

CartInfo _CartInfoMapperBase;

typedef int(*tiNESLoad)(const char* name, FCEUFILE* fp, int OverwriteVidMode);
tiNESLoad sysiNESLoad = (tiNESLoad)iNESLoad;
int  myiNESLoad(const char* name, FCEUFILE* fp, int OverwriteVidMode)
{
	auto r = sysiNESLoad(name, fp, OverwriteVidMode);
	if(r == LOADER_OK)
		pTools->OnLoadROM(name);
	return r;
}
static bool biNESLoad = Mhook_SetHook((PVOID*)&sysiNESLoad, myiNESLoad);

static int myiNES_Init(int num) {
	BMAPPINGLocal* tmp = bmap;
	extern BMAPPINGLocal* GetBMAPPINGLocal(int num);
	MapIRQHook = nullptr;
	PPU_hook = nullptr;
	if (nullptr != MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = nullptr;
	MMC5Hack = 0;
	extern void SetMemViewPRamSize(int nSize);
	SetMemViewPRamSize(0);
	_CartInfoMapperBase = iNESCart;
	auto p = GetBMAPPINGLocal(num);
	if (p)
		tmp = p;

	CHRRAMSize = -1;

	if (GameInfo->type == GIT_VSUNI)
		AddExState(FCEUVSUNI_STATEINFO, ~0, 0, 0);

	while (tmp->init) {
		if (num == tmp->number) {
			UNIFchrrama = NULL;	// need here for compatibility with UNIF mapper code
			if (!VROM_size) {
				if (!iNESCart.ines2)
				{
					switch (num) {	// FIXME, mapper or game data base with the board parameters and ROM/RAM sizes
					case 13:  CHRRAMSize = 16 * 1024; break;
					case 6:
					case 29:
					case 30:
					case 45:
					case 96:  CHRRAMSize = 32 * 1024; break;
					case 176: CHRRAMSize = 128 * 1024; break;
					default:  CHRRAMSize = 8 * 1024; break;
					}
					iNESCart.vram_size = CHRRAMSize;
				}
				else
				{
					CHRRAMSize = iNESCart.battery_vram_size + iNESCart.vram_size;
				}
				if (CHRRAMSize > 0)
				{
					int mCHRRAMSize = (CHRRAMSize < 1024) ? 1024 : CHRRAMSize; // VPage has a resolution of 1k banks, ensure minimum allocation to prevent malicious access from NES software
					if ((UNIFchrrama = VROM = (uint8*)FCEU_dmalloc(mCHRRAMSize)) == NULL) return 2;
					FCEU_MemoryRand(VROM, CHRRAMSize);
					SetupCartCHRMapping(0, VROM, CHRRAMSize, 1);
					AddExState(VROM, CHRRAMSize, 0, "CHRR");
				}
				else {
					// mapper 256 (OneBus) has not CHR-RAM _and_ has not CHR-ROM region in iNES file
					// so zero-sized CHR should be supported at least for this mapper
					VROM = NULL;
				}
			}
			if (head.ROM_type & 8)
			{
				if (ExtraNTARAM != NULL)
				{
					AddExState(ExtraNTARAM, 2048, 0, "EXNR");
				}
			}
			tmp->init(&iNESCart);
			return 0;
		}
		tmp++;
	}
	return 1;
}
typedef int(*tiNES_Init)(int num);
tiNES_Init sysiNES_Init = (tiNES_Init)iNES_Init;
static bool biNES_Init = Mhook_SetHook((PVOID*)&sysiNES_Init, myiNES_Init);
