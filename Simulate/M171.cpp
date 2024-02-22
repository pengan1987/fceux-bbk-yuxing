/*
记得曾经有人说中文DOS显示的问题好像和AV10 AV11有关
*/
#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include<map>
#include<list>
#include "VNes.h"
#include "Tools.h"
#include <dinput.h>
#include"resource.h"
#include "MapperBase.h"

static bool bRead0xffa0Log = true, bWrite4205Log = true;
static VectorByte vByte;

class BBKFDCDriver
{
public:
	int nFdcDrvSel;
	Tools& mTools;
	bool bWriteFloppy;
	typedef BYTE(BBKFDCDriver::* pFdcCallback)(BYTE data);
	struct FDC_CMD_DESC
	{
		FDC_CMD_DESC(int a, int b, pFdcCallback c, pFdcCallback d) :bWLength(a), bRLength(b), callbackRead(c), callbackWrite(d) {
			int ga = 0;
		}
		int	bWLength;
		int	bRLength;
		pFdcCallback callbackRead, callbackWrite;
	};

	typedef std::map<int, FDC_CMD_DESC*> MAP_FdcCallback;
	MAP_FdcCallback mMAP_FdcCallback;
	struct FDC_DATA
	{
		int n4304Count;
		bool bFdcIrq, bFormat;
		LPBYTE pFdcDataPtr;
		BYTE nFdcMainStatus, bFdcCommands[0x10], bFdcResults[0x10];
		int nFdcReadSize, nFdcResultsIndex, nTotalBytes;
		FDC_CMD_DESC* pFdcCmd;
		bool bReading;
		int nFdcRead, nFdcWrite;
	};
	FDC_DATA mFdcData[5];
#define DEF_FDC(n,a,b,c,d) mMAP_FdcCallback.insert(std::make_pair(n, new FDC_CMD_DESC(a, b, &BBKFDCDriver::c, &BBKFDCDriver::d)))

	BBKFDCDriver(Tools& tools) : mTools(tools)
	{
		bWriteFloppy = false;
		nFdcDrvSel = 0;
		ZeroMemory(mFdcData, sizeof(mFdcData));

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
		//nFdcHeadAddres = (mFdcData[nFdcDrvSel].bFdcCommands[1] & 7) >> 2;
		//nFdcCylinder = mFdcData[nFdcDrvSel].bFdcCommands[2];
		//nFDCStatus = FDC_S0_SE;
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSeekWrite PC=%04X data=%02X\n", _CPU_PC, data);
		return 0;
	}
	BYTE FdcReadDataRead(BYTE data) {
		if (!mFdcData[nFdcDrvSel].pFdcCmd)
			return 0;
		mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;

		if (bRead0xffa0Log) {
			bRead0xffa0Log = false;
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcReadDataRead PC=%04X\n", _CPU_PC);
		}
		data = *mFdcData[nFdcDrvSel].pFdcDataPtr++;
		vByte.push_back(data);
		mFdcData[nFdcDrvSel].nFdcRead = -1;
		if (vByte.size() >= 512)
		{
			mTools.WriteVByte(vByte);
			mTools.SetTitle("读盘中...", 1500);
		}
		return data;
	}

