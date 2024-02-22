/*
猜测
4190 $8000~$9FFF RAM
41ab RW
光驱 41ae W 41af RW
41B4 $C000~$DFFF ROM
*/
//#define _RELEASE
#include "MapperBase.h"
#undef IRQ_START
#undef IRQ_CLEAR
#define IRQ_START() X6502_IRQBegin(FCEU_IQTEMP)
#define IRQ_CLEAR() X6502_IRQEnd(FCEU_IQTEMP)

#include "M174.h"
#include "Tools.h"
#include<list>
/*
 ,33 DIK_COMMA ;27 DIK_SEMICOLON '28 DIK_APOSTROPHE [1A DIK_LBRACKET =D DIK_EQUALS ]1B DIK_RBRACKET DIK_RBRACKET \2B DIK_BACKSLASH
 `29 /b5 DIK_DIVIDE *37 DIK_MULTIPLY  NumEnter9c DIK_NUMPADENTER
*/
static const SHORT cbKeyMap[] = {
	DIK_F9,1,DIK_F5,3,DIK_F3,4,DIK_F1,5,DIK_F2,6,DIK_F12,7,DIK_F10,9,DIK_F8,0xa,DIK_F6,0xb,DIK_F4,0xc,DIK_TAB,0xd,DIK_LSHIFT,0x59,DIK_LALT,0x11,
	DIK_Q,0x15,DIK_1,0x16,DIK_Z,0x1a,DIK_S,0x1b,DIK_A,0x1c,DIK_W,0x1d,DIK_2,0x1e,DIK_C,0x21,DIK_X,0x22,DIK_D,0x23,DIK_E,0x24,DIK_4,0x25,
	DIK_3,0x26,DIK_SPACE,0x29,DIK_V,0x2a,DIK_F,0x2b,DIK_T,0x2c,DIK_R,0x2d,DIK_5,0x2e,DIK_N,0x31,DIK_B,0x32,DIK_H,0x33,DIK_G,0x34,DIK_Y,0x35,DIK_6,0x36,
	DIK_M,0x3a,DIK_J,0x3b,DIK_U,0x3c,DIK_7,0x3d,DIK_8,0x3e,DIK_K,0x42,DIK_I,0x43,DIK_O,0x44,DIK_0,0x45,DIK_9,0x46,DIK_PERIOD,0x49,DIK_SLASH,0x4a,
	DIK_L,0x4b,DIK_P,0x4d,DIK_MINUS,0x4e,DIK_CAPITAL,0x58,DIK_RSHIFT,0x59,DIK_RETURN,0x5a,DIK_BACK,0x66,DIK_NUMPAD1,0x69,DIK_NUMPAD4,0x6b,
	DIK_NUMPAD7,0x6c,DIK_NUMPAD0,0x70,DIK_DECIMAL,0x71,DIK_NUMPAD2,0x72,DIK_NUMPAD5,0x73,DIK_NUMPAD6,0x74,DIK_NUMPAD8,0x75,DIK_ESCAPE,0x76,
	DIK_NUMLOCK,0x77,DIK_F11,0x78,DIK_ADD,0x79,DIK_NUMPAD3,0x7a,DIK_SUBTRACT,0x7b,DIK_SCROLL,0x7e,DIK_F7,0x83,DIK_COMMA,0x41,DIK_SEMICOLON,0x4c,
	DIK_APOSTROPHE,0x52,DIK_LBRACKET,0x54,DIK_EQUALS,0x55,DIK_RBRACKET,0x5b,DIK_BACKSLASH,0x5d,DIK_GRAVE,0xe,
	DIK_LEFT,0xe06b,DIK_RIGHT,0xe074,DIK_UP,0xe075,DIK_DOWN,0xe072,DIK_DIVIDE,0xe04a,DIK_NUMPADENTER,0xe05a, DIK_MULTIPLY,0x7c,
	//DIK_PAUSE,
	DIK_HOME,0xe06c,DIK_END,0xe069,DIK_INSERT,0xe070,DIK_DELETE,0xe071,DIK_PGUP,0xe07d,DIK_PGDN,0xe07a,DIK_RMENU,0xe011,DIK_RCONTROL,0xe014,DIK_LMENU,0x11,DIK_LCONTROL,0xe014
};
class KW3000KeyBoard : public BaseKeyInput {
public:
	bool bInit;
	std::list<BYTE> vSend;
	BYTE cbKeyCode, cbKeyScan, cbKeySend, cbCurKey, nChang, cbReadCount;

