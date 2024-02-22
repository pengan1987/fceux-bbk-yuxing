#include "M171.h"
#undef IRQ_START
#undef IRQ_CLEAR
#define IRQ_START() X6502_IRQBegin(FCEU_IQTEMP)
#define IRQ_CLEAR() X6502_IRQEnd(FCEU_IQTEMP)
#include "KeyBoardSubor.h"
extern uint8 PPU[4];
extern uint32 RefreshAddr;
static bool bWrite2007 = false;
static WORD PC2007;

int nIRQCount = 0;
#define GRAYSCALE   (PPU[1] & 0x01)	//Grayscale (AND palette entries with 0x30)
#define READPAL(ofs)    (PALRAM[(ofs)] & (GRAYSCALE ? 0x30 : 0xFF))
#define READUPAL(ofs)   (UPALRAM[(ofs)] & (GRAYSCALE ? 0x30 : 0xFF))
#define SpriteON    (PPU[1] & 0x10)	//Show Sprite
#define ScreenON    (PPU[1] & 0x08)	//Show screen
#define INC32       (PPU[0] & 0x04)	//auto increment 1/32

class BBK10Create : public BBK10
{
public:
	KeyBoardSubor mKeyBoard;

	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return mKeyBoard.WindowProc(hwnd, uMsg, wParam, lParam);
	}

	static uint8 Read0x2007(uint32 addr) {
		uint8 ret;
		auto p = (BBK10Create*)pMapper;
		//if (!(p->reg[1] & 0x40)) {
		//	return AReadBack[addr](addr);
		//}
		auto &info = *GetAddr2007();
		auto &W2007Addr = info.addr;
		uint32 tmp = W2007Addr & 0x3FFF;
		SetVRam2K(0, info.vram & nChrRamCount2);

		if (tmp >= 0x3F00) {	// Palette RAM tied directly to the output data, without VRAM buffer
			if (!(tmp & 3)) {
				if (!(tmp & 0xC))
					ret = READPAL(0x00);
				else
					ret = READUPAL(((tmp & 0xC) >> 2) - 1);
			}
			else
				ret = READPAL(tmp & 0x1F);
#ifdef FCEUDEF_DEBUGGER
			if (!fceuindbg)
#endif
			{
				if ((tmp - 0x1000) < 0x2000)
					VRAMBuffer = VPage[(tmp - 0x1000) >> 10][tmp - 0x1000];
				else
					VRAMBuffer = vnapage[((tmp - 0x1000) >> 10) & 0x3][(tmp - 0x1000) & 0x3FF];
			}
		}
		else {
			ret = VRAMBuffer;
#ifdef FCEUDEF_DEBUGGER
			if (!fceuindbg)
#endif
			{
				PPUGenLatch = VRAMBuffer;
				if (tmp < 0x2000) {
					VRAMBuffer = VPage[tmp >> 10][tmp];

				}
				else if (tmp < 0x3F00)
					VRAMBuffer = vnapage[(tmp >> 10) & 0x3][tmp & 0x3FF];
			}
		}

#ifdef FCEUDEF_DEBUGGER
		if (!fceuindbg)
#endif
		{
			if ((ScreenON || SpriteON) && (scanline < 240)) {
				uint32 rad = W2007Addr;
				if ((rad & 0x7000) == 0x7000) {
					rad ^= 0x7000;
					if ((rad & 0x3E0) == 0x3A0)
						rad ^= 0xBA0;
					else if ((rad & 0x3E0) == 0x3e0)
						rad ^= 0x3e0;
					else
						rad += 0x20;
				}
				else
					rad += 0x1000;
				W2007Addr = rad;
			}
			else {
				if (INC32)
					W2007Addr += 32;
				else
					W2007Addr++;
			}
		}
		return ret;
	}
	static uint8 Read0x4016(uint32 addr) {
		if (!bUseKeyBoard)
			return AReadBack[addr] ? AReadBack[addr](addr) : CartBR(addr);
		BYTE data;
		auto p = (BBK10Create*)pMapper;
		if (p->OnReadRam(addr, data))
			return data;
		return p->mKeyBoard.OnRead(addr);
	}
	static void Write0x2006(uint32 A, uint8 V) {
		auto p = (BBK10Create*)pMapper;
		auto& info = *GetAddr2007();
		auto& W2007Addr = info.addr;
		auto& bFirst = info.bFirst;
		info.nByte = 0;
		if (bFirst) {
			W2007Addr = V << 8;
		}
		else {
			info.PC = 0;
			info.vram = p->reg[3];
			W2007Addr |= V;
			info.addrStart = W2007Addr;
			auto& vByte2007 = info.vByte;
			if (vByte2007.size() > 0) {
				DebugString("Write0x2006 I=%d N=%d PC=%04X staddr=%04X %s", p->bIRQ, p->bNMI, PC2007, info.addrStart, pTools->WriteVByte(vByte2007));
			}
			if (bIDC_CHECK_LogOut) {
				DebugString("0x2006 I=%d N=%d PC=%04X addr=%04X", p->bIRQ, p->bNMI, _CPU_PC, W2007Addr);
			}
		}
		bFirst = !bFirst;
	}
	static void Write0x2007(uint32 A, uint8 V) {
		auto p = (BBK10Create*)pMapper;
		auto& info = *GetAddr2007();
		auto& W2007Addr = info.addr;
		static uint32 pcPrev;
		static bool bLoop;
		static bool bMe;
		if (!p->bNMI && !p->bIRQ) {
			SetVRam2K(0, info.vram & nChrRamCount2);
			if (p->_irqEnabled)
				p->bCanIRQ = false;
			if(info.PC != 0 && info.PC != _CPU_PC)
				p->bCanIRQ = true;
			info.PC = _CPU_PC;
		}
		info.nByte++;
		uint32 tmp = W2007Addr & 0x3FFF;
		bWrite2007 = true;
		PC2007 = _CPU_PC;
		if (bIDC_CHECK_LogOut) {
			static uint32 add;
			auto& vByte2007 = info.vByte;
			if (add + 1 != tmp) {
				if (vByte2007.size() > 0) {
					DebugString("Write0x2007 I=%d N=%d PC=%04X staddr=%04X %s", p->bIRQ, p->bNMI, _CPU_PC, info.addrStart, pTools->WriteVByte(vByte2007));
				}
				info.addrStart = tmp;
			}
			add = tmp;
			vByte2007.push_back(V);
		}
		{
			PPUGenLatch = V;
			if (tmp < 0x2000) {
				if (PPUCHRRAM & (1 << (tmp >> 10)))
					VPage[tmp >> 10][tmp] = V;
			}
			else if (tmp < 0x3F00) {
				if (QTAIHack && (qtaintramreg & 1)) {
					QTAINTRAM[((((tmp & 0xF00) >> 10) >> ((qtaintramreg >> 1)) & 1) << 10) | (tmp & 0x3FF)] = V;
				}
				else {
					if (PPUNTARAM & (1 << ((tmp & 0xF00) >> 10)))
						vnapage[((tmp & 0xF00) >> 10)][tmp & 0x3FF] = V;
				}
			}
			else {
				if (!(tmp & 3)) {
					if (!(tmp & 0xC))
						PALRAM[0x00] = PALRAM[0x04] = PALRAM[0x08] = PALRAM[0x0C] = V & 0x3F;
					else
						UPALRAM[((tmp & 0xC) >> 2) - 1] = V & 0x3F;
				}
				else
					PALRAM[tmp & 0x1F] = V & 0x3F;
			}
			if (INC32)
				W2007Addr += 32;
			else
				W2007Addr++;
		}
	}

	static void Write0x4016(uint32 addr, BYTE data)
	{
		if (!bUseKeyBoard)
		{
			BWriteBack[addr] ? BWriteBack[addr](addr, data) : CartBW(addr, data);
			return;
		}
		auto p = (BBK10Create*)pMapper;
		if (p->OnWriteRam(addr, data))
			return;
		p->mKeyBoard.OnWrite(addr, data);
	}

	virtual void Power(void) override
	{
		__super::Power();
		SetChrRamSize(nChrRamSize);
		HookPPU();

		char szPath[1024];
		GetModuleFileNameA(NULL, szPath, sizeof(szPath));
		auto p = strrchr(szPath, '\\');
		*++p = 0;
		strcat(p, "BBKRoot");
		mBBKPCCard.strBBKRoot = szPath;
		bUseFloppy = true;
		bPCCard = false;
		if(GetFileAttributesA((mBBKPCCard.strBBKRoot + "\\BBGDOS.SYS").c_str()) != INVALID_FILE_ATTRIBUTES
			&& GetFileAttributesA((mBBKPCCard.strBBKRoot + "\\COMMAND.CMD").c_str()) != INVALID_FILE_ATTRIBUTES){
			bPCCard = true;
			bUseFloppy = (GetFileAttributesA((mBBKPCCard.strBBKRoot + "\\NoFloppy").c_str()) == INVALID_FILE_ATTRIBUTES);
		}

		SetWriteHandler(0x4018, 0x4019, Write);
		SetReadHandler(0x4018, 0x4019, Read);
		SetWriteHandler(0x4016, 0x4016, Write0x4016);

		SetWriteHandler(0x4400, 0xFFFF, CartBW);

		SetWriteHandler(0xFF00, 0xFFFF, Write);

		SetReadHandler(0x4400, 0xFFFF, CartBR);

		SetReadHandler(0xFF00, 0xFFF9, Read);
		SetReadHandler(0x5ff2, 0x5ff3, Read);
		SetReadHandler(0x5ffc, 0x5ffc, Read);

		SetReadHandler(0x4016, 0x4017, Read0x4016);

		SetWriteHandler(0x4100, 0x4400, Write);
		SetReadHandler(0x4100, 0x4400, Read);
		SetWriteHandler(0x2007, 0x2007, Write0x2007);
		SetReadHandler(0x2007, 0x2007, Read0x2007);
		SetWriteHandler(0x2006, 0x2006, Write0x2006);
	}

	virtual void PPUhook(WORD scanline, WORD addr)
	{
		static int nLinePrev;
		if (!(reg[1] & 0x4) || nVRIndex == 0 || bIRQ)
		{
			return;
		}

		WriteRam(0x6000, ReadRam(0x6000) | 0x20);

		int nAdd = 0;
		bUseKeyBoard = true;
		if (nVRIndex == 8)
		{//сно╥
			bUseKeyBoard = false;
			if (scanline > 120)
				nAdd = -8;
		}
		int nStep = 240 / nVRIndex;
		int nLine = (scanline + nAdd) / nStep;

		SetVRam2K(0, (regVR[nLine] >> 0) & nChrRamCount2);
		SetVRam2K(2, (regVR[nLine] >> 4) & nChrRamCount2);
		
		if (!bIDC_CHECK_LogOut && _irqEnabled && bCanIRQ && nLinePrev != scanline && scanline == regSR[nLine]) {
			bCanIRQ = false;
			IRQ_START();
			nIRQCount++;
			_irqEnabled = false;
		}
		nLinePrev = scanline;
	}
};