	BYTE FdcReadDataWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcReadDataWrite PC=%04X\n", _CPU_PC);
		bRead0xffa0Log = true;
		auto CNT = mFdcData[nFdcDrvSel].bFdcCommands[1] + 1;	//读取的扇区数
		auto C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		auto H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		auto R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		auto N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcReadSize = 0x200 * CNT;
		mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY SEEK C=%04X H=%04X R=%04X N=%04X\n", C, H, R, N);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;

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
		return 0;
	}

	BYTE FdcWriteDataWrite(BYTE data) {
		mFdcData[nFdcDrvSel].bReading = false;
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcWriteDataWrite PC=%04X\n", _CPU_PC);
		bRead0xffa0Log = false;
		bWrite4205Log = false;
		auto C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		auto H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		auto R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		auto N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;

		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY SEEK C=%04X H=%04X R=%04X N=%04X\n", C, H, R, N);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;
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
		return 0;
	}

	BYTE FdcSenseDriveStatusRead(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSenseDriveStatusRead PC=%04X\n", _CPU_PC);
		return 0;// 0x40; 0/0x40
	}

	BYTE FdcSenseDriveStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSenseDriveStatusWrite PC=%04X\n", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// | FDC_MS_EXECUTION;
		return 0;
	}

	BYTE FdcReadDefault(BYTE data) {
		data = 0;
		data = mFdcData[nFdcDrvSel].bFdcResults[mFdcData[nFdcDrvSel].nFdcResultsIndex++];
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcReadDefault PC=%04X data=%02X\n", _CPU_PC, data);
		return data;
	}
	BYTE FdcWriteDefault(BYTE data) {
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		return 0;
	}
	BYTE FdcFormatTrackRead(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcFormatTrackRead PC=%04X\n", _CPU_PC);
		return 0;
	}
	BYTE FdcFormatTrackWrite(BYTE data) {
		//bCPUHookLog = true;
		mFdcData[nFdcDrvSel].bReading = false;
		//bRead0xffa0Log = false;
		mFdcData[nFdcDrvSel].bFormat = true;
		vByte.clear();
		auto C = mFdcData[nFdcDrvSel].bFdcCommands[1];	//磁道号
		auto H = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁头号
		auto R = mFdcData[nFdcDrvSel].bFdcCommands[3];	//扇区号
		auto N = mFdcData[nFdcDrvSel].bFdcCommands[4];
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM; //格式化
		mFdcData[nFdcDrvSel].nFdcReadSize = 4 * R;// R组扇区信息
		mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
		mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcFormatTrackWrite PC=%04X data=%02X SEEK C=%04X H=%04X R=%04X N=%04X\n", _CPU_PC, data, C, H, R, N);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;
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
		data = d[mFdcData[nFdcDrvSel].nFdcRead];
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcReadIDRead PC=%04X data=%02X\n", _CPU_PC, data);
		return data;
	}
	BYTE FdcReadIDWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcReadIDWrite PC=%04X\n", _CPU_PC);
		ZeroMemory(mFdcData[nFdcDrvSel].bFdcResults, sizeof(mFdcData[nFdcDrvSel].bFdcResults));
		return 0;
	}

	BYTE FdcRecalibrateWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcRecalibrateWrite PC=%04X\n", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;
		return 0;
	}

	BYTE FdcSenseIntStatusRead(BYTE data) {
		//读2字节
		BYTE d[2] = { 0x20, 0 };
		/*
		0->326 0~20 0:驱动器错误
			  D958 STA $0326 = #$FF
			DAFA LDA $0326 = #$20
			DAFD AND #$20
		1->327->31F
		D808 LDA $032E = #$00
		D80B CMP $031F = #$20
		*/
		data = d[mFdcData[nFdcDrvSel].nFdcRead];
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSenseIntStatusRead PC=%04X data=%02X\n", _CPU_PC, data);
		return data;
	}
	BYTE FdcSenseIntStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSenseIntStatusWrite PC=%04X\n", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		return 0;
	}

	BYTE FdcSpecifyRead(BYTE data) {
		return 0;
	}
	BYTE FdcSpecifyWrite(BYTE data) {
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY FdcSpecifyWrite PC=%04X\n", _CPU_PC);
		return 0;
	}
	void Write0xff90(WORD addr, BYTE data) {
		mFdcData[nFdcDrvSel].n4304Count = 0;
		static bool bInit = true;
		static BYTE nFdcDrvSelPrev;
		static char* szName[] = { "A", "B", "C", "D" };
		DEBUGOUTFLOPPY("DEBUGOUTFLOPPY Write0xff90 PC=%04X data=%s\n", _CPU_PC, mTools.GetBitNumber(data));
		nFdcDrvSel = data & 3;
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// data;
		if (mFdcData[nFdcDrvSel].nFdcMainStatus == 0)
		{
			nFdcDrvSelPrev = nFdcDrvSel;
			mFdcData[nFdcDrvSel].bFormat = false;
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY 启动驱动器[%s:]\n", szName[nFdcDrvSel]);
		}
		else if (nFdcDrvSelPrev != nFdcDrvSel)
		{
			nFdcDrvSelPrev = nFdcDrvSel;
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY 切换驱动器[%s:]\n", szName[nFdcDrvSel]);
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// data;
		}
		if (data == 0xc)
		{//关闭驱动器
			mTools.SetTitle();
			bCPUHookLog = false;
			if (bWriteFloppy)
				mTools.FloppyIMGSave();
			bWriteFloppy = false;
			ZeroMemory(&mFdcData[nFdcDrvSel], sizeof(mFdcData[0]));
		}
		else if (data == 0x18)
		{
			mFdcData[nFdcDrvSel].bFormat = false;
			for (int i = vByte.size() / 4 - 1; i >= 0; i--)
			{//可能漏了扇区
				auto C = vByte[i * 4 + 0];
				auto H = vByte[i * 4 + 1];
				auto R = vByte[i * 4 + 2];
				INT LBA = H * 18 + C * 36 + (R - 1);
				auto pFdcDataPtr = mTools.lpFloppy + LBA * 512;
				//ZeroMemory(pFdcDataPtr, 512);
			}
			bWriteFloppy = true;
			vByte.clear();
		}
		//if (data & 0x8)//0x10) {
		{
			vByte.clear();
			bInit = true;
			mFdcData[nFdcDrvSel].nTotalBytes = 0;
			mFdcData[nFdcDrvSel].bReading = false;
			bWrite4205Log = true;
			//bCPUHookLog = true;
			nCPUHookLogIndex = 6;
			ZeroMemory(mFdcData[nFdcDrvSel].bFdcCommands, sizeof(mFdcData[nFdcDrvSel].bFdcCommands));
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// data;
			mFdcData[nFdcDrvSel].pFdcCmd = nullptr;
			mFdcData[nFdcDrvSel].nFdcRead = mFdcData[nFdcDrvSel].nFdcWrite = 0;
		}
		if (data & 4)
		{
			//if (nFdcDrvSel == 0)
			mFdcData[nFdcDrvSel].bFdcIrq = true;
			//mFdcData[nFdcDrvSel].bFdcIrq = nFdcDrvSel == 0;
			return;
		}
		mFdcData[nFdcDrvSel].bFdcIrq = false;
	}
	void Write0x4205(WORD addr, BYTE data) {
		mFdcData[nFdcDrvSel].n4304Count = 0;
		if (bWrite4205Log) {
			static int n = 0;
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY Write0x4205 PC=%04X data=%02X\n", _CPU_PC, data);
			if (data == 0x45) {
				if (n == 1) {
					n = 1;
				}
				n++;
			}
		}
		if (vByte.size() >= 0x200) {
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY Write0x4205 数据异常 %03d PC=%04X 0x4205=%02X\n", vByte.size(), _CPU_PC, data);
			mTools.WriteVByte(vByte);
		}
		if (mFdcData[nFdcDrvSel].pFdcCmd && !mFdcData[nFdcDrvSel].bReading) {
			mFdcData[nFdcDrvSel].bFdcCommands[mFdcData[nFdcDrvSel].nFdcWrite++] = data;
		}
		else {
			auto nFdcCmd = data & 0x1f;
			mFdcData[nFdcDrvSel].nFdcRead = 0;
			mFdcData[nFdcDrvSel].nFdcWrite = 0;
			mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
			mFdcData[nFdcDrvSel].nFdcReadSize = 0;
			mFdcData[nFdcDrvSel].bFdcCommands[mFdcData[nFdcDrvSel].nFdcWrite++] = data;
			auto it = mMAP_FdcCallback.find(nFdcCmd);
			if (it == mMAP_FdcCallback.end()) {
				mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
				DEBUGOUTFLOPPY("DEBUGOUTFLOPPY Unkonw Command Write0x4205 PC=%04X 0x4205=%02X\n", _CPU_PC, data);
				return;
			}
			mFdcData[nFdcDrvSel].pFdcCmd = it->second;
			bRead0xffa0Log = true;
		}
		if (mFdcData[nFdcDrvSel].nFdcWrite >= mFdcData[nFdcDrvSel].pFdcCmd->bWLength) {
			mFdcData[nFdcDrvSel].bReading = true;
			if (mFdcData[nFdcDrvSel].pFdcCmd->bRLength > 0)
				mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;
			(this->*mFdcData[nFdcDrvSel].pFdcCmd->callbackWrite)(data);
			if (mFdcData[nFdcDrvSel].nFdcRead >= mFdcData[nFdcDrvSel].pFdcCmd->bRLength) {
				mFdcData[nFdcDrvSel].bReading = false;
				mFdcData[nFdcDrvSel].pFdcCmd = nullptr;
			}
		}
	}
	//获取驱动器当前状态
	BYTE Read0xffa0(WORD addr) {
		if (++mFdcData[nFdcDrvSel].n4304Count > 20)//解决慢速(快速读的方式有区别,读完一个扇区后会有正常的4205写入)读盘/D型磁盘管理/其它应用软件正常读盘
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		if (mFdcData[nFdcDrvSel].n4304Count > 40)//说明读盘程序进入死循环了,暴力终止进程
			ExitProcess(0);
		if (bRead0xffa0Log) {
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY  Read0xffa0 PC=%04X addr=%04X data=%02X\n", _CPU_PC, addr, mFdcData[nFdcDrvSel].nFdcMainStatus);
		}
		return mFdcData[nFdcDrvSel].nFdcMainStatus;
	}

	bool Read(WORD addr, BYTE & data)
	{
		bool bDefault = false;
		switch (addr)
		{
		case 0xFF18:	// 真人语音系统相关;
			data = 0x8F;
			break;
		case 0xFF80:	// FDCDMADackIO
			data = FdcReadDataRead(data);
			break;
			return true;
		case 0xFF88:	// FDCDMATcIO
			data = FdcReadDataRead(data);
			break;
			return true;
		case 0xFF90:	// FDCDRQPortI/FDCCtrlPortO
			data = 0x40;
			break;
			return true;
		case 0xFF98:	// FDCIRQPortI/FDCDMADackIO
			// I: D6 : IRQ
			data = mFdcData[nFdcDrvSel].bFdcIrq ? 0x40 : 0;
			break;
			return true;
		case 0xFFA0:	// FDCResetPortO/FDCStatPortI
			// I: D7 : FDC ready
			// I: D6 : FDC dir
			// I: D5 : FDC busy
			data = Read0xffa0(addr);
			break;
			return true;
		case 0xFFA8:	// FDCDataPortIO
			if (!mFdcData[nFdcDrvSel].pFdcCmd)
			{
				data = 0;
				break;
			}
			data = mFdcData[nFdcDrvSel].bFdcResults[mFdcData[nFdcDrvSel].nFdcResultsIndex++];
			if (++mFdcData[nFdcDrvSel].nFdcRead >= mFdcData[nFdcDrvSel].pFdcCmd->bRLength) {
				mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
				mFdcData[nFdcDrvSel].pFdcCmd = nullptr;
				mFdcData[nFdcDrvSel].bReading = false;
				bRead0xffa0Log = true;
			}
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY  Read PC=%04X addr=%04X data=%02X\n", _CPU_PC, addr, data);
			break;
			return true;
		case 0xFFB8:	// FDCChangePortI/FDCSpeedPortO
			// I: D7 : Disk changed
			data = 0;
			return true;
		default:
			return false;
			break;
		}
		if (bRead0xffa0Log)
		{
			DEBUGOUTFLOPPY("DEBUGOUTFLOPPY BBKFDCDriver.Read PC=%04X addr=%04X data=%02X\n", _CPU_PC, addr, data);
		}
		return true;
	}

	bool Write(WORD addr, BYTE data)
	{
		switch (addr)
		{
		//case 0xffb8:
		//	break;
		case 0xff80:	//磁盘写数据
		{
			if (!mFdcData[nFdcDrvSel].bFormat)
			{
				*mFdcData[nFdcDrvSel].pFdcDataPtr = data;
				mFdcData[nFdcDrvSel].pFdcDataPtr++;
				if (vByte.size() >= 512)
				{
					mTools.WriteVByte(vByte);
					mTools.SetTitle("写盘中...", 1500);
				}
			}
			bWriteFloppy = true;
			vByte.push_back(data);
			if (bWrite4205Log)
			{
				DEBUGOUTFLOPPY("DEBUGOUTFLOPPY Write PC=%04X addr=%04X data=%02X\n", _CPU_PC, addr, data);
			}
			return true;
			break;
		}
		case 0xff90:	// FDCDRQPortI/FDCCtrlPortO
			Write0xff90(addr, data);
			return true;
			break;
		case 0xffA0:	// FDCResetPortO/FDCStatPortI
			// O: D6 : FDC pin reset
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
			return true;
			break;
		case 0xffA8:	// FDCDataPortIO
			Write0x4205(addr, data);
			return true;
			break;
		default:
			return false;
		}
		return true;
	}
};