	KW3000KeyBoard() {
		bInit = false;
		cbKeyScan = 0xff;
		cbReadCount = 0;
		nChang = 1;
	}

	SHORT ConvertScanCode(SHORT cbKeyScan) {
		for (int i = 0; i < sizeof(cbKeyMap) / sizeof(cbKeyMap[0]); i += 2) {
			if (cbKeyScan == cbKeyMap[i]) {
				return cbKeyMap[i + 1];
			}
		}
		return 0xfa;
	}
	
	BYTE OnRead(uint32 addr) {
		//BASIC
		static BYTE p1[] = { 0, 2, 2 };
		static BYTE p2[] = { 0, 2 };
		//DOS
		static BYTE p3[] = { 0, 2 };
		static BYTE p4[] = { 2, 0 };
		static BYTE n1, n2, n3, n4, nTime;
		if (bInit) {
			nTime = 0;
			static BYTE d = 0;
			if (0 != cbReadCount && d != cbReadCount) {
				//DebugString("Write cbReadCount=%03d", cbReadCount);
				d = cbReadCount;
			}
			cbReadCount = 0;
			bInit = false;
			n1 = n2 = n3 = n4 = 0;
		}
		cbReadCount++;
		BYTE r = 0xff;
		do {
			if (nChang % 2 == 0) {
				if (n1 < sizeof(p1) / sizeof(p1[0])) {
					r = p1[n1++];
					break;
				}

				if (n2 < sizeof(p2) / sizeof(p2[0])) {
					r = p2[n2++];
					break;
				}
			}
			else {
				if (n1 < sizeof(p3) / sizeof(p3[0])) {
					r = p3[n1++];
					break;
				}

				if (n2 < sizeof(p4) / sizeof(p4[0])) {
					r = p4[n2++];
					break;
				}
			}

			if (n3 < 8) {
				n2 = 0;
				//DebugString("Keyrboad: cbKeySend=%02X", cbKeySend);
				r = (cbKeySend >> (n3++)) & 1;
				break;
			}
			if (n4 == 0) {
				n4++;
				n2 = 0;
				BYTE crc = 1;
				for (int i = 0; i < 8; i++) {
					crc += ((cbKeySend >> i) & 1);
				}
				//DebugString("Keyrboad: crc=%02X", crc);
				r = crc;
				break;
			}
		} while (false);

		if (r == 0xff)
			r = 1;

		//DebugString("Keyrboad: n1=%02X n2=%02X n4=%02X r=%02X", n1, n2, n4, r);
		return r;
	}
	virtual void OnWrite(uint32 addr, uint8 data)
	{
		//DebugString("Write PC=%04X addr=%04X data=%03d", _CPU_PC, addr, data);
		static SHORT keySend = cbKeyScan;
		if (data == 2) {
			cbKeyScan = 0xaa;
			nChang = 1;
			return;
		}
		if (data == 0x03) {
			switch (_CPU_PC) {
			case 0xA03A: {//DOS
				nChang = 1;
				break;
			}
			case 0xE2BB: {//BASIC
				nChang = 0;
				break;
			}
			}
			bInit = true;
			if (vSend.size() > 0) {
				cbKeySend = vSend.front();
				vSend.pop_front();
				return;
			}
			static DWORD dwT;
			static SHORT key;

			if (GetReleaseKey(cbKeyScan)){
				keySend = ConvertScanCode(cbKeyScan);
				//DebugString("GetReleaseKey cbKeyScan=%02X keySend=%04X", cbKeyScan, keySend);
				if (keySend & 0xff00)
				{
					cbKeySend = (keySend >> 8) & 0xff;
					vSend.push_back(0xf0);
				}
				else
				{
					cbKeySend = 0xf0;
					vSend.push_back(0xfa);
				}
				vSend.push_back(keySend & 0xff);
				keySend = 0xfafa;
				cbKeyScan = 0xfa;
				return;
			}
			if (GetPressKey(cbKeyScan)){
				keySend = ConvertScanCode(cbKeyScan);
				//DebugString("GetPressKey cbKeyScan=%02X keySend=%04X", cbKeyScan, keySend);
			}
			if (key == cbKeyScan) {
				if (GetTickCount() - dwT < 600){
					cbKeySend = 0xfa;
					//DebugString("GetPressKey GetTickCount cbKeyScan=%02X keySend=%04X", cbKeyScan, keySend);
					return;
				}
			}
			else {
				//DebugString("GetPressKey NEW cbKeyScan=%02X keySend=%04X", cbKeyScan, keySend);
				dwT = GetTickCount();
			}
			key = cbKeyScan;
			vSend.push_back(keySend & 0xff);
			cbKeySend = (keySend >> 8) & 0xff;
			return;
		}
		if (data == 0x01) {
		}
	}
};

