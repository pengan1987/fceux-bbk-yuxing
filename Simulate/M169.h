/*
启动 9.2/F 后出现 电脑VCD画面, 因为读5002没返回0xFF   虚函数 ReadLow/Read/WriteLow/Write 不能直接调用基类函数,要直接读写 CPU_MEM_BANK
VCD 文件(BIN)读取以0x800(2048)字节为单位,也是光驱的一个扇区
*/
#pragma once
#include<map>
#include "VNes.h"
#include "Tools.h"
#include "MapperBase.h"
#include "resource.h"
static bool bRead0xffa0Log = true, bWrite4205Log = true;
static VectorByte vByte;

class YuXingKeyBoard : public BaseKeyInput
{
	INT key_map_row;
	BYTE* keys;
public:
	YuXingKeyBoard()
	{
		key_map_row = 0;
		keys = nullptr;
	}
	virtual ~YuXingKeyBoard()
	{
	}

	bool OnRead(WORD addr, BYTE& data)
	{
		bool bDefault = false;
		switch (addr)
		{
		case 0x4207:
		{
			if (!keys)
				return true;
			static int key_map[14][8] = {	//YuXing_Keyboard data by tpu (fix by temryu)
				{DIK_ESCAPE, DIK_F9,         DIK_7,      DIK_R,       DIK_A,      0x00,          0x00,         DIK_LSHIFT},
				{0x00,       DIK_NUMPADENTER,0x00,       DIK_MULTIPLY,DIK_DIVIDE, DIK_UP,        DIK_BACKSPACE,DIK_F12   },
				{0x00,       0x00,           0x00,       DIK_ADD,     DIK_NUMLOCK,DIK_LEFT,      DIK_DOWN,     DIK_RIGHT },
				{0x00,       DIK_NUMPAD7,    DIK_NUMPAD8,DIK_SUBTRACT,DIK_NUMPAD9,DIK_F11,       DIK_END,      DIK_PGDN  },
				{DIK_F8,     DIK_6,          DIK_E,      DIK_RBRACKET,DIK_L,      DIK_Z,         DIK_N,        DIK_SPACE },
				{DIK_F7,     DIK_5,          DIK_W,      DIK_LBRACKET,DIK_K,      DIK_DELETE,    DIK_B,        DIK_SLASH },
				{DIK_F6,     DIK_4,          DIK_Q,      DIK_P,       DIK_J,      DIK_BACKSLASH, DIK_V,        DIK_PERIOD},
				{DIK_F5,     DIK_3,          DIK_EQUALS, DIK_O,       DIK_H,      DIK_APOSTROPHE,DIK_C,        DIK_COMMA },
				{DIK_F4,     DIK_2,          DIK_MINUS,  DIK_I,       DIK_G,      DIK_SEMICOLON, DIK_X,        DIK_M     },
				{DIK_F3,     DIK_1,          DIK_0,      DIK_U,       DIK_F,      DIK_CAPITAL,   DIK_LWIN,     0x00      },
				{DIK_F2, DIK_GRAVE, DIK_9, DIK_Y, DIK_D, DIK_LCONTROL, DIK_LMENU, DIK_SCROLL},
				{ DIK_F1,     DIK_RETURN,     DIK_8,      DIK_T,       DIK_S,      0x00,          DIK_RWIN,     DIK_APPS },
				{ DIK_DECIMAL,DIK_F10,        DIK_NUMPAD4,DIK_NUMPAD6, DIK_NUMPAD5,DIK_INSERT,    DIK_HOME,     DIK_PGUP },
				{ 0x00,       DIK_NUMPAD0,    DIK_NUMPAD1,DIK_NUMPAD3, DIK_NUMPAD2,DIK_SYSRQ,     DIK_TAB,      DIK_PAUSE },
			};
			//if (YX_type == 0)	//for D (without new keys)
			//{
			//	key_map[9][6] = 0x00;
			//	key_map[11][6] = 0x00;
			//	key_map[11][7] = 0x00;
			//}
			int r, i;
			data = 0;
			for (r = 0; r < 14; r++) {
				if (key_map_row & (1 << r)) {
					for (i = 0; i < 8; i++) {
						if (key_map[r][i]) {
							if (keys[key_map[r][i]])
								data |= 1 << i;
						}
						//special cases
						if ((r == 0) && (i == 7))
							if (keys[DIK_RSHIFT]) data |= 1 << i;
						if ((r == 10) && (i == 5))
							if (keys[DIK_RCONTROL]) data |= 1 << i;
						if ((r == 10) && (i == 6))
							if (keys[DIK_RMENU]) data |= 1 << i;
					}
				}
			}
			return true;
		}

		}
		return false;
	};
	bool OnWrite(WORD addr, BYTE data)
	{
		switch (addr)
		{
		case 0x4202:
		{
			static BYTE d;
			if (data == 0)
				keys = GetKeyboard();
			d = data;
			key_map_row &= 0xff00;
			key_map_row |= data;
			return true;
		}
		case 0x4203:
		case 0x4303:
		{
			key_map_row &= 0x00ff;
			key_map_row |= (data & 0x3f) << 8;
			return true;
		}
		}
		return false;
	}
};
class YuXingMouse
{
public:
	Tools& mTools;
	bool bUseMouse;
	BYTE reg[2][3], cbReadTimes, cbCurBit;
	DWORD dwMouseData;
	WORD wInit;
	YuXingMouse() : mTools(*pTools)
	{
		cbReadTimes = 0xff;
		ZeroMemory(reg, sizeof(reg));
	}
	void InitPos() {
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(mTools.mWndMain, &ptCursor);
		RECT rc;
		GetClientRect(mTools.mWndMain, &rc);
		BYTE cx = rc.right / 2 - ptCursor.x - 1;
		BYTE cy = rc.bottom / 2 - ptCursor.y - 1;
		POINT pt;
		pt.x = rc.right / 2;
		pt.y = rc.bottom / 2;
		ClientToScreen(mTools.mWndMain, &pt);
		SetCursorPos(pt.x, pt.y);
		int cbData = !mTools.bLButtonDown << 5 | !mTools.bRButtonDown << 4 | (cy & 0xc0) >> 6 | (cx & 0xc0) >> 4;
		dwMouseData = cbData << 24;
		dwMouseData |= (cy & 0x7f) << 16;
		dwMouseData |= (cx & 0x7f) << 8;

		mTools.WriteBits(dwMouseData, 24);
		dwMouseData = dwMouseData << 1;
	}
	bool OnWrite(WORD addr, BYTE data)
	{
		switch (addr)
		{
		case 0x4016:
		case 0x4017:
		{
			bUseMouse = false;
			if (!mTools.bCapture || !mTools.bCaptureMouse)
				return false;
			if (data != 0xfe && data != 0xff && data != 0x0 && data != 0x1)
				return false;
			wInit = (wInit << 8) | data;
			if (wInit != 0xfeff && wInit != 0x1)
				return false;
			bUseMouse = true;
			if (cbReadTimes == 0) {
				cbReadTimes++;
				InitPos();
			}
			break;
		}
		}
		return false;
	}
	virtual void OnNMI(WORD addr) {
		cbReadTimes = 0;
	}
	bool OnReadPos(WORD addr, BYTE& data) {
		if (cbReadTimes == 1) {
			data = 0xff;
			cbReadTimes++;
			cbCurBit = 0;
			return true;
		}
		if (cbCurBit == 7) {
			cbCurBit++;
			data = 0;
			return true;
		}
		if (cbCurBit == 8) {
			cbCurBit = 0;
			data = 0xff;
			dwMouseData = dwMouseData << 1;
			return true;
		}
		if (cbReadTimes++ <= 26) {
			cbCurBit++;
			data = dwMouseData & 0x80000000 ? 0xff : 0;
			dwMouseData = dwMouseData << 1;
			return true;
		}

		data = 8;
		return true;
	}

