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

/// \file
/// \brief implements a built-in NSF player.  This is a perk--not a part of the emu core
#include "nsf.cpp"

#if 1

#include "mhook.h"

static DWORD dwTickStart = 0;

void myDrawNSF(uint8* XBuf)
{
	char snbuf[16];
	int x;

	if (vismode == 0) return;

	memset(XBuf, 0, 256 * 240);
	memset(XDBuf, 0, 256 * 240);


	{
		int32* Bufpl;
		int32 mul = 0;

		int l;
		l = GetSoundBuffer(&Bufpl);

		if (special == 0)
		{
			if (FSettings.SoundVolume)
				mul = 8192 * 240 / (16384 * FSettings.SoundVolume / 50);
			for (x = 0; x < 256; x++)
			{
				uint32 y;
				y = 142 + ((Bufpl[(x * l) >> 8] * mul) >> 14);
				if (y < 240)
					XBuf[x + y * 256] = 3;
			}
		}
		else if (special == 1)
		{
			if (FSettings.SoundVolume)
				mul = 8192 * 240 / (8192 * FSettings.SoundVolume / 50);
			for (x = 0; x < 256; x++)
			{
				double r;
				uint32 xp, yp;

				r = (Bufpl[(x * l) >> 8] * mul) >> 14;
				xp = 128 + r * cos(x * M_PI * 2 / 256);
				yp = 120 + r * sin(x * M_PI * 2 / 256);
				xp &= 255;
				yp %= 240;
				XBuf[xp + yp * 256] = 3;
			}
		}
		else if (special == 2)
		{
			static double theta = 0;
			if (FSettings.SoundVolume)
				mul = 8192 * 240 / (16384 * FSettings.SoundVolume / 50);
			for (x = 0; x < 128; x++)
			{
				double xc, yc;
				double r, t;
				uint32 m, n;

				xc = (double)128 - x;
				yc = 0 - ((double)(((Bufpl[(x * l) >> 8]) * mul) >> 14));
				t = M_PI + atan(yc / xc);
				r = sqrt(xc * xc + yc * yc);

				t += theta;
				m = 128 + r * cos(t);
				n = 120 + r * sin(t);

				if (m < 256 && n < 240)
					XBuf[m + n * 256] = 3;

			}
			for (x = 128; x < 256; x++)
			{
				double xc, yc;
				double r, t;
				uint32 m, n;

				xc = (double)x - 128;
				yc = (double)((Bufpl[(x * l) >> 8] * mul) >> 14);
				t = atan(yc / xc);
				r = sqrt(xc * xc + yc * yc);

				t += theta;
				m = 128 + r * cos(t);
				n = 120 + r * sin(t);

				if (m < 256 && n < 240)
					XBuf[m + n * 256] = 3;

			}
			theta += (double)M_PI / 256;
		}
	}

	static const int kFgColor = 1;
	DrawTextTrans(ClipSidesOffset + XBuf + 10 * 256 + 4 + (((31 - strlen((char*)NSFHeader.SongName)) << 2)), 256, NSFHeader.SongName, kFgColor);
	DrawTextTrans(ClipSidesOffset + XBuf + 26 * 256 + 4 + (((31 - strlen((char*)NSFHeader.Artist)) << 2)), 256, NSFHeader.Artist, kFgColor);
	DrawTextTrans(ClipSidesOffset + XBuf + 42 * 256 + 4 + (((31 - strlen((char*)NSFHeader.Copyright)) << 2)), 256, NSFHeader.Copyright, kFgColor);

	DrawTextTrans(ClipSidesOffset + XBuf + 70 * 256 + 4 + (((31 - strlen("Song:")) << 2)), 256, (uint8*)"Song:", kFgColor);
	if (dwTickStart == 0)
		dwTickStart = GetTickCount();
	auto d = (GetTickCount() - dwTickStart) / 1000;
	char buf[256];
	sprintf(buf, "<%d/%d> %02d:%02d:%02d", CurrentSong, NSFHeader.TotalSongs, d / 60 / 60, d / 60, d % 60);
	DrawTextTrans(XBuf + 82 * 256 + 4 + (((31 - strlen(buf)) << 2)), 256, (uint8*)buf, kFgColor);
	//sprintf(snbuf, "<%d/%d>", CurrentSong, NSFHeader.TotalSongs);
	//DrawTextTrans(XBuf + 82 * 256 + 4 + (((31 - strlen(snbuf)) << 2)), 256, (uint8*)snbuf, kFgColor);

	{
		static uint8 last = 0;
		uint8 tmp;
		tmp = FCEU_GetJoyJoy();
		if ((tmp & JOY_RIGHT) && !(last & JOY_RIGHT))
		{
			if (++CurrentSong > NSFHeader.TotalSongs)
				CurrentSong = 1;
			SongReload = 0xFF;
		}
		else if ((tmp & JOY_LEFT) && !(last & JOY_LEFT))
		{
			if (--CurrentSong <= 0)
				CurrentSong = NSFHeader.TotalSongs;
			SongReload = 0xFF;
		}
		else if ((tmp & JOY_UP) && !(last & JOY_UP))
		{
			CurrentSong += 10;
			if (CurrentSong > NSFHeader.TotalSongs) CurrentSong = NSFHeader.TotalSongs;
			SongReload = 0xFF;
		}
		else if ((tmp & JOY_DOWN) && !(last & JOY_DOWN))
		{
			CurrentSong -= 10;
			if (CurrentSong < 1) CurrentSong = 1;
			SongReload = 0xFF;
		}
		else if ((tmp & JOY_START) && !(last & JOY_START))
			SongReload = 0xFF;
		else if ((tmp & JOY_A) && !(last & JOY_A))
		{
			special = (special + 1) % 3;
		}
		last = tmp;
		if (SongReload == 0xFF) {
			dwTickStart = GetTickCount();
		}
	}
}
typedef void(*tDrawNSF)(uint8* XBuf);
tDrawNSF sysDrawNSF = (tDrawNSF)DrawNSF;
static bool bDrawNSF = Mhook_SetHook((PVOID*)&sysDrawNSF, myDrawNSF);

#endif