#ifndef HEADER_MapperBase
#define HEADER_MapperBase

#include <windows.h>
#include <list>
#include "types.h"
#include "x6502.h"
#include "fceu.h"
#include "cart.h"
#include "driver.h"
#include "drivers/win/KeyBoard.h"

#include "ines.h"
#include "mapinc.h"

#include "Tools.h"
#include <dinput.h>

#include "win/resource.h"

#include "../asm.h"
#include "../debug.h"
#include "input.h"

#include "mhook.h"

#define _CPU_PC X.PC
extern bool bRTIStopRun, bDisableSetVram;
extern const int CHRRAMIndex, PRGRAMIndex;
//extern const int PRGRAMIndex;
extern const int WORKRAMIndex;
#define SetVRam1K(nBanker, nPage) setchr1r(CHRRAMIndex, 0x400 * nBanker, nPage)
#define SetVRom1K(nBanker, nPage) setchr1(0x400 * nBanker, nPage)
#define SetVRam2K(nBanker, nPage) setchr2r(CHRRAMIndex, 0x400 * nBanker, nPage)
#define SetVRom2K(nBanker, nPage) setchr2(0x400 * nBanker, nPage)
#define SetVRam4K(nBanker, nPage) setchr4r(CHRRAMIndex, 0x400 * nBanker, nPage)
#define SetVRom4K(nBanker, nPage) setchr4(0x400 * nBanker, nPage)
#define SetVRam8K(nPage) setchr8r(CHRRAMIndex, nPage)
#define SetVRom8K(nPage) setchr8(nPage)

#define SetRam8K(nBanker, nPage) setprg8r(PRGRAMIndex, 0x2000 * nBanker, nPage)
#define SetRom8K(nBanker, nPage) setprg8(0x2000 * nBanker, nPage);
#define SetRam16K(nBanker, nPage) setprg16r(PRGRAMIndex, 0x2000 * nBanker, nPage)
#define SetRom16K(nBanker, nPage) setprg16(0x2000 * nBanker, nPage);
#define SetRam32K(nPage) setprg32r(PRGRAMIndex, 0x2000 * 4, nPage)
#define SetRom32K(nPage) setprg32(0x2000 * 4, nPage)

#define SetWRam8K(nBanker, nPage) setprg8r(WORKRAMIndex, 0x2000 * nBanker, nPage)

#define HashPrgCrc32 nes->rom->GetPROM_CRC()
#define MirroringType int
#define PRGRomSize head.ROM_size
#define MirroringTypeVertical MI_V
#define MirroringTypeHorizontal MI_H
#define MirroringTypeAOnly MI_0
#define MirroringTypeBOnly MI_1
#define SetMirroringType setmirror
//#define IRQ_START() X6502_IRQBegin(FCEU_IQTEMP)
//#define IRQ_CLEAR() X6502_IRQEnd(FCEU_IQTEMP)
#define IRQ_START() X6502_IRQBegin(FCEU_IQEXT)
#define IRQ_CLEAR() X6502_IRQEnd(FCEU_IQEXT)
#define VideoModeNTSC 0
#define VideoModePAL 1
#define VideoModeDendy 2
#define VideoMode(x) FCEUI_SetRegion(x)

#define Write_Handler(a,b,f) SetWriteHandler(a, b, [](uint32 addr, uint8 data) { pMapper->f(addr, data);})
#define Read_Handler(a,b,f) SetReadHandler(a, b, [](uint32 addr) { return pMapper->f(addr);})
#define CartInfo_Call(a) []() { return pMapper->a();};
#define FCEU_MemDelete(a) if(a){FCEU_gfree(a);a = NULL;}

#define FCEU_MemNewPRG(var, index, size) { var=(uint8*)FCEU_gmalloc(size);\
SetupCartPRGMapping(index, (uint8 *)var, size, 1);\
AddExState(var, size, 0, ""#var);\
ZeroMemory(var, size);\
}

#define FCEU_MemNewCHR(var, index, size) { var=(uint8*)FCEU_gmalloc(size);\
SetupCartCHRMapping(index, (uint8 *)var, size, 1);\
AddExState(var, size, 0, ""#var);\
ZeroMemory(var, size);\
}

#include<map>
typedef BMAPPINGLocal* (*pCreateMapper)(int num);
typedef std::map< int, pCreateMapper> MAP_Mapper;
MAP_Mapper& AddMapper(int no, pCreateMapper pFun);
typedef std::vector<WORD> VectorCallStack;