	bool OnRead(WORD addr, BYTE& data)
	{
		switch (addr)
		{
		case 0x4016:
		case 0x4017:
		{
			if (!mTools.bCapture || !mTools.bCaptureMouse || !bUseMouse)
				return false;
			return OnReadPos(addr, data);
		}
		}
		return false;
	}
};
static const BYTE CMD_YXVCD[][10] = {
{0x0,1,0x1},
{0x96,1,0x69},
{0xa5,6,0x6},
{0xaa,1,0x6}
};
/*
 ,33 DIK_COMMA ;27 DIK_SEMICOLON '28 DIK_APOSTROPHE [1A DIK_LBRACKET =D DIK_EQUALS ]1B DIK_RBRACKET DIK_RBRACKET \2B DIK_BACKSLASH
 `29 DIK_GRAVE /b5 DIK_DIVIDE *37 DIK_MULTIPLY  NumEnter 9c DIK_NUMPADENTER .34 DIK_PERIOD -0C DIK_MINUS MAIN  Enter 1C DIK_RETURN
 +0x4EDIK_ADD NUM. DIK_DECIMAL

0023 buff 00 05 06 07 08 0A 0F 10 11 12 14 18 4E 4F 55 56 57 5D 5E 68 6D 6F 7A
没有 [  Num-
11 显示恢复中文输入 15 恢复中文输入  68 显示恢复中文输入/恢复中文输入 DIK_NUMPAD6,0x76 DIK_R,0x73, DIK_A,0x74
可能  61-F10 VCD不能存盘不退 72-7 &
 */
static const SHORT cbKeyMap[] = {
	DIK_7,0x2,DIK_R,0x3,DIK_A,0x4,DIK_UP,0xd,DIK_BACK,0xe,
	DIK_ADD,0x13,DIK_LEFT,0x15,DIK_DOWN,0x16,DIK_RIGHT,0x17,DIK_NUMPAD7,0x19,DIK_NUMPAD8,0x1a,DIK_MINUS,0x1b,DIK_NUMPAD9,0x1c,DIK_END,0x1e,
	DIK_PGDN,0x1f,DIK_6,0x21,DIK_E,0x22,DIK_RBRACKET,0x23,DIK_L,0x24,DIK_Z,0x25,DIK_N,0x26,DIK_SPACE,0x27,DIK_END,0x28,DIK_5,0x29,DIK_W,0x2a,
	DIK_RBRACKET,0x2b,DIK_K,0x2c,DIK_DELETE,0x2d,DIK_B,0x2e,DIK_SLASH,0x2f,DIK_4,0x31,DIK_Q,0x32,DIK_P,0x33,DIK_J,0x34,DIK_BACKSLASH,0x35,
	DIK_V,0x36,DIK_PERIOD,0x37,DIK_3,0x39,DIK_EQUALS,0x3a,DIK_O,0x3b,DIK_H,0x3c,DIK_APOSTROPHE,0x3d,DIK_C,0x3e,
	DIK_COMMA,0x3f,DIK_2,0x41,DIK_MINUS,0x42,DIK_I,0x43,DIK_G,0x44,DIK_SEMICOLON,0x45,DIK_X,0x46,DIK_M,0x47,DIK_1,0x49,DIK_0,0x4a,
	DIK_U,0x4b,DIK_F,0x4c,DIK_GRAVE,0x51,DIK_9,0x52,DIK_Y,0x53,DIK_D,0x54,DIK_RETURN,0x59,DIK_8,0x5a,DIK_T,0x5b,DIK_S,0x5c,
	DIK_DECIMAL,0x60,DIK_F10,0x61,DIK_4,0x62,DIK_NUMPAD6,0x63,DIK_NUMPAD5,0x64,DIK_INSERT,0x65,DIK_HOME,0x66,DIK_PGUP,0x67,DIK_NUMPAD0,0x69,DIK_NUMPAD1,0x6A,DIK_NUMPAD3,0x6b,
	DIK_NUMPAD2,0x6c,DIK_TAB,0x6e,DIK_ESCAPE,0x70,DIK_7,0x72,
	DIK_NUMPADENTER,0x79,DIK_MULTIPLY,0x7b,DIK_DIVIDE,0x7c,DIK_UP,0x7d,
	//以下占位
	DIK_A,0x1,DIK_A,0x0,DIK_A,0x9,DIK_A,0xB,DIK_A,0xc,DIK_A,0x1d,DIK_A,0x20,DIK_A,0x30,DIK_A,0x38,DIK_A,0x40,DIK_A,0x48,DIK_A,0x4d,DIK_A,0x50,DIK_A,0x58,DIK_A,0x5f,
	DIK_A,0x71,DIK_A,0x73,DIK_A,0x74,DIK_A,0x75,DIK_A,0x76,DIK_A,0x77,DIK_A,0x78,DIK_A,0x7e,DIK_A,0x7a,DIK_A,0x7f,
	/*
1d 文件改名 20 打印设置  38 前选块 40 后选块 48 放弃编辑 4d 光标变长 50 VCD不能存盘退 58 帮助 5f 菜单第二下不退   71 打印设置 7e 菜单第二下退
		   30 向前跳一段 
1-打印设置  9-换行 B-*Num C-/Num 73-R 74-A 75-5Num 76-6 Shift+ 77-7 Shift+ 78-8 Shift+ 7A-.Shift+
0-ESC 7A-.Shift+
		   */
};

