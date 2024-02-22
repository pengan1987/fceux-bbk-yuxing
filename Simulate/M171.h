/*
0xff00~0xffff 必须做IO使用

注意:
0x4016 0x4017 //必须返回0xff这个值,不然启动DOS后无限重启,并有嘀嘀声
加载BBGDOS时,一直停在提示那,可能是 0x4000~0x7fff 的内存设置有问题

写磁盘时:
PC=D850 往$FF80写时只有511字节
PC=D7EF 往$FF80写正常 512字节
复杂的程序  015 049

记得曾经有人说中文DOS显示的问题好像和AV10 AV11有关
在DoLineHook函数里 0.9版 scanline == 0xdb 时最稳定 2.0 0xed
scanline 最大 0xef
游戏用bbgdos.sys 2.1版，3.0加载有问题

$ff03管显示码00一7f
$ff0b管显示码80一ff
同一分裂行可用2bank显存，显示00一ff显示码
gamshell.cmd读取15个gam会触发
$ff1a高4bit送入$ff0b，低4bit送入$ff03，一个分裂行才能用到00一ff显示码
从gamshell得出 一个nmi，一个irq

步步高的GAM游戏就是由NES游戏移植而来的。凡是有切BANK指令的游戏，里面需要加一小段模拟切BANK程序。模块类型为02.常驻于低端内存（$8000以下）.
NES游戏中原有的写内存指令改成跳转指令。跳转到02模块中的切页子程序。执行完子程序再返回游戏的下一条指令。
BBK10 未处理的IO
OnWrite default PC=CC2D addr=FF09 data= 20=20+
OnWrite default PC=CB8A addr=FF40 data= 00=
OnWrite default PC=CB8D addr=FF50 data= 00=

OnWrite	addr=FF10	data=	FF=80+40+20+10+08+04+02+01+
BBK98 未处理的IO
OnWrite	addr=FF05	data=	07=04+02+01+
OnWrite	addr=FF08	data=	00=
OnWrite	addr=FF09	data=	20=20+
OnWrite	addr=FF0C	data=	FE=80+40+20+10+08+04+02+
OnWrite	addr=FF19	data=	10=10+
OnWrite	addr=FFB0	data=	00=

OnRead addr=FFB0

*/
#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include "VNes.h"
#include "Tools.h"
#define CN_ASSERT
#include "../common/CnComm.h"
#include <dinput.h>
#include <deque>
#include<map>
#include<list>
#include<time.h>
#include "MapperBase.h"
#include"resource.h"

#ifndef _RELEASE
#undef DEBUGOUTIODetail
#define DEBUGOUTIODetail DebugString
#endif

static bool bRead0xffa0Log = true, bWrite4205Log = true;
const int nWRamSize = 32;
const int nPrgRamSize = 512;;
const int nChrRamSize = 32;
const int nChrRamCount1 = nChrRamSize - 1;
const int nChrRamCount2 = nChrRamSize / 2 - 1;
const int nChrRamCount8 = nChrRamSize / 8 - 1;

const int nPrgRamCount8 = nPrgRamSize / 8 - 1;
const int nPrgRamCount16 = nPrgRamSize / 16 - 1;
const int nPrgRamCount32 = nPrgRamSize / 32 - 1;
static bool bUseKeyBoard;

struct W2007AddrInfo {
	WORD addr;
	WORD vram;
	bool bFirst;
	WORD nByte;
	WORD PC;
	VectorByte vByte;
	uint32 addrStart;
};
static W2007AddrInfo W2007Info[3];

class BBKPCCard
{
public:
	typedef bool(BBKPCCard::* PWRITEFun)(WORD addr, BYTE data);
	typedef std::map<int, PWRITEFun> MAP_PWRITEFun;
	MAP_PWRITEFun mMAP_PWRITEFun;
#define WRITE_FUN(addr,fun) mMAP_PWRITEFun.insert(std::make_pair(addr, &BBKPCCard::fun))

	typedef bool(BBKPCCard::* PREADFun)(WORD addr, BYTE& data);
	typedef std::map<int, PREADFun> MAP_PREADFun;
	MAP_PREADFun mMAP_PREADFun;
#define READ_FUN(addr,fun) mMAP_PREADFun.insert(std::make_pair(addr, &BBKPCCard::fun))

	typedef void(BBKPCCard::* PCMDWRITEFun)(BYTE data);
	typedef std::map<BYTE, PCMDWRITEFun> MAP_PCMDWRITEFun;
	MAP_PCMDWRITEFun mMAP_PCMDWRITEFun;
#define CMDWRITE_FUN(addr,fun) mMAP_PCMDWRITEFun.insert(std::make_pair(addr, &BBKPCCard::fun))

	typedef bool(BBKPCCard::* PCMDREADFun)(BYTE &data);
	typedef std::map<BYTE, PCMDREADFun> MAP_PCMDREADFun;
	MAP_PCMDREADFun mMAP_PCMDREADFun;
#define CMDREAD_FUN(addr,fun) mMAP_PCMDREADFun.insert(std::make_pair(addr, &BBKPCCard::fun))

#pragma pack(1)
	struct FileInfo
	{
		BYTE FILECMD;
		BYTE CURDRIVE;// CURDRIVE = 1; File driver.
		BYTE HPAGE;//HPAGE = 2; byte of file buffer page.
		WORD HBUF;//HBUF = 3; word of file buffer or filename.
		WORD BYTES;//BYTES = 5; word of read / write length.
		DWORD CURP;//CURP = 7; Dword of file point.
		BYTE STATUS;//STATUS = 11 0xB; byte
		BYTE HANDLE;//HANDLE = 12 0xC; word of File handle. BYTE OR WORD ?
		DWORD dwLength;// 13 0xD; file length.
		FILE *f;
		DWORD dwReadRemain;
		DWORD dwReadBytes;
		BYTE cbCMD;
		char chBuf[512];
	};
	struct FileAttrib {
		BYTE cbAttrib;
		WORD fTime;
		WORD fDate;
		DWORD dwSize;
		char chBuf[512];
	};
#pragma pack()
	FileInfo & fResult;
	std::map<BYTE, FileInfo> mMAP_OpenFile;
	std::list<BYTE> mListFreeFileNumber;
	std::list<FileInfo> mListAction;
	bool bCmdSub, bFirstCmd, bFirstCall;
	BYTE cbCmdCur, cbPageCur;
	BYTE reg4348, reg4340, write4340Times, cbCMD[128 * 1024], cbCMDResult[20], cbAction, cbActionBack;
	WORD wIndexCMD, nTime, wLength, wAddr;
	const BYTE cbIndexFileNumber = 7;
	std::string strBBKRoot, strDirCurrent, strDirCurFind;
	HANDLE hFind;
	WIN32_FIND_DATA mWIN32_FIND_DATA;

	BBKPCCard():fResult(*(FileInfo*)&cbCMDResult[0])
	{
		hFind = INVALID_HANDLE_VALUE;

		CMDREAD_FUN(0, CMDReadDefault);
		CMDREAD_FUN(1, CMDReadFileOpenPath02);
		CMDREAD_FUN(2, CMDReadFileRead02);
		CMDREAD_FUN(3, CMDReadFileWrite03);
		CMDREAD_FUN(4, CMDReadFileSeek04);
		CMDREAD_FUN(5, CMDReadFileClose05);
		CMDREAD_FUN(6, CMDReadFileFirst06);
		CMDREAD_FUN(7, CMDReadFileNext07);
		CMDREAD_FUN(8, CMDReadFileOpenPath02);
		CMDREAD_FUN(0xa, CMDReadFileDelete0A);
		CMDREAD_FUN(0xb, CMDReadDriveSize0B);
		CMDREAD_FUN(0xc, CMDReadDefault);
		CMDREAD_FUN(0xd, CMDReadDefault);
		CMDREAD_FUN(0xe, CMDReadDefault);
		CMDREAD_FUN(0xf, CMDReadDefault);
		CMDREAD_FUN(0x10, CMDReadFileChangeDir10);
		CMDREAD_FUN(0x11, CMDReadFileDelete0A);
		CMDREAD_FUN(0x12, CMDReadFileDelete0A);
		CMDREAD_FUN(0x15, CMDReadFileOpenPath02);
		CMDREAD_FUN(0x16, CMDReadFileFirst06);
		CMDREAD_FUN(0x17, CMDReadFileDelete0A);

		CMDREAD_FUN(0x13, CMDReadGetCurrentDir13);

		CMDREAD_FUN(0x14, CMDReadFileOpenPath02);
		CMDWRITE_FUN(0x14, CMDWriteFileOpen01);

		WRITE_FUN(0xff40, WritePramFF40);
		WRITE_FUN(0xff50, WritePramFF50);
		READ_FUN(0xff50, ReadPramFF50);
		READ_FUN(0xff48, ReadPramPCDataPortff48);
		READ_FUN(0xff40, ReadPramFF40);

		READ_FUN(0x4340, ReadPram4340);//读写 数据
		WRITE_FUN(0x4340, WritePram4340);
		READ_FUN(0x4348, ReadPram4348);//读写 bit $4348 状态
		WRITE_FUN(0x4348, WritePram4348);
	}

