/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
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


#include "cheat.cpp"

#if 1

#include "mhook.h"

static uint8 mySubCheatsRead(uint32 A)
{
	CHEATF_SUBFAST* s = SubCheats;
	int x = numsubcheats;

	do
	{
		if (s->addr == A)
		{
			BWrite[A](A, s->val);
			if (s->compare >= 0)
			{
				uint8 pv = s->PrevRead(A);

				if (pv == s->compare)
					return(s->val);
				else return(pv);
			}
			else return(s->val);
		}
		s++;
	} while (--x);
	return(0);	/* We should never get here. */
}
typedef uint8(*tSubCheatsRead)(uint32 A);
tSubCheatsRead sysSubCheatsRead = (tSubCheatsRead)SubCheatsRead;
static bool bSubCheatsRead = Mhook_SetHook((PVOID*)&sysSubCheatsRead, mySubCheatsRead);


void myFCEU_SaveGameCheats(FILE* fp, int release)
{
	struct CHEATF* next = cheats;
	auto fn = strdup(FCEU_MakeFName(FCEUMKF_CHEAT, 0, 0).c_str());
	auto n = strrchr(fn, '\\');
	n[0] = 0;
	std::string name = fn;
	name += "\\NESenum\\";
	CreateDirectoryA(name.c_str(), nullptr);
	name += &n[1];
	auto fp1 = FCEUD_UTF8fopen(name.c_str(), "wb");
	char buff[512];
	while (next)
	{
		name = "";
		if (!next->type)
			name += ":";
		//if (next->compare >= 0)
		//	fputc('C', fp1);

		if (!next->status)
			name += ":";

		if (next->compare >= 0)
			sprintf(buff, "%04x:%02x:%02x:%s\n", next->addr, next->val, next->compare, next->name);
		else
			sprintf(buff, "%04x:%02x:%s\n", next->addr, next->val, next->name);
		name += buff;
		extern std::string ansi2utf8(const std::string & str);
		name = ansi2utf8(name);
		fputs(name.c_str(), fp1);
		next = next->next;
	}
	fclose(fp1);

	next = cheats;
	while (next)
	{
		if (next->type)
			fputc('S', fp);
		if (next->compare >= 0)
			fputc('C', fp);

		if (!next->status)
			fputc(':', fp);

		if (next->compare >= 0)
			fprintf(fp, "%04x:%02x:%02x:%s\n", next->addr, next->val, next->compare, next->name);
		else
			fprintf(fp, "%04x:%02x:%s\n", next->addr, next->val, next->name);

		if (release) free(next->name);
		struct CHEATF* t = next;
		next = next->next;
		if (release) free(t);
	}
}
typedef void(*tFCEU_SaveGameCheats)(FILE* fp, int release);
tFCEU_SaveGameCheats sysFCEU_SaveGameCheats = (tFCEU_SaveGameCheats)FCEU_SaveGameCheats;
static bool bFCEU_SaveGameCheats = Mhook_SetHook((PVOID*)&sysFCEU_SaveGameCheats, myFCEU_SaveGameCheats);


void myFCEUI_CheatSearchEnd(int type, uint8 v1, uint8 v2)
{
	uint32 x;

	if (!CheatComp)
	{
		if (!InitCheatComp())
		{
			CheatMemErr();
			return;
		}
	}

	for (x = 0; x < 0x10000; ++x)
	{
		switch (type)
		{
		default:
		case FCEU_SEARCH_SPECIFIC_CHANGE: // Change to a specific value
			if (!(CheatComp[x] & CHEATC_NOSHOW) && (CheatComp[x] != v1 || CheatRPtrs[x >> 10][x] != v2))
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_RELATIVE_CHANGE: // Search for relative change (between values).
			if (!(CheatComp[x] & CHEATC_NOSHOW) && (CheatComp[x] != v1 || CAbs(CheatComp[x] - CheatRPtrs[x >> 10][x]) != v2))
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_PUERLY_RELATIVE_CHANGE: // Purely relative change.
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CAbs(CheatComp[x] - CheatRPtrs[x >> 10][x]) != v2)
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_ANY_CHANGE: // Any change.
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CheatComp[x] == CheatRPtrs[x >> 10][x])
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_NEWVAL_KNOWN: // new value = known
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CheatRPtrs[x >> 10][x] != v1)
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_NEWVAL_GT: // new value greater than
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CheatComp[x] >= CheatRPtrs[x >> 10][x])
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_NEWVAL_LT: // new value less than
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CheatComp[x] <= CheatRPtrs[x >> 10][x])
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_NEWVAL_GT_KNOWN: // new value greater than by known value
			if (!(CheatComp[x] & CHEATC_NOSHOW) && CheatRPtrs[x >> 10][x] - CheatComp[x] != v2)
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		case FCEU_SEARCH_NEWVAL_LT_KNOWN: // new value less than by known value
			if (!(CheatComp[x] & CHEATC_NOSHOW) && (CheatComp[x] - CheatRPtrs[x >> 10][x]) != v2)
				CheatComp[x] |= CHEATC_EXCLUDED;
			break;
		}
		if (CheatRPtrs[x >> 10])
			*(uint8*)&CheatComp[x] = CheatRPtrs[x >> 10][x];
	}

}
typedef void(*tFCEUI_CheatSearchEnd)(int type, uint8 v1, uint8 v2);
tFCEUI_CheatSearchEnd sysFCEUI_CheatSearchEnd = (tFCEUI_CheatSearchEnd)FCEUI_CheatSearchEnd;
static bool bFCEUI_CheatSearchEnd = Mhook_SetHook((PVOID*)&sysFCEUI_CheatSearchEnd, myFCEUI_CheatSearchEnd);

#endif