class YuXingCDDriver
{
/*
写4016: 值:  2:接收命令数据(8BIT),下一次写是一BIT(0:0 非0:1)
命令,长度(包含命令本身): 0x96,1 0xaa,2 0xa5,6
*/
public:
	bool bLog, bShift, bCtrl, bAlt;
	bool bMove, bRead, bCanReadData, bSuccess, bCDRead, bReadComplete, bKeyBoard;
	BYTE cb4016, cbKeyByteIndex;
	BYTE cbWrite, cbRead4016, cbCMD[20], cbBaseSector[3];
	BYTE* buf, *keys, cbKey;
	WORD wKeySend;
	const BYTE* pCMD;
	DWORD dwBaseSector;
	int nBuf, nBufBase, nBufSeek, nBufMax, nBitIndex, nCMDIndex, nKeySendBit;

	BYTE cbEnd;
	BYTE cbReg4016[0xff];
	BaseKeyInput& keyInput;
	YuXingCDDriver(BaseKeyInput& key) : keyInput(key){
	}

	const BYTE* FindCMD(BYTE cbCMD) {
		for (int i = 0; i < sizeof(CMD_YXVCD) / sizeof(CMD_YXVCD[0]); i++) {
			if (CMD_YXVCD[i][0] == cbCMD)
				return CMD_YXVCD[i];
		}
		return nullptr;
	}
	void Init() {
		//memset(&bLog, 0, (char*)&cbEnd - (char*)&bLog);
		cbReg4016[0] = 0x0;
		cbReg4016[2] = 0xf;
		cbReg4016[4] = 0;
		cbReg4016[6] = 0xf;
		nBufSeek = nBuf = 0;
		dwBaseSector = nKeySendBit = nBitIndex = 0;
		cbBaseSector[2] = 0xff;
		bCanReadData = bRead = bSuccess = bReadComplete = bKeyBoard = false;
		bShift = bCtrl = bAlt = bMove = false;
		bCDRead = true;
		cbKeyByteIndex = cbWrite = 0;
		cbKey = 0xff;
		pCMD = FindCMD(0);
		cbRead4016 = pCMD[2];
		//PrintUnMapKeys();
	}