	void ResetResult(int s = 0, int e = 0xf) {
		for (int i = s; i < e; i++) {
			cbCMDResult[i] = 0x10 + i;
		}
	}

	void Reset()
	{
		reg4340 = 0x80;
		reg4348 = ~reg4340;
		wIndexCMD = cbAction = nTime = 0;
		bCmdSub = false;
		if (INVALID_HANDLE_VALUE != hFind)
			FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
		memset(cbCMD, 0, sizeof(cbCMD));
		write4340Times = cbCmdCur = 0;
		memset(cbCMDResult, 0, sizeof(cbCMDResult));
		strDirCurrent = "\\";

		for (int i = 2; i < 255; i++) {
			mListFreeFileNumber.push_back(i);
		}
		mMAP_OpenFile.clear();
	}

	void CMDWriteFileOpen01(BYTE data) {//打开文件
		auto &f = *(FileInfo*)&cbCMD[1];
		f.chBuf[wIndexCMD++] = data;
	}
	bool CMDReadFileOpenPath02(BYTE& data) {//带路径打开文件
		auto& f = *(FileInfo*)&cbCMD[1];
		switch (cbAction)
		{
		case 0: {
			cbAction = 0xf6;
			cbActionBack = 5;
			ActionInit(data);
			return true;
		}
		case 5: {
			ConvertFileName(f);
			if (!FileOpen(f, f.FILECMD == 8 || f.FILECMD == 0x15)) {
				fResult.STATUS = 2;
			}
			else {
				f.STATUS = 0;
				f.CURP = 0;
				f.HPAGE = 0x20;
				f.HANDLE = mListFreeFileNumber.front(); //文件号
				GetDataTime(GetFullPath(f).c_str(), f.HBUF, f.BYTES);
				memcpy(cbCMDResult, &cbCMD[1], sizeof(cbCMDResult));
				mMAP_OpenFile.insert(std::make_pair(mListFreeFileNumber.front(), f));
				mListFreeFileNumber.pop_front();
			}
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		default:
			break;
		}
	}
	bool CMDReadFileWrite03(BYTE& data) {//写文件
		auto& f = *(FileInfo*)&cbCMD[1];
		auto it = mMAP_OpenFile.find(f.HANDLE);
		if (it == mMAP_OpenFile.end()) {
			auto& fi = *(FileInfo*)cbCMDResult;
			fi.STATUS = 5;
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		auto& info = it->second;
		switch (cbAction)
		{
		case 0: {
			if (f.CURP > 0) {
				fseek(info.f, f.CURP, SEEK_SET);
				info.CURP = f.CURP;
			}
			cbAction = 0xf6;
			cbActionBack = 2;
			ActionInit(data);
			return true;
		}
		case 2: {
			fwrite(f.chBuf, 1, f.BYTES, f.f);
			fResult.BYTES = f.BYTES;
			fResult.HBUF = f.HBUF + f.BYTES;
			fResult.CURP = ftell(f.f);
			fResult.STATUS = 0;
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		}
	}
	bool CMDReadFileRead02(BYTE &data) {//读文件
		auto &f = *(FileInfo*)&cbCMD[1];
		auto it = mMAP_OpenFile.find(f.HANDLE);
		if (it == mMAP_OpenFile.end()) {
			auto& fi = *(FileInfo*)cbCMDResult;
			fi.STATUS = 5;
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		auto& info = it->second;
		switch (cbAction)
		{
		case 0: {
			if (f.CURP > 0) {
				fseek(info.f, f.CURP, SEEK_SET);
				info.CURP = f.CURP;
			}
			info.BYTES = f.BYTES;
			if (f.BYTES > info.dwLength - info.CURP)// || f.BYTES == 0)
				info.BYTES = info.dwLength - info.CURP;
			if (info.BYTES == 0) {
				fResult.BYTES = 0;
				fResult.STATUS = 0;
				cbAction = 0xf0;
				wIndexCMD = 0;
				cbCMD[0] = 0;
				ActionInit(data);
				return true;
			}
			cbAction = 0xf3;
			wLength = info.BYTES;
			wAddr = f.HBUF;
			cbActionBack = 5;
			info.dwReadRemain = wLength;
			info.BYTES = 0;
			ActionInit(data);
			return true;
		}
		case 5: {
			if (info.dwReadRemain == 0) {
				wIndexCMD = 0;
				cbCMD[0] = 0;
				info.CURP = ftell(info.f);
				fResult.BYTES = info.BYTES;
				fResult.STATUS = 0;
				fResult.HBUF = f.HBUF + info.BYTES;
				fResult.CURP = info.CURP;
				if (info.HANDLE == 0) {
					cbAction = 0;
					wIndexCMD = 0;
					bCmdSub = false;
					auto it = mMAP_OpenFile.find(0);
					mMAP_OpenFile.erase(0);
					data = 0;
				}
				else {
					cbAction = 0xf0;
					ActionInit(data);
				}
				return true;
			}
			if(info.CURP < info.dwLength){
				fread(&data, 1, 1, info.f);
				info.BYTES++;
			}
			info.dwReadRemain--;
			return true;
		}
		default:
			break;
		}
		return false;
	}
	bool ActionInit(BYTE &data) {
		auto& f = *(FileInfo*)&cbCMD[1];
		bool b = true;
		switch (cbAction)
		{
		case 0xe0: {
			bCmdSub = true;
			cbAction++;
			data = 0xa;
			break;
		}
		case 0xe1: {
			cbAction++;
			cbPageCur = f.chBuf[0];
			data = 0x8;
			break;
		}
		case 0xe2: {
			cbAction++;
			bCmdSub = false;
			data = f.HPAGE;
			break;
		}
		case 0xf0: {
			cbAction = 0xff;
			cbCMD[0] = 0;
			bCmdSub = false;
			wIndexCMD = 0;
			data = 0;
			break;
			cbAction++;
			data = 8;
			break;
		}
		case 0xf1: {
			cbAction++;
			data = f.HPAGE;// cbPageCur;
			break;
		}
		case 0xf2: {
			cbAction = 0xff;
			cbCMD[0] = 0;
			bCmdSub = false;
			wIndexCMD = 0;
			data = 0;
			break;
		}
		case 0xf3: {
			cbAction ++;
			data = 0xb;
			wIndexCMD = 0;
			break;
		}
		case 0xf4: {
			if (wIndexCMD++ == 0) {
				data = wLength>>8;
				break;
			}
			cbAction++;
			wIndexCMD = 0;
			data = wLength;
			break;
		}
		case 0xf5: {
			if (wIndexCMD++ == 0) {
				data = wAddr >> 8;
				break;
			}
			cbAction = cbActionBack;
			wIndexCMD = 0;
			data = wAddr;
			break;
		}
		case 0xf6: {
			bCmdSub = true;
			if (wLength == 0) {
				if (f.FILECMD == 1)
					wLength = 11;
				else
					wLength = GetStringBufLength(f.HBUF);
			}
			if(wAddr == 0)
				wAddr = f.HBUF;
			cbAction = 0xf4;
			data = 0x15;
			wIndexCMD = 0;
			break;
		}
		case 0xff: {//返回结果
			auto it = mMAP_OpenFile.find(f.HANDLE);
			BYTE* r = cbCMDResult;
			data = r[wIndexCMD];
			if (++wIndexCMD >= 0xe) {
				for (int i = 1; i < wIndexCMD; i++)
					vByte.push_back(cbCMD[i]);
				vByte.push_back(0xff);
				vByte.push_back(0xfe);
				for (int i = 0; i < wIndexCMD; i++)
					vByte.push_back(r[i]);
				DEBUGOUTIODetail("ret %s", pTools->WriteVByte(vByte));
				cbAction = 0;
				wIndexCMD = 0;
			}
			break;
		}
		default:
			b = false;
			break;
		}
		return b;
	}
	bool CMDReadFileChangeDir10(BYTE& data) {//10 改变当前目录
		bCmdSub = true;
		auto& f = *(FileInfo*)&cbCMD[1];
		switch (cbAction)
		{
		case 0: {
			cbAction = 0xf6;
			cbActionBack = 3;
			ActionInit(data);
			return true;
		}
		case 3: {
			std::string strTemp = strBBKRoot;
			if (f.chBuf[0] == '\\') {
				strTemp += f.chBuf;
			}
			else {
				strTemp += strDirCurrent;
				if(strDirCurrent[strDirCurrent.length() - 1] != '\\')
					strTemp += "\\";
				strTemp += f.chBuf;
			}
			char szPath[1024];
			if (0 != GetFullPathName(strTemp.c_str(), sizeof(szPath), szPath, nullptr))
				strTemp = szPath;
			if (strBBKRoot.length() > strTemp.length())
				strTemp = strBBKRoot;
			DEBUGOUTIODetail("CMDReadFileChangeDir10 path=%s", strTemp.c_str());
			if (!CheckFileAttrib(strTemp.c_str(), FILE_ATTRIBUTE_DIRECTORY)) {
				fResult.STATUS = 5;
			}
			else {
				fResult.STATUS = 0;
				strDirCurrent = strTemp.substr(strBBKRoot.length());
				if (strDirCurrent == "")
					strDirCurrent = "\\";
			}
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		}
		return true;
	}
	bool CMDReadGetCurrentDir13(BYTE& data) {//13 取当前目录
		const char *szPath = strDirCurrent.c_str();
		auto& f = *(FileInfo*)&cbCMD[1];
		switch (cbAction)
		{
		case 0:
			cbActionBack = 5;
			wLength = strDirCurrent.length()+1;
			wAddr = f.HBUF;
			cbAction = 0xf3;
			ActionInit(data);
			return true;
		case 5: {
			data = szPath[wIndexCMD++];
			if (wIndexCMD >= strDirCurrent.length() + 2) {
				fResult.CURDRIVE = 'C';
				fResult.HBUF = f.HBUF;
				fResult.STATUS = 0;
				cbAction = 0xf0;
				ActionInit(data);
				return true;
			}
			return true;
		}
		default:
			break;
		}
		return false;
	}
	bool CMDReadDriveSize0B(BYTE& data) {//0B 取磁盘大小
		cbAction = 0xff;
		wIndexCMD = 0;
		cbCMD[0] = 0;
		memset(cbCMDResult, 1, sizeof(cbCMDResult));
		auto& fResult = *(FileInfo*)&cbCMDResult[0];
		fResult.HBUF = 64;
		fResult.BYTES = 512;
		fResult.CURP = 8192;
		*(WORD*)&fResult.STATUS = 12800;
		return 0;
	}

	void SetFindFileInfo(FileInfo &f) {
		FileAttrib p;
		auto* pb = (BYTE*)&p;
		f.HBUF = 0x417;
		p.dwSize = mWIN32_FIND_DATA.nFileSizeLow;
		p.cbAttrib = mWIN32_FIND_DATA.dwFileAttributes;
		if (strcmp(mWIN32_FIND_DATA.cFileName, ".") == 0) {
			p.cbAttrib = 8;
			strcpy(p.chBuf, "PCCardDrive");
		}
		else {
			strcpy(p.chBuf, mWIN32_FIND_DATA.cFileName);
			ConvertFileNameToDOS(p.chBuf);
			strcpy(f.chBuf, p.chBuf);
			GetDataTime(GetFullPath(f).c_str(), p.fDate, p.fTime);
		}
		DEBUGOUTIODetail("SetFindFileInfo %s", mWIN32_FIND_DATA.cFileName);
		for (int i = sizeof(p) - 1; i >= 0; i--) {
			MapperBase::pMapper->WriteRam(f.HBUF + i, pb[i]);
		}
		MapperBase::pMapper->WriteRam(f.HBUF + 20, f.HPAGE);
		MapperBase::pMapper->WriteRam(f.HBUF + 21, f.HBUF + 23);
		MapperBase::pMapper->WriteRam(f.HBUF + 22, (f.HBUF + 23)>>8);
		std::string str = strDirCurFind.substr(strBBKRoot.length());
		if (str == "")
			str = "\\";
		for (int i = str.length(); i >= 0; i--) {
			MapperBase::pMapper->WriteRam(f.HBUF + 23 + i, str[i]);
		}
	}
	bool CMDReadFileNext07(BYTE& data) {//查找文件
		cbAction = 0xff;
		wIndexCMD = 0;
		cbCMD[0] = 0;
		auto& f = *(FileInfo*)&cbCMD[1];
		if (hFind == INVALID_HANDLE_VALUE) {
			fResult.STATUS = 0x12;
		}
		else if (!FindNextFile(hFind, &mWIN32_FIND_DATA)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
			fResult.STATUS = 0x12;
		}
		else if (strcmp(mWIN32_FIND_DATA.cFileName, "..") == 0) {
			return CMDReadFileNext07(data);
		}
		else {
			SetFindFileInfo(f);
		}
		return 0;
	}
	bool CMDReadFileFirst06(BYTE& data) {//查找文件
		auto& f = *(FileInfo*)&cbCMD[1];
		switch (cbAction)
		{
		case 0: {
			if (f.FILECMD == 6) {
				cbAction = 3;
				wIndexCMD = 0;
				memset(f.chBuf, 0, sizeof(f.chBuf));
				memcpy(f.chBuf, &cbCMD[1 + 5], 11);
				ConvertFileName(f, false);
				return CMDReadFileFirst06(data);
			}
			cbAction = 0xf6;
			cbActionBack = 3;
			wLength = GetStringBufLength(f.BYTES);
			wAddr = f.BYTES;
			ActionInit(data);
			return true;
		}
		case 3: {
			if (INVALID_HANDLE_VALUE != hFind)
				FindClose(hFind);
			std::string strTemp = GetFullPath(f);
			auto n = strTemp.rfind(f.chBuf);
			if (f.chBuf[0] != '\\')
				n--;
			strDirCurFind = strTemp.substr(0, n);
			//do {
			//	hFind = FindFirstFile(strTemp.c_str(), &mWIN32_FIND_DATA);
			//	DEBUGOUTIODetail("CMDReadFileFirst06 %s", strTemp.c_str());
			//	if (hFind != INVALID_HANDLE_VALUE)
			//		break;

			//	strDirCurFind = strDirCurFind.substr(0, strDirCurFind.rfind('\\'));
			//	if (strcmpi(strDirCurFind.c_str(), strBBKRoot.c_str()) == 0)
			//		break;
			//	strTemp = strDirCurFind + strTemp.substr(strTemp.rfind('\\'));
			//} while (true);
			hFind = FindFirstFile(strTemp.c_str(), &mWIN32_FIND_DATA);
			DEBUGOUTIODetail("CMDReadFileFirst06 %s", strTemp.c_str());
			if (hFind == INVALID_HANDLE_VALUE) {
				fResult.STATUS = 0x12;
			}
			else {
				fResult.STATUS = 0;
				SetFindFileInfo(f);
			}
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		}
		return false;
	}
	bool CMDReadDefault(BYTE& data) {
		fResult.STATUS = 0;
		cbAction = 0xf0;
		ActionInit(data);
		return true;
	}
	bool CMDReadFileDelete0A(BYTE& data) {
		auto& f = *(FileInfo*)&cbCMD[1];
		switch (cbAction)
		{
		case 0: {
			cbAction = 0xf6;
			cbActionBack = 2;
			ActionInit(data);
			return true;
		}
		case 2: {
			auto str = GetFullPath(f);
			switch (f.FILECMD)
			{
			case 0x11: {
				fResult.STATUS = CreateDirectory(str.c_str(), NULL) ? 0 : 5;
				break;
			}
			case 0x12: {
				fResult.STATUS = RemoveDirectory(str.c_str()) ? 0 : 5;
				break;
			}
			case 0x17: {
				fResult.STATUS = DeleteFileA(str.c_str()) ? 0 : 5;
				break;
			}
			default:
				break;
			}
			cbAction = 0xf0;
			ActionInit(data);
			return true;
		}
		}
		return false;
	}
	bool CMDReadFileSeek04(BYTE& data) {
		auto& f = *(FileInfo*)&cbCMD[1];
		auto it = mMAP_OpenFile.find(f.HANDLE);
		if (it == mMAP_OpenFile.end()) {
			auto& fi = *(FileInfo*)cbCMDResult;
			fseek(fi.f, f.CURP, SEEK_SET);
			return 0;
		}
		cbAction = 0xf0;
		ActionInit(data);
		return true;
	}
	bool CMDReadFileClose05(BYTE& data) {//打开文件
		auto& f = *(FileInfo*)&cbCMD[1];
		auto it = mMAP_OpenFile.find(f.HANDLE);
		if (it == mMAP_OpenFile.end()) {
			auto& fi = *(FileInfo*)cbCMDResult;
			fi.STATUS = 5;
			return 0;
		}
		FileClose(it->second);
		cbAction = 0xff;
		cbCMDResult[0x0] = 0;
		mMAP_OpenFile.erase(it);
		cbCMDResult[0xb] = 0; //必须为0
		cbCMDResult[0x1] = 0; //文件不存在 打开文件结果 DOS文件通道
		return 0;
	}
	bool FileClose(FileInfo& info) {
		fclose(info.f);
		DEBUGOUTIODetail("FileClose %s", info.chBuf);
		return true;
	}
	void GetDataTime(const char* pszFile, WORD &wDate, WORD &wTime) {
		struct stat st;
		if (-1 == stat(pszFile, &st))
			return;
		auto t = _gmtime32(&st.st_mtime);
		t->tm_mon++;
		t->tm_year -= 0x50;
		wDate = t->tm_mday | t->tm_mon << 5 | t->tm_year << 9;
		wTime = t->tm_sec | t->tm_min << 5 | t->tm_hour << 11;
	}
	const std::string GetFullPath(FileInfo& f, bool bOpen = false) {
		std::string strTemp = strDirCurFind;
		if (bOpen) {
			if (strTemp != "") {
				strTemp += f.chBuf;
				return strTemp;
			}
		}
		strTemp = strBBKRoot;
		if (f.chBuf[0] != '\\') {
			if(strDirCurrent.length() > 1)
				strTemp += strDirCurrent;
			strTemp += "\\";
		}
		strTemp += f.chBuf;
		return strTemp;
	}
	bool FileOpen(FileInfo& info, bool bCreate = false) {
		std::string strTemp = GetFullPath(info, true);
		strcpy(info.chBuf, strTemp.c_str());
		if (bCreate) {
			for (auto it = mMAP_OpenFile.begin(); it != mMAP_OpenFile.end(); ) {
				auto& f = it->second;
				if (strTemp == f.chBuf) {
					FileClose(info);
					it = mMAP_OpenFile.erase(it);
					continue;
				}
				++it;
			}
		}
		info.f = fopen(strTemp.c_str(), bCreate ? "a+b" : "rb");
		DEBUGOUTIODetail("OpenFile %s", strTemp.c_str());
		if (!info.f) {
			return false;
		}
		fseek(info.f, 0, SEEK_END);
		info.dwLength = ftell(info.f);
		fseek(info.f, 0, SEEK_SET);
		info.CURP = 0;
		return true;
	}

	bool OnRead(WORD addr, BYTE& data)
	{
		switch (addr)
		{
		//case 0x4350:
		//case 0x4358:
		//	data = reg4340;
		//	return true;
		default:
			break;
		}
		MAP_PREADFun::iterator it = mMAP_PREADFun.find(addr);
		if (it != mMAP_PREADFun.end())
		{
			bool b = (this->*it->second)(addr, data);
			DEBUGOUTTEST("BBKPCCard.Read PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
			return true;
		}
		return false;
	}
	bool OnWrite(WORD addr, BYTE data)
	{
		MAP_PWRITEFun::iterator it = mMAP_PWRITEFun.find(addr);
		if (it != mMAP_PWRITEFun.end())
		{
			bool b = (this->*it->second)(addr, data);
			DEBUGOUTTEST("BBKPCCard.Write PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
			return true;
		}
		return false;
	}

	bool ReadPram4340(WORD addr, BYTE& data)
	{
		write4340Times = 0;
		auto& f = *(FileInfo*)&cbCMD[1];
		if (cbAction >= 0xf0 && ActionInit(data))
			return true;

		if (cbCMD[0] == 1) {
			auto it = mMAP_PCMDREADFun.find(cbCMD[1]);
			if (it != mMAP_PCMDREADFun.end()) {
				if (bFirstCmd) {
					wAddr = wLength = 0;
					if (cbAction < 0xe0) {
						cbAction = 0xe0;
					}
					if (cbAction < 0xe3) {
						ActionInit(data);
						return true;
					}
					memset(cbCMDResult, 0, sizeof(cbCMDResult));
					memset(f.chBuf, 0, sizeof(f.chBuf));
					memcpy(cbCMDResult, &cbCMD[1], 0x10);
					for (int i = 1; i < wIndexCMD; i++)
						vByte.push_back(cbCMD[i]);
					DEBUGOUTIODetail("cmd %s", pTools->WriteVByte(vByte));
					cbAction = 0;
					wIndexCMD = 0;
				}
				bFirstCall = true;
				(this->*it->second)(data);
				bFirstCmd = false;
				return true;
			}
		}

		if (reg4340 == 2) {
			FileInfo fi;
			strcpy(fi.chBuf, "\\BBGDOS.SYS");
			FileOpen(fi);
			cbCMD[0] = 1;
			cbCMD[1] = 2;
			cbAction = 0xf3;
			wLength = fi.dwLength;
			wAddr = 0x7c00;
			cbActionBack = 5;
			f.HANDLE = fi.HANDLE = 0;
			fi.dwReadRemain = fi.dwLength;
			mMAP_OpenFile.insert(std::make_pair(0, fi));
			bFirstCmd = false;
			wIndexCMD = 0;
			bCmdSub = true;
			ActionInit(data);
			return true;
		}
		wIndexCMD = 0;
		data = reg4340;
		return true;
	}

	bool WritePram4340(WORD addr, BYTE data)
	{
		//高4位有效   两次为一个BYTE,先低后高
		if (write4340Times++ == 0) {
			reg4340 = data | data >> 4;
		}
		else {
			reg4340 &= 0xf;
			reg4340 |= data & 0xf0;
			write4340Times = 0;
			if (bCmdSub) {
				CMDWriteFileOpen01(reg4340);
				return true;
				//auto it = mMAP_PCMDWRITEFun.find(cbCMD[1]);
				//if (it != mMAP_PCMDWRITEFun.end()) {
				//	(this->*it->second)(reg4340);
				//	return true;
				//}
			}
			if (cbCMD[0] != 1) {
				wIndexCMD = 0;
			}
			else {
				cbAction = 0;
				if (wIndexCMD == 1) {
					bFirstCmd = true;
					//DEBUGOUTIODetail("CMD 4340 CMD=%02X", reg4340);
				}
			}
			cbCMD[wIndexCMD] = reg4340;
			if (wIndexCMD == 7)
				wIndexCMD = wIndexCMD;
			wIndexCMD++;
		}
		//reg4340 = data;
		return true;
	}

	bool ReadPram4348(WORD addr, BYTE& data)
	{
		data = reg4348;
		return true;
		bool b = true;
		switch (reg4348) {
		case 0:
			data = 0;
			break;
		case 0x80:
			data = 0x80;
			break;
		default:
			b = false;
			break;
		}
		return b;
	}

	bool WritePram4348(WORD addr, BYTE data)
	{
		reg4348 = data;
		return true;
	}

	bool ReadPramPCDataPortff48(WORD addr, BYTE& data)
	{
		return false;
	}

	bool ReadPramFF40(WORD addr, BYTE& data)
	{
		return false;
	}

	bool ReadPramFF50(WORD addr, BYTE& data)
	{
		return false;
	}

	bool WritePramFF40(WORD addr, BYTE data)
	{
		return false;
	}

	bool WritePramFF50(WORD addr, BYTE data)
	{
		return false;
	}
	char* ConvertFileNameToDOS(char* pszName) {
		char szTemp[20];
		memset(szTemp, 0x20, sizeof(szTemp));
		strupr(pszName);
		auto* p = strrchr(pszName, '.');
		int b = p ? (p - pszName) : (strlen(pszName) > 8 ? 8 : strlen(pszName));
		for (int i = 0; i < 11; i++) {
			if (i < 8) {
				if (i < b)
					szTemp[i] = pszName[i];
			}
			else if (p) {
				szTemp[i] = *++p;
				if (*p == 0)
					break;
			}
		}
		szTemp[11] = 0;
		strcpy(pszName, szTemp);
		return pszName;
	}
	char *ConvertFileName(FileInfo &f, bool bHaveSlash = true) {
		char *szFile = f.chBuf;
		auto ext = strrchr(szFile, '.');
		auto nEnd = 12;
		if (ext) {
			if (szFile[0] == '\\')
				return szFile;
			nEnd = strlen(szFile) + 1;
		}
		else {
			szFile[11] = 0;
			ext = strrchr(szFile, ' ');
			if (ext) {
				auto name = strstr(szFile, " ");
				*name++ = '.';
				for (int i = 0; i < 4; i++) {
					name[i] = ext[i + 1];
				}
				//strcat(szFile, ++name);
			}
			else {
				for (int i = 12; i > 8; i--)
					szFile[i] = szFile[i - 1];
				szFile[8] = '.';
			}
		}
		if (!bHaveSlash)
			return szFile;
		for (int i = 12; i >= 1; i--)
			szFile[i] = szFile[i - 1];
		szFile[0] = '\\';
		return szFile;
	}

	int GetStringBufLength(WORD addr) {
		int n = sizeof(FileInfo::chBuf);
		for (int i = 0; i < sizeof(FileInfo::chBuf); i++) {
			if (0 == MapperBase::pMapper->ReadRam(addr + i)) {
				n = i + 1;
				break;
			}
		}
		return n;
	}

	bool CheckFileAttrib(const char* pszPath, DWORD dwFlag) {
		auto f = GetFileAttributesA(pszPath);
		if (f == INVALID_FILE_ATTRIBUTES)
			return false;
		if (f & dwFlag)
			return true;
		return false;
	}
};

class BBKMouse
{
public:
	BYTE regWrite[20], chData4017;
	bool bMouseReady, bReadyRead;
	int nIndexRead4016, nIndexRead4017;
	BBKMouse()
	{
		nIndexRead4017 = nIndexRead4016 = 0;
		bMouseReady = false;
		bReadyRead = false;
		ZeroMemory(regWrite, sizeof(regWrite));
	}
	bool OnRead(WORD addr, BYTE& data)
	{
		if (addr < 0x4016 || addr > 0x4017)
			return false;
		//return false;
		if (_CPU_PC < 0x8000 || _CPU_PC > 0x9000)
			return false;
		DEBUGOUTTEST("BBKMouse.Read PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		return false;
		bool bSetData = false;
		if (bReadyRead)
		{
			data = 0;
			bSetData = true;
			//84D0  8bit
			//84F7
			//8610
			switch (_CPU_PC)
			{
				//case 0x83FF:
				//case 0x8610:
				//case 0x8601:
				//case 0x8636:
				//	data = 0;
				//	bSetData = true;
				//	break;
			case 0x840B:
			case 0x8432:
			case 0x84A5:
			case 0x84B6:
			case 0x85F5:
			case 0x8659:

			case 0x84D0:
			case 0x84F7:
			case 0x8610:

				data = 1;
				break;
			}
		}
		else
			//if (!bSetData)
		{
			switch (addr)
			{
			case 0x4016:
			{
				if (++nIndexRead4016 >= 9)
				{
					bMouseReady = true;
					nIndexRead4017 = 0;
				}
				else
				{
					bMouseReady = false;
				}
				break;
			}
			case 0x4017:
			{
				//if (!bReadyRead)
				//{
				//	nIndexRead4016 = 0;
				//}
				//else
				//{
				//	data = chData4017;
				//	bSetData = true;
				//	break;
				//}
				if (bMouseReady)
				{
					if (++nIndexRead4017 <= 4)
					{
						if (nIndexRead4017 == 4)
						{
							bReadyRead = true;
						}
						data = 1;
						bSetData = true;
						break;
					}
				}
				break;
			}
			}
		}
		DEBUGOUTTEST("BBKMouse.Read PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		return bSetData;
	}

	bool OnWrite(WORD addr, BYTE data)
	{
		if (addr < 0x4016 || addr > 0x4017)
			return false;
		//return false;
		if (_CPU_PC < 0x8000 || _CPU_PC > 0x9000)
			return false;
		DEBUGOUTTEST("BBKMouse.Write PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		return false;
		if (!bReadyRead)
		{
			nIndexRead4016 = 0;
			return false;
		}
		switch (addr)
		{
		case 0x4016:
		{
			switch (data)
			{
			case 0:
				chData4017 = 0;
				break;
			default:
				chData4017 = 1;
				break;
			}
			break;
		}
		}
		return false;
	}
};

class BBKRs232 : public CnComm
{
public:
	typedef std::deque<BYTE> DequeByte;
	DequeByte mDequeByteRead, mDequeByteWrite;
	int nBitRead, nBitWrite;
	BYTE nByteWrite, nByteRead;
	Tools& mTools;
	VectorByte bbb;

	struct FlagReadWrite
	{
		bool bReady;
		int nCount, nStep;
	};
	FlagReadWrite mFlagRead, mmFlagWrite;

	BBKRs232() : mTools(*pTools)
	{
		nBitRead = nBitWrite = 0;
		nBitWrite = 0;
		nByteWrite = 0;
		mDequeByteRead.push_back('A');
	}
	~BBKRs232()
	{
	}
	void Reset()
	{
		SetState(19200);
		//BindPort(10);
		//wsprintf(szName_, _T("\\\\.\\CNCA0"));

		DEBUGOUTIODetail("打开串口 COM%d %s ...", GetPort(), Open(10) ? "成功" : "失败");
	}
	bool OnRead(WORD addr, BYTE& data)
	{
		if (addr != 0x4019)
			return false;
		static DWORD dwTick = 0;
		DEBUGOUTTEST("BBKRs232.Read dwTick=%d", GetTickCount() - dwTick);
		if (GetTickCount() - dwTick > 0)
		{
			dwTick = GetTickCount();
			//mDequeByteRead.clear();
			//static BYTE c = 'C';
			//mDequeByteRead.push_back(c++);
			data = 2;
			nBitRead = 0;
			vByte.clear();
			return true;
		}
		dwTick = GetTickCount();
		bool bWrite = false;
		if (nBitRead >= 9)
		{
			data = 2;
			nBitRead = 0;
			bWrite = true;
			if (mDequeByteRead.size() <= 0)
				mDequeByteRead.push_back('A');
			goto RetData;
		}
		if (nBitRead++ == 0)
		{
			if (mDequeByteRead.size() <= 0)
			{
				data = 0;
				nBitRead = 0;
				goto RetData;
			}
			nByteRead = mDequeByteRead.front();
			vByte.push_back(nByteRead);
			nByteRead = ~nByteRead;
			mDequeByteRead.pop_front();
			data = 2;
			goto RetData;
		}
		data = nByteRead >> (nBitRead - 2);
		data = data & 1 ? 2 : 0;
	RetData:
		vByte.push_back(data);
		if (bWrite)
			mTools.WriteVByte(vByte);
		return true;
	}
	bool OnWrite(WORD addr, BYTE data)
	{
		if (addr != 0x4018)
			return false;
		static DWORD dwTick = 0;
		DEBUGOUTTEST("BBKRs232.Write dwTick=%d", GetTickCount() - dwTick);
		if (GetTickCount() - dwTick > 1000)
		{
			mFlagRead.bReady = false;
			mFlagRead.nCount = 0;
			mFlagRead.nStep = 0;

			mmFlagWrite.bReady = false;
			mmFlagWrite.nCount = 0;
			mmFlagWrite.nStep = 0;
		}

		if (GetTickCount() - dwTick > 0)
		{
			if (!mFlagRead.bReady)
			{
				mFlagRead.nCount = 0;
				mFlagRead.nStep = 0;
			}
			if (!mmFlagWrite.bReady)
			{
				mmFlagWrite.nCount = 0;
				mmFlagWrite.nStep = 0;
			}
			dwTick = GetTickCount();
			nBitWrite = 0;
			vByte.clear();
		}
		else
		{
		}
		if (!mmFlagWrite.bReady)
			return false;
		//bbb.push_back(data);
		//mTools.WriteVByte(bbb);
		if (nBitWrite != 0 && nBitWrite != 9)
		{
			data = (~data);
			data = data >> 2;
			nByteWrite |= (data << (nBitWrite - 1));
		}
		if (nBitWrite++ >= 9)
		{
			nByteWrite = ~nByteWrite;
			CnComm::Write(&nByteWrite, sizeof(nByteWrite));
			vByte.push_back(nByteWrite);
			mTools.WriteVByte(vByte);
			mDequeByteWrite.push_back(nByteWrite);
			nBitWrite = 0;
			nByteWrite = 0;
		}
		return true;
	}
	virtual void OnReceive()
	{
		do {

			char buffer[1024];
			int len = CnComm::Read(buffer, 1023);
			buffer[len] = _T('\0');
			for (int i = 0; i < len; i++)
				mDequeByteRead.push_back(buffer[i]);
			//! 接收缓冲模式下，要确保把已经在Comm_.Input()缓冲区的数据处理完
			//! 否则如果没有新的数据再来，不会继续通知你
		} while (IsRxBufferMode() && Input().SafeSize());
	}
};

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

	BBKFDCDriver() : mTools(*pTools)
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
		DEBUGOUTFLOPPY("FDC FdcSeekWrite PC=%04X data=%02X", _CPU_PC, data);
		return 0;
	}
	BYTE FdcReadDataRead(BYTE data) {
		if (!mFdcData[nFdcDrvSel].pFdcCmd)
			return 0;
		mFdcData[nFdcDrvSel].nFdcMainStatus |= FDC_MS_DATA_IN;

		if (bRead0xffa0Log) {
			bRead0xffa0Log = false;
			DEBUGOUTFLOPPY("FDC FdcReadDataRead PC=%04X", _CPU_PC);
		}
		data = *mFdcData[nFdcDrvSel].pFdcDataPtr++;
		mFdcData[nFdcDrvSel].nFdcRead = -1;
		vByte.push_back(data);
		if (vByte.size() >= 512)
		{
			mTools.WriteVByte(vByte);
			vByte.clear();
		}
		return data;
	}

	BYTE FdcReadDataWrite(BYTE data) {
		DEBUGOUTFLOPPY("FDC FdcReadDataWrite PC=%04X", _CPU_PC);
		bRead0xffa0Log = true;
		BYTE CNT = mFdcData[nFdcDrvSel].bFdcCommands[1] + 1;	//读取的扇区数
		BYTE C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		BYTE H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		BYTE R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		BYTE N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcReadSize = 0x200 * CNT;
		mFdcData[nFdcDrvSel].nFdcResultsIndex = 0;
		//18:12 36:24
		INT LBA = H * 18 + C * 36 + (R - 1);
		BPB* pBPB = (BPB*)&mTools.lpFloppy[0xb];
		//INT LBA = H * pBPB->BPB_SecPerTrk + C * (pBPB->BPB_SecPerTrk * (pBPB->BPB_FATSz16 - 1)) + (R - 1);
		DEBUGOUTFLOPPY("FDC SEEK C=%04X H=%04X R=%04X N=%04X", C, H, R, N);
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
				//if (80 == C)
				//    C = 0;
			}
		}
		return 0;
	}

	BYTE FdcWriteDataWrite(BYTE data) {
		mFdcData[nFdcDrvSel].bReading = false;
		DEBUGOUTFLOPPY("FDC FdcWriteDataWrite PC=%04X", _CPU_PC);
		bRead0xffa0Log = false;
		bWrite4205Log = false;
		BYTE C = mFdcData[nFdcDrvSel].bFdcCommands[2];	//磁道号
		BYTE H = mFdcData[nFdcDrvSel].bFdcCommands[3];	//磁头号
		BYTE R = mFdcData[nFdcDrvSel].bFdcCommands[4];	//扇区号
		BYTE N = mFdcData[nFdcDrvSel].bFdcCommands[5];
		mFdcData[nFdcDrvSel].nFdcWrite = mFdcData[nFdcDrvSel].pFdcCmd->bWLength - 1;

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
				//if (80 == C)
				//    C = 0;
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
		//bRead0xffa0Log = false;
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
	void Write0xff90(WORD addr, BYTE data) {
		mFdcData[nFdcDrvSel].n4304Count = 0;
		static bool bInit = true;
		static BYTE nFdcDrvSelPrev;
		static char* szName[] = { "A", "B", "C", "D" };
		DEBUGOUTFLOPPY("FDC Write0xff90 PC=%04X data=%s", _CPU_PC, mTools.GetBitNumber(data));
		nFdcDrvSel = data & 3;
		mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// data;
		if (mFdcData[nFdcDrvSel].nFdcMainStatus == 0)
		{
			nFdcDrvSelPrev = nFdcDrvSel;
			mFdcData[nFdcDrvSel].bFormat = false;
			DEBUGOUTFLOPPY("FDC 启动驱动器[%s:]", szName[nFdcDrvSel]);
		}
		else if (nFdcDrvSelPrev != nFdcDrvSel)
		{
			nFdcDrvSelPrev = nFdcDrvSel;
			DEBUGOUTFLOPPY("FDC 切换驱动器[%s:]", szName[nFdcDrvSel]);
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;// data;
			//if (nFdcDrvSel != 0)//不启用Ｂ盘
			//	return;
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
				BYTE C = vByte[i * 4 + 0];
				BYTE H = vByte[i * 4 + 1];
				BYTE R = vByte[i * 4 + 2];
				INT LBA = H * 18 + C * 36 + (R - 1);
				BYTE* pFdcDataPtr = mTools.lpFloppy + LBA * 512;
				//ZeroMemory(pFdcDataPtr, 512);
			}
			bWriteFloppy = true;
			vByte.clear();
		}
		//if (data & 0x8)//0x10) {
		{
			if (vByte.size() > 0) {
				mTools.WriteVByte(vByte);
			}
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
			DEBUGOUTFLOPPY("FDC Write0x4205 PC=%04X data=%02X", _CPU_PC, data);
			if (data == 0x45) {
				if (n == 1) {
					n = 1;
				}
				n++;
			}
		}
		if (vByte.size() >= 1) {//0x200) {
			DEBUGOUTFLOPPY("FDC Write0x4205 数据异常 %03d PC=%04X 0x4205=%02X", vByte.size(), _CPU_PC, data);
			mTools.WriteVByte(vByte);
		}
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
				DEBUGOUTFLOPPY("FDC Unkonw Command Write0x4205 PC=%04X 0x4205=%02X", _CPU_PC, data);
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
		//if (mFdcData[nFdcDrvSel].n4304Count > 40)//说明读盘程序进入死循环了,暴力终止进程
		//	ExitProcess(0);
		if (bRead0xffa0Log) {
			DEBUGOUTFLOPPY("FDC  Read0xffa0 PC=%04X addr=%04X data=%02X", _CPU_PC, addr, mFdcData[nFdcDrvSel].nFdcMainStatus);
		}
		return mFdcData[nFdcDrvSel].nFdcMainStatus;
	}

	bool OnRead(WORD addr, BYTE& data)
	{
		bool bDefault = false;
		switch (addr)
		{
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
			DEBUGOUTFLOPPY("FDC  Read PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
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
			DEBUGOUTFLOPPY("FDC BBKFDCDriver.Read PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
		}
		return true;
	}

	bool OnWrite(WORD addr, BYTE data)
	{
		switch (addr)
		{
			//case 0xffb8:
			//	break;
		case 0xff80:	//磁盘写数据
		{
			if (!mFdcData[nFdcDrvSel].pFdcDataPtr)
				return true;
			bWriteFloppy = true;
			if (vByte.size() == 510) {
				bWrite4205Log = true;
			}
			if (bWrite4205Log)
			{
				DEBUGOUTFLOPPY("FDC Write PC=%04X addr=%04X data=%02X", _CPU_PC, addr, data);
				bWrite4205Log = false;
			}
			if (mFdcData[nFdcDrvSel].bFormat)
				return true;

			*mFdcData[nFdcDrvSel].pFdcDataPtr = data;
			mFdcData[nFdcDrvSel].pFdcDataPtr++;
			vByte.push_back(data);
			if (vByte.size() >= 512)
			{
				mTools.WriteVByte(vByte);
				vByte.clear();
			}
			//return true;
			break;
		}
		case 0xff90:	// FDCDRQPortI/FDCCtrlPortO
			Write0xff90(addr, data);
			//return true;
			break;
		case 0xffA0:	// FDCResetPortO/FDCStatPortI
			// O: D6 : FDC pin reset
			mFdcData[nFdcDrvSel].nFdcMainStatus = FDC_MS_RQM;
			//return true;
			break;
		case 0xffA8:	// FDCDataPortIO
			Write0x4205(addr, data);
			//return true;
			break;
		case 0xffB8:
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
	typedef void(BBK10::* PWRITEFun)(WORD addr, BYTE data);
	typedef std::map<int, PWRITEFun> MAP_PWRITEFun;
	typedef std::list<BYTE> LIST_ChrQueue;
	VectorByte vByteVoice;
	LIST_ChrQueue mLIST_ChrQueue;
	MAP_PWRITEFun mMAP_PWRITEFun;
#define WRITE_FUN(addr,fun) mMAP_PWRITEFun.insert(std::make_pair(addr, &BBK10::fun))
	typedef BYTE(BBK10::* PReadFun)(WORD addr);
	typedef std::map<int, PReadFun> MAP_PReadFun;
	MAP_PReadFun mPReadFun;
#define READ_FUN(addr,fun) mPReadFun.insert(std::make_pair(addr, &BBK10::fun))
	BBKFDCDriver mFDCDriver;
	BBKRs232 mBBKRs232;
	//BBKMouse mBBKMouse;
	BBKPCCard mBBKPCCard;
protected:
	BYTE reg[0xff], regVR[20], regSR[20];
	int nVRIndex, nSRIndex;
	bool _irqEnabled, bIRQRunning;
	bool bWriteIO, bPCCard, bUseFloppy;
	DWORD dwVoiceTick;

public:
	BBK10() {
	}
	~BBK10()
	{
	}
	virtual int GetPRGRamSize() { return nPrgRamSize * 1024; }
	virtual int GetCHRRamSize() { return nChrRamSize * 1024; }
	virtual int GetWorkRamSize() { return nWRamSize * 1024; }
	virtual void OnRUN(WORD addr, BYTE value)
	{
		__super::OnRUN(addr, value);
		if (dwVoiceTick != 0 && GetTickCount() - dwVoiceTick > 150)
		{
			dwVoiceTick = 0;
			mTools.PlayVoice(vByteVoice);
		}
	}
	virtual void Reset(bool) override {
		mBBKPCCard.Reset();
	}

	virtual void Power() override
	{
		__super::Power();
		bNMI = bIRQ = false;
		for (int i = 0; i < sizeof(W2007Info) / sizeof(W2007Info[0]); i++) {
			W2007Info[i].bFirst = true;
		}

		bUseKeyBoard = true;
		bIRQRunning = false;
		dwVoiceTick = 0;
		mTools.bCanSetTitle = false;//硬件重置时 SetTitle 会阻塞
		memset(reg, 0, sizeof(reg));
		memset(regVR, 0, sizeof(regVR));
		memset(regSR, 0, sizeof(regSR));
		nSRIndex = nVRIndex = 0;
		mBBKRs232.Reset();
		mBBKPCCard.Reset();
		mTools.Reset();
		mTools.FloppyIMGLoad();
		mTools.bCanSetTitle = true;
		mTools.bCaptureMouse = true;
		VideoMode(VideoModeDendy);//中文需要VideoModePAL模式 某些非中文软件会有一条花屏线需要VideoModeNTSC模式 Dendy:2 
		SetVRam8K(0);
		SetWRam8K(2, 0);
		SetWRam8K(3, 1);
		SetRom16K(4, 0);
		SetRom16K(6, PRGRomSize - 1);

		WRITE_FUN(0xff03, WriteVramFF03);
		WRITE_FUN(0xff0B, WriteVramFF0B);
		WRITE_FUN(0xff13, WriteVramFF13);
		WRITE_FUN(0xff1B, WriteVramFF1B);

		WRITE_FUN(0xff01, WritePramFF01);
		WRITE_FUN(0xff04, WritePramFF04);
		WRITE_FUN(0xff14, WritePramFF14);
		WRITE_FUN(0xFF10, WritePramSoundFF10);
		WRITE_FUN(0xFF18, WritePramSoundFF18);
		READ_FUN(0xFF18, ReadPramSoundFF18);
		WRITE_FUN(0xff1C, WritePramFF1C);
		WRITE_FUN(0xff24, WritePramFF24);
		WRITE_FUN(0xff2C, WritePramFF2C);
	}

	//
	void WritePramSoundFF10(WORD addr, BYTE data)
	{// FF 00 FF 开始语音
		bWriteIO = false;
	}
	//语音数据
	BYTE ReadPramSoundFF18(WORD addr)
	{
		bWriteIO = false;
		return 0x8f;
	}
	//语音数据
	void WritePramSoundFF18(WORD addr, BYTE data)
	{
		bWriteIO = false;
		dwVoiceTick = GetTickCount();
		vByteVoice.push_back(data);
	}

	void WritePramFF01(WORD addr, BYTE data)
	{
		const MirroringType nMonitor[] = { MirroringTypeVertical, MirroringTypeHorizontal, MirroringTypeAOnly, MirroringTypeBOnly };
		static MirroringType nType = MirroringTypeVertical;
		if (nType != nMonitor[data & 3])
		{
			//DEBUGOUTTEST("WritePramFF01 nType=%d data=%d", nType, nMonitor[data & 3]);
			nType = nMonitor[data & 3];
			SetMirroringType(nMonitor[data & 3]);
		}
		// map C000-FFF to ROM
		if (reg[1] & 0x8)//0x8
		{
			//SetRam8K(6, reg[0x24] & nPrgRamCount8);
			//SetRam8K(7, nPrgRamCount8);
			SetRam16K(6, nPrgRamCount16);
			//SetRam8K(7, 0x3f);
		}
		else
		{
			SetRom16K(6, PRGRomSize - 1);
		}
	}

	void SetMouseStatus(WORD addr)
	{
		if (!mTools.bCaptureMouse || !mTools.bCapture) {
			WriteRam(0x5ffc, 0);
			return;
		}
		POINT ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(mTools.mWndMain, &ptCursor);
		RECT rc;
		GetClientRect(mTools.mWndMain, &rc);
		char cx = rc.right / 2 - ptCursor.x;
		char cy = rc.bottom / 2 - ptCursor.y;
		POINT pt;
		pt.x = rc.right / 2;
		pt.y = rc.bottom / 2;
		ClientToScreen(mTools.mWndMain, &pt);
		SetCursorPos(pt.x, pt.y);
		int x = ReadRam(0x5ff2);
		x -= cx;
		if (x < 0)
			x = 0;
		if (x > 247)
			x = 247;
		WriteRam(0x5ff2, x);
		int y = ReadRam(0x5ff3);
		y -= cy;
		if (y < 0)
			y = 0;
		if (y > 239)
			y = 239;
		WriteRam(0x5ff3, y);
		if (addr == 0x5ffc)
		{
			static bool p1 = false, p2 = false;
			static int cnt1 = 0, cnt2 = 0;
			BYTE b;
			const int nMax = 16;
			if (mTools.bLButtonDown && cnt1 < nMax)
				p1 = true;
			else
				p1 = false;
			if (mTools.bRButtonDown && cnt2 < nMax)
				p2 = true;
			else
				p2 = false;
			if (++cnt1 > nMax)
				cnt1 = nMax;
			if (++cnt2 > nMax)
				cnt2 = nMax;
			if (!mTools.bLButtonDown)
				cnt1 = 0;
			if (!mTools.bRButtonDown)
				cnt2 = 0;
			WriteRam(0x5ffc, ((p1 << 0) | (p2 << 1)));
			//y = ReadRam(0x5ffc);
			//if (y > 0 && ++cnt > 8)
			//	y = 0;
			//else
			//	cnt = 0;
			//if ((mTools.bLButtonDown && p1) && y > 0)
			//	b = 0;
			//else
			//	b = mTools.bLButtonDown;
			//if ((mTools.bRButtonDown && p2) && y > 0)
			//	b |= 0 << 1;
			//else
			//	b |= mTools.bRButtonDown << 1;
			//	//b = ((mTools.bLButtonDown << 0) | (mTools.bRButtonDown << 1));
			//p1 = mTools.bLButtonDown;
			//p2 = mTools.bRButtonDown;
			//WriteRam(0x5ffc, b);
		}
		y = ReadRam(0x5c65);
		WriteRam(0x5c65, mTools.bLButtonDown ? y + 1 : 0);
		y = ReadRam(0x5c65 + 1);
		WriteRam(0x5c65 + 1, mTools.bRButtonDown ? y + 1 : 0);
	}

	void WritePramFF04(WORD addr, BYTE data)
	{
		//如果没有以下语句,003.IMG 指法练习,五笔打字 中文会有部分乱码
		//if (((reg[1] & 0x40) || reg[1] == 4) && (data < ~nPrgRamCount16))//0xe0))
		if ((data < (~nPrgRamCount16&0xff)))//这样 输入法提示行正常
		//if ((reg[1] & 0x40) && (reg[0x04] < (~nPrgRamCount16 & 0xff)))// && (data < ~nPrgRamCount16))//0xe0))
		//if (((reg[1] & 0x40) || reg[1] == 4) && (data < 0xe0))//这样中文输入法提示行会有乱码
		{
			SetRom16K(4, data & 7);
			//DebugString("WritePramFF04 ROM data=%02X", data);
		}
		else
		{
			//DebugString("WritePramFF04 RAM data=%02X", data);
			SetRam16K(4, (data & nPrgRamCount16));
		}
	}
	void WritePramFF14(WORD addr, BYTE data)
	{
		SetRam8K(4, data & nPrgRamCount8);
	}
	void WritePramFF1C(WORD addr, BYTE data)
	{
		SetRam8K(5, data & nPrgRamCount8);
	}
	void WritePramFF24(WORD addr, BYTE data)
	{
		if (reg[1] & 8) {
			SetRam8K(6, data & nPrgRamCount8);
			//DebugString("WritePramFF24 RAM data=%02X", data);
		}
	}
	void WritePramFF2C(WORD addr, BYTE data)
	{
		if (reg[1] & 8) {
			SetRam8K(7, data & nPrgRamCount8);
			//DebugString("WritePramFF2C RAM data=%02X", data);
		}
	}

	void WriteVramFF03(WORD addr, BYTE data)
	{
		SetVRam2K(0, data & nChrRamCount2);
	}

	void WriteVramFF0B(WORD addr, BYTE data)
	{
		SetVRam2K(2, data & nChrRamCount2);
	}
	void WriteVramFF13(WORD addr, BYTE data)
	{
		SetVRam2K(4, data & nChrRamCount2);
	}
	void WriteVramFF1B(WORD addr, BYTE data)
	{
		SetVRam2K(6, data & nChrRamCount2);
	}
	bool WriteVram(WORD addr, BYTE data)
	{
		switch (addr)
		{
		case 0xff23:
		{
			SetVRam1K(0, data & nChrRamCount1);
			break;
		}
		case 0xff2b:
		{
			SetVRam1K(1, data & nChrRamCount1);
			break;
		}
		case 0xff33:
		{
			SetVRam1K(2, data & nChrRamCount1);
			break;
		}
		case 0xff3b:
		{
			SetVRam1K(3, data & nChrRamCount1);
			break;
		}
		case 0xff43:
		{
			SetVRam1K(4, data & nChrRamCount1);
			break;
		}
		case 0xff4b:
		{
			SetVRam1K(5, data & nChrRamCount1);
			break;
		}
		case 0xff53:
		{
			SetVRam1K(6, data & nChrRamCount1);
			break;
		}
		case 0xff5b:
		{
			SetVRam1K(7, data & nChrRamCount1);
			break;
		}
		default:
			return false;
		}
		return true;
	}

	virtual bool OnReadRam(WORD addr, BYTE& data) override
	{
		if (addr == 0x5ff2 || addr == 0x5ff3 || addr == 0x5ffc)
		{
			SetMouseStatus(addr);
			return false;
		}
		if (addr == 0x6000)
		{
			if (reg[1] & 0x44)
				WriteRam(0x6000, ReadRam(0x6000) | 0x20);
			return false;
		}
		if (bPCCard && mBBKPCCard.OnRead(addr, data))
			return true;

		if (addr < 0xff00)
			return false;

		if (bUseFloppy && mFDCDriver.OnRead(addr, data))
			return true;
		//if (mBBKRs232.OnRead(addr, data))
		//	return true;
		auto it = mPReadFun.find(addr);
		if (it != mPReadFun.end())
		{
			data = (this->*it->second)(addr);
			return true;
		}
		//DEBUGOUTIO("OnRead default PC=%04X addr=%04X", _CPU_PC, addr);
		return false;
	}
	virtual bool OnWriteRam(WORD addr, BYTE& data) override
	{
		if (bPCCard && mBBKPCCard.OnWrite(addr, data))
			return true;
		if (addr < 0xff00)
			return false;

		reg[addr & 0xff] = data;
		if (mFDCDriver.OnWrite(addr, data))
			return true;
		//if (mBBKRs232.OnWrite(addr, data))
		//	return true;
		MAP_PWRITEFun::iterator it = mMAP_PWRITEFun.find(addr);
		if (it != mMAP_PWRITEFun.end())
		{
			(this->*it->second)(addr, data);
			//if(bIDC_CHECK_LogOut && bWriteIO)
			//	DEBUGOUTIODetail("OnWriteRam I=%d N=%d PC=%04X addr=%04X data=%s", bIRQ, bNMI, _CPU_PC, addr, mTools.GetBitNumber(data));
			bWriteIO = true;
			return true;
		}
		static BYTE s_regVR[20], s_regSR[20];
		bool bRes = true;
		switch (addr)
		{
		case 0xff09://必须返回true,否则无限循环
			break;
			//屏幕分裂
		case 0xff0a://Write to SR queue
		{
			static WORD v = 0;
			regSR[nSRIndex++] = data;
			break;
		}
		case 0xff12://Queue INIT
		{
			static WORD v = 0;
			if (data & 0x40)
			{
				nVRIndex = 0;
				nSRIndex = 0;
			}
			break;
		}
		case 0xff1a:// Write to VR queue
		{
			regVR[nVRIndex++] = data;
			break;
		}
		case 0xff22://START
		{
			if (!bIRQ) {
				_irqEnabled = true;
				bCanIRQ = true;
			}
			memcpy(s_regVR, regVR, sizeof(regVR));
			break;
		}
		default:
			//DEBUGOUTIO("OnWrite default PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
			break;
			/*
			case 0xff02:
				return true;
			case 0xff06:
				return true;
			case 0xff09:
				return true;
			case 0xff00://KebBoardLEDPort
			{
				DEBUGOUTIODetail("OnWriteRam PC=%04X addr=%04X data=%s", _CPU_PC, addr, mTools.GetBitNumber(data));
				return true;
			}
			*/
		}
		return true;
	}
};

static W2007AddrInfo* GetAddr2007() {
	auto p = (BBK10*)MapperBase::pMapper;
	if (p->bIRQ)
		return &W2007Info[0];
	else if (p->bNMI)
		return &W2007Info[1];
	return &W2007Info[2];
}
