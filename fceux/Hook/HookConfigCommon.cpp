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

#include "common/config.cpp"

#if 1

#include "mhook.h"

typedef void(*tcfg_NewToOld)(CFGSTRUCT* cfgst);
tcfg_NewToOld syscfg_NewToOld = (tcfg_NewToOld)cfg_NewToOld;
void  mycfg_NewToOld(CFGSTRUCT* cfgst)
{
	int x = 0;

	while (cfgst[x].ptr)
	{
		//structure contains another embedded structure. recurse.
		if (!cfgst[x].name) {
			cfg_NewToOld((CFGSTRUCT*)cfgst[x].ptr);
			x++;
			continue;
		}

		//if the key was not found, skip it
		if (cfgmap.find(std::string(cfgst[x].name)) == cfgmap.end())
		{
			x++;
			continue;
		}

		if (cfgst[x].len)
		{
			//binary data
			if (!StringToBytes(cfgmap[cfgst[x].name], cfgst[x].ptr, cfgst[x].len))
				FCEUD_PrintError("Config error: error parsing parameter");
		}
		else
		{
			//string data
			auto d = *(char**)cfgst[x].ptr;
			if (d)
				free(d);
			std::string& str = cfgmap[cfgst[x].name];
			if (str == "")
				*(char**)cfgst[x].ptr = 0;
			else
				*(char**)cfgst[x].ptr = strdup(cfgmap[cfgst[x].name].c_str());
		}
		extern void DebugString(const char* fmt, ...);
		//DebugString("x=%d", x);
		x++;
	}
}
//static bool bcfg_NewToOld = Mhook_SetHook((PVOID*)&syscfg_NewToOld, mycfg_NewToOld);

typedef void(*tcfg_OldToNew)(CFGSTRUCT* cfgst);
tcfg_OldToNew syscfg_OldToNew = (tcfg_OldToNew)cfg_OldToNew;
void  mycfg_OldToNew(CFGSTRUCT* cfgst)
{
	int x = 0;
	while (cfgst[x].ptr)
	{
		//structure contains another embedded structure. recurse.
		if (!cfgst[x].name) {
			cfg_OldToNew((CFGSTRUCT*)cfgst[x].ptr);
			x++;
			continue;
		}

		if (cfgst[x].len)
		{
			//binary data
			cfgmap[cfgst[x].name] = BytesToString(cfgst[x].ptr, cfgst[x].len);
		}
		else
		{
			//string data
			//extern void DebugString(const char* fmt, ...);
			//DebugString("name=%s", cfgst[x].name);
			if (*(char**)cfgst[x].ptr && nullptr == strstr(cfgst[x].name, "recentProjectsArray")) {
				cfgmap[cfgst[x].name] = *(char**)cfgst[x].ptr;
			}
			else { cfgmap[cfgst[x].name] = ""; }
		}

		x++;
	}
}
static bool bcfg_OldToNew = Mhook_SetHook((PVOID*)&syscfg_OldToNew, mycfg_OldToNew);

typedef void(*tSaveFCEUConfig)(const char* filename, const CFGSTRUCT* cfgst);
tSaveFCEUConfig sysSaveFCEUConfig = (tSaveFCEUConfig)SaveFCEUConfig;
void  mySaveFCEUConfig(const char* filename, const CFGSTRUCT* cfgst)
{
	cfg_OldToNew(cfgst);
	auto fp = fopen(filename, "wb");
	if (fp == NULL)
		return;
	cfg_Save(fp);
	fclose(fp);
}
static bool bSaveFCEUConfig = Mhook_SetHook((PVOID*)&sysSaveFCEUConfig, mySaveFCEUConfig);

#endif