	void PrintUnMapKeys() {
		vByte.clear();
		for (int j = 0; j <= 0x7f; j++) {
			bool bFind = false;
			for (int i = 0; i < sizeof(cbKeyMap) / sizeof(cbKeyMap[0]); i += 2) {
				if (j == cbKeyMap[i+1]) {
					bFind = true;
					break;
				}
			}
			if (!bFind)
				vByte.push_back(j);
		}
		DebugString(pTools->WriteVByte(vByte));
	}
	bool OnRead(WORD addr, BYTE& data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4016: {
			if (bKeyBoard) {
				if (cbKeyByteIndex != 1 && cbKeyByteIndex != 2)
					return false;
				cbKeyByteIndex = 2;
				data = KeyRead();
				break;
			}
			if (bCDRead) {
				data = cbRead4016 & 1 ? 0 : 0xf;
				bRead = false;
			}
			break;
		}
		case 0x4017: {
			if (bKeyBoard) {
				if (cbKeyByteIndex != 2 && cbKeyByteIndex != 0) {
					cbKeyByteIndex = 0;
					return false;
				}
				data = KeyRead();
				cbKeyByteIndex = 0;
				break;
			}
			if (bCDRead) {
				data = cbReg4016[cb4016];
				bCanReadData = true;
			}
			break;
		}
		case 0x4207: {
			if (!bCanReadData || !bCDRead)
				return false;
			bCanReadData = false;
			data = buf[nBuf];
			if (++nBuf >= nBufMax)
				bReadComplete = true;
			char szTitle[100];
			if (bReadComplete || nBuf % 0x800 == 0) {
				if (nBufMax < 1024 * 1024) {
					sprintf(szTitle, "[读取中...%02d/100]", int(double(nBuf) / (double)nBufMax * 100));
				}
				else {
					strcpy(szTitle, "[读取中...]");
				}
				pTools->SetTitle(szTitle, 1500);
			}
			break;
		}
		default:
			b = false;
			break;
		}
		return b;
	}
	bool OnWrite(WORD addr, BYTE data)
	{
		if (!buf)
			return false;
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4202:
		case 0x4203:
		case 0x4303: {
			bCanReadData = false;
			return false;
		}
		case 0x4016: {
			if (bMove) {
				bMove = false;
				cb4016 = data;
				if (data == 0 || data == 4) {
					cbWrite = cbWrite >> 1;
					if (data == 4)
						cbWrite = cbWrite | 0x80;
					//DebugString("Write PC=%04X addr=%04X cbWrite=%02X nBitIndex=%d data=%s", _CPU_PC, addr, cbWrite, nBitIndex, mTools.GetBitNumber(data));
					nBitIndex++;
					break;
				}
			}
			if (data == 0xff || data == 0xfe) {//键盘/鼠标
				bCDRead = false;
				KeyWrite(data);
				return false;
			}
			bool bUse = true;
			switch (data) {
			case 0: {
				break;
			}
			case 2: {//写入串行数据  (命令序列)
				bMove = true;
				bRead = false;
				break;
			}
			case 4: {//送出串行数据 响应程序
				cbRead4016 = cbRead4016 >> 1;
				bRead = true;
				break;
			}
			case 6: {//操作完成
				if (!bRead) {
					UpdateStatus();
				}
				break;
			}
			default: {
				bUse = false;
				break;
			}
			}
			if (bUse) {
				bCDRead = true;
				cb4016 = data;
				if (!bMove)
					nBitIndex = 0;
			}
			break;
		}
		default:
			b = false;
			break;
		}
		return b;
	}

	SHORT ConvertScanCode(SHORT cbKeyScan) {
		for (int i = 0; i < sizeof(cbKeyMap) / sizeof(cbKeyMap[0]); i += 2) {
			if (cbKeyScan == cbKeyMap[i]) {
				return cbKeyMap[i + 1];
			}
		}
		return 0xff;
	}
	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (WM_TIMER == uMsg && wParam == 500) {
			KillTimer(hwnd, wParam);
			cbKey = 0xff;
			return false;
		}
		if (uMsg != WM_COMMAND) {
			return false;
		}
		auto cmd = HIWORD(wParam);
		auto id = LOWORD(wParam);
		if(cmd != BN_CLICKED || id != ID_KEY_SEND)
			return false;
		static BYTE cbK = 0x59;//20
		char szText[20];

		if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_CHECK_SET_VALUE, BM_GETCHECK, 0, 0)) {
			SendDlgItemMessage(hwnd, IDC_EDIT_KEY_BOARD, WM_GETTEXT, sizeof(szText), (LPARAM)szText);
			cbK = strtol(szText, nullptr, 16);
		}
		bShift = (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_CHECK_STATUS1, BM_GETCHECK, 0, 0));
		bCtrl = (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_CHECK_STATUS2, BM_GETCHECK, 0, 0));
		bAlt = (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_CHECK_STATUS3, BM_GETCHECK, 0, 0));
		sprintf(szText, "%02X ", cbK, MapVirtualKey(cbK, MAPVK_VSC_TO_VK));
		SendDlgItemMessage(hwnd, IDC_EDIT_KEY_BOARD, WM_SETTEXT, 0, (LPARAM)szText);
		cbKey = cbK++;
		SetTimer(hwnd, 500, 500, nullptr);
		return false;
	}
	//键盘 准备读取
	bool KeyWrite(BYTE& data) {
		static WORD wInit;
		wInit = (wInit << 8) | data;
		if (wInit != 0xfffe)
			return false;

		bKeyBoard = true;
		if (cbKeyByteIndex != 0)
			return false;

		cbKeyByteIndex = 1;
		static DWORD dwPress;
		static BYTE tm = 0, keyPrev = 0xff;
		if (keyInput.GetPressKey(cbKey)) {
			auto k = ConvertScanCode(cbKey);
			if (k != 0xff) {
				tm = 0;
				cbKey = k;
			}
			else
				cbKey = 0xff;
		}
		BYTE c;
		if (keyInput.GetReleaseKey(c)) {
			dwPress = 0;
			tm = 0;
			cbKey = 0xff;
		}
		c = cbKey;
		if (cbKey != 0xff)
		{
			if (GetTickCount() - dwPress > 500 || tm <= 1) {
				if (tm++ == 0)
					dwPress = GetTickCount();
			}
			else {
				c = 0xff;
			}
		}
		c = ~c;
		wKeySend = (((c >> 2) & 0x18) | 3);
		BYTE mod = 0;
		if (HIWORD(GetKeyState(VK_CONTROL)))
			mod |= 1 << 6;
		if (HIWORD(GetKeyState(VK_SHIFT)))
			mod |= 1 << 5;
		if (HIWORD(GetKeyState(VK_MENU)))
			mod |= 1 << 7;
		if (bCtrl)
			mod |= 1 << 7;
		if (bShift)
			mod |= 1 << 6;
		if (bAlt)
			mod |= 1 << 5;
		mod = ~mod;
		mod &= 0xe0;
		wKeySend |= mod;
		if (cbKey == 0xff) {
			wKeySend &= 0xfc;
		}
		wKeySend = wKeySend << 8;
		wKeySend &= 0xff00;
		wKeySend |= (((c << 3) | 3) & 0xfb);
		while (BYTE(~c) != 0xff) {
			char szNum[20], szTemp[100];
			strcpy(szTemp, "000000000000000000000000");
			itoa(wKeySend, szNum, 2);
			strcpy(&szTemp[16 - strlen(szNum)], szNum);
			DebugString("cbKey=%02X wKeySend=%04X %s", cbKey & 0x7f, wKeySend, szTemp);
			break;
		}
		nKeySendBit = 16;
		return true;
	}

	//读取按键
	BYTE KeyRead() {
		BYTE b = wKeySend;
		if (--nKeySendBit <= 0)
			bKeyBoard = false;
		b = (wKeySend >> nKeySendBit)& 1;
		//DebugString("b=%d", b & 0xff);
		return b;
	}

	//操作完成
	void UpdateStatus() {
		cbCMD[nCMDIndex] = cbWrite;
		//DebugString("Write PC=%04X addr=%04X cbWrite=%02X cbRead4016=%02X nBitIndex=%d data=%s", _CPU_PC, addr, cbWrite, cbRead4016, nBitIndex, mTools.GetBitNumber(data));
		if (nCMDIndex == 0) {
			if (cbWrite == 0x6) {//传成功
				if (cbBaseSector[2] == 0xfe) {
					memcpy(cbBaseSector, &cbCMD[1], 3);
					nBufBase = nBufSeek;
				}
				bSuccess = true;
				//DebugString("Write 06 PC=%04X nBufSeek=%05X nBuf=%05X %s", _CPU_PC, nBufSeek, nBuf, mTools.WriteVByte(vByte));
			}
			if (cbWrite == 0x15) {//重传
				//MapperBase::pMapper->LogCallStack();
				//DebugString("Write 15 PC=%04X nBufSeek=%05X nBuf=%05X %s", _CPU_PC, nBufSeek, nBuf, mTools.WriteVByte(vByte));
				if (bSuccess)
					nBufSeek = 0x4800;
				else
					nBufSeek += 0x800;
				bSuccess = false;
				if (nBufSeek + 0x800 > nBufMax)
					nBufSeek = 0;
				nBuf = nBufSeek;
				return;
			}
			pCMD = FindCMD(cbWrite);
			vByte.clear();
			if (pCMD)
				cbRead4016 = pCMD[2];
		}
		nBitIndex = 0;
		if (!pCMD) {
			nCMDIndex = 0;
			vByte.clear();
			return;
		}
		vByte.push_back(cbWrite);
		if (++nCMDIndex >= pCMD[1]) {
			if (pCMD[0] == 0xa5) {//定位要读取数据的位置 pCMD 0:命令 1 2:扇区(0~0x4B),pCMD[3]>0x4b则进位 3:扇区(0~0x4B)
				DebugString("Write 4016 PC=%04X cbWrite=%02X nBuf=%05X %s", _CPU_PC, cbWrite, nBuf, pTools->WriteVByte(vByte));
				if (cbBaseSector[2] == 0xff) {
					cbBaseSector[2] = 0xfe;
				}
				else {
					int sec = (cbCMD[2] - cbBaseSector[1]) * 0x4b;
					if (cbCMD[3] >= cbBaseSector[2]) {
						sec = sec + (cbCMD[3] - cbBaseSector[2]);
					}
					else {
						sec -= 0x4b;
						sec = sec + ((cbCMD[3] + 0x4b) - cbBaseSector[2]);
					}
					nBuf = sec * 0x800 + nBufBase;
				}
				nBufSeek = nBuf;
			}
			nCMDIndex = 0;
		}
	}
};
class YuXingFDCDriver
{
public:
	int nFdcDrvSel;
	Tools& mTools;
	bool bWriteFloppy, bRead0x4304Log, bLog;
	LPBYTE pFdcDataPtr;
	typedef BYTE(YuXingFDCDriver::* pFdcCallback)(BYTE data);
	BYTE nFdcMainStatus, bFdcCommands[0x10], bFdcResults[0x10];
	int nFdcReadSize, nFdcResultsIndex, n4304Count;
	int nFDCStatus, nFdcHeadAddres, nFdcCylinder, nFdcRecord, nFdcNumber, nTotalBytes;
	struct FDC_CMD_DESC
	{
		FDC_CMD_DESC(int a, int b, pFdcCallback c, pFdcCallback d) :bWLength(a), bRLength(b), callbackRead(c), callbackWrite(d) {
			int ga = 0;
		}
		int	bWLength;
		int	bRLength;
		pFdcCallback callbackRead, callbackWrite;
	};
	FDC_CMD_DESC* pFdcCmd;
	bool bReading;