extern HWND hAppWnd;
extern void SetChrRamSize(int nSzie, uint8 *pAddr);
typedef void (*PUpdateStackList)();
extern PUpdateStackList pUpdateStackList;
extern bool bMemViewUpdateMemory, bDisassembleReadMem;
//const int nSizeRAM = 1024 * 1024 * 8;
//const int nSizeVRAM = 1024 * 1024 * 2;
class MapperBase
{
private:
	void BasePower();
	void BaseClose();
	void BaseReset();

public:
	CartInfo* pCartInfo;
	static MapperBase* pMapper;
	void* pCHRRAM, * pPRGRAM, *pWORKRAM;
	HWND hWndMain, hWndView;
	bool bNMI, bIRQ, bCanIRQ;
	Tools& mTools;

	struct StackInfo {
		WORD wSP, wAddr;
		bool bNMI, bIRQ;
	};
	typedef std::list<StackInfo> ListStackInfo;
	ListStackInfo mListStackInfo, listLatest10;
	void CleanListStackInfo();

	static readfunc AReadBack[0xffff];
	static writefunc BWriteBack[0xffff];

	MapperBase();
	virtual ~MapperBase();
	virtual int GetPRGRamSize() = 0;
	virtual int GetCHRRamSize() = 0;
	virtual int GetWorkRamSize() = 0;
	virtual void OnRTS();
	virtual void OnRTI();
	virtual void OnNMI(WORD addr);
	virtual void OnIRQ(WORD addr);
	virtual void OnRUN(WORD addr, BYTE value);
	virtual void OnRUNBefore(WORD addr);
	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnMenuCommandKeyBoard(UINT nID) {}
	virtual void OnMenuCommandMouse(UINT nID) {}

	virtual bool OnDropFiles(const std::string& strFile) { return false; }
	virtual void Reset(bool softReset) {}
	virtual bool OnReadRam(WORD addr, BYTE& data) { return false; }
	virtual bool OnWriteRam(WORD addr, BYTE& data) { return false; }
	virtual void PPUhook(WORD scanline, WORD addr) {}
	virtual void CPUClock(int nCycles) {}
	virtual void OnLoadROM(const char* pszPath) {}
	virtual void MMC5_hb(int scanline) {}

	void LogCallStack();

	WORD GetPRGPages(WORD wPageSize) {
		return PRGsize[0] / wPageSize;
	}
	WORD GetCHRPages(WORD wPageSize) {
		return CHRsize[0] / wPageSize;
	}
	void SetChrRamSize(int nSzie)
	{
		::SetChrRamSize(nSzie * 1024, (uint8*)pCHRRAM);
	}
	static INLINE uint8 RdMem(unsigned int A)
	{
		return(ARead[A](A));
	}
	virtual void Power() {}
	virtual void Close() {}
	virtual BYTE ReadRam(WORD addr);
	virtual void WriteRam(WORD addr, BYTE data);
	static void Write(uint32 addr, uint8 data);
	static BYTE Read(uint32 addr);

	void HookPPU(bool bClear = false)
	{
		if (!bClear) {
			PPU_hook = [](uint32 addr) {
				pMapper->PPUhook(scanline, addr);
			};
			return;
		}
		PPU_hook = nullptr;
	}

	void HookIRQ(bool bClear = false)
	{
		if (!bClear) {
			MapIRQHook = [](int nCycles) {
				pMapper->CPUClock(nCycles);
			};
			return;
		}
		MapIRQHook = nullptr;
	}

	virtual void MapperInit(CartInfo* info)
	{
		pCartInfo = info;
		info->Reset = [] { pMapper->BaseReset(); };
		info->Power = [] { pMapper->BasePower(); };
		info->Close = [] { pMapper->BaseClose(); };;

		FCEU_MemNewCHR(pCHRRAM, CHRRAMIndex, GetCHRRamSize());//数值必须是 2 的幂
		FCEU_MemNewPRG(pPRGRAM, PRGRAMIndex, GetPRGRamSize());//数值必须是 2 的幂
		FCEU_MemNewPRG(pWORKRAM, WORKRAMIndex, GetWorkRamSize());//数值必须是 2 的幂
		extern void SetMemViewPRamSize(int nSize);
		SetMemViewPRamSize(GetPRGRamSize());
	}

	const UINT AddCustomMenu_Joy = 1;
	const UINT AddCustomMenu_KeyBoard = 2;
	const UINT AddCustomMenu_Mouse = 4;
	UINT nIDMenu_Joy, nIDMenu_KeyBoard, nIDMenu_Mouse;
	void AddCustomMenu(UINT e);

	static void s_Mapper_Init(CartInfo* info) {
		pMapper->MapperInit(info);
	}
};

#endif