class BBK10 : public MapperBase
{
public:
	BYTE a3;
	bool bInitKeyboard, bCanIRQ, bUseKeyBoard;
	BBK10() : mFDCDriver(mTools)
	{
	}
	virtual void Reset() override
	{
		nPRomSize = nes->rom->GetPROM_SIZE();
		bInitKeyboard = false;
		mTools.bCanSetTitle = false;//硬件重置时 SetTitle 会阻塞
		Reset(false);
		mTools.bCanSetTitle = true;
		mTools.HookWindow(CApp::GetHWnd(), NULL);
	}
	void HSync(INT scanline)
	{
		if (!(reg[1] & 0x40))// || scanline == 0)
		{
			nVRLineIndex = 0;
			return;
		}
		if (scanline == regVR[nVRLineIndex])
		{
			if (++nVRLineIndex > nVRIndex)
				nVRLineIndex = 0;
			SetVRam2K(0, (scanline & 0xF0) >> 4);
		}
		return;
		//安装鼠标
		WORD addr = 0x8299;
		auto& p = *(DWORD*)& CPU_MEM_BANK[addr >> 13][addr & 0x1FFF];
		if (p == 0xa983bd20)
		{
			p = 0xa9eaeaea;
		}
		//addr = 0x4cfe;
		//CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = 0xff;

		addr = 0x5c47;
		CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;
		addr = 0x5c48;
		CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;

		addr = 0x5c6d;
		CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;
		addr = 0x5c6e;
		CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;
		//addr = 0x5ff3;
		//CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;

		//addr = 0x5ff2;
		//CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;
		//addr = 0x5ff3;
		//CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = rand() % 255;
	}
	//virtual	BYTE ReadLow(WORD addr)
	//{
	//	return CPU_MEM_BANK[addr >> 13][addr & 0x1FFF];
	//}
	//virtual void WriteLow(WORD addr, BYTE data) override
	//{
	//	CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = data;
	//}
	// $8000-$FFFF Memory write
	virtual	void Write(WORD addr, BYTE data)
	{
		if (!bInitKeyboard)
		{
			bInitKeyboard = true;
			//nes->pad->SetExController(PAD::EXCONTROLLER_Subor_KEYBOARD);
			//nes->SetVideoMode(2);
		}
		if (addr < 0xFF00)
		{
			CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = data;
			return;
		}
		if (addr < 0xfff0) {
			OnWriteRam(addr, data);
			return;
		}
			CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = data;
		//__super::Write(addr, data);
	}
	//virtual	BYTE Read(WORD addr)override
	//{
	//	BYTE data;
	//	if (OnReadRam(addr, data))
	//		return data;
	//	return CPU_MEM_BANK[addr >> 13][addr & 0x1FFF];
	//	//return __super::Read(addr);
	//}
	typedef void(BBK10::*PWRITEFun)(WORD addr, BYTE value);
	typedef std::map<int, PWRITEFun> MAP_PWRITEFun;
	typedef std::list<BYTE> LIST_ChrQueue;
	LIST_ChrQueue mLIST_ChrQueue;
	MAP_PWRITEFun mMAP_PWRITEFun;
#define WRITE_FUN(addr,fun) mMAP_PWRITEFun.insert(std::make_pair(addr, &BBK10::fun))
	typedef BYTE(BBK10::*PReadFun)(WORD addr);
	typedef std::map<int, PReadFun> MAP_PReadFun;
	MAP_PWRITEFun mPReadFun;
#define READ_FUN(addr,fun) mPReadFun.insert(std::make_pair(addr, &BBK10::fun))
	BBKFDCDriver mFDCDriver;
protected:
	BYTE reg[0xff], regKeep[10], nRegKeep, regVR[20], regSR[20];
	int nVRIndex, nVRLineIndex, nSRIndex, nSRLineIndex;
	int nPRomSize;
	const int nWRamSize = 32;
	const int nPrgRamSize = 512;
	const int nChrRamSize = 32;
	const int nChrRamCount1 = nChrRamSize - 1;
	const int nChrRamCount2 = nChrRamSize / 2 - 1;
	const int nPrgRamCount8 = nPrgRamSize / 8 - 1;
	const int nPrgRamCount16 = nPrgRamSize / 16 - 1;
	const int nWRamIndex = nPrgRamSize / 8 + 2;
	Tools mTools;
	int _irqCounter, _irqEnabledValue;
	bool _irqEnabled, _irqReload, _bSplit;

public:
	~BBK10()
	{
		mTools.Close();
	}