	int nFdcRead, nFdcWrite;
	typedef std::map<int, FDC_CMD_DESC*> MAP_FdcCallback;
	MAP_FdcCallback mMAP_FdcCallback;

#define DEF_FDC(n,a,b,c,d) mMAP_FdcCallback.insert(std::make_pair(n, new FDC_CMD_DESC(a, b, &YuXingFDCDriver::c, &YuXingFDCDriver::d)))

	YuXingFDCDriver() : mTools(*pTools)
	{
		bWriteFloppy = false;
		pFdcCmd = nullptr;
		n4304Count = 0;

		DEF_FDC(3, 3, 0, FdcReadDefault, FdcSpecifyWrite);
		DEF_FDC(4, 2, 1, FdcSenseDriveStatusRead, FdcSenseDriveStatusWrite);
		DEF_FDC(5, 9, 7, FdcReadDefault, FdcWriteDataWrite);
		DEF_FDC(6, 9, 7, FdcReadDataRead, FdcReadDataWrite);
		DEF_FDC(7, 2, 0, FdcReadDefault, FdcRecalibrateWrite);
		DEF_FDC(8, 1, 2, FdcSenseIntStatusRead, FdcSenseIntStatusWrite);
		DEF_FDC(0xA, 2, 7, FdcReadIDRead, FdcReadIDWrite);
		DEF_FDC(0xD, 6, 7, FdcFormatTrackRead, FdcFormatTrackWrite);
		DEF_FDC(0xF, 3, 0, FdcReadDefault, FdcSeekWrite);
	}
	BYTE FdcSeekWrite(BYTE data) {
		nFdcHeadAddres = (bFdcCommands[1] & 7) >> 2;
		nFdcCylinder = bFdcCommands[2];
		nFDCStatus = FDC_S0_SE;
		DEBUGOUTFLOPPY("FDC FdcSeekWrite PC=%04X data=%02X", _CPU_PC, data);
		return 0;
	}
	BYTE FdcReadDataRead(BYTE data) {
		nFdcMainStatus |= FDC_MS_DATA_IN;
		if (--nFdcReadSize >= 0) {
			if (bRead0x4304Log) {
				bRead0x4304Log = false;
				DEBUGOUTFLOPPY("FDC FdcReadDataRead PC=%04X", _CPU_PC);
			}
			data = *pFdcDataPtr++;
			vByte.push_back(data);
			nFdcRead = -1;
			return data;
		}

		mTools.WriteVByte(vByte);
		mTools.SetTitle("读盘中...", 1500);

		bRead0x4304Log = true;
		data = bFdcResults[nFdcResultsIndex++];

		if (bRead0x4304Log) {
			DEBUGOUTFLOPPY("FDC FdcReadDataRead PC=%04X", _CPU_PC);
		}
		return data;
	}

	BYTE FdcReadDataWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcReadDataWrite PC=%04X", _CPU_PC);
		bRead0x4304Log = true;
		BYTE C = bFdcCommands[2];	//磁道号
		BYTE H = bFdcCommands[3];	//磁头号
		BYTE R = bFdcCommands[4];	//扇区号
		BYTE N = bFdcCommands[5];
		nFdcReadSize = 2 * 0x100;
		nFdcResultsIndex = 0;
		//pFdcCmd->bRLength = nFdcReadSize + 0x7;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK C=%04X H=%04X R=%04X N=%04X", C, H, R, N);
		pFdcDataPtr = mTools.lpFloppy + LBA * 512;

		if (nFdcHeadAddres != H || nFdcCylinder != C || nFdcRecord != R) {
			DEBUGOUTBUFF("buff changeC=%04X H=%04X R=%04X N=%04X", C, H, R, N);
		}