static BYTE KeyStatus[] = { VK_SHIFT, VK_SHIFT, VK_CONTROL, VK_MENU, VK_SCROLL, VK_NUMLOCK, VK_CAPITAL, VK_INSERT };
class KW3000Create : public KW3000
{
public:
	KW3000KeyBoard mKeyBoard;
	bool bKeyBoard;
	int n418ECnt;
	void* pCHRROM;

	KW3000Create() {
		pCHRROM = nullptr;
	}
	//virtual void MapperInit(CartInfo* info) {
	//	__super::MapperInit(info);
	//	auto n = GetPRGRamSize() / 1024 / 16;
	//	char szDesc[16];
	//	auto buf = (BYTE*)pPRGRAM;
	//	for (int i = 0; i < n; i++) {
	//		sprintf(szDesc, "RAM%02X", i);
	//		//AddExState(&buf[0x2000 * i], 0x2000, 0, szDesc);
	//	}
	//}

	virtual void PPUhook(WORD scanline, WORD addr) {
		//if (Reg[0x8d] > 0 && !bDisableSetVram) {
		//	int nLine = scanline / (240 / (Reg[0x8d]+1));
		//	static int nLinePrev;
		//	if (!bNMI && nLinePrev != nLine) {
		//		//SetVRam8K(nLine+1);
		//	}
		//	nLinePrev = nLine;
		//}
		if (!bIRQEnabled)
			return;
		static WORD scanlinePrev = 0;
		if (scanlinePrev == scanline)
			return;
		if (--IRQCount <= 0)
		{
			IRQCount = Reg[0xa0];
			IRQ_START();
		}
		scanlinePrev = scanline;
	}