	void Reset(bool softReset)
	{
		memset(reg, 0, sizeof(reg));
		memset(regKeep, 0, sizeof(regKeep));
		memset(regVR, 0, sizeof(regVR));
		memset(regSR, 0, sizeof(regSR));
		nRegKeep = 0;
		_bSplit = false;
		nVRLineIndex = nSRIndex = nVRIndex = 0;
		mTools.Reset();
		mTools.FloppyIMGLoad();

		SetVRam8K(0);
		SetRam16K(2, nWRamIndex);
		SetRom16K(4, 0);
		SetRom16K(6, nPRomSize - 1);

		//WRITE_FUN(0xff00, WritePramFF00);
		WRITE_FUN(0xff03, WriteVramFF03);
		WRITE_FUN(0xff0B, WriteVramFF0B);
		WRITE_FUN(0xff13, WriteVramFF13);
		WRITE_FUN(0xff1B, WriteVramFF1B);

		WRITE_FUN(0xff01, WritePramFF01);
		WRITE_FUN(0xff04, WritePramFF04);
		WRITE_FUN(0xff14, WritePramFF14);
		WRITE_FUN(0xff1C, WritePramFF1C);
		WRITE_FUN(0xff24, WritePramFF24);
		WRITE_FUN(0xff2C, WritePramFF2C);
	}