		R++;
		if (19 == R)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
				if (80 == C)
					C = 0;
			}
		}
		bFdcResults[2] = C;	//磁道号
		bFdcResults[3] = H;	//磁头号
		bFdcResults[4] = R;	//扇区号
		bFdcResults[5] = N;
		bFdcResults[6] = R;
		//ZeroMemory(bFdcResults, 0);
		nFdcHeadAddres = H;
		nFdcCylinder = C;
		nFdcRecord = R;
		nFdcNumber = N;
		return 0;
	}

	BYTE FdcWriteDataWrite(BYTE data) {
		bReading = false;
		if (nFdcReadSize > 0) {
			nFdcWrite = pFdcCmd->bWLength - 1;
			nFdcMainStatus |= FDC_MS_DATA_IN;
			*pFdcDataPtr = data;
			pFdcDataPtr++;
			bWriteFloppy = true;
			vByte.push_back(data);
			if (--nFdcReadSize >= 1) {
				return 0;
			}

			bRead0x4304Log = true;
			nFdcResultsIndex = 0;
			nFdcRead = 0;
			return 0;
		}
		if (vByte.size() > 0) {
			DEBUGOUTFLOPPY("FDC FdcWriteDataWrite 读数据异常 PC=%04X", _CPU_PC);
		}
		DEBUGOUTFLOPPY("FDC FdcWriteDataWrite PC=%04X", _CPU_PC);
		bRead0x4304Log = false;
		bWrite4205Log = false;
		BYTE C = bFdcCommands[2];	//磁道号
		BYTE H = bFdcCommands[3];	//磁头号
		BYTE R = bFdcCommands[4];	//扇区号
		BYTE N = bFdcCommands[5];
		nFdcReadSize = 2 * 0x100;
		nFdcResultsIndex = 0;
		nFdcWrite = pFdcCmd->bWLength - 1;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK C=%04X H=%04X R=%04X N=%04X", C, H, R, N);
		pFdcDataPtr = mTools.lpFloppy + LBA * 512;
		R++;
		if (19 == R)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
				if (80 == C)
					C = 0;
			}
		}
		nFdcHeadAddres = H;
		nFdcCylinder = C;
		nFdcRecord = R;
		nFdcNumber = N;
		return 0;
	}

	BYTE FdcSenseDriveStatusRead(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseDriveStatusRead PC=%04X", _CPU_PC);
		/*
		>21:DBBA: 29 40     AND #$40
		 21:DBBC: D0 06     BNE $DBC4
		 */
		return 0x0;// 0x40; 0/0x40
	}

	BYTE FdcSenseDriveStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseDriveStatusWrite PC=%04X", _CPU_PC);
		//nFdcMainStatus = FDC_MS_RQM;// | FDC_MS_EXECUTION;
		return 0;
	}

	BYTE FdcReadDefault(BYTE data) {
		data = 0;
		data = bFdcResults[nFdcResultsIndex++];
		DEBUGOUTFLOPPY("FDC FdcReadDefault PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcWriteDefault(BYTE data) {
		nFdcMainStatus = FDC_MS_RQM;
		return 0;
	}
	BYTE FdcFormatTrackRead(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcFormatTrackRead PC=%04X", _CPU_PC);
		return 0;
	}
	BYTE FdcFormatTrackWrite(BYTE data) {
		//bCPUHookLog = true;
		bReading = false;
		if (nFdcReadSize > 0) {
			nFdcWrite = pFdcCmd->bWLength - 1;
			nFdcMainStatus = FDC_MS_RQM; //未完成时必须这个状态
			bWriteFloppy = true;
			vByte.push_back(data);
			if (--nFdcReadSize >= 1) {
				return 0;
			}
			for (int i = vByte.size() / 4 - 1; i >= 0; i--)
			{//可能漏了扇区
				BYTE C = vByte[i * 4 + 0];
				BYTE H = vByte[i * 4 + 1];
				BYTE R = vByte[i * 4 + 2];
				INT LBA = H * 18 + C * 36 + (R - 1);
				pFdcDataPtr = mTools.lpFloppy + LBA * 512;
				ZeroMemory(pFdcDataPtr, 512);
			}
			mTools.WriteVByte(vByte);
			mTools.SetTitle("写盘中...", 1500);

			bRead0x4304Log = true;
			nFdcResultsIndex = 0;
			nFdcRead = 0;
			nFdcMainStatus |= FDC_MS_DATA_IN;//
			return 0;
		}
		if (vByte.size() > 0) {
			DEBUGOUTFLOPPY("FDC FdcWriteDataWrite 读数据异常 PC=%04X", _CPU_PC);
		}
		DEBUGOUTFLOPPY("FDC FdcFormatTrackWrite PC=%04X data=%02X", _CPU_PC, data);
		bRead0x4304Log = false;
		//bWrite4205Log = false;
		BYTE C = bFdcCommands[1];	//磁道号
		BYTE H = bFdcCommands[2];	//磁头号
		BYTE R = bFdcCommands[3];	//扇区号
		BYTE N = bFdcCommands[4];
		nFdcMainStatus = FDC_MS_RQM; //格式化
		nFdcReadSize = 4 * R;// R组扇区信息
		nFdcResultsIndex = 0;
		nFdcWrite = pFdcCmd->bWLength - 1;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK C=%04X H=%04X R=%04X N=%04X", C, H, R, N);
		pFdcDataPtr = mTools.lpFloppy + LBA * 512;
		if (++R >= 19)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
				if (80 == C)
					C = 0;
			}
		}
		return 0;
	}
	BYTE FdcReadIDRead(BYTE data) {
		//读7字节
		BYTE d[7] = { 0 };
		d[1] = 0;// 其它可能没用到 第1字节返回磁盘格式(0/1/4/5) 1/4/5格式不对
		//d[4] = 2;
		//d[5] = 2;
		//d[6] = 2;
		/*
	DB5A STA $0326,X @ $0327 = #$00
  DB87 LDA $0327 = #$00
  DB8A AND #$05
  DB8C BEQ $DB91
		*/
		//data = bFdcResults[nFdcResultsIndex++];
		//return data;
		//return bFdcResults[nFdcRead];
		data = d[nFdcRead];// d[nFdcRead];
		DEBUGOUTFLOPPY("FDC FdcReadIDRead PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcReadIDWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcReadIDWrite PC=%04X", _CPU_PC);
		ZeroMemory(bFdcResults, sizeof(bFdcResults));
		bFdcResults[0] = 0;
		bFdcResults[1] = nFDCStatus;
		bFdcResults[4] = nFdcHeadAddres;
		bFdcResults[5] = nFdcRecord;
		bFdcResults[6] = nFdcNumber;
		return 0;
	}

	BYTE FdcRecalibrateWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcRecalibrateWrite PC=%04X", _CPU_PC);
		nFDCStatus = FDC_S0_SE;
		return 0;
	}

	BYTE FdcSenseIntStatusRead(BYTE data) {
		//读2字节
		BYTE d[2] = { 0x20, 0 };
		/* 8.3WPS 返回 0x20 会错误    9.2 不返回0x20会错误
		0->326 0~20 0:驱动器错误
			  D958 STA $0326 = #$FF
			DAFA LDA $0326 = #$20
			DAFD AND #$20
		1->327->31F
		D808 LDA $032E = #$00
		D80B CMP $031F = #$20
		*/
		data = d[nFdcRead];
		DEBUGOUTFLOPPY("FDC FdcSenseIntStatusRead PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcSenseIntStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseIntStatusWrite PC=%04X", _CPU_PC);
		return 0;
	}

	BYTE FdcSpecifyRead(BYTE data) {
		return 0;
	}
	BYTE FdcSpecifyWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSpecifyWrite PC=%04X", _CPU_PC);
		return 0;
	}
	//获取驱动器当前状态
	BYTE Read0x4304(WORD addr) {
		if (++n4304Count > 20)//解决慢速(快速读的方式有区别,读完一个扇区后会有正常的4205写入)读盘/D型磁盘管理/其它应用软件正常读盘
			nFdcMainStatus = FDC_MS_RQM;
		if (n4304Count > 40)//说明读盘程序进入死循环了,暴力终止进程
			ExitProcess(0);
		//if (bRead0x4304Log)
		{
			bLog = false;
			DEBUGOUTFLOPPY("FDC Read0x4304 PC=%04X addr=%04X data=%02X", _CPU_PC, addr, nFdcMainStatus);
		}
		return nFdcMainStatus;
	}
	//读扇区或软驱状态
	BYTE Read0x4305(WORD addr) {
		n4304Count = 0;
		if (!pFdcCmd) {
			DEBUGOUTFLOPPY("FDC Read0x4305 PC=%04X addr=%04X data=%02X", _CPU_PC, addr, 0);
			return 0;
		}
		nTotalBytes++;
		BYTE data = (this->*pFdcCmd->callbackRead)(nFdcRead);
		if (++nFdcRead >= pFdcCmd->bRLength) {
			nFdcMainStatus = FDC_MS_RQM;
			pFdcCmd = nullptr;
			bReading = false;
		}
		//if (bRead0x4304Log)
		{
			bLog = false;
			DEBUGOUTFLOPPY("FDC Read0x4305 PC=%04X addr=%04X 0x4305=%02X", _CPU_PC, addr, data);
		}
		return data;
	}
	bool OnRead(WORD addr, BYTE & data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4304:
			data = Read0x4304(data);
			break;
		case 0x4305:	// FDCDMADackIO
			data = Read0x4305(data);
			break;
		default:
			b = false;
			break;
		}
		if (b && bLog)
		{
			DEBUGOUTFLOPPY("FDC OnRead PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		}
		return b;
	}
	void Write0x4201(WORD addr, BYTE data) {
		bLog = false;
		DEBUGOUTFLOPPY("FDC Write0x4201 PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		if (data == 4) {
			mTools.SetTitle();
			bCPUHookLog = false;
			if (bWriteFloppy)
				mTools.FloppyIMGSave();
			bWriteFloppy = false;
		}
		if (data & 0x80) {
			vByte.clear();
			nTotalBytes = 0;
			bReading = false;
			bWrite4205Log = true;
			//bCPUHookLog = true;
			nCPUHookLogIndex = 6;
			ZeroMemory(bFdcCommands, sizeof(bFdcCommands));
			nFdcMainStatus = FDC_MS_RQM;// data;
			pFdcCmd = nullptr;
			nFdcRead = nFdcWrite = 0;
			return;
		}
	}
	void Write0x4205(WORD addr, BYTE data) {
		n4304Count = 0;
		//if (bWrite4205Log)
		{
			static int n = 0;
			bLog = false;
			DEBUGOUTFLOPPY("FDC Write0x4205 PC=%04X addr=%02X data=%02X n=%04d", _CPU_PC,addr, data, n);
			if (data == 0x45) {
				if (n == 1) {
					n = 1;
				}
				n++;
			}
		}
		//if (vByte.size() >= 0x200) {
		//	DEBUGOUTFLOPPY("FDC Write0x4205 数据异常 %03d PC=%04X 0x4205=%02X", vByte.size(), _CPU_PC, data);
		//	WriteVByte();
		//}
		if (pFdcCmd && !bReading) {
			bFdcCommands[nFdcWrite++] = data;
		}
		else {
			BYTE nFdcCmd = data & 0x1f;
			nFdcRead = 0;
			nFdcWrite = 0;
			nFdcResultsIndex = 0;
			nFdcReadSize = 0;
			bFdcCommands[nFdcWrite++] = data;
			MAP_FdcCallback::iterator it = mMAP_FdcCallback.find(nFdcCmd);
			if (it == mMAP_FdcCallback.end()) {
				nFdcMainStatus = FDC_MS_RQM;
				nFDCStatus = FDC_S0_IC1;
				DEBUGOUTFLOPPY("FDC Unkonw Command Write0x4205 PC=%04X addr=%04X 0x4205=%02X", _CPU_PC, addr, data);
				return;
			}
			pFdcCmd = it->second;
		}
		if (nFdcWrite >= pFdcCmd->bWLength) {
			bReading = true;
			if (pFdcCmd->bRLength > 0)
				nFdcMainStatus |= FDC_MS_DATA_IN;
			(this->*pFdcCmd->callbackWrite)(data);
			if (nFdcRead >= pFdcCmd->bRLength) {
				bReading = false;
				pFdcCmd = nullptr;
			}
		}
	}

	bool OnWrite(WORD addr, BYTE data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4201:	// FDCDRQPortI/FDCCtrlPortO
			Write0x4201(addr, data);
			break;
		case 0x4205:	// FDCDataPortIO
			Write0x4205(addr, data);
			break;
		default:
			b = false;
			break;
		}
		if (b && bLog)
		{
			DEBUGOUTFLOPPY("FDC OnWrite PC=%04X addr=%04X 0x4205=%02X", _CPU_PC, addr, data);
		}
		return b;
	}
};