	virtual ~KW3000Create() {}
	virtual void Power(void)
	{
		__super::Power();
		if (pCHRROM)
			free(pCHRROM);
		pCHRROM = nullptr;
		memset(RegIO, 0, sizeof(RegIO));
		//MMC5Hack = 1;//开了DOS无法显示字符
		HookPPU();
		HookIRQ();
		mKeyBoard.bUseKeyBoard = true;
		SetWriteHandler(0x4100, 0xffff, Write);
		SetReadHandler(0x4100, 0xffff, Read);
		SetReadHandler(0x4100, 0xffff, Read);
		//SetReadHandler(0xa, 0xb, Read);

		//SetWriteHandler(0x2000, 0x2000, Write0x2000);
		//SetWriteHandler(0x2006, 0x2006, Write0x2006);
		//SetWriteHandler(0x2007, 0x2007, Write0x2007);
		//SetWriteHandler(0x4016, 0x4017, Write);//启用会运行不起来
		//SetReadHandler(0x4016, 0x4017, Read);
		SetChrRamSize(nChrRamSize);

		bKeyBoard = false;
		SetVRam8K(0);
		SetWRam8K(2, 0);
		SetWRam8K(3, 1);
		SetRam32K(4, 0);
		SetRam8K(6, 1);
		SetRom8K(7, 0);
	}
	void SetVRam(BYTE nBanker, BYTE data) {
		if (bDisableSetVram)
			return;
		auto b = Reg[0x81] & 3;
		auto chr = Reg[0x82] & 0x80;
		bLog = false;
		const int nK[] = { 1, 2, 4, 8 };
		if (!bNMI) {
			//DEBUGOUTIO("SetVR%sm %dK I=%d N=%d PC=%04X addr=%04X data=%s", chr == 0 ? "o" : "a", nK[b], bIRQ, bNMI, _CPU_PC, 0x4198 + nBanker, mTools.GetBitNumber(data));
		}
		if (_CPU_PC == 0x9850 /* 进DOS *//* || 0x9840 == _CPU_PC  mccdos /u */) {//DOS
			data = 0x3e;
		}
		if (_CPU_PC == 0x9826) {//
			SetVRam4K(0, 0x3e * 2 + 1);
			SetVRam4K(4, 0x3e * 2);
			return;
		}
		chr = 1;
		switch (b)
		{
		case 0: {
			chr == 0 ? SetVRom1K(nBanker, data & CHRRomCount[0]) : SetVRam1K(nBanker, data & CHRRamCount[0]);
			break;
		}
		case 1: {
			chr == 0 ? SetVRom2K(nBanker, data & CHRRomCount[1]) : SetVRam2K(nBanker, data & CHRRamCount[1]);
			break;
		}
		case 2: {
			chr == 0 ? SetVRom4K(nBanker, data & CHRRomCount[2]) : SetVRam4K(nBanker, data & CHRRamCount[2]);
			break;
		}
		case 3: {
			chr == 0 ? SetVRom8K(data & CHRRomCount[3]) : SetVRam8K(data & CHRRamCount[3]);
			break;
		}
		}
	}
	void SetRam(BYTE nBanker, BYTE data) {
		//return;
		if (nBanker == 6) {
			if (Reg[0xb5] & 0x20) {
				return;
			}
		}
		if (X.PC == 0x4cd) {
			//SetRam16K(4, 0);
			//SetRam16K(6, 1);
			//return;
		}
		auto b = (Reg[0x81] >> 2) & 3;
		const int nK[] = { 8,16,32 };
		if(bLog && bIDC_CHECK_LogOut)
			DEBUGOUTIO("SetRam $%04X %dK I=%d N=%d PC=%04X addr=%04X data=%s", 0x2000 * nBanker, nK[b], bIRQ, bNMI, _CPU_PC, 0x4190 + nBanker - 4, mTools.GetBitNumber(data));
		bLog = false;
		switch (b)
		{
		case 0: {
			SetRam8K(nBanker, data & PRGRamCount[0]);
			break;
		}
		case 1: {
			SetRam16K(nBanker, data & PRGRamCount[1]);
			break;
		}
		case 2: {
			SetRam32K(nBanker, data & PRGRamCount[2]);
			break;
		}
		default:
			break;
		}
		if (nBanker == 6) {
			if (Reg[0xb5] & 0x20) {
				SetRom8K(7, Reg[0xb5] & (GetPRGPages(8192) - 1));
			}
		}
	}

