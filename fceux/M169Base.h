#include "M169.h"

class YuXingCreate : public YuXing
{
public:

public:
	virtual void PPUhook(WORD scanline, WORD addr) {
		if ((addr & 0xF000) == 0x2000) {
			reg[4] = (addr >> 8) & 0x03;
			PEC586Hack = cmd_4800_8 && !cmd_5500_8;//ppu.cpp的开关,解决图像问题
			if (cmd_4800_8 && cmd_5500_8) {
				SetVRam8K((reg[4] << 2) + (reg[3] & 0x03));
				SetVRam4K(0, reg[3] << 1);
			}
		}
	}

	virtual ~YuXingCreate() {}
	virtual void Power(void)
	{
		extern CartInfo _CartInfoMapperBase;
		bVCD = (_CartInfoMapperBase.CRC32 == 0x011d8ebc);
		HookPPU();
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		SetWriteHandler(0x8000, 0xFFFF, Write);

		SetWriteHandler(0x4800, 0x4800, Write);
		SetWriteHandler(0x5500, 0x5501, Write);
		SetWriteHandler(0x4200, 0x4201, Write);
		SetWriteHandler(0x4205, 0x4205, Write);
		SetWriteHandler(0x4700, 0x4700, Write);
		//SetWriteHandler(0x8000, 0x8001, Write);

		SetWriteHandler(0x4202, 0x4203, Write);
		SetWriteHandler(0x4303, 0x4303, Write);

		SetReadHandler(0x6000, 0xFFFF, CartBR);

		SetWriteHandler(0x4016, 0x4017, Write);
		SetReadHandler(0x4016, 0x4017, Read);//神奇画板这贱货要两个端口同时开启才能检测到鼠标
		SetReadHandler(0x4207, 0x4207, Read);

		SetReadHandler(0x4304, 0x4305, Read);
		SetReadHandler(0x5002, 0x5002, Read);

		SetChrRamSize(nChrRamSize);

		if (bVCD) {
			AddCustomMenu(AddCustomMenu_KeyBoard | AddCustomMenu_Mouse);
		}

		__super::Power();
	}
	virtual bool OnReadRam(WORD addr, BYTE& data)
	{
		static DWORD dwT = 0;
		if (dwT != 0 && GetTickCount() - dwT > 1000) {
			dwT = 0;
			SetReadHandler(0x4016, 0x4016, AReadBack[0x4016]);
			SetReadHandler(0x4017, 0x4017, AReadBack[0x4017]);
			SetWriteHandler(0x4016, 0x4016, BWriteBack[0x4016]);
			SetWriteHandler(0x4017, 0x4017, BWriteBack[0x4017]);
		}
		auto b = __super::OnReadRam(addr, data);
		if (bVCD && addr == 0x4207 && mCDDriver.bReadComplete && mCDDriver.nBufMax < 1024 * 1024) {
			dwT = GetTickCount();
			//mTools.bCaptureMouse = false;
			mCDDriver.bReadComplete = false;
		}
		return b;
	}
	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		auto b = mKeyBoard.WindowProc(hwnd, uMsg, wParam, lParam);
		if (b)
			return b;
		b = mCDDriver.WindowProc(hwnd, uMsg, wParam, lParam);
		if (b)
			return b;
		return __super::WindowProc(hwnd, uMsg, wParam, lParam);
	}
	virtual bool OnDropFiles(const std::string& strFile)
	{
		auto b = __super::OnDropFiles(strFile);
		if (!b)
			return b;
		SetWriteHandler(0x4016, 0x4017, Write);
		SetReadHandler(0x4016, 0x4017, Read);
	}
	virtual void OnMenuCommandKeyBoard(UINT nID) {
		OnMenuCommandMouse(nID);
	}
	virtual void OnMenuCommandMouse(UINT nID) {
		if (bVCD) {
			SetReadHandler(0x4016, 0x4017, Read);
			SetWriteHandler(0x4016, 0x4017, Write);
			mTools.bCaptureMouse = true;
		}
	}

};