class YuXing : public MapperBase
{
public:
	YuXing() : mCDDriver(mKeyBoard)
	{
	}
	BYTE reg[8], cmd_4800_6, cmd_4800_7, cmd_4800_8, cmd_5500_3;
	BYTE cmd_5500_8, cmd_5501_8;
	typedef void(YuXing::*PWRITEFun)(WORD addr, BYTE data);
	typedef std::map<int, PWRITEFun> MAP_PWRITEFun;
	MAP_PWRITEFun mMAP_PWRITEFun;
#define WRITE_FUN(addr,fun) mMAP_PWRITEFun.insert(std::make_pair(addr, &YuXing::fun))
	typedef BYTE(YuXing::*PReadFun)(WORD addr);
	typedef std::map<int, PReadFun> MAP_PReadFun;
	MAP_PWRITEFun mPReadFun;
#define READ_FUN(addr,fun) mPReadFun.insert(std::make_pair(addr, &YuXing::fun))
	YuXingFDCDriver mFDCDriver;
	YuXingKeyBoard mKeyBoard;
	YuXingMouse mMouse;
	YuXingCDDriver mCDDriver;
protected:
	BYTE	MMC3_mode, MMC3_reg, MMC3_prg0, MMC3_prg1;
	BYTE	MMC3_chr4, MMC3_chr5, MMC3_chr6, MMC3_chr7;

	static const int nWRamSize = 0;
	static const int nPrgRamSize = 1024;
	static const int nChrRamSize = 128;;
	static const int nChrRamCount1 = nChrRamSize - 1;
	static const int nChrRamCount2 = nChrRamSize / 2 - 1;
	static const int nPrgRamCount8 = nPrgRamSize / 8 - 1;
	static const int nPrgRamCount16 = nPrgRamSize / 16 - 1;
	bool bMapper, bVCD;

public:
	virtual ~YuXing()
	{
	}

	virtual int GetPRGRamSize() { return nPrgRamSize * 1024; }
	virtual int GetCHRRamSize() { return nChrRamSize * 1024; }
	virtual int GetWorkRamSize() { return nWRamSize * 1024; }
	virtual void OnNMI(WORD addr) {
		__super::OnNMI(addr);
		mMouse.OnNMI(addr);
	}

	bool TryLoadVCDFile(const std::string& strFile) {
		if (!bVCD)
			return false;
		auto f = fopen(strFile.c_str(), "rb");
		if (!f) {
			return false;
		}
		fseek(f, 0, SEEK_END);
		mCDDriver.nBufMax = ftell(f);
		if (mCDDriver.nBufMax > 1492992) {
			fclose(f);
			return false;
		}
		mCDDriver.buf = new BYTE[mCDDriver.nBufMax];
		fseek(f, 0, SEEK_SET);
		fread(mCDDriver.buf, mCDDriver.nBufMax, 1, f);
		fclose(f);

		mCDDriver.Init();
		mTools.strImageFile = strFile;
		mTools.enumTitleStatus = Tools::EnumTitleStatusNone;
		mTools.SetTitle("");
		ResetBank();
		ResetNES();
		return true;
	}