	virtual bool OnReadRam(WORD addr, BYTE& data) override
	{
		//if (addr == 0x5fe6) {
		//	data = 0;
		//	return true;
		//}
		if (addr == 0xa && X.PC == 0xe162) {
			data = 1;
			data = RdMem(0x22);
			return true;
		}
		if (addr == 0xb && X.PC == 0xe168) {
			data = RdMem(0x23);
			return true;
		}
		if (addr < 0x4100 || addr > 0x42ff)
			return __super::OnReadRam(addr, data);
		if (X.PC == 0x97AB)
			return true;
		bool bRes = true;
		bLog = true;
		
		switch (addr) {
		case 0x418F: {
			data = 0x10;
			break;
		}
		case 0x41AB: {
			bLog = false;
			data = bVCD ? 0 : 0x10; //不返回 0x10 位则进入VCD模式, 否则进入软盘驱动
			break;
		}
		default:
			bRes = false;
			break;
		}
		if (addr == 0x418E) {
			if (bKeyBoard) {
				data = mKeyBoard.OnRead(addr);
				//DebugString("0x418e PC=%04X n418ECnt=%03d", _CPU_PC, n418ECnt);
				bRes = true;
			}
		}
		if (!bRes)
			bRes = __super::OnReadRam(addr, data);
		if (!bRes){// && !(RegIO[addr & 0xff] & 2)) {
			RegIO[addr & 0xff] |= 2;
			if(bLog && _CPU_PC != 0x5c37)
				DEBUGOUTIO("OnReadRam I=%d N=%d PC=%04X addr=%04X", bIRQ, bNMI, _CPU_PC, addr);
		}
		return bRes;
	}
	virtual bool OnWriteRam(WORD addr, BYTE& data) override
	{
		if (Reg[0xA7] == 0 && addr >= 0x8000) {
			return true;
		}
		if (addr < 0x4100 || addr > 0x42ff)
			return __super::OnWriteRam(addr, data);
		if (X.PC == 0x97AB)
			return true;

		bool bRes = true;
		bLog = true;
		Reg[addr & 0xff] = data;

		switch (addr) {
		case 0x4180: {
			break;
		}
		case 0x4181: {
			break;
		}
		case 0x4186: {//Parallel / Printer port 
			break;
		}
		case 0x418D: {
			break;
		}
		case 0x418E: {//KeyBoard
			bKeyBoard = (data == 3);
			mKeyBoard.OnWrite(addr, data);
			break;
		}
		case 0x4190: {
			if (X.PC == 0xFE95 || X.PC == 0xFE9F)
				bLog = false;
		}
		case 0x4191:
		case 0x4192:
		case 0x4193: {
			SetRam(addr - 0x4190 + 4, data);
			break;
		}
		case 0x4194:
		case 0x4195:
		case 0x4196:
		case 0x4197: {
			DoNTARAMROM(addr - 0x4194, data);
			break;
		}
		case 0x4198:
		case 0x4199:
		case 0x419A:
		case 0x419B:
		case 0x419C:
		case 0x419D:
		case 0x419E:
		case 0x419F: {
			SetVRam(addr - 0x4198, data);
			break;
		}
		case 0x41AB:
		case 0x41A7: {
			bLog = false;
			break;
		}
		case 0x41B4: {
			if (data & 0x80) {
				SetRom8K(6, data & (GetPRGPages(8192) - 1));
				//DEBUGOUTIO("SetRom8K $%04X 8K I=%d N=%d PC=%04X addr=%04X data=%s", 0x2000 * 6, bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			}
			else {
				SetRam8K(6, data & PRGRamCount[0]);
				//DEBUGOUTIO("SetRam8K $%04X 8K I=%d N=%d PC=%04X addr=%04X data=%s", 0x2000 * 6, bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			}
			break;
		}
		case 0x41B5:{//$41B5=40,PRG 8Mbit
			if (data & 0x20) {
				SetRom8K(7, data & (GetPRGPages(8192) - 1));
				//DEBUGOUTIO("SetRom8K $%04X 8K I=%d N=%d PC=%04X addr=%04X data=%s", 0x2000 * 7, bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			}
			else {//必须切到 $C000 16K $4192 上次设置的值
				SetRam16K(6, Reg[0x92] & PRGRamCount[1]);
				//DEBUGOUTIO("SetRam8K $%04X 8K I=%d N=%d PC=%04X addr=%04X data=%s", 0x2000 * 7, bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			}
			break;
		}
		case 0x41B7: {//SRAM BANK                   |4 * 8K RAM
			SetWRam8K(3, nWRamIndex + (data & 3));
			break;
		}
		default:
			bRes = false;
			break;
		}

		if (!bRes) {

			auto it = mMAP_PWRITEFun.find(addr);
			if (it != mMAP_PWRITEFun.end())
			{
				(this->*it->second)(addr, data);
				//if(bIDC_CHECK_LogOut && bWriteIO)
				//	DebugString("OnWriteRam I=%d N=%d PC=%04X addr=%04X data=%s", bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
				bRes = true;
			}
		}
		if(!bRes)
		{
			bRes = __super::OnWriteRam(addr, data);
		}
		if (!bRes)
		{
			RegIO[addr & 0xff] |= 1;
		}
		//if(bLog)
			//DEBUGOUTIO("OnWriteRam I=%d N=%d PC=%04X addr=%04X data=%s", bIRQ, bNMI, _CPU_PC, addr, pTools->GetBitNumber(data));
		return bRes;
	}

	virtual void OnRUNBefore(WORD addr) {
		__super::OnRUNBefore(addr);
		//X.IRQlow &= ~FCEU_IQNMI;
	}

	static void Write0x2000(uint32 addr, uint8 data)
	{
		auto p = pMapper;
		//DebugString("Write0x2000 I=%d N=%d PC=%04X addr=%04X data=%s", p->bIRQ, p->bNMI, _CPU_PC, addr, pTools->GetBitNumber(data));
		//if (_CPU_PC == 0xA773) {
		//	data &= 0xef;
		//}
		p->BWriteBack[addr](addr, data);
	}

	static void Write0x2006(uint32 addr, uint8 data)
	{
		auto p = pMapper;
		//if (_CPU_PC == 0x9855) {
		//	data = 0x10;
		//}
		//DebugString("Write0x2006 I=%d N=%d PC=%04X", p->bIRQ, p->bNMI, _CPU_PC);
		p->BWriteBack[addr](addr, data);
	}

	static void Write0x2007(uint32 addr, uint8 data)
	{
		auto p = pMapper;
		//DebugString("Write0x2007 I=%d N=%d PC=%04X", p->bIRQ, p->bNMI, _CPU_PC);
		extern uint32 TempAddr, RefreshAddr, DummyRead, NTRefreshAddr;
		static uint32 prev, AddrStart, AddrPC;
		uint32 tmp = RefreshAddr & 0x3FFF;
		if (prev + 1 != tmp) {
			if (vByte1.size() > 0) {
				//DebugString("W2007 PC=%04X AddrStart=%04X %s", AddrPC, AddrStart, pTools->WriteVByte(vByte1));
				vByte1.clear();
			}
		}
		if (vByte1.size() == 0) {
			AddrStart = tmp;
			AddrPC = _CPU_PC;
		}
		vByte1.push_back(data);
		prev = tmp;

		p->BWriteBack[addr](addr, data);
	}
	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return mKeyBoard.WindowProc(hwnd, uMsg, wParam, lParam);
	}

	virtual bool OnDropFiles(const std::string& strFile){
		auto f = fopen(strFile.c_str(), "rb");
		if (!f) {
			return false;
		}
		fseek(f, 0, SEEK_END);
		BYTE buf[0x50];
		auto s = ftell(f);
		if (s >= 0x9500) {
			fseek(f, 0x9340, SEEK_SET);
			fread(buf, sizeof(buf), 1, f);
			if (memcmp(buf, "BUNGCVCD", 8) == 0) {
				bVCD = true;
				fclose(f);
				mTools.enumTitleStatus = Tools::EnumTitleStatusNone;
				mTools.strImageFile = strFile;
				mTools.SetTitle("");
				return true;
			}
		}
		bVCD = false;
		fseek(f, 0, SEEK_SET);
		const char* pszFcGames = "FC GAMES";
		fread(buf, sizeof(buf), 1, f);
		if (memcmp(buf, pszFcGames, 8) != 0) {
			fclose(f);
			return false;
		}
		fseek(f, 0x800 + buf[0xd] * 512, SEEK_SET);
		auto nSizePRG = buf[8] * 8192;
		fread(pPRGRAM, nSizePRG, 1, f);
		{
			//auto n = nSizePRG / 1024;
			//PRGRamCount[0] = n / 8 - 1;
			//PRGRamCount[1] = n / 16 - 1;
			//PRGRamCount[2] = n / 32 - 1;
		}
		auto nSizeCHR = buf[0x9] * 8192;
		if (nSizeCHR > 0) {
			fread(pCHRRAM, nSizeCHR, 1, f);
			SetupCartCHRMapping(0, (BYTE*)pCHRRAM, nSizeCHR, 1);
			if (!(buf[0x12] & 0x80)) {
				//fread(pCHRRAM, nSizeCHR, 1, f);
			}
			else {
				//if (pCHRROM)
				//	free(pCHRROM);
				//pCHRROM = FCEU_malloc(nSizeCHR);
				//fread(pCHRROM, nSizeCHR, 1, f);
				//SetupCartCHRMapping(0, (BYTE*)pCHRROM, nSizeCHR, 0);
				//auto n = nSizeCHR / 1024;
				//CHRRomCount[0] = n / 1 - 1;
				//CHRRomCount[1] = n / 2 - 1;
				//CHRRomCount[2] = n / 4 - 1;
				//CHRRomCount[3] = n / 8 - 1;
			}
		}
		mKeyBoard.bUseKeyBoard = false;

		//buf[0x12] |= 0x80;
		OnWriteRam(0x4181, buf[0x11]);
		OnWriteRam(0x4182, buf[0x12]);
		OnWriteRam(0x41B5, buf[0x47]);
		OnWriteRam(0x4183, buf[0x13]);
		OnWriteRam(0x41A4, buf[0x34]);
		//for (WORD i = 0x4181; i <=0x41BA; i++) {
		//	auto data = buf[i - 0x4181 + 0x11];
		//	if (data == 0)
		//		continue;
		//	OnWriteRam(i, buf[i-0x4181+0x11]);
		//}
		switch (buf[0xe]>>5) {
		case 0:
		case 1:
		case 2:
		{
			auto n = nSizePRG / 1024;
			SetRam8K(4, 0);
			n = n / 16 - 1;
			SetRam16K(6, n);
			break;
		}
		case 3: {
			auto n = nSizePRG / 1024 / 16 - 1;
			SetRam16K(4, n);
			break;
		}
		case 4:
		case 5:
		case 6:
			//CHRRomCount[3] = 3;
		case 7: {
			SetRam32K(0);
			if (nSizePRG < 32 * 1024) {
				SetRam16K(6, 0);
			}
			break;
		}
		}
		//if(buf[0x12] & 0x80)
		//	SetVRam8K(0);
		//else
		//	 SetVRom8K(0);
		SetVRam8K(0);

		fclose(f);
		auto p = strrchr(strFile.c_str(), '\\');
		mTools.enumTitleStatus = Tools::EnumTitleStatusNone;
		mTools.strImageFile = ++p;
		mTools.SetTitle(p);
		X.S = 0x0;
		ResetNES();
		//SetForegroundWindow(mTools.mWndMain);
		return true;
	}
	void DoNTARAMROM(int w, uint8 V) {
		auto mod = (Reg[0x82] >> 4) & 7;
		if(mod != 7)
			return;
		if (V >= 0xE0) {
			setntamem(NTARAM + ((V & 1) << 10), 1, w);
		}
		else {
			V &= CHRmask1[0];
			setntamem(CHRptr[0] + (V << 10), 0, w);
		}
	}
	virtual void MMC5_hb(int scanline) {
		auto mod = (Reg[0x82] >> 4) & 7;
		if (mod != 1)
			return;
	}
};