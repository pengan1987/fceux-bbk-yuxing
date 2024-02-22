#pragma once

//#include "MapperBase.h"
#include "VNes.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

//for FDC code by fanoble
#define FDC_MS_BUSYS0		0x01	// FDD0 in SEEK mode
#define FDC_MS_BUSYS1		0x02	// FDD1 in SEEK mode
#define FDC_MS_BUSYS2		0x04	// FDD2 in SEEK mode
#define FDC_MS_BUSYS3		0x08	// FDD3 in SEEK mode
#define FDC_MS_BUSYRW		0x10	// Read or Write in progress
#define FDC_MS_EXECUTION	0x20	// Execution Mode
#define FDC_MS_DATA_IN		0x40	// Data input or output
#define FDC_MS_RQM			0x80	// Request for Master, Ready

#define FDC_S0_US0			0x01	// Unit Select 0
#define FDC_S0_US1			0x02	// Unit Select 1
#define FDC_S0_HD			0x04	// Head Address
#define FDC_S0_NR			0x08	// Not Ready
#define FDC_S0_EC			0x10	// Equipment Check
#define FDC_S0_SE			0x20	// Seek End
#define FDC_S0_IC0			0x40	// Interrupt Code
#define FDC_S0_IC1			0x80	// NT/AT/IC/XX

#define FDC_S1_MA			0x01	// Missing Address Mark
#define FDC_S1_NW			0x02	// Not Writable
#define FDC_S1_ND			0x04	// No Data
#define FDC_S1_OR			0x10	// Over Run
#define FDC_S1_DE			0x20	// Data Error
#define FDC_S1_EN			0x80	// End of Cylinder

#define FDC_S2_MD			0x01	// Missing Address Mark in Data Field
#define FDC_S2_BC			0x02	// Bad Cylinder
#define FDC_S2_SN			0x04	// Scan Not Satisfied
#define FDC_S2_SH			0x08	// Scan Equal Hit
#define FDC_S2_WC			0x10	// Wrong Cylinder
#define FDC_S2_DD			0x20	// Data Error in Data Field
#define FDC_S2_CM			0x40	// Control Mark

#define FDC_S3_US0			0x01	// Unit Select 0
#define FDC_S3_US1			0x02	// Unit Select 1
#define FDC_S3_HD			0x04	// Side Select
#define FDC_S3_TS			0x08	// Two Side
#define FDC_S3_T0			0x10	// Track 0
#define FDC_S3_RY			0x20	// Ready
#define FDC_S3_WP			0x40	// Write Protect
#define FDC_S3_FT			0x80	// Fault

#define FDC_CC_MASK						0x1F
#define FDC_CF_MT						0x80
#define FDC_CF_MF						0x40
#define FDC_CF_SK						0x20
#define FDC_CC_READ_TRACK				0x02
#define FDC_CC_SPECIFY					0x03
#define FDC_CC_SENSE_DRIVE_STATUS		0x04
#define FDC_CC_WRITE_DATA				0x05
#define FDC_CC_READ_DATA				0x06
#define FDC_CC_RECALIBRATE				0x07
#define FDC_CC_SENSE_INTERRUPT_STATUS	0x08
#define FDC_CC_WRITE_DELETED_DATA		0x09
#define FDC_CC_READ_ID					0x0A
#define FDC_CC_READ_DELETED_DATA		0x0C
#define FDC_CC_FORMAT_TRACK				0x0D
#define FDC_CC_SEEK						0x0F
#define FDC_CC_SCAN_EQUAL				0x11
#define FDC_CC_SCAN_LOW_OR_EQUAL		0x19
#define FDC_CC_SCAN_HIGH_OR_EQUAL		0x1D
#define FDC_PH_IDLE						0
#define FDC_PH_COMMAND					1
#define FDC_PH_EXECUTION				2
#define FDC_PH_RESULT					3

#pragma pack(1)
struct BPB {
	u16  BPB_BytsPerSec;	//每扇区字节数
	u8   BPB_SecPerClus;	//每簇扇区数
	u16  BPB_RsvdSecCnt;	//Boot记录占用的扇区数
	u8   BPB_NumFATs;	//FAT表个数
	u16  BPB_RootEntCnt;	//根目录最大文件数
	u16  BPB_TotSec16;
	u8   BPB_Media;
	u16  BPB_FATSz16;	//FAT扇区数
	u16  BPB_SecPerTrk;
	u16  BPB_NumHeads;
	u32  BPB_HiddSec;
	u32  BPB_TotSec32;	//如果BPB_FATSz16为0，该值为FAT扇区数
};
#pragma pack()

typedef std::map<int, int> MAPData;
typedef std::vector<BYTE> VectorByte;
typedef std::vector<VectorByte> VectorByteVectorByte;
extern bool bCPUHookLog, bIDC_CHECK_IRQ, bIDC_CHECK_KEYBOARD, bIDC_CHECK_LogOut;
extern int nCPUHookLogIndex;
extern VectorByte vByte, vByte1, vByte2;
typedef void (*PInitFunction)();
void AddInitFunction(PInitFunction pPInitFunction);

class Tools {
public:
	Tools();
	~Tools();
	LPBYTE FloppyIMGLoad(LPBYTE lpFloppyWrite = nullptr, bool bChange = false);
	LPBYTE FloppyIMGSave();
	void SetTitle(const std::string strSet = "", DWORD dwDelayHide = 0);
	void ScanFolder(const std::string& strFile, std::vector<std::string>& vectorPath);
	void ScanRoms(std::vector<std::string>& vectorPath);
	SHORT ShowCursor(bool bShow);
	void Close(void);
	void Reset();
	char* GetBitNumber(BYTE data);
	char* LogMAPData(MAPData &mapData);
	char* GetDefaultMedia(const char* pszExt = nullptr /*扩展名不带点 小写*/);//获取最近打开列表中的媒体文件 
	char* WriteBits(DWORD dwData, int nBits);
	char* WriteVByte(VectorByte& vByte, bool bClear = true);
	bool OnDropFiles(const char* ftmp);
	bool isDisableIRQ();
	void HookWindow(HWND hMain, HWND hView);
	void PlayVoice(VectorByte&vbByte);
	void MenuImgListUpdate();
	void MenuImgListSave(const char *pszPath = nullptr);
	void MenuImgListCommand(UINT nID);
	bool ExplorerFolder(const char* pszPath);
	void OnLoadROM(const char* pszPath);
	void ReleadROM();
	UINT NewIDMenu(int nMax = 1);

	bool bCanSetTitle, bScanRom, bLButtonDown, bRButtonDown, bCapture, bCaptureMouse;
	bool bDebugString, bFloppySave, bFocus, bMe;
	HWND mWndMain, mWndTitle, mWndDebug, mWndView;
	HMENU mMenuImgList;
	LPBYTE	lpFloppy;
	static HINSTANCE hInstance;
	std::string strImageFile, strNesFile;
	enum EnumTitleStatus {
		EnumTitleStatusNone,
		EnumTitleStatusReadOnly,
		EnumTitleStatusWriteable,
	};
	EnumTitleStatus enumTitleStatus;

protected:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool WindowProcCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static DWORD WINAPI sThreadVoice(LPVOID lParam);
	static DWORD WINAPI s_WindowDialogThreadProc(__in  LPVOID lpParameter);

	LONG_PTR mWNDPROC, mWNDPROCView;
	DWORD dwTickTitle;
};
extern Tools* pTools;

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>
#include <dinputd.h>
#include <list>
class BaseKeyInput {
protected:
	LPDIRECTINPUTDEVICE7 lpdid = 0;
	LPDIRECTINPUT7 lpDI = 0;
	BYTE keys[256], keysPress[256];
public:
	struct KeyInfo {
		BYTE cbKey;
		bool bPress;
	};
	std::list<KeyInfo> listKeyInfo;
	bool bUseKeyBoard;

	BaseKeyInput() {
		bUseKeyBoard = false;
		KeyboardInitialize();
		memset(keys, 0, sizeof(keys));
		memset(keysPress, 0, sizeof(keysPress));
	}
	~BaseKeyInput() {
		if (lpDI) {
			lpdid->Acquire();
			IDirectInput7_Release(lpDI);
		}
	}
	BYTE* GetKeyboard(bool bTestRelease = false);
	virtual bool BaseKeyInput::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool GetReleaseKey(BYTE& key);
	bool GetPressKey(BYTE& key);
	int KeyboardInitialize(void)
	{
		//mbg merge 7/17/06 changed:
		auto ddrval = DirectInputCreateEx(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput7, (LPVOID*)&lpDI, 0);
		ddrval = IDirectInput7_CreateDeviceEx(lpDI, GUID_SysKeyboard, IID_IDirectInputDevice7, (LPVOID*)&lpdid, 0);
		if (ddrval != DI_OK)
		{
			return 0;
		}ddrval = IDirectInputDevice7_SetCooperativeLevel(lpdid, pTools->mWndMain, (true ? DISCL_BACKGROUND : DISCL_FOREGROUND) | DISCL_NONEXCLUSIVE);
		if (ddrval != DI_OK)
		{
			return 0;
		}

		ddrval = IDirectInputDevice7_SetDataFormat(lpdid, &c_dfDIKeyboard);
		if (ddrval != DI_OK)
		{
			return 0;
		}

		ddrval = IDirectInputDevice7_Acquire(lpdid);
		//lpdid->Acquire();
		return 1;
	}

};