	virtual bool OnDropFiles(const std::string& strFile) {
		return TryLoadVCDFile(strFile);
	}

	void Power()
	{
		__super::Power();
		if (!bVCD) {
			ResetBank();
			auto p = mTools.GetDefaultMedia();
			if (p)
				mTools.strImageFile = p;
			mTools.FloppyIMGLoad();
			return;
		}
		auto p = mTools.GetDefaultMedia("bin");
		if (p)
			TryLoadVCDFile(p);
		else
			ResetBank();
	}

	void ResetBank() {
		mTools.bCanSetTitle = false;//硬件重置时 SetTitle 会阻塞
		mTools.Reset();
		mTools.bCaptureMouse = true;
		VideoMode(VideoModeDendy);
		mTools.bCanSetTitle = true;
		mTools.bDebugString = true;
		memset(reg, 0, sizeof(reg));
		reg[0] = 0xFF;	//$5002
		reg[7] = 0x01;
		cmd_4800_6 = 0x00;
		cmd_4800_7 = 0x00;
		cmd_4800_8 = 0x00;
		cmd_5500_3 = 0x00;
		cmd_5500_8 = 0x00;
		cmd_5501_8 = 0x00;
		MMC3_mode = 0x00;

		SetVRam8K(0);
		SetRom32K(0);
	}

	virtual bool OnReadRam(WORD addr, BYTE& data)
	{
		if (bVCD && mCDDriver.OnRead(addr, data))
			return true;
		if (mKeyBoard.OnRead(addr, data))
			return true;
		if (mFDCDriver.OnRead(addr, data))
			return true;
		if (mMouse.OnRead(addr, data))
			return true;
		switch (addr)
		{
		case 0x5002:
			bVCD ? 0 : data = 0xff;//没有这句 9.2/F 会停留在VCD画面
			return true;
		case 0x4016:
		case 0x4017:
			return AReadBack[addr](addr);
		default:
			break;
		}
		return false;
	}
	void MMC3_SetBank_CPU()
	{
		SetRam8K(4, MMC3_prg0);
		SetRam8K(5, MMC3_prg1);
	}
	void MMC3_SetBank_PPU()
	{
		SetVRam1K(4, MMC3_chr4 & 0x0F);
		SetVRam1K(5, MMC3_chr5 & 0x0F);
		SetVRam1K(6, MMC3_chr6 & 0x0F);
		SetVRam1K(7, MMC3_chr7 & 0x0F);
	}
	bool MMC3_WriteH(WORD addr, BYTE data)
	{
		auto b = true;
		switch (addr) {
		case	0x8000:
			MMC3_reg = data;
			MMC3_SetBank_CPU();
			MMC3_SetBank_PPU();
			break;
		case	0x8001: {
			switch (MMC3_reg & 0x07) {
			case	0x02:
				MMC3_chr4 = data;
				MMC3_SetBank_PPU();
				break;
			case	0x03:
				MMC3_chr5 = data;
				MMC3_SetBank_PPU();
				break;
			case	0x04:
				MMC3_chr6 = data;
				MMC3_SetBank_PPU();
				break;
			case	0x05:
				MMC3_chr7 = data;
				MMC3_SetBank_PPU();
				break;
			case	0x06:
				MMC3_prg0 = data;
				MMC3_SetBank_CPU();
				break;
			case	0x07:
				MMC3_prg1 = data;
				MMC3_SetBank_CPU();
				break;
			}
			break;
		}
		default:
			b = false;
		}
		return b;
	}
	virtual bool OnWriteRam(WORD addr, BYTE& data)
	{
		MAP_PWRITEFun::iterator it = mMAP_PWRITEFun.find(addr);
		if (it != mMAP_PWRITEFun.end())
		{
			(this->*it->second)(addr, data);
			return true;
		}
		if (bVCD && mCDDriver.OnWrite(addr, data))
			return true;
		if (mKeyBoard.OnWrite(addr, data))
			return true;
		if (mFDCDriver.OnWrite(addr, data))
			return true;
		if (mMouse.OnWrite(addr, data))
			return true;
		bool bWrite = true;
		switch (addr)
		{
		case 0x4700:
		{
			//DEBUGOUTTEST("OnWriteRam PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
			//mTools.WriteVByte(vByte2);
			//vByte1.clear();
			break;
		}
		case 0x4800:
		{
			reg[1] = data;
			cmd_4800_6 = reg[1] & 0x20;	//获取第6位，RAM开关，1打开，0关闭 bit5=1就是IO保护
			cmd_4800_7 = reg[1] & 0x40;	//获取第7位
			cmd_4800_8 = reg[1] & 0x80;	//获取第8位，用于控制PPU_ExtLatch

			SetBank_CPU();
			break;
		}
		case 0x5500:
		{
			reg[2] = data;
			cmd_5500_3 = reg[2] & 0x04;	//获取第3位，与控制RAM有关
			cmd_5500_8 = reg[2] & 0x80;	//获取第8位，用于控制PPU_Latch

			SetBank_CPU();
			break;
		}
		case 0x5501:
		{
			reg[3] = data;
			cmd_5501_8 = reg[3] & 0x80;	//获取第8位，开启MMC3模式，调用“神奇画板”选项(???)

			SetVRam8K((reg[3] & 0x7F));
			if (cmd_5501_8) {
				MMC3_mode = 1;
				SetRam16K(6, 0x1F);
			}
			else
			{
				MMC3_mode = 0;
			}
			break;
		}
		case 0x4016:
		case 0x4017: {
			BWriteBack[addr](addr, data);
			return true;
		}
		default:
		{
			if (cmd_5500_3) {
				if (MMC3_mode) {
					MMC3_WriteH(addr, data);
					break;
				}
				else if (cmd_4800_6) {
					reg[5] = data & 0x3F;
					break;
				}
			}
			bWrite = false;
			break;
		}
		}
		if (bVCD && cmd_5500_3 && cmd_4800_6 && addr >= 0x8000) {
			auto d = (addr >> 14) & 3;
			switch (d)
			{
			case 2: {
				reg[5] = data & nPrgRamCount16;
				SetBank_CPU();
				break;
			}
			//case 3: {
			//	SetRam16K(6, data & nPrgRamCount16);
			//	break;
			//}
			default:
				break;
			}
		}
		return bWrite;
	}
	//存储器切换
	void SetBank_CPU()
	{
		if (cmd_5500_3) {
			if (MMC3_mode)
				return;
			SetRam16K(4, reg[5]);
			SetRam16K(6, 0x3f);
		}
		else
		{
			SetRom32K((reg[1] & 0x1F));
		}
		if ((reg[2] & 0x3) == 0)
			SetRam8K(3, 0x3C);
		else
			SetRam8K(3, reg[2] & 0x3);
	}
};