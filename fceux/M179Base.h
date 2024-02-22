/*
²Â²â
*/
#include "MapperBase.h"
#include "Tools.h"


const int nWRamSize = 32;
const int nPrgRamSize = 1024;;
const int nChrRamSize = 512;
const int nChrRamCount1 = nChrRamSize - 1;
const int nChrRamCount2 = nChrRamSize / 2 - 1;
const int nChrRamCount8 = nChrRamSize / 8 - 1;

const int nPrgRamCount8 = nPrgRamSize / 8 - 1;
const int nPrgRamCount16 = nPrgRamSize / 16 - 1;
const int nPrgRamCount32 = nPrgRamSize / 32 - 1;

class DrPCCreate : public MapperBase
{
public:
	BYTE RegIO[0xff];
	virtual void PPUhook(WORD scanline, WORD addr) {
	}

	virtual int GetPRGRamSize() { return nPrgRamSize * 1024; }
	virtual int GetCHRRamSize() { return nChrRamSize * 1024; }
	virtual int GetWorkRamSize() { return nWRamSize * 1024; }

	virtual ~DrPCCreate() {}
	virtual void Power(void)
	{
		__super::Power();
		memset(RegIO, 0, sizeof(RegIO));
		SetWriteHandler(0x4100, 0xffff, Write);
		SetReadHandler(0x4100, 0xffff, Read);

	}

	virtual void Reset(bool softReset) override
	{
		__super::Reset(softReset);
		SetVRam8K(0);
		setprg4(0xf000, GetPRGPages(4096) - 1);
		//auto a = ReadRam(0xf000);
		//setprg16(0x7, 1);
		setprg32r(PRGRAMIndex, 0x6000, 0);
	}
	virtual bool OnReadRam(WORD addr, BYTE& data) override
	{
		if (addr >= 0x4100 && addr <= 0x41ff) {
			if (!(RegIO[addr & 0xff] & 2)) {
				RegIO[addr & 0xff] |= 2;
				DebugString("OnReadRam I=%d N=%d PC=%04X addr=%04X", bIRQ, bNMI, _CPU_PC, addr);
			}
		}
		return false;
	}
	virtual bool OnWriteRam(WORD addr, BYTE& data) override
	{
		//return false;
		if (addr >= 0x4100 && addr <= 0x41ff) {
			switch (addr) {
			//case 0x4180: {
			//	if(data & 0x80)
			//	SetRam8K(3, data & nPrgRamCount8);
			//	else
			//	SetRom8K(7, data & 7);
			//	break;
			//}
			//case 0x4190: {
			//	SetRam16K(3, data & nPrgRamCount32);
			//	break;
			//}
			//case 0x41B4: {
			//	SetRom8K(6, data & 7);
			//	break;
			//}
			}
			//if (!(RegIO[addr & 0xff] & 1))
			{
				RegIO[addr & 0xff] |= 1;
				DebugString("OnWriteRam I=%d N=%d PC=%04X addr=%04X data=%s", bIRQ, bNMI, _CPU_PC, addr, pTools->GetBitNumber(data));
			}
		}
		return false;
	}
};