	void WritePramFF01(WORD addr, BYTE value)
	{
		/*
		0x40 MODE
		*/
		const MirroringType nMonitor[] = { MirroringTypeVertical, MirroringTypeHorizontal, MirroringTypeAOnly, MirroringTypeBOnly };
		SetMirroringType(nMonitor[value & 3]);
		DEBUGOUTIODetail("WritePramFF01 PC=%04X addr=%04X value=%s\n", _CPU_PC, addr, mTools.GetBitNumber(value));
		// map C000-FFF to ROM
		if (reg[1] & 0x8)//0x8
		{
			SetRam8K(6, reg[0x24] & nPrgRamCount8);
			SetRam8K(7, reg[0x2C] & nPrgRamCount8);
		}
		else
		{
			SetRom16K(6, nPRomSize -1);
		}
		if ((reg[1] & 0x4))//0x4
		{
			_irqEnabled = true;
			IRQStart();
		}
		else
		{
			_irqEnabled = false;
			IRQ_CLEAR();
		}
	}
	void IRQStart()
	{
		if(BST_CHECKED == IsDlgButtonChecked(mTools.mWndDebug, IDC_CHECK_IRQ))
			return;
		//if (_cpu.IRQVectorAddress == CPU::IRQVector)
		//	return;
		IRQ_START();
	}
	void WritePramFF04(WORD addr, BYTE value)
	{
		DEBUGOUTIODetail("WritePramFF04 PC=%04X addr=%04X value=%s\n", _CPU_PC, addr, mTools.GetBitNumber(value));
		//if (!(value & 0x80))
		//{
		//	//SetRom16K(4, nPRomSize / 0x4000 -2);
		//	return;
		//}
		SetRam16K(4, (reg[0x04]) & nPrgRamCount16);
		//如果没有以下语句,003.IMG 指法练习,五笔打字 中文会有部分乱码
		if (((reg[1] & 0x40) || reg[0] == 4) && (value < nPrgRamCount16))
			SetRom16K(4, value & 7);
	}
	void WritePramFF14(WORD addr, BYTE value)
	{
		SetRam8K(4, reg[0x14] & nPrgRamCount8);
	}
	void WritePramFF1C(WORD addr, BYTE value)
	{
		SetRam8K(5, reg[0x1C] & nPrgRamCount8);
	}
	void WritePramFF24(WORD addr, BYTE value)
	{
		if (reg[1] & 8)
			SetRam8K(6, reg[0x24] & nPrgRamCount8);
	}
	void WritePramFF2C(WORD addr, BYTE value)
	{
		if (reg[1] & 8)
			SetRam8K(7, reg[0x2C] & nPrgRamCount8);
	}

