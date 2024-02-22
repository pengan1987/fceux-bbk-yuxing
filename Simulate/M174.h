/*
猜测
4190 $8000~$9FFF RAM
41ab RW
光驱 41ae W 41af RW
41B4 $C000~$DFFF ROM
*/
#include "MapperBase.h"
#include "Tools.h"
#include "libCDImage.h"

static const int nWRamSize = 64;
static const int nPrgRamSize = 1024;
static const int nChrRamSize = 512;
static const int nChrRamCount1 = nChrRamSize - 1;
static const int nChrRamCount2 = nChrRamSize / 2 - 1;
static const int nChrRamCount4 = nChrRamSize / 4 - 1;
static const int nChrRamCount8 = nChrRamSize / 8 - 1;

static const int nPrgRamCount8 = nPrgRamSize / 8 - 1;
static const int nPrgRamCount16 = nPrgRamSize / 16 - 1;
static const int nPrgRamCount32 = nPrgRamSize / 32 - 1;
static const int nWRamIndex = 1;

class KWCDDriver {
public:
	Tools& mTools;
	bool bLog;
	KWCDDriver() : mTools(*pTools) {

	}
	bool OnRead(WORD addr, BYTE& data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x41AE:
			break;
		case 0x41AF: {
			//if (_CPU_PC == 0xFB5B) {
			//	data = 0;
			//	break;
			//}
			data = 0x40;
			break;
		}
		default:
			b = false;
			bLog = false;
			break;
		}
		if (bLog) {
			DEBUGOUTIO("KWCDDriver.OnRead PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		}
		return b;
	}
	bool OnWrite(WORD addr, BYTE data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x41AE:
			break;
		default:
			b = false;
			bLog = false;
			break;
		}
		if (bLog) {
			DEBUGOUTIO("KWCDDriver.OnWrite PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		}
		return b;
	}
};
class KWFDCDriver
{
public:
	int nFdcDrvSel;
	Tools& mTools;
	bool bWriteFloppy, bLog;
	typedef BYTE(KWFDCDriver::* pFdcCallback)(BYTE data);
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
#define DEF_FDC(n,a,b,c,d) mMAP_FdcCallback.insert(std::make_pair(n, new FDC_CMD_DESC(a, b, &KWFDCDriver::c, &KWFDCDriver::d)))

	KWFDCDriver() : mTools(*pTools)
	{
		bWriteFloppy = bLog = false;
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
		DEBUGOUTFLOPPY("FDC FdcSeekWrite PC=%04X data=%02X", _CPU_PC, data);
		return 0;
	}
	BYTE FdcReadDataRead(BYTE data) {
		if (!mFdcData[nFdcDrvSel].pFdcCmd)
			return 0;
		if (--mFdcData[nFdcDrvSel].nFdcReadSize >= 0) {
			bLog = false;
			data = *mFdcData[nFdcDrvSel].pFdcDataPtr++;
			mFdcData[nFdcDrvSel].nFdcRead = -1;
			vByte.push_back(data);
			mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;
			if (vByte.size() >= 512)
			{
				mTools.WriteVByte(vByte);
				//DEBUGOUTFLOPPY("FDC FdcReadDataRead PC=%04X", _CPU_PC);
			}
			return data;
		}
		data = mFdcData[nFdcDrvSel].bFdcResults[mFdcData[nFdcDrvSel].nFdcResultsIndex++];
		return data;
	}

	BYTE FdcReadDataWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcReadDataWrite PC=%04X", _CPU_PC);
		auto &cmd = mFdcData[nFdcDrvSel].bFdcCommands;
		BYTE CNT = mFdcData[nFdcDrvSel].bFdcCommands[1] + 1;	//读取的扇区数
		BYTE C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		BYTE H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		BYTE R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		BYTE N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcReadSize = 0x200 * 1;// CNT;
		mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
		//18:12 36:24
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK 0=%04X 1=%04X C=%04X H=%04X R=%04X N=%04X 6=%04X", cmd[0], cmd[1], C, H, R, N, cmd[6]);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;
		mTools.SetTitle("读盘中...", 1500);

		R++;
		if (19 == R)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
			}
		}
		return 0;
	}

	BYTE FdcWriteDataWrite(BYTE data) {
		mFdcData[nFdcDrvSel].bReading = false;
		if (mFdcData[nFdcDrvSel].nFdcReadSize > 0) {
			mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;
			mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;
			*mFdcData[nFdcDrvSel].pFdcDataPtr = data;
			mFdcData[nFdcDrvSel].pFdcDataPtr++;
			bWriteFloppy = true;
			bLog = false;
			vByte.push_back(data);
			if (--mFdcData[nFdcDrvSel].nFdcReadSize >= 1) {
				return 0;
			}

			mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
			mFdcData[nFdcDrvSel].nFdcRead = 0;
			mTools.bFloppySave = true;
			return 0;
		}
		if (vByte.size() > 0) {
			DEBUGOUTFLOPPY("FDC FdcWriteDataWrite 读数据异常 PC=%04X", _CPU_PC);
		}
		DEBUGOUTFLOPPY("FDC FdcWriteDataWrite PC=%04X", _CPU_PC);
		BYTE C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		BYTE H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		BYTE R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		BYTE N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;
		mFdcData[nFdcDrvSel].nFdcReadSize = 0x200 * 1;// CNT;

		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK C=%04X H=%04X R=%04X N=%04X", C, H, R, N);
		mTools.SetTitle("写盘中...", 1500);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;
		R++;
		if (19 == R)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
			}
		}
		return 0;
	}

	BYTE FdcSenseDriveStatusRead(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseDriveStatusRead PC=%04X", _CPU_PC);
		return 0;// 0x40; 0/0x40
	}

	BYTE FdcSenseDriveStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseDriveStatusWrite PC=%04X", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// | FDC_MS_EXECUTION;
		return 0;
	}

	BYTE FdcReadDefault(BYTE data) {
		data = 0;
		data = mFdcData[nFdcDrvSel].bFdcResults[mFdcData[nFdcDrvSel].nFdcResultsIndex++];
		DEBUGOUTFLOPPY("FDC FdcReadDefault PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcWriteDefault(BYTE data) {
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		return 0;
	}
	BYTE FdcFormatTrackRead(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcFormatTrackRead PC=%04X", _CPU_PC);
		return 0;
	}
	BYTE FdcFormatTrackWrite(BYTE data) {
		//bCPUHookLog = true;
		mFdcData[nFdcDrvSel].bReading = false;
		//bReadFDCStatus4188Log = false;
		mFdcData[nFdcDrvSel].bFormat = true;
		vByte.clear();
		BYTE C = mFdcData[nFdcDrvSel].bFdcCommands[1];	//磁道号
		BYTE H = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁头号
		BYTE R = mFdcData[nFdcDrvSel].bFdcCommands[3];	//扇区号
		BYTE N = mFdcData[nFdcDrvSel].bFdcCommands[4];
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM; //格式化
		mFdcData[nFdcDrvSel].nFdcReadSize = 4 * R;// R组扇区信息
		mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
		mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;
		INT LBA = H * 18 + C * 36 + (R - 1);
		DEBUGOUTFLOPPY("FDC FdcFormatTrackWrite PC=%04X data=%02X SEEK C=%04X H=%04X R=%04X N=%04X", _CPU_PC, data, C, H, R, N);
		mFdcData[nFdcDrvSel].pFdcDataPtr = mTools.lpFloppy + LBA * 512;
		if (++R >= 19)
		{
			R = 1;
			H++;
			if (2 == H)
			{
				C++;
				//if (80 == C)
				//    C = 0;
			}
		}
		return 0;
	}
	BYTE FdcReadIDRead(BYTE data) {
		//读7字节
		BYTE d[7] = { 0 };
		d[1] = 0;// 其它可能没用到 第1字节返回磁盘格式(0/1/4/5) 1/4/5格式不对
		data = d[mFdcData[nFdcDrvSel].nFdcRead];
		DEBUGOUTFLOPPY("FDC FdcReadIDRead PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcReadIDWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcReadIDWrite PC=%04X", _CPU_PC);
		ZeroMemory(mFdcData[nFdcDrvSel].bFdcResults, sizeof(mFdcData[nFdcDrvSel].bFdcResults));
		return 0;
	}

	BYTE FdcRecalibrateWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcRecalibrateWrite PC=%04X", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;
		return 0;
	}

	BYTE FdcSenseIntStatusRead(BYTE data) {
		//读2字节
		BYTE d[2] = { 0x20, mFdcData[nFdcDrvSel].bFdcCommands[2] };
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
		DEBUGOUTFLOPPY("FDC FdcSenseIntStatusRead PC=%04X data=%02X", _CPU_PC, data);
		return data;
	}
	BYTE FdcSenseIntStatusWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSenseIntStatusWrite PC=%04X", _CPU_PC);
		//mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		return 0;
	}

	BYTE FdcSpecifyRead(BYTE data) {
		return 0;
	}
	BYTE FdcSpecifyWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcSpecifyWrite PC=%04X", _CPU_PC);
		return 0;
	}
	void WriteFDCCtrlPortO418B(WORD addr, BYTE data) {
		mFdcData[nFdcDrvSel].n4304Count = 0;
		static bool bInit = true;
		static BYTE nFdcDrvSelPrev;
		bLog = false;
		DEBUGOUTFLOPPY("FDC WriteFDCCtrlPortO418B PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		//DebugString("FDC WriteFDCCtrlPortO418B PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		nFdcDrvSel = data & 3;
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
		if (data == 8) {
			mFdcData[nFdcDrvSel].bFdcIrq = true;
			return;
		}
		if (data == 0x1c)//关闭软驱
		{
			if (vByte.size() > 0) {
				mTools.WriteVByte(vByte);
			}
			mTools.SetTitle("");
			mFdcData[nFdcDrvSel].bFdcIrq = false;
			bInit = true;
			mFdcData[nFdcDrvSel].nTotalBytes = 0;
			mFdcData[nFdcDrvSel].bReading = false;
			nCPUHookLogIndex = 6;
			ZeroMemory(mFdcData[nFdcDrvSel].bFdcCommands, sizeof(mFdcData[nFdcDrvSel].bFdcCommands));
			mFdcData[nFdcDrvSel].pFdcCmd = nullptr;
			mFdcData[nFdcDrvSel].nFdcRead = mFdcData[nFdcDrvSel].nFdcWrite = 0;
			return;
		}
		if (data & 4)
		{
			mFdcData[nFdcDrvSel].bFdcIrq = true;
			return;
		}
		mFdcData[nFdcDrvSel].bFdcIrq = false;
	}

	void FDCCMDRead4189(WORD addr, BYTE &data) {
		if (!mFdcData[nFdcDrvSel].pFdcCmd)
		{
			data = 0;
			return;
		}
		data = (this->*mFdcData[nFdcDrvSel].pFdcCmd->callbackRead)(data);
		if (++mFdcData[nFdcDrvSel].nFdcRead >= mFdcData[nFdcDrvSel].pFdcCmd->bRLength) {
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
			mFdcData[nFdcDrvSel].pFdcCmd = nullptr;
			mFdcData[nFdcDrvSel].bReading = false;
		}
	}

	void FDCCMDWrite4189(WORD addr, BYTE &data) {
		mFdcData[nFdcDrvSel].n4304Count = 0;
		bLog = false;
		//DEBUGOUTFLOPPY("FDC FDCCMDWrite4189 PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		if (mFdcData[nFdcDrvSel].pFdcCmd && !mFdcData[nFdcDrvSel].bReading) {
			mFdcData[nFdcDrvSel].bFdcCommands[mFdcData[nFdcDrvSel].nFdcWrite++] = data;
		}
		else {
			BYTE nFdcCmd = data & 0x1f;
			mFdcData[nFdcDrvSel].nFdcRead = 0;
			mFdcData[nFdcDrvSel].nFdcWrite = 0;
			mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
			mFdcData[nFdcDrvSel].nFdcReadSize = 0;
			mFdcData[nFdcDrvSel].bFdcCommands[mFdcData[nFdcDrvSel].nFdcWrite++] = data;
			MAP_FdcCallback::iterator it = mMAP_FdcCallback.find(nFdcCmd);
			if (it == mMAP_FdcCallback.end()) {
				mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
				DEBUGOUTFLOPPY("FDC Unkonw Command FDCCMDWrite4189 PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
				return;
			}
			mFdcData[nFdcDrvSel].pFdcCmd = it->second;
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
	BYTE ReadFDCStatus4188(WORD addr) {
		{
			bLog = false;
			DEBUGOUTFLOPPY("FDC ReadFDCStatus4188 PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(mFdcData[nFdcDrvSel].nFdcMainStatus));
		}
		return mFdcData[nFdcDrvSel].nFdcMainStatus;
	}

	bool OnRead(WORD addr, BYTE& data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4188:
			data = ReadFDCStatus4188(addr);
			break;
		case 0x4189: {
			FDCCMDRead4189(addr, data);
			if(bLog)
				DEBUGOUTFLOPPY("FDC FDCCMDRead4189 PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
			bLog = false;
			break;
		}
		case 0x418E: {
			static WORD PC418E;
			if (PC418E == _CPU_PC)
				bLog = false;
			PC418E = _CPU_PC;
			data = (mFdcData[nFdcDrvSel].bFdcIrq ? 0xC0 : 0);
			data = 0xc0;
			break;
		}
		default:
			b = false;
			break;
		}
		if (b && bLog)
		{
			DEBUGOUTFLOPPY("FDC OnRead PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		}
		return b;
	}

	bool OnWrite(WORD addr, BYTE data)
	{
		bool b = true;
		bLog = true;
		switch (addr)
		{
		case 0x4189:
			FDCCMDWrite4189(addr, data);
			break;
		case 0x418A:
			break;
		case 0x418B:
			WriteFDCCtrlPortO418B(addr, data);
			break;
		default:
			b = false;
			break;
		}
		if (b && bLog)
		{
			DEBUGOUTFLOPPY("FDC OnWrite PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
		}
		return b;
	}
};

class KW3000 : public MapperBase
{
public:
	KWFDCDriver mFDCDriver;
	KWCDDriver mCDDriver;
	bool bLog, bIRQEnabled, bVCD;
	BYTE RegIO[0xff], Reg[0xff], IRQa;
	WORD PRGRamCount[3], CHRRamCount[4], CHRRomCount[4], IRQCount;
	typedef void(KW3000::* PWRITEFun)(WORD addr, BYTE data);
	typedef std::map<int, PWRITEFun> MAP_PWRITEFun;
	typedef std::list<BYTE> LIST_ChrQueue;
	VectorByte vByteVoice;
	LIST_ChrQueue mLIST_ChrQueue;
	MAP_PWRITEFun mMAP_PWRITEFun;
#define WRITE_FUN(addr,fun) mMAP_PWRITEFun.insert(std::make_pair(addr, &KW3000::fun))
	typedef BYTE(KW3000::* PReadFun)(WORD addr);
	typedef std::map<int, PReadFun> MAP_PReadFun;
	MAP_PReadFun mPReadFun;
#define READ_FUN(addr,fun) mPReadFun.insert(std::make_pair(addr, &KW3000::fun))

	KW3000() {
		bVCD = false;
	}
	virtual int GetPRGRamSize() { return nPrgRamSize * 1024; }
	virtual int GetCHRRamSize() { return nChrRamSize * 1024; }
	virtual int GetWorkRamSize() { return nWRamSize * 1024; }
	virtual void Power(void)
	{
		__super::Power();
		WRITE_FUN(0x4182, Write4182);
		PRGRamCount[0] = nPrgRamCount8;
		PRGRamCount[1] = nPrgRamCount16;
		PRGRamCount[2] = nPrgRamCount32;

		CHRRamCount[0] = nChrRamCount1;
		CHRRamCount[1] = nChrRamCount2;
		CHRRamCount[2] = nChrRamCount4;
		CHRRamCount[3] = nChrRamCount8;

		memset(Reg, 0, sizeof(Reg));
		memset(RegIO, 0, sizeof(RegIO));
		VideoMode(VideoModeDendy);
		mTools.bCanSetTitle = false;//硬件重置时 SetTitle 会阻塞
		mTools.Reset();
		if (!bVCD) {
			mTools.FloppyIMGLoad();
		}
		else {
			auto p = pTools->GetDefaultMedia("bin");
			if (p)
				OnDropFiles(p);
		}
		mTools.bCanSetTitle = true;
		mTools.bCaptureMouse = true;
	}
	
	virtual bool OnReadRam(WORD addr, BYTE& data) override
	{
		if (mFDCDriver.OnRead(addr, data))
			return true;
		if (mCDDriver.OnRead(addr, data))
			return true;
		switch (addr)
		{
		case 0x41A0: {// clock counter Low 
			IRQCount = data;
			break;
		}
		case 0x41A1: {// clock counter Low 
			data = IRQCount;
			break;
		}
		case 0x41A2: {// clock counter High
			data = (IRQCount >> 8);
			break;
		}
		default:
			break;
		}
		return false;
	}
	virtual bool OnWriteRam(WORD addr, BYTE& data) override
	{
		if (mFDCDriver.OnWrite(addr, data))
			return true;
		if (mCDDriver.OnWrite(addr, data))
			return true;
		switch (addr)
		{
		case 0x4183: {//Address-line Masks
			auto n = ((data & 0xf0) >> 4) + 1;
			n |= Reg[0xb5] == 0x40 ? 0x10 : 0;
			n = 32 * n;
			PRGRamCount[0] = n / 8 - 1;
			PRGRamCount[1] = n / 16 - 1;
			PRGRamCount[2] = n / 32 - 1;
			n = (data & 0xf) + 1;
			n = 32 * n;
			//if (Reg[0x82] & 0x80) {
			//	CHRRomCount[0] = n / 8 - 1;
			//	CHRRomCount[1] = n / 16 - 1;
			//	CHRRomCount[2] = n / 32 - 1;
			//}
			//else {
			//	CHRRamCount[0] = n / 8 - 1;
			//	CHRRamCount[1] = n / 16 - 1;
			//	CHRRamCount[2] = n / 32 - 1;
			//}
			CHRRamCount[0] = n / 1 - 1;
			CHRRamCount[1] = n / 2 - 1;
			CHRRamCount[2] = n / 4 - 1;
			CHRRamCount[3] = n / 8 - 1;
			break;
		}
		case 0x41A0: {
			DEBUGOUTIO("0x41A0 I=%d N=%d PC=%04X addr=%04X data=%s", bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			break;
		}
		case 0x41A1: {// clock counter Low 
			IRQCount &= 0xFF00;
			IRQCount |= data;
			IRQ_CLEAR();
			break;
		}
		case 0x41A2: {// clock counter High
			IRQCount &= 0x00ff;
			IRQCount |= (data & 0x7F) << 8;
			IRQa = data & 0x80;
			IRQ_CLEAR();
			break;
		}
		case 0x41A3: {// IRQ Enable/Disable 1/0
			bIRQEnabled = (data == 1);
			break;
		}
		case 0x41A4:{
			SetMirroringType(data == 0 ? MirroringTypeVertical : MirroringTypeHorizontal);
			break;
		}
		default:
			break;
		}
		return false;
	}
	void Write4182(WORD addr, BYTE data)
	{
		const MirroringType nMonitor[] = { MirroringTypeAOnly, MirroringTypeBOnly, MirroringTypeVertical, MirroringTypeHorizontal };
		static MirroringType nType = -1;
		auto mType = (data >> 4) & 3;
		if (nType != mType)
		{
			//DEBUGOUTTEST("WritePramFF01 nType=%d data=%d", nType, nMonitor[data & 3]);
			nType = mType;
			SetMirroringType(nMonitor[mType]);
		}

		auto vRamLayout = (data >> 2) & 3;
		static BYTE vRamLayoutPrev = 255;
		if (vRamLayoutPrev != vRamLayout) {
			vRamLayoutPrev = vRamLayout;
			if (vRamLayout == 0)
				setmirrorw(0, 0, 0, 0);
		}

		auto irq = data & 3;
		if (irq > 0) {//IRQ
			irq = irq;
			bIRQEnabled = true;
		}

		auto chr = data & 0x80;
		if (chr == 0) {
			chr = 0;
		}
	}

	virtual void CPUClock(int nCycles) {
		auto irq = Reg[0x82] & 3;
			return;
		if (irq == 0 || !IRQa) {
			return;
		}
		if (irq == 3) {//CPU clock counter (down)
			IRQCount -= nCycles;
			if (IRQCount <= 0) {
				IRQ_START();
				IRQCount = 0;
				IRQa = 0;
			}
			return;
		}
		//CPU clock counter (up)
		IRQCount += nCycles;
		if (IRQCount >= 0x7FFF) {
			IRQ_START();
			IRQa = 0;
			IRQCount = 0x7FFF;
		}
	}

};