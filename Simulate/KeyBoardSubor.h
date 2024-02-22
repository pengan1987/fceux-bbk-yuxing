#ifndef HEADER_KeyBoardSubor
#define HEADER_KeyBoardSubor
#include <Windows.h>
#include <dinput.h>
#include "VNes.h"
#include "Tools.h"

class KeyBoardSubor : public BaseKeyInput
{
public:
	bool bOut;
	BYTE ScanNo, *keys;

	KeyBoardSubor()
	{
		keys = nullptr;
	}
	BYTE OnRead(uint32 addr)
	{
		BYTE data = 0xff;
		if (0x4017 != addr || nullptr == keys)
			return data;

		switch (ScanNo) {
		case	1:
			if (bOut) {
				if (keys[DIK_4]) data &= ~0x02;
				if (keys[DIK_G]) data &= ~0x04;
				if (keys[DIK_F]) data &= ~0x08;
				if (keys[DIK_C]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F2]) data &= ~0x02;
				if (keys[DIK_E]) data &= ~0x04;
				if (keys[DIK_5]) data &= ~0x08;
				if (keys[DIK_V]) data &= ~0x10;
			}
			break;
		case	2:
			if (bOut) {
				if (keys[DIK_2]) data &= ~0x02;
				if (keys[DIK_D]) data &= ~0x04;
				if (keys[DIK_S]) data &= ~0x08;
				if (keys[DIK_END]) data &= ~0x10;
				//if (keys[DIK_END] || keys[DIK_LMENU]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F1]) data &= ~0x02;
				if (keys[DIK_W]) data &= ~0x04;
				if (keys[DIK_3]) data &= ~0x08;
				if (keys[DIK_X]) data &= ~0x10;
			}
			break;
		case	3:
			if (bOut) {
				if (keys[DIK_INSERT]) data &= ~0x02;
				if (keys[DIK_BACK]) data &= ~0x04;
				if (keys[DIK_NEXT]) data &= ~0x08;
				if (keys[DIK_RIGHT]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F8]) data &= ~0x02;
				if (keys[DIK_PRIOR]) data &= ~0x04;
				if (keys[DIK_DELETE]) data &= ~0x08;
				if (keys[DIK_HOME]) data &= ~0x10;
			}
			break;
		case	4:
			if (bOut) {
				if (keys[DIK_9]) data &= ~0x02;
				if (keys[DIK_I]) data &= ~0x04;
				if (keys[DIK_L]) data &= ~0x08;
				if (keys[DIK_COMMA]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F5]) data &= ~0x02;
				if (keys[DIK_O]) data &= ~0x04;
				if (keys[DIK_0]) data &= ~0x08;
				if (keys[DIK_PERIOD]) data &= ~0x10;
			}
			break;
		case	5:
			if (bOut) {
				if (keys[DIK_RBRACKET]) data &= ~0x02;
				//if (keys[DIK_RETURN]) data &= ~0x04;
				if (keys[DIK_RETURN] || keys[DIK_NUMPADENTER]) data &= ~0x04;
				if (keys[DIK_UP]) data &= ~0x08;
				if (keys[DIK_LEFT]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F7]) data &= ~0x02;
				if (keys[DIK_LBRACKET]) data &= ~0x04;
				if (keys[DIK_BACKSLASH]) data &= ~0x08;
				if (keys[DIK_DOWN]) data &= ~0x10;
			}
			break;
		case	6:
			if (bOut) {
				if (keys[DIK_Q]) data &= ~0x02;
				if (keys[DIK_CAPITAL]) data &= ~0x04;
				if (keys[DIK_Z]) data &= ~0x08;
				//if (keys[DIK_TAB]) data &= ~0x10;
				if (keys[DIK_PAUSE]) data &= ~0x10;
			}
			else {
				if (keys[DIK_ESCAPE]) data &= ~0x02;
				if (keys[DIK_A]) data &= ~0x04;
				if (keys[DIK_1]) data &= ~0x08;
				if (keys[DIK_LCONTROL]) data &= ~0x10;
			}
			break;
		case	7:
			if (bOut) {
				if (keys[DIK_7]) data &= ~0x02;
				if (keys[DIK_Y]) data &= ~0x04;
				if (keys[DIK_K]) data &= ~0x08;
				if (keys[DIK_M]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F4]) data &= ~0x02;
				if (keys[DIK_U]) data &= ~0x04;
				if (keys[DIK_8]) data &= ~0x08;
				if (keys[DIK_J]) data &= ~0x10;
			}
			break;
		case	8:
			if (bOut) {
				if (keys[DIK_MINUS]) data &= ~0x02;
				if (keys[DIK_SEMICOLON]) data &= ~0x04;
				if (keys[DIK_APOSTROPHE]) data &= ~0x08;
				if (keys[DIK_SLASH]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F6]) data &= ~0x02;
				if (keys[DIK_P]) data &= ~0x04;
				if (keys[DIK_EQUALS]) data &= ~0x08;
				if (keys[DIK_LSHIFT] ||
					keys[DIK_RSHIFT]) data &= ~0x10;
			}
			break;
		case	9:
			if (bOut) {
				if (keys[DIK_T]) data &= ~0x02;
				if (keys[DIK_H]) data &= ~0x04;
				if (keys[DIK_N]) data &= ~0x08;
				if (keys[DIK_SPACE]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F3]) data &= ~0x02;
				if (keys[DIK_R]) data &= ~0x04;
				if (keys[DIK_6]) data &= ~0x08;
				if (keys[DIK_B]) data &= ~0x10;
			}
			break;
		case	10:
		{
			static bool bbb = true;
			if (bbb)
				break;
			if (bOut) {
				data &= ~0x1E;
			}
			else {
				data &= ~0x1E;
				//data &= ~0x02;
			}
			break;
		}
		case	11:
			if (bOut) {
				//if (keys[DIK_LMENU])
				//	data &= ~0x02;
				if (keys[DIK_NUMPAD4]) data &= ~0x04;
				if (keys[DIK_NUMPAD7]) data &= ~0x08;
				if (keys[DIK_F11]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F12]) data &= ~0x02;
				if (keys[DIK_NUMPAD1]) data &= ~0x04;
				if (keys[DIK_NUMPAD2]) data &= ~0x08;
				if (keys[DIK_NUMPAD8]) data &= ~0x10;
			}
			break;
		case	12:
			if (bOut) {
				if (keys[DIK_SUBTRACT]) data &= ~0x02;
				if (keys[DIK_ADD]) data &= ~0x04;
				if (keys[DIK_MULTIPLY]) data &= ~0x08;
				if (keys[DIK_NUMPAD9]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F10]) data &= ~0x02;
				if (keys[DIK_NUMPAD5]) data &= ~0x04;
				if (keys[DIK_DIVIDE]) data &= ~0x08;
				if (keys[DIK_NUMLOCK]) data &= ~0x10;
			}
			break;
		case	13:
			if (bOut) {
				if (keys[DIK_GRAVE]) data &= ~0x02;
				if (keys[DIK_NUMPAD6]) data &= ~0x04;
				//if (keys[DIK_PAUSE]) data &= ~0x08;
				if (keys[DIK_LMENU]) data &= ~0x08;
				//if (keys[DIK_SPACE]) data &= ~0x10;
				if (keys[DIK_TAB]) data &= ~0x10;
			}
			else {
				if (keys[DIK_F9]) data &= ~0x02;
				if (keys[DIK_NUMPAD3]) data &= ~0x04;
				if (keys[DIK_DECIMAL]) data &= ~0x08;
				if (keys[DIK_NUMPAD0]) data &= ~0x10;
			}
			break;
		}
		return	data;
	}
	virtual void OnWrite(uint32 addr, uint8 data)
	{
		if (addr != 0x4016)
			return;
		if (data == 0x05) {
			bOut = FALSE;
			ScanNo = 0;
			keys = GetKeyboard();
		}
		else if (data == 0x04) {
			if (++ScanNo > 13)
				ScanNo = 0;
			bOut = !bOut;
		}
		else if (data == 0x06) {
			bOut = !bOut;
		}
	}
};

#endif