	void WriteVramFF03(WORD addr, BYTE value)
	{
		SetVRam2K(0, (reg[3] & nChrRamCount2) * 1);
	}

	void WriteVramFF0B(WORD addr, BYTE value)
	{
		SetVRam2K(2, value & nChrRamCount2);
	}
	void WriteVramFF13(WORD addr, BYTE value)
	{
		SetVRam2K(4, value & nChrRamCount2);
	}
	void WriteVramFF1B(WORD addr, BYTE value)
	{
		SetVRam2K(6, value & nChrRamCount2);
	}

	bool OnReadRam(WORD addr, BYTE & data)
	{
		if (mFDCDriver.Read(addr, data))
			return true;
		return false;
	}
	bool OnWriteRam(WORD addr, BYTE value)
	{
		reg[addr & 0xff] = value;
		if (mFDCDriver.Write(addr, value))
		{
			return true;
		}
		//DEBUGOUTIO("WriteRegister PC=%04X addr=%04X value=%s\n", _CPU_PC, addr, mTools.GetBitNumber(value));
		auto it = mMAP_PWRITEFun.find(addr);
		if (it != mMAP_PWRITEFun.end())
		{
			(this->*it->second)(addr, value);
			return true;
		}
		switch (addr)
		{
			//屏幕分裂
		case 0xff0a://Write to SR queue
		{
			static WORD v = 0;
			regSR[nSRIndex++] = value;
			return true;
		}
		case 0xff12://Queue INIT
		{
			static WORD v = 0;
			_bSplit = false;
			if (value & 0x40)
			{
				nVRLineIndex = nVRIndex = 1;
				nSRIndex = nSRLineIndex = 0;
			}
			return true;
		}
		case 0xff1a:// Write to VR queue
		{
			regVR[nVRIndex++] = value;
			return true;
		}
		case 0xff22://START
		{
			_bSplit = true;
			return true;
		}
		case 0xff00://KebBoardLEDPort
			return true;
		case 0xff02:
			return true;
		case 0xff06:
			return true;
		case 0xff09:
			//SetCpuMemoryMapping(0x8000, 0x9fff, value & nPrgRamCount8, PrgMemoryType::WorkRam);
			return true;
			//微机通迅卡（并行）端口
		case 0xff40://PCC port base + 0
			return true;
		case 0xff50://PCC port base+2
			return true;
		}
		return false;
	}

public:
};
