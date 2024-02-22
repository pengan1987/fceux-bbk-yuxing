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

#include "x6502.cpp"

#if 1

#include "MapperBase.h"

typedef void(*tX6502_RunDebug)(int32 cycles);
tX6502_RunDebug sysX6502_RunDebug = (tX6502_RunDebug)X6502_RunDebug;
void  myX6502_RunDebug(int32 cycles)

{
    if (PAL)
        cycles *= 15;    // 15*4=60
    else
        cycles *= 16;    // 16*4=64

    _count += cycles;
    extern int test; test++;
    while (_count > 0)
    {
        int32 temp;
        uint8 b1;

        if (nullptr != MapperBase::pMapper)
            MapperBase::pMapper->OnRUNBefore(_PC);
        if (_IRQlow)
        {
            if (_IRQlow & FCEU_IQRESET)
            {
                DEBUG(if (debug_loggingCD) LogCDVectors(0xFFFC); )
                    _PC = RdMem(0xFFFC);
                _PC |= RdMem(0xFFFD) << 8;
                _jammed = 0;
                _PI = _P = I_FLAG;
                _IRQlow &= ~FCEU_IQRESET;
            }
            else if (_IRQlow & FCEU_IQNMI2)
            {
                _IRQlow &= ~FCEU_IQNMI2;
                _IRQlow |= FCEU_IQNMI;
            }
            //else if (nullptr != MapperBase::pMapper && !MapperBase::pMapper->bIRQ && (_IRQlow & FCEU_IQNMI))
            else if (_IRQlow & FCEU_IQNMI)
            {
                if (!_jammed)
                {
                    ADDCYC(7);
                    PUSH(_PC >> 8);
                    PUSH(_PC);
                    PUSH((_P & ~B_FLAG) | (U_FLAG));
                    _P |= I_FLAG;
                    DEBUG(if (debug_loggingCD) LogCDVectors(0xFFFA));
                    _PC = RdMem(0xFFFA);
                    _PC |= RdMem(0xFFFB) << 8;
                    _IRQlow &= ~FCEU_IQNMI;
                    if (nullptr != MapperBase::pMapper)
                        MapperBase::pMapper->OnNMI(_PC);
                }
            }
            else
            {
                if (!(_PI & I_FLAG) && !_jammed)
                {
                    ADDCYC(7);
                    PUSH(_PC >> 8);
                    PUSH(_PC);
                    PUSH((_P & ~B_FLAG) | (U_FLAG));
                    _P |= I_FLAG;
                    DEBUG(if (debug_loggingCD) LogCDVectors(0xFFFE));
                    _PC = RdMem(0xFFFE);
                    _PC |= RdMem(0xFFFF) << 8;

                    if (nullptr != MapperBase::pMapper)
                        MapperBase::pMapper->OnIRQ(_PC);
                }
            }
            _IRQlow &= ~(FCEU_IQTEMP);
            if (_count <= 0)
            {
                _PI = _P;
                return;
            } //Should increase accuracy without a
                     //major speed hit.
        }

        //will probably cause a major speed decrease on low-end systems
        DEBUG(DebugCycle());

        IncrementInstructionsCounters();

        _PI = _P;
        b1 = RdMem(_PC);

        ADDCYC(CycTable[b1]);

        temp = _tcount;
        _tcount = 0;
        if (MapIRQHook) MapIRQHook(temp);

        if (!overclocking)
            FCEU_SoundCPUHook(temp);
#ifdef _S9XLUA_H
        CallRegisteredLuaMemHook(_PC, 1, 0, LUAMEMHOOK_EXEC);
#endif
        auto pc = _PC;
        _PC++;
        switch (b1)
        {
#include "ops.inc"
        }
        if (nullptr != MapperBase::pMapper)
            MapperBase::pMapper->OnRUN(pc, b1);
    }
}
static bool bX6502_RunDebug = Mhook_SetHook((PVOID*)&sysX6502_RunDebug, myX6502_RunDebug);

#endif