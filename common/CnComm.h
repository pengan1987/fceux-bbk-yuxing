/*! \mainpage CnComm v1.51 多线程串口通讯库 
 *	\section About 关于
 *  
 *  \n 版本: CnComm v1.51
 *  \n 用途: WINDOWS/WINCE 多线程串口通讯库
 *  \n 语言: C++ (ANSI/UNICODE)
 *  \n 平台: WINDOWS(WIN98/NT/2000/XP/2003/Vista); WINCE 5.0 模拟器; Pocket PC 2003 模拟器;
 *  \n 硬件: PC串口; 串口服务器; USB串口; 虚拟串口;
 *  \n 编译: BC++ 5(free tool); C++ BUILDER 4, 5, 6, X; EVC 4(sp4); G++ 3, 4; Intel C++ 7, 8, 9; VC++ 6(sp6), .NET, 2003, 2005;
 *  \n 作者: llbird
 *  \n 邮箱: wushaojian@21cn.com
 *  \n 博客: http://blog.csdn.net/wujian53 http://www.cppblog.com/llbird  
 *  \n 维护: 2002.10 - 2009.8
 *
 *  \section Announce 说明
 *  \n 1) 可以自由使用及传播, 请保留相关声明;                                           
 *  \n 2) 不推荐直接在本代码上修改, 应通过C++继承扩展机制扩展本代码;                          
 *  \n 3) 如果您直接修改本代码, 请发一份给我，便于同网友分享您有益的改动;                              
 *  \n 4) 不兼容cnComm1.4以下版本, 有很大改动，同时也更名CnComm;
 *  \n 5) 还是那句老话, 水平有限, 错误在所难免, 欢迎来信指正, 收入有限, 时间有限, 不提供除CnComm内部问题外的咨询;
 *  
 *  \section Log 日志
 *  \n 2009 v1.51 修正版; 考虑到将来的工作中可能不会再和串口打交道，这很可能是最后一版;
 *  \n 2009 v1.5  增加内置分块链表缓冲区; 增加对WINCE的支持(模拟器下测试通过);
 *  \n 2008 v1.4  增加对同步IO的多线程支持; 增加C++异常的支持; 改名CnComm; Cn == C Next;
 *  \n 2007 v1.3  细节部分修订;
 *  \n 2006 v1.2  细节部分修订;
 *  \n 2005 v1.1  细节部分修订;
 *  \n 2004 v1.0  采用VC命名风格(匈牙利), 在多个WINDOW平台、编译器测试通过, 首次公开发布cnComm;
 *  \n 2002 v0.1  因工作需要开发串口通讯基础类, 传统C++的继承机制, 传统C命名风格;
 */

#ifndef _CN_COMM_H_
#define _CN_COMM_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

#if defined(UNDER_CE) && !defined(CN_COMM_FOR_CE)
	#define CN_COMM_FOR_CE		UNDER_CE			//!< 配置WINCE的支持   
#endif

#ifndef CN_COMM_FOR_CE
	#include <mcx.h>
	#include <process.h>							// WINCE没有process.h
#endif

#ifndef ON_COM_MSG_BASE 
	#define ON_COM_MSG_BASE		WM_USER + 618		//!< 消息编号的基点  
#endif

// 送到窗口的消息 WPARAM COM端口号
#define ON_COM_RECEIVE			ON_COM_MSG_BASE + 0	//!< EV_RXCHAR 
#define ON_COM_RXCHAR			ON_COM_MSG_BASE + 0 //!< EV_RXCHAR 
#define ON_COM_CTS				ON_COM_MSG_BASE + 1 //!< EV_CTS  LPARAM==1 CTS ON 
#define ON_COM_DSR				ON_COM_MSG_BASE + 2 //!< EV_DSR  LPARAM==1 DSR ON
#define ON_COM_RING				ON_COM_MSG_BASE + 3 //!< EV_RING LPARAM==1 RING ON
#define ON_COM_RLSD				ON_COM_MSG_BASE + 4 //!< EV_RLSD LPARAM==1 RLSD ON
#define ON_COM_BREAK			ON_COM_MSG_BASE + 5 //!< EV_BREAK
#define ON_COM_TXEMPTY			ON_COM_MSG_BASE + 6 //!< EV_TXEMPTY 
#define ON_COM_ERROR			ON_COM_MSG_BASE + 7 //!< EV_ERR  LPARAM Error ID 
#define ON_COM_RXFLAG			ON_COM_MSG_BASE + 8 //!< EV_RXFLAG
#define ON_COM_POWER			ON_COM_MSG_BASE + 9 //!< EV_POWER 
#define ON_COM_EVENT1			ON_COM_MSG_BASE + 10//!< EV_EVENT1
#define ON_COM_EVENT2			ON_COM_MSG_BASE + 11//!< EV_EVENT2 
#define ON_COM_RX80FULL			ON_COM_MSG_BASE + 12//!< EV_RX80FULL
#define ON_COM_PERR				ON_COM_MSG_BASE + 13//!< EV_PERR

#ifndef CN_COMM_WAIT_EVENT
	#ifdef CN_COMM_FOR_CE
		#define CN_COMM_WAIT_EVENT	EV_RXCHAR | EV_ERR | EV_CTS | EV_DSR | EV_BREAK | EV_TXEMPTY | EV_RING | EV_RLSD | EV_POWER //!< WINCE 默认的等待事件| EV_RXFLAG 
	#else
		#define CN_COMM_WAIT_EVENT	EV_RXCHAR | EV_ERR | EV_CTS | EV_DSR | EV_BREAK | EV_TXEMPTY | EV_RING | EV_RLSD 			//!< WIN32 默认的等待事件| EV_RXFLAG
	#endif
#endif

#ifndef CN_COMM_BUFFER_MIN_BLOCK_SIZE
	#define CN_COMM_BUFFER_MIN_BLOCK_SIZE 1024		//!< 定义缓冲区块的最小值 
#endif

#if CN_COMM_BUFFER_MIN_BLOCK_SIZE < 4
	#error CN_COMM_BUFFER_MIN_BLOCK_SIZE must >= 4	//!< 缓冲区块的最小值不允许小于4 
#endif

#ifndef CN_ASSERT
	#define CN_2STR(L)		_T(#L)					//!< 将表达式L转换成字符串
	#define CN_LINE(L)		CN_2STR(L)				//!< 将行号L转换成字符串
	/*! 内部断言 启用异常将抛出异常 否则调试版将退出 发行版未启用异常将不做任何处理 */
	#define CN_ASSERT(E)	((E) ? true : CnComm::Assert(_T("CN_ASSERT(")_T(#E)_T(") failed; CnComm(")CN_LINE(__LINE__)_T("); ")))
#endif

//CN_COMM_STD_EXCEPTION CN_ASSERT 将抛出标准C++异常			
#ifdef CN_COMM_STD_EXCEPTION
	#include <stdexcept> //throw runtime_error(msg)
#endif

//CN_COMM_VCL_EXCEPTION CN_ASSERT 将抛出VCL异常(C++ Builder)	
#if defined(CN_COMM_VCL_EXCEPTION) && defined(__BORLANDC__)
	#include <vcl.h> //throw new Exception(msg)	
#endif

//CN_COMM_MFC_EXCEPTION CN_ASSERT 将抛出MFC异常(VC++)			
#ifdef CN_COMM_MFC_EXCEPTION
	#include <Afx.h> //throw new MfcException(msg)	
#endif

/*! \class CnComm 
	\version 1.5 
	\date 2002.10-2009.4
	\author llbird(wushaojian@21cn.com http://www.cppblog.com/llbird  http://blog.csdn.net/wujian53) 
	\brief WIN32/WINCE C++ (ANSI/UNICODE) 多线程串口通讯基础库  
	\example doc_0.cpp 例子0 \example doc_1.cpp \example doc_2.cpp \example doc_3.cpp \example SerialDlg.cpp
 */
class CnComm    
{
public:
	//! 临界区
	struct	InnerLock;
	//! 缓冲区类
	class	BlockBuffer;	
	//! MFC异常
	class	MfcException;				
	//! 用于配置模式的枚举值, 32位掩码
	enum OptionEnum
	{
		EN_THREAD		= 0x00000001,	//!< 启用监视线程 伴随串口打开启动 WatchThread
		EN_OVERLAPPED	= 0x00000002,	//!< 启用异步重叠IO方式 
		EN_RX_BUFFER	= 0x00000004,	//!< 启用读缓冲
		EN_TX_BUFFER	= 0x00000008,	//!< 启用写缓冲
		EN_RX_THREAD	= 0x00000010,	//!< 启动读线程 暂时未用 ReadThread
		EN_TX_THREAD	= 0x00000020,	//!< 启动写线程 用于WINCE的双工操作 应同时启用写缓冲 伴随串口打开启动 WriteThread
		EN_SUSPEND		= 0x00000040,	//!< 启动线程时暂停 
		EN_ABOVE_NORMAL	= 0x00000080,	//!< 启动线程优先级高一个级别
		EN_FLUSH		= 0x00000100,	//!< 当关闭串口时输出队列未发送完的数据(端口缓冲区) 并阻塞等待                          
		EN_FLUSH_ALL	= 0x00000200	//!< 同上(包括写缓冲及端口队列) 您如果重载了写模块而又没有写好 可能导致线程挂起无法正常关闭 
	};
	//! 构造函数 配置具体应用模式 \param[in] dwOption 根据需要由OptionEnum组合而成
#ifdef CN_COMM_FOR_CE
	//! WINCE:默认打开串口时启动监视线程 启用写独立线程 启用写缓冲
	CnComm(DWORD dwOption = EN_THREAD ) 
#else 
	//! WIN32:默认打开串口时启动监视线程 异步重叠方式 
	CnComm(DWORD dwOption = EN_THREAD | EN_OVERLAPPED)
#endif
	{
		Init(); 
		SetOption(dwOption);
	}
	//! 另一模式构造 兼容cnComm1~1.3 \param[in] bThread 启动监视线程 \param[in] bOverlapped 启用重叠I/O
	CnComm(bool bThread, bool bOverlapped)
	{
		DWORD dwOption = 0;

		if (bThread)
			dwOption |= EN_THREAD;

		if (bOverlapped)
			dwOption |= EN_OVERLAPPED;

		Init(); 
		SetOption(dwOption);
	}
	//! 析构 自动关闭串口 
	virtual ~CnComm()
	{
		Close(); 
		Destroy();
	}
	//! 判断串口是或打开
	bool IsOpen()
	{
		return hComm_ != INVALID_HANDLE_VALUE;
	}
	//! 判断串口是或打开
	operator bool ()
	{
		return hComm_ != INVALID_HANDLE_VALUE;
	}
	//! 获得串口句炳
	HANDLE GetHandle()
	{
		return hComm_;
	}
	//! 获得串口句炳
	operator HANDLE()
	{
		return hComm_;
	}
	//! 获得串口序号
	DWORD GetPort()
	{
		return dwPort_;
	}
	//! 获得串口全名
	LPCTSTR GetPortName()
	{
		return szName_;
	}
	//! 获得CnComm的基本配置参数 返回32位配置掩码
	DWORD GetOption()
	{
		return dwOption_;
	}
	//! 设置CnComm的基本配置参数 在打开串口前设置有意义 \param[in] dwOption 32位配置掩码
	void SetOption(DWORD dwOption)
	{
		CN_ASSERT(!IsOpen());//! 打开状态下不可以设置参数

		dwOption_ = dwOption;

	#ifdef CN_COMM_FOR_CE
		CN_ASSERT(!IsOverlappedMode()); //! WINCE不允许使用重叠IO 即EN_OVERLAPPED掩码
		dwOption_ &= (~EN_OVERLAPPED);
	#endif
	}
	//! 修改CnComm的基本配置参数 在打开串口前设置有意义 \param[in] dwRemove 删除的32位配置掩码 \param[in] dwAdd 添加的32位配置掩码
	void ModifyOption(DWORD dwRemove, DWORD dwAdd)
	{
		CN_ASSERT(!IsOpen());//! 打开状态下不可以设置参数

		dwOption_ &= ~dwRemove;
		dwOption_ |= dwAdd;

	#ifdef CN_COMM_FOR_CE
		CN_ASSERT(!IsOverlappedMode()); //! WINCE不允许使用重叠IO 即EN_OVERLAPPED掩码
		dwOption_ &= (~EN_OVERLAPPED);
	#endif
	}
	//! 是否重叠IO模式
	bool IsOverlappedMode()
	{
		return dwOption_ & EN_OVERLAPPED ? true : false;
	}
	//! 是否输出缓冲区模式
	bool IsTxBufferMode()
	{
		return dwOption_ & EN_TX_BUFFER ? true : false;
	}
	//! 是否输入缓冲区模式
	bool IsRxBufferMode()
	{
		return dwOption_ & EN_RX_BUFFER ? true : false;
	}
	//! 关联消息的窗口句柄
	void SetWnd(HWND hWnd)
	{
		CN_ASSERT(::IsWindow(hWnd));
		hNotifyWnd_ = hWnd;
	}
	//! 关联消息的窗口句柄
	HWND GetWnd()
	{
		return hNotifyWnd_;
	}
	//! 关联消息的窗口句柄
	void SetNotifyThreadId(DWORD dwId)
	{
		hNotifyThreadId_ = dwId;
	}
	//! 关联消息的窗口句柄
	DWORD GetNotifyThreadId()
	{
		return hNotifyThreadId_;
	}
	//! 设定发送通知, 接受字符最小值
	void SetNotifyNum(DWORD dwNum)
	{
		dwNotifyNum_ = dwNum;
	}
	//! 获得线程句柄 \param[in] iOption EN_THREAD获得hWatchThread_ EN_RX_THREAD获得hReadThread_ EN_TX_THREAD获得hWriteThread_ \return HANDLE 线程句柄
	HANDLE GetThread(int iOption = EN_THREAD)
	{
		return iOption == EN_THREAD ? hWatchThread_ : ((iOption == EN_RX_THREAD) ? hReadThread_ : hWriteThread_);
	}
	//! 设置要监视的事件， 打开前设置有效
	void SetWaitEvent(DWORD dwEvent = CN_COMM_WAIT_EVENT)
	{
		CN_ASSERT(!IsOpen());///打开状态下不可以设置参数

		dwWaitEvent_ = dwEvent;
	}
	//! 输入缓冲区
	BlockBuffer& Input()
	{
		return I_;
	}
	//! 输出缓冲区
	BlockBuffer& Output()
	{
		return O_;
	}
	//! 获得输入输出的计数 \param[in] bInput 为true获得输入计数,默认为true 重叠IO下输出是不精确的 因为输出由系统后台控制 假设完全输出 
	DWORD GetCounter(bool bInput = true)
	{
		return bInput ? dwInCount_ : dwOutCount_;
	}
	//! 输入输出计数器清零
	void ResetCounter()
	{
		dwInCount_ = dwOutCount_ = 0;
	}
	//! 打开串口 请注意与cnComm1~1.3的区别 cnComm1~1.3将使用9600, n, 8, 1配置端口 而1.5将只打开端口不配置波特率等参数  \param[in] dwPort 串口序号 1~1024
	bool Open(DWORD dwPort)
	{
		if (!CN_ASSERT(dwPort>=1 && dwPort<=1024))
			return false;
		
		BindPort(dwPort);
		
		if(!CN_ASSERT(OpenPort()))
			return false;
		
		if(!CN_ASSERT(SetupPort()))
			return Close(), false;

		if ((dwOption_ & (EN_THREAD|EN_RX_THREAD|EN_TX_THREAD)) && !CN_ASSERT(BeginThread()))
			return Close(), false;

		return true;
	}
	/*! \param[in] dwPort 串口序号 1~1024 \param[in] dwBaudRate 波特率 \param[in] btParity  奇偶校验 \param[in] btByteSize 数据位数	\param[in] btStopBits 停止位数	*/
	//! 打开串口 缺省 9600, n, 8, 1 \sa bool Open(DWORD dwPort, LPCTSTR szSetStr)
	bool Open(DWORD dwPort, DWORD dwBaudRate, BYTE btParity = NOPARITY, BYTE btByteSize = 8, BYTE btStopBits = ONESTOPBIT)
	{
		if (!CN_ASSERT(dwPort>=1 && dwPort<=1024))
			return false;
		
		BindPort(dwPort);
		
		if (!CN_ASSERT(OpenPort()))
			return false;
		
		if (!CN_ASSERT(SetState(dwBaudRate, btParity, btByteSize, btStopBits)))
			return Close(), false;

		if (!CN_ASSERT(SetupPort()))
			return Close(), false;

		if ((dwOption_ & (EN_THREAD|EN_RX_THREAD|EN_TX_THREAD)) && !CN_ASSERT(BeginThread()))
			return Close(), false;

		return true;
	}
#ifndef CN_COMM_FOR_CE
	/*! \param[in] dwPort 串口序号 1~1024 \param[in] szSetStr 字符串参数 "BBBB,P,D,S"  例: "9600,N,8,1"  "1200,O,7,2" 
		 BBBB为波特率 P为奇偶校验(E | M | N | O | S) D为数据位数(4 ~ 8) S为停止位数(1 | 1.5 | 2)
		 \code Open(1, "9600,E,8,2"); \endcode  \b 请注意字符串参数的顺序 并检查返回值
		 不支持WINCE, 原本我自己写了一个替代函数, 考虑到WINCE硬件的复杂性, 移植性可能不好, 故从这个版本中删除了	*/
	//! 打开串口, 字符串设置串口
	bool Open(DWORD dwPort, LPCTSTR szSetStr)
	{
		if (!CN_ASSERT(dwPort>=1 && dwPort<=1024))
			return false;
		
		BindPort(dwPort);
		
		if (!CN_ASSERT(OpenPort()))
			return false;
		
		if (!CN_ASSERT(SetState(szSetStr))) 
			return Close(), false;

		if (!CN_ASSERT(SetupPort()))
			return Close(), false;

		if ((dwOption_ & (EN_THREAD|EN_RX_THREAD|EN_TX_THREAD)) && !CN_ASSERT(BeginThread()))
			return Close(), false;
		
		return true;
	}
#endif
	/*! \param[in] dwPort 此时该参数没有具体意义，仅用于消息通知中辨别端口号，应注意不和其他端口号重复 
		\param[in] szPortName 为指定的端口名称, 就像WINDOWS下\\\\.\\COM1或者WINCE下COM1: , 有些虚拟串口可以有特殊的名称
		\param[in] dwBaudRate 波特率	\param[in] btParity 奇偶校验	\param[in] btByteSize 数据位数	\param[in] btStopBits 停止位数
		\code Open(9999, "COM3:", 2400); \endcode 	\code Open(1028, "COM3:", 9600, N, 7, ONESTOPBIT); \endcode	*/
	//! 指定的端口名称打开串口
	bool Open(DWORD dwPort, LPCTSTR szPortName, DWORD dwBaudRate, BYTE btParity = NOPARITY, BYTE btByteSize = 8, BYTE btStopBits = ONESTOPBIT)
	{
		if(!CN_ASSERT(_tcslen(szPortName) < 64 - 1))
			return false;
		
		_tcscpy(szName_, szPortName);
		dwPort_ = dwPort;//用于通知消息
		
		if (!CN_ASSERT(OpenPort()))
			return false;
		
		if (!CN_ASSERT(SetState(dwBaudRate, btParity, btByteSize, btStopBits)))
			return Close(), false;

		if (!CN_ASSERT(SetupPort()))
			return Close(), false;

		if ((dwOption_ & (EN_THREAD|EN_RX_THREAD|EN_TX_THREAD)) && !CN_ASSERT(BeginThread()))
			return Close(), false;
		
		return true;
	}
	//! 直接读物理端口 \param[out] pBuffer 目标缓冲区 \param[in] dwLength 读取长度 \param[in] dwWaitTime 等待时间(默认INFINITE) \return 返回实际读取字节数
	DWORD ReadPort(LPVOID pBuffer, DWORD dwLength, DWORD dwWaitTime = INFINITE)
	{
		COMSTAT Stat;
		DWORD	dwError, dwReadResult = 0;
		
		if(!CN_ASSERT(IsOpen()) || !dwLength)
			return 0;
		
		if(::ClearCommError(hComm_, &dwError, &Stat) && dwError > 0)
			::PurgeComm(hComm_, PURGE_RXABORT);

	#ifndef CN_COMM_FOR_CE
		if (IsOverlappedMode())//! 重叠I/O下 dwWaitTime==INFINITE 将等待到自然超时或任务完成 具体时间由超时结构决定 默认设定0.5秒
		{
			if (dwWaitTime == INFINITE)//! 如果你没有启用读反冲 直接读取端口将导致阻塞 相当于原来1.3版本 Read(pBuffer, dwLength, false);
			{
				if (!::ReadFile(hComm_, pBuffer, dwLength, &dwReadResult, &RO_))
				{//! 在界面里使用CnComm推荐使用读缓冲区或者在dwWaitTime指定超时时间
					if (CN_ASSERT(::GetLastError() == ERROR_IO_PENDING))
					{
						while (!CN_ASSERT(::GetOverlappedResult(hComm_, &RO_, &dwReadResult, TRUE)))
							;
					}
				}
			}
			else//! 重叠I/O下 dwWaitTime!=INFINITE 将等待到超时或任务完成 具体时间由dwWaitTime决定 
			{	//! dwWaitTime = 0 相当于原来1.3版本的功能
				DWORD dwBegin = GetTickCount(), dwEnd, dwCost, uReadLength, uReadReturn;
				
				uReadLength = Stat.cbInQue > dwLength ? dwLength : Stat.cbInQue;
				CN_ASSERT(::ReadFile(hComm_, pBuffer, uReadLength, &uReadReturn, &RO_));
				dwReadResult += uReadReturn;

				do
				{
					if (!::ReadFile(hComm_, (LPBYTE)pBuffer + dwReadResult, 1, &uReadReturn, &RO_))
					{
						if (dwWaitTime > 5 && WaitForSingleObject(RO_.hEvent, dwWaitTime) == WAIT_OBJECT_0)
						{
							dwEnd = GetTickCount();
							dwCost = dwEnd>=dwBegin ? dwEnd-dwBegin : DWORD(-1L)-dwBegin+dwEnd;
							CN_ASSERT(::GetOverlappedResult(hComm_, &RO_, &uReadReturn, FALSE));
							dwWaitTime = dwWaitTime > dwCost ? dwWaitTime-dwCost : 0;
						} 
						else
						{
							CN_ASSERT(::PurgeComm(hComm_, PURGE_RXABORT));
							break;
						}
					}
				}
				while (uReadReturn && ++dwReadResult < dwLength);
			}
			return dwInCount_ += dwReadResult, dwReadResult;
		}
	#endif
		//! 阻塞I/O和WinCE的I/O下 dwWaitTime无意义 超时时间由超时结构决定 默认设定1/4秒
		CN_ASSERT(::ReadFile(hComm_, pBuffer, dwLength, &dwReadResult, NULL));
		return dwInCount_ += dwReadResult, dwReadResult;
	}
	//! 读取串口 dwLength个字符到 pBuffer 返回实际读到的字符数  可读任意数据
	DWORD Read(LPVOID pBuffer, DWORD dwLength, DWORD dwWaitTime = INFINITE)
	{
		CN_ASSERT(pBuffer);

		if (dwOption_ & EN_RX_BUFFER)
		{
			BlockBuffer::InnerLock locker(&I_);
			return I_.Read(pBuffer, dwLength);
		}

	#ifdef CN_COMM_FOR_CE
		return ReadPort(pBuffer, dwLength, dwWaitTime);
	#else
		return ReadPort(pBuffer, dwLength, dwWaitTime);
	#endif
	}
	//! 读取串口 dwLength - 1 个ANSI字符到 szBuffer 返回 C 模式字符串指针 适合一般字符通讯
	char * ReadString(char *szBuffer, DWORD dwLength, DWORD dwWaitTime = INFINITE)
	{
		CN_ASSERT(szBuffer);
		szBuffer[Read(szBuffer, dwLength - 1, dwWaitTime)] = '\0';
		return szBuffer;
	}
	//! 读取串口 dwLength - 1 个UNICODE字符到 szBuffer 返回 C 模式字符串指针 适合一般字符通讯
	wchar_t * ReadString(wchar_t *szBuffer, DWORD dwLength, DWORD dwWaitTime = INFINITE)
	{
		CN_ASSERT(szBuffer);
		szBuffer[(Read(szBuffer, (dwLength - 1)*sizeof(wchar_t), dwWaitTime) +1)/ sizeof(wchar_t)] = L'\0';
		return szBuffer;
	}
	//! 直接写入端口
	DWORD WritePort(LPCVOID pBuffer, DWORD dwLength)
	{
		if(!CN_ASSERT(IsOpen()) || !dwLength)
			return 0;

		DWORD dwError;
		unsigned long uWriteLength = 0;

		if(::ClearCommError(hComm_, &dwError, NULL) && dwError > 0) 
			::PurgeComm(hComm_, PURGE_TXABORT); 

	#ifndef CN_COMM_FOR_CE
		if (IsOverlappedMode())//! 使用重叠IO是返回0，写出计数直接加上待写入长度，即假设写入成功
		{//! 重叠IO下超时由超时结构控制 默认为长度*1毫秒+10秒
			if(!::WriteFile(hComm_, pBuffer, dwLength, &uWriteLength, &WO_) 
				&& !CN_ASSERT(::GetLastError() == ERROR_IO_PENDING))
				uWriteLength = 0;
			else
				dwOutCount_ += dwLength;
		}
		else
	#endif //! 使用阻塞IO或WINCE下是返回实际写入长度，写出计数直接加上实际写入长度, 超时默认1/4秒
			::WriteFile(hComm_, pBuffer, dwLength, &uWriteLength, NULL), dwOutCount_ += uWriteLength;

		return uWriteLength;
	}
	//! 写 EN_TX_BUFFER 决定写入缓冲区还是直接写入端口
	DWORD Write(LPCVOID pBuffer, DWORD dwLength)
	{
		if (dwLength && (dwOption_&EN_TX_BUFFER))
		{
			BlockBuffer::InnerLock locker(&O_);

			O_.Write(pBuffer, dwLength);

			if (dwOption_ & EN_TX_THREAD)
				SetEvent(hWatchEvent_);
			else
			{
			#ifdef CN_COMM_FOR_CE
				SetCommMask(hComm_, dwWaitEvent_);
			#else
				if (IsOverlappedMode())
					SetCommMask(hComm_, dwWaitEvent_);
				else
					SetEvent(hWatchEvent_);
			#endif
			}
			return 0;
		}
			
	#ifdef CN_COMM_FOR_CE
		return WritePort(pBuffer, dwLength);
	#else
		return WritePort(pBuffer, dwLength);
	#endif
	}
	//! 写串口 ANSI字符 写ANSI C 模式字符串指针 
	DWORD Write(const char *szBuffer)
	{
		CN_ASSERT(szBuffer);
		
		return Write((LPCVOID)szBuffer, strlen(szBuffer));
	}
	//! 写串口 UNICODE字符 写ANSI C 模式字符串指针 
	DWORD Write(const wchar_t *szBuffer)
	{
		CN_ASSERT(szBuffer);
		
		return Write((LPCVOID)szBuffer, wcslen(szBuffer)*sizeof(wchar_t));
	}
	//! 写串口 szBuffer 可以输出格式字符串 包含缓冲区长度
	DWORD Write(char *szBuffer, DWORD dwLength, char * szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);
		_vsnprintf(szBuffer, dwLength, szFormat, va);
		va_end(va);
		
		return Write(szBuffer);
	}
	//! 写串口 UNICODE szBuffer 可以输出格式字符串 包含缓冲区长度
	DWORD Write(wchar_t *szBuffer, DWORD dwLength, wchar_t * szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);
		_vsnwprintf(szBuffer, dwLength, szFormat, va);
		va_end(va);
		
		return Write(szBuffer);
	}
	//! 写串口 szBuffer 可以输出格式字符串 不检查缓冲区长度 小心溢出
	DWORD Write(char *szBuffer, char * szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);
		vsprintf(szBuffer, szFormat, va);
		va_end(va);
		
		return Write(szBuffer);
	}
	//! 写串口 szBuffer 可以输出格式字符串 不检查缓冲区长度 小心溢出
	DWORD Write(wchar_t *szBuffer, wchar_t * szFormat, ...)
	{
		va_list va;
		va_start(va, szFormat);
		vswprintf(szBuffer, szFormat, va);
		va_end(va);
		
		return Write(szBuffer);
	}
	//! 强制输出队列内数据并等待
	void FlushPort()
	{
		if(CN_ASSERT(IsOpen()))
			FlushFileBuffers(hComm_);
	}
	//! 强制输出写缓冲区并强制输出队列内数据并等待
	void Flush()
	{
		if(CN_ASSERT(IsOpen()))
		{
			if (dwOption_ & EN_TX_BUFFER)
			{
				while(O_.SafeSize())
					Sleep(50);
			}

			FlushFileBuffers(hComm_);
		}
	}
	//! 抢先输出字符 
	bool TransmitChar(char c)
	{
		if (CN_ASSERT(IsOpen()))
		{
			if (::TransmitCommChar(hComm_, c))
				return dwOutCount_++, true;
		}

		return false;
	}
	//! 启动辅助线程控制 
	bool BeginThread(DWORD dwThreadOption = 0) 
	{
		DWORD dwCreationFlags;

		if(CN_ASSERT(!hWatchThread_ && !hReadThread_ && !hWriteThread_)) 
		{//! 必须是没有线程在运行状态下才可以启动线程
			bContinue_		= true;
			dwCreationFlags	= dwOption_ & EN_SUSPEND ? CREATE_SUSPENDED : 0;

			if (dwThreadOption)//! 根据配置创建监视线程，独立读或写线程
				dwOption_ |= dwThreadOption;
			
		#if defined(_MT) && !defined(CN_COMM_FOR_CE)
			unsigned int id, rid, wid;

			if (dwOption_ & EN_THREAD)
				hWatchThread_ = (HANDLE)_beginthreadex(NULL, 0, WatchThreadProc, this, dwCreationFlags, &id);

			if (dwOption_ & EN_RX_THREAD)
				hReadThread_ = (HANDLE)_beginthreadex(NULL, 0, ReadThreadProc, this, dwCreationFlags, &rid);

			if (dwOption_ & EN_TX_THREAD)
				hWriteThread_ = (HANDLE)_beginthreadex(NULL, 0, WriteThreadProc, this, dwCreationFlags, &wid);
		#else
			DWORD id, rid, wid;

			if (dwOption_ & EN_THREAD)
				hWatchThread_ = ::CreateThread(NULL, 0, WatchThreadProc, this, dwCreationFlags , &id); 

			if (dwOption_ & EN_RX_THREAD)
				hReadThread_ = ::CreateThread(NULL, 0, ReadThreadProc, this, dwCreationFlags, &rid); 

			if (dwOption_ & EN_TX_THREAD)
				hWriteThread_ = ::CreateThread(NULL, 0, WriteThreadProc, this, dwCreationFlags, &wid); 
		#endif

			if (dwOption_ & EN_THREAD)
			{
				CN_ASSERT(hWatchThread_ != NULL);
				hWatchThreadId_ = id;

				if (!hWatchThread_)
				{
					EndThread();
					return false;
				}
				else
				{
					if (dwOption_ & EN_ABOVE_NORMAL)
						SetThreadPriority(hWatchThread_, THREAD_PRIORITY_ABOVE_NORMAL);
				}
			}

			if (dwOption_ & EN_RX_THREAD)
			{
				CN_ASSERT(hReadThread_ != NULL);
				hReadThreadId_	= rid;

				if (!hReadThreadId_)
				{
					EndThread();
					return false;
				}
				else
				{
					if (dwOption_ & EN_ABOVE_NORMAL)
						SetThreadPriority(hReadThread_, THREAD_PRIORITY_ABOVE_NORMAL);
				}
			}

			if (dwOption_ & EN_TX_THREAD)
			{
				CN_ASSERT(hWriteThread_ != NULL);
				hWriteThreadId_	= wid;

				if (!hWriteThreadId_)
				{
					EndThread();
					return false;
				}
				else
				{
					if (dwOption_ & EN_ABOVE_NORMAL)
						SetThreadPriority(hWriteThread_, THREAD_PRIORITY_ABOVE_NORMAL);
				}
			}

			return true; 
		}

		return false;
	}
	//! 暂停线程
	bool SuspendThread(int iOption = EN_THREAD)
	{
		return ::SuspendThread(GetThread(iOption)) != 0xFFFFFFFF;
	}
	//! 恢复线程
	bool ResumeThread(int iOption = EN_THREAD)
	{
		return ::ResumeThread(GetThread(iOption)) != 0xFFFFFFFF;
	}
	//! 终止线程
	bool EndThread(DWORD dwWaitTime = 500)
	{
		if(hWatchThread_ || hReadThread_ || hWriteThread_) 
		{
			if ((dwOption_&EN_FLUSH_ALL) && (dwOption_&EN_TX_BUFFER))
			{//! 如果启用EN_FLUSH_ALL，将循环等待写缓冲区清空，如果写入线程不能正常工作，将挂起
				while(O_.Size())
					Sleep(50);
			}

			bContinue_ = false;

		#ifdef CN_COMM_FOR_CE
			::SetCommMask(hComm_, 0);
		#else
			if (IsOverlappedMode())
				::SetCommMask(hComm_, 0);
		#endif

			if (hWatchThread_)
			{
				SetEvent(hWatchEvent_);
				if(::WaitForSingleObject(hWatchThread_, dwWaitTime) != WAIT_OBJECT_0)
					if(!::TerminateThread(hWatchThread_, 0))
						return false;

				::CloseHandle(hWatchThread_);
				hWatchThread_ = NULL;
			}

			if (hReadThread_)
			{
				SetEvent(hReadEvent_);
				if(::WaitForSingleObject(hReadThread_, dwWaitTime) != WAIT_OBJECT_0)
					if(!::TerminateThread(hReadThread_, 0))
						return false;

				::CloseHandle(hReadThread_);
				hReadThread_ = NULL;
			}
			
			if (hWriteThread_)
			{
				SetEvent(hWriteEvent_);
				if(::WaitForSingleObject(hWriteThread_, dwWaitTime) != WAIT_OBJECT_0)
					if(!::TerminateThread(hWriteThread_, 0))
						return false;

				::CloseHandle(hWriteThread_);
				hWriteThread_ = NULL;
			}
				
			return true;
		}

		return false;
	}
	//! 关闭串口 同时也关闭关联线程
	virtual void Close(DWORD dwWaitTime = 500)
	{
		if(IsOpen())  
		{
			EndThread(dwWaitTime);//! 同步结束线程

			if (dwOption_&EN_FLUSH || dwOption_&EN_FLUSH_ALL)
				FlushFileBuffers(hComm_);

			::PurgeComm(hComm_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT); 
			::CloseHandle(hComm_);
			hComm_ = INVALID_HANDLE_VALUE;
		}
	}
	//! 锁定
	void Lock()							
	{	
		::EnterCriticalSection(&CS_);		
	}
	//! 解锁
	void Unlock()						
	{	
		::LeaveCriticalSection(&CS_);		
	}
	//! 自动锁 用于函数内部 利用对象的生命周期完成锁定及解锁
	struct InnerLock
	{
		CnComm* ptr;//!< CnComm 对象指针
		//! 锁定
		InnerLock(CnComm* p) : ptr(p)	
		{	
			ptr->Lock();						
		}
		//! 解锁
		~InnerLock()					
		{	
			ptr->Unlock();						
		}
	};
	//! 获得串口参数 DCB
	DCB *GetState(DCB *pDcb = NULL)
	{
		return CN_ASSERT(IsOpen()) && ::GetCommState(hComm_, pDcb == NULL ? &DCB_ : pDcb) == TRUE ? (pDcb == NULL ? &DCB_ : pDcb) : NULL;
	}
	//! 设置串口参数 DCB
	bool SetState(DCB *pDcb = NULL)
	{
		return CN_ASSERT(IsOpen()) ? ::SetCommState(hComm_, pDcb == NULL ? &DCB_ : pDcb) == TRUE : false;
	}
	//! 设置串口参数：波特率，停止位，等 ***
	bool SetState(DWORD dwBaudRate, BYTE btParity = NOPARITY, BYTE btByteSize = 8, BYTE btStopBits = ONESTOPBIT)
	{
		if(CN_ASSERT(IsOpen()))
		{
			if(::GetCommState(hComm_, &DCB_) != TRUE)
				return false;

			DCB_.BaudRate = dwBaudRate;
			DCB_.ByteSize = btByteSize;
			DCB_.Parity   = btParity;
			DCB_.StopBits = btStopBits;
			DCB_.fParity  = (btParity != NOPARITY);

			return ::SetCommState(hComm_, &DCB_) == TRUE;
		}
		return false;
	}
#ifndef CN_COMM_FOR_CE
	//! 设置串口参数 支持设置字符串 "9600, n, 8, 1"
	bool SetState(LPCTSTR szSetStr) 
	{
		if(CN_ASSERT(IsOpen()))
		{
			if(!::GetCommState(hComm_, &DCB_))
				return false;

			if(!BuildCommDCB(szSetStr, &DCB_))
				return false;

			DCB_.fParity  = (DCB_.Parity != NOPARITY);
			return ::SetCommState(hComm_, &DCB_) == TRUE;
		}

		return false;
	}
#endif
	//! 获得超时结构
	LPCOMMTIMEOUTS GetTimeouts(LPCOMMTIMEOUTS lpCO = NULL)
	{
		return CN_ASSERT(IsOpen()) && ::GetCommTimeouts(hComm_, lpCO ? lpCO : &CO_) == TRUE  ? (lpCO ? lpCO : &CO_) : NULL;
	}
	//! 设置超时
	bool SetTimeouts(LPCOMMTIMEOUTS lpCO = NULL)
	{
		return CN_ASSERT(IsOpen()) ? ::SetCommTimeouts(hComm_, lpCO ? lpCO : &CO_) == TRUE : false;
	}
	//! 设置串口的I/O缓冲区大小
	bool Setup(DWORD dwInputSize = 4096, DWORD dwOutputSize = 4096)
	{
		return CN_ASSERT(IsOpen()) ? ::SetupComm(hComm_, dwInputSize, dwOutputSize) == TRUE : false; 
	}
	//! 调整端口功能
	bool Escape(DWORD dwType)
	{
		return CN_ASSERT(IsOpen()) ? EscapeCommFunction(hComm_, dwType) != 0 : false;
	}
	//! 获得调制解调器相关信号状态
	DWORD GetModemStatus()
	{
		DWORD dwModemStat = 0;
		return CN_ASSERT(IsOpen()) && GetCommModemStatus(hComm_, &dwModemStat) ? dwModemStat : 0;
	}
	//! 获得端口参数 \param[in] pCP 结构指针 如果pCP==NULL, CnComm将从堆分配内存, 并由CnComm负责释放, 用户不需要自己释放内存
	LPCOMMPROP GetProperties(LPCOMMPROP pCP = NULL)	
	{
		if (CN_ASSERT(IsOpen()))
		{
			if (!pCP)
			{
			#ifdef CN_COMM_FOR_CE
				USHORT dwSize = sizeof(COMMPROP);
			#else
				USHORT dwSize = sizeof(COMMPROP) + sizeof(MODEMDEVCAPS);
			#endif

				if (!pCP_)
					pCP_ = (LPCOMMPROP) new BYTE[dwSize];

				if (pCP_)
				{
					memset(pCP_, 0, dwSize);

					pCP_->wPacketLength = dwSize;
				#ifndef CN_COMM_FOR_CE
					pCP_->dwProvSubType = PST_MODEM;
				#endif
					pCP_->dwProvSpec1	= COMMPROP_INITIALIZED;
					pCP = pCP_;
				}
			}
		}

		return pCP && GetCommProperties(hComm_, pCP) ? pCP : NULL;
	}
	//! 获取事件标识
	DWORD GetMask()
	{
		DWORD dwMask;
		return CN_ASSERT(IsOpen()) && GetCommMask(hComm_, &dwMask) ? dwMask : 0;	
	}
	//! 清除端口缓冲区
	bool Purge(DWORD dwPara = PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR)
	{
		return CN_ASSERT(IsOpen()) ? ::PurgeComm(hComm_, dwPara)==TRUE : false;
	}
	//! 获得错误代码
	DWORD ClearError()
	{
		DWORD dwError;
		return CN_ASSERT(IsOpen()) && ::ClearCommError(hComm_, &dwError, NULL) ? dwError : 0;
	}
	//! 获得读物理缓冲区的字符数
	DWORD GetQueueCount(bool bInput = true)
	{
		COMSTAT Stat;
		DWORD dwError;
		
		return CN_ASSERT(IsOpen()) && ::ClearCommError(hComm_, &dwError, &Stat) ? (bInput ? Stat.cbInQue : Stat.cbOutQue) : (DWORD)-1L;
	}
	//! 调制解调器 CTS ON
	bool CheckCTS()				
	{	
		return (GetModemStatus()&MS_CTS_ON) != 0;		
	}
	//! 调制解调器 DSR ON
	bool CheckDSR()				
	{	
		return (GetModemStatus()&MS_DSR_ON) != 0;		
	}
	//! 调制解调器 Ring ON
	bool CheckRING()				
	{	
		return (GetModemStatus()&MS_RING_ON) != 0;		
	}
	//! 调制解调器 RLSD ON
	bool CheckRLSD()				
	{	
		return (GetModemStatus()&MS_RLSD_ON) != 0;		
	}
	//! DTR 电平控制
	bool SetDTR(bool bSetOrClr = true)	
	{	
		return Escape(bSetOrClr ? SETDTR : CLRDTR);		
	}
	//! RTS 电平控制
	bool SetRTS(bool bSetOrClr = true)	
	{	
		return Escape(bSetOrClr ? SETRTS : CLRRTS);		
	}
	//! 调制解调器
	bool SetBreak(bool bSetOrClr = true)	
	{	
		return Escape(bSetOrClr ? SETBREAK : CLRBREAK);	
	}
#ifdef CN_COMM_FOR_CE
	//! WINCE
	bool SetIR(bool bSetOrClr)	
	{	
		return Escape(bSetOrClr ? SETIR : CLRIR);	
	}
#endif
	//! 流控制 SETXON SETXOFF
	bool SetX(bool bOnOrOff)				
	{	
		return Escape(bOnOrOff ? SETXON : SETXOFF);							
	}
	//! 根据通讯特点开发的缓冲区类 单向链表内存块 有一些扩展以支持和API挂接 
	class BlockBuffer
	{
	public:
		//! 缓冲区内存块
		struct Block
		{
			DWORD			B_;		//!< 开始偏移
			DWORD			E_;		//!< 结束偏移
			DWORD			S_;		//!< 块大小 内存块最大值不限 内存块最小值由CN_COMM_BUFFER_MIN_BLOCK_SIZE决定 
			Block*			N_;		//!< 下一个块指针 
			BYTE			P_[4];	//!< 缓冲区指针 实际大小由S_决定 
		
			//! 容量
			DWORD Capacity()		{	return S_;			}
			//! 实际大小
			DWORD Size()			{	return E_ - B_;		}
			//! 开始缓冲区指针
			BYTE* Begin()			{	return P_ + B_;		}
			//! 末端缓冲区指针
			BYTE* End()				{	return P_ + E_;		}
			//! 下一个块
			Block* Next()			{	return N_;			}
			//! 是否空
			bool IsEmpty()			{	return B_ == E_;	}
			//! 空闲大小
			DWORD FreeSize()		{	return S_ - E_;		}
		};

		//! 用于缓冲区单向迭代子
		struct Iterator
		{
			BlockBuffer*	P_;		//!< 对象指针
			Block*			B_;		//!< 当前块指针
			DWORD			I_;		//!< 当前块偏移
			DWORD			O_;		//!< 全缓冲区偏移

			//! 构造
			Iterator(BlockBuffer *P = NULL, Block* B = NULL, DWORD I = 0, DWORD O = 0) 
				: P_(P), B_(B), I_(I), O_(O)	
			{										
			}
			//! 是否有效
			operator bool()				
			{	
				return B_ && I_<B_->E_&&I_>=B_->B_;	
			}
			//! 是否可写
			bool CanWrite()				
			{	
				return B_ && I_ < B_->S_;			
			}
			//! 取值引用
			BYTE& operator *()			
			{	
				CN_ASSERT( P_ && B_ && I_<B_->E_&&I_>=B_->B_);
				return B_->P_[I_];					
			}
			//! 移动迭代子 ++Iter;
			Iterator& operator ++ ()		
			{
				return operator +=(1);			
			}
			//! 移动迭代子 Iter++;
			Iterator& operator ++ (int)	
			{
				return operator +=(1);				
			}
			//! 移动迭代子  \param dwOffset 为偏移量
			Iterator& operator += (DWORD dwOffset)
			{
				while (dwOffset) 
				{
					if (I_+dwOffset < B_->E_)
						I_ += dwOffset, O_ += dwOffset, dwOffset = 0; 
					else
						dwOffset -= B_->E_-I_, I_ += B_->E_-I_, O_ += B_->E_-I_, B_ = B_->N_, I_ = 0;
				}

				return *this;
			}
			//! 比较
			bool operator == (const Iterator& iter)
			{
				return (P_ == iter.P_) && (B_ == iter.B_) && (I_ == iter.I_);
			}
		};
		//! 友元
		friend struct Iterator;
		//! 锁定
		void Lock()							
		{	
			::EnterCriticalSection(&C_);			
		}
		//! 解锁
		void Unlock()						
		{	
			::LeaveCriticalSection(&C_);			
		}
		//! 自动锁
		struct InnerLock
		{
			BlockBuffer* ptr;//!<对象指针
			///锁定
			InnerLock(BlockBuffer* p) : ptr(p)	
			{
				if (ptr)
					ptr->Lock();						
			}
			///解锁
			~InnerLock()					
			{
				if (ptr)
					ptr->Unlock();						
			}
		};
		//! 构造
		BlockBuffer()
		{
			::InitializeCriticalSection(&C_);
			S_ = 0, F_ = L_ = NULL, M_ = CN_COMM_BUFFER_MIN_BLOCK_SIZE;
		}
		//! 析构自动释放空间
		virtual ~BlockBuffer()
		{
			Clear();
			::DeleteCriticalSection(&C_);
		}
		//! 获得起始迭代子
		Iterator Begin()					
		{	
			return Iterator(this, F_, F_? F_->B_ : 0, 0);	
		}
		//! 设置块的最小长度
		void SetMinBlockSize(DWORD dwMinSize)
		{
			M_ = dwMinSize;
		}
		//! 获得块的最小长度
		DWORD GetMinBlockSize()
		{
			return M_;
		}
		//! 缓冲区内数据字节数
		DWORD Size()						
		{	
			return S_;								
		}
		//! 缓冲区大小
		DWORD SafeSize()						
		{
			InnerLock lock(this);
			return S_;								
		}
		//! 写入ANSI字符串缓冲区 \sa Write(LPCVOID lpBuf, DWORD dwSize)
		DWORD Write(const char* lpBuf)
		{
			return Write(lpBuf, strlen(lpBuf));
		}
		//! 写入UNICODE字符串缓冲区 \sa Write(LPCVOID lpBuf, DWORD dwSize)
		DWORD Write(const wchar_t* lpBuf)
		{
			return Write(lpBuf, wcslen(lpBuf)*sizeof(wchar_t));
		}
		//! 写入缓冲区 \param[out] lpBuf 目标缓冲区 \param[in] dwSize 数据字节数 \return 实际复制数据字节数
		DWORD Write(LPCVOID lpBuf, DWORD dwSize)
		{
			DWORD dwTemp = dwSize, dwFree = FreeSize(), dwCopy = 0;
			
			if (dwFree)//! 首先查找末尾空闲，并写入数据
			{
				dwCopy = dwFree > dwSize ? dwSize : dwFree;
				memcpy(L_->P_ + L_->E_, lpBuf, dwCopy);
				dwTemp -= dwCopy, L_->E_ += dwCopy;
			}

			if (dwTemp)//! 剩余的数据分配新的空间并写入
			{
				memcpy(NewBlock(dwSize)->P_, ((LPBYTE)lpBuf )+ dwCopy, dwTemp);
				L_->E_ += dwTemp;
			}

			S_ += dwSize;
			return dwSize;
		}
		//! 线程安全写入缓冲区 \sa Write(LPCVOID lpBuf, DWORD dwSize)
		DWORD SafeWrite(LPCVOID lpBuf, DWORD dwSize)
		{
			InnerLock lock(this);
			return Write(lpBuf, dwSize);
		}
		//! 线程安全写入ANSI字符串缓冲区 \sa Write(LPCVOID lpBuf, DWORD dwSize)
		DWORD SafeWrite(const char* lpBuf)
		{
			InnerLock lock(this);
			return Write(lpBuf, strlen(lpBuf));
		}
		//! 线程安全写入UNICODE字符串缓冲区 \sa Write(LPCVOID lpBuf, DWORD dwSize)
		DWORD SafeWrite(const wchar_t* lpBuf)
		{
			InnerLock lock(this);
			return Write(lpBuf, wcslen(lpBuf)*sizeof(wchar_t));
		}
		//! 复制数据 \param[out] lpBuf 目标缓冲区 \param[in] dwSize 数据字节数 \param[in] dwStart 源缓冲区开始偏移值 \return 实际复制数据字节数
		DWORD Copy(LPVOID lpBuf, DWORD dwSize, DWORD dwStart = 0)
		{
			CN_ASSERT(lpBuf);

			if (!F_ || !dwSize || dwStart >= S_)
				return 0;

			DWORD dwTemp, dwIndex;
			Block * pCur = F_, *pNext = F_->N_;

			for (dwTemp = 0, dwIndex = pCur->B_; dwTemp < dwStart; pCur = pCur->N_, dwIndex = pCur ? pCur->B_ : 0)
			{
				if (dwStart - dwTemp < pCur->E_ - pCur->B_)
				{
					dwIndex = pCur->B_ + dwStart - dwTemp;
					break;
				}

				dwTemp += pCur->E_ - pCur->B_;
			}

			for (dwTemp = 0; dwTemp < dwSize && pCur; pCur = pCur->N_, dwIndex = pCur ? pCur->B_ : 0)
			{
				if (dwSize - dwTemp < pCur->E_ - dwIndex)
				{
					memcpy((LPBYTE)lpBuf+dwTemp, pCur->P_ + dwIndex, dwSize - dwTemp );
					dwTemp = dwSize;
					break;
				}
				else
				{
					memcpy((LPBYTE)lpBuf+dwTemp, pCur->P_ + dwIndex, pCur->E_ - dwIndex);
					dwTemp += pCur->E_ - dwIndex;
				}
			}

			return dwTemp;
		}
		//! 线程安全复制数据 \sa Copy(LPVOID lpBuf, DWORD dwSize, DWORD dwStart=0)
		DWORD SafeCopy(LPVOID lpBuf, DWORD dwSize, DWORD dwStart = 0)
		{
			InnerLock lock(this);
			return Copy(lpBuf, dwSize, dwStart);
		}
		/*! \param[out] lpBuf 目标缓冲区 为NULL时仅删除不复制数据  \param[in] dwSize 操作数据字节数 \return 实际复制或删除字节数 */
		//! 复制并删除缓冲区的数据到lpBuf
		DWORD Read(LPVOID lpBuf, DWORD dwSize)
		{
			DWORD dwTemp = 0, dwCopy;

			for (Block * pCur = F_, *pNext = NULL; dwTemp<dwSize && pCur; pCur = pNext)
			{
				if (dwSize-dwTemp > pCur->E_-pCur->B_ )
					dwCopy = pCur->E_ - pCur->B_; 
				else
					dwCopy = dwSize - dwTemp;

				if (lpBuf) 
					memcpy((LPBYTE)lpBuf+dwTemp, pCur->P_ + pCur->B_, dwCopy);

				pNext = pCur->N_, dwTemp += dwCopy;

				if (dwCopy == pCur->E_-pCur->B_)
				{//! 删除回收完全空闲块, 当一般保留1个块, 在小于4*CN_COMM_BUFFER_MIN_BLOCK_SIZE的情况下 
					if (pNext || pCur->S_>(M_<<2))
					{
						delete[] (BYTE*)pCur;
						F_ = pNext;
						if (!F_)
							L_ = NULL;
					}
					else
						pCur->B_ = pCur->E_ = 0;
				}
				else
					pCur->B_ += dwCopy;

				S_ -= dwCopy;
			}

			return dwTemp;
		}
		//! 读入ANSI字符串缓冲区
		char* ReadString(char* lpBuf, DWORD dMaxSize)
		{
			lpBuf[Read(lpBuf, dMaxSize)] = '\0';
			return lpBuf;
		}
		//! 读入UNICODE字符串缓冲区
		wchar_t* ReadString(wchar_t* lpBuf, DWORD dMaxSize)
		{
			lpBuf[(Read(lpBuf, dMaxSize*sizeof(wchar_t))+1) / sizeof(wchar_t)] = L'\0';
			return lpBuf;
		}
		//! 线程安全的读函数
		DWORD SafeRead(LPVOID lpBuf, DWORD dwSize)
		{
			InnerLock lock(this);
			return  Read(lpBuf, dwSize);
		}
		//! 线程安全读入ANSI字符串缓冲区
		char* SafeReadString(char* lpBuf, DWORD dMaxSize)
		{
			InnerLock lock(this);
			return ReadString(lpBuf, dMaxSize);
		}
		//! 线程安全读入UNICODE字符串缓冲区
		wchar_t* SafeReadString(wchar_t* lpBuf, DWORD dMaxSize)
		{
			InnerLock lock(this);
			return ReadString(lpBuf, dMaxSize);
		}
		//! 清除 \param bDeleteAll 为true时释放所有内存, 否则保留一个内存块以提高效率
		void Clear(bool bDeleteAll = false)
		{
			if (F_ && (F_==L_) && F_->S_>(M_<<2))
			{
                DWORD S = F_->S_;
                memset(F_, 0, sizeof(Block)), F_->S_ = S;
            }
			else
			{
				for (Block* t = F_;  t; delete  F_)
					F_ = t, t = t->N_;

				F_ = L_ = NULL, S_ = 0;
			}
		}
		//! 线程安全的清除函数  \sa void Clear(bool bDeleteAll = false)
		void SafeClear(bool bDeleteAll = false)
		{
			InnerLock lock(this);
			Clear(bDeleteAll);
		}
		//! 获得内存块指针 \param bFirst 为true时获得链表头部指针否则获得链表尾部指针
		Block* GetBlockPtr(bool bFirst = true)	
		{	
			return bFirst ? F_ : L_;						
		}
		//! 缓冲区尾部的空闲空间
		DWORD FreeSize()					
		{	
			return L_ ? L_->S_-L_->E_ : 0 ;			
		}
		//! 获得供API插入数据的直写缓冲区 传入所需大小dwSize 空闲不足分配新块 传入0返回尾部的空闲指针 尾部无空闲返回NULL 
		LPVOID GetFreePtr(DWORD dwSize = 0)
		{
			if (dwSize)
				return FreeSize() > dwSize ? L_->P_ + L_->E_ : NewBlock(dwSize)->P_;
			else
				return L_ ? L_->P_ + L_->E_ : NULL;
		}
		/*! \param[in] dwSize 传入增量并非全量
			\code strcpy(GetFreePtr(100), "test"), Release(4); //获取100字节的缓冲区 拷入四个字节 调整大小增加4个字节 \endcode
		*/
		//! 利用API直接写入用GetFreePtr()获得空闲指针, 同步调整缓冲区大小 
		DWORD Release(DWORD dwSize)
		{
			return (dwSize <= L_->S_) ? (L_->E_ += dwSize, S_ += dwSize, dwSize) : 0;		
		}
		//! 获得第一个有效块的缓冲区指针
		LPVOID GetPtr()
		{
			return F_ ? F_->P_ + F_->B_ : NULL;
		}
		//! 获得第一个有效块的缓冲区指针指向的数据大小
		DWORD GetPtrSize()
		{
			return F_ ? F_->E_ - F_->B_ : 0;
		}
		//! 数组访问 如果块数超过1 效率很低 推荐用迭代子访问可以获得理想性能
		BYTE& operator[](DWORD dwOffset)
		{
			CN_ASSERT(F_);

			if (F_ == L_)	
			{
				CN_ASSERT(F_->S_ > dwOffset);
				return F_->P_[dwOffset];
			}
			else			
			{
				Iterator iter = Begin();
				iter += dwOffset;
				CN_ASSERT(iter);
				return *iter;
			}
		}
		
	protected:

		//! 新建块 自动添加在尾部
		Block* NewBlock(DWORD dwSize)
		{
			dwSize = dwSize < M_ ? M_ : dwSize;
			Block * pNew = (Block *) new BYTE[sizeof(Block) - 4 + dwSize];

			if (pNew)
			{
				memset(pNew, 0, sizeof(Block));
				pNew->S_ = dwSize;

				if (L_)
					L_->N_ = pNew, L_ = pNew;
				else
					F_ = L_ = pNew;
			}
			
			return pNew;
		}

		Block*				F_;//!< 头指针
		Block*				L_;//!< 尾指针
		DWORD				S_;//!< 大小
		DWORD				M_;//!< 块最小长度
		CRITICAL_SECTION	C_;//!< 锁结构
	};

#if defined(CN_COMM_MFC_EXCEPTION)
	/*! 需要定义宏 CN_COMM_MFC_EXCEPTION CN_ASSERT \n 将使用throw new MfcException(msg);语句抛出MFC异常(VC++) */
	//! 用于MFC的异常  
	class MfcException : public CException
	{
	public:
		//! 构造函数，要求CException自动析构
		MfcException(LPCTSTR szMsg) : CException(TRUE)
		{
			lstrcpy(szMsg_, szMsg);

		#ifdef _DEBUG
			CException::m_bReadyForDelete = TRUE;
		#endif
		}
		//! 错误提示信息
		BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL )
		{
		#ifndef CN_COMM_FOR_CE
			lstrcpyn(lpszError, szMsg_, nMaxError);
		#else
			_tcsncpy(lpszError, szMsg_, nMaxError - 1);
			lpszError[nMaxError - 1] = _T('\0');
		#endif
			return TRUE;
		}
	protected:
		TCHAR szMsg_[256]; //!< 错误信息 格式：CN_ASSERT(表达式) failed; CnComm(行号); Code(GetLastError()); 错误代码描述
	};
#endif

	//! 用户启用异常 则抛出异常; 未启用异常: DEBUG版本 控制台版本输出错误信息 并退出; Release版本弹出提示框并响应用户要求
	static bool Assert(LPCTSTR szMessage)
	{
	#if defined(_DEBUG) || defined(CN_COMM_STD_EXCEPTION) || defined(CN_COMM_VCL_EXCEPTION) || defined(CN_COMM_MFC_EXCEPTION)
		TCHAR szMsg[256];
		DWORD dwError, dwLength;
		
		_tcscpy(szMsg, szMessage);

		dwError = GetLastError();
		if (dwError)//! 错误代码(GetLastError())不为 0 输出错误描述  
		{
			dwLength = _tcslen(szMsg);
			_sntprintf(szMsg + dwLength, 256 - _tcslen(szMsg), _T("Code:%d; "), dwError);
			dwLength = _tcslen(szMsg);

			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	//! 错误描述采用本地语言
				szMsg + dwLength - 1,							
				256 - dwLength - 1,									
				NULL
				);
		}
	#else 
		;// Release版本 什么都不做  预留
	#endif

	#if	defined(CN_COMM_STD_EXCEPTION)

		#ifdef _UNICODE
			char szTemp[512] = {0};
			WideCharToMultiByte(CP_ACP, 0, szMsg, -1, szTemp, wcslen(szMsg)*sizeof(wchar_t), NULL, NULL);
			throw std::runtime_error(szTemp);
		#else
			throw std::runtime_error(szMsg);
		#endif

	#elif defined(CN_COMM_VCL_EXCEPTION)

		throw Exception(szMsg);

	#elif defined(CN_COMM_MFC_EXCEPTION)

		throw (new MfcException(szMsg));

	#elif defined(_DEBUG)

		OutputDebugString(szMsg);
		#ifdef _CONSOLE
			// 需要 setlocale(LC_ALL, "chs"); 控制台才可以正确输出UNICODE中文 这里转换ANSI 避免这样问题
			#ifdef _UNICODE
				char szTemp[512] = {0};
				WideCharToMultiByte(CP_ACP, 0, szMsg, -1, szTemp, wcslen(szMsg)*sizeof(wchar_t), NULL, NULL);
				fprintf(stderr, szTemp);
			#else
				fprintf(stderr, szMsg);
			#endif

			system("pause");
			exit(1);
		#else
			switch(MessageBox(NULL, szMsg, _T("CN_ASSERT"), MB_ABORTRETRYIGNORE))
			{
			case IDABORT:
				exit(1);

			case IDRETRY:
				DebugBreak();
				break;

			case IDIGNORE:
				break;
			}
		#endif

	#else 
		;// Release版本 什么都不做  预留
	#endif
		return false;
	}

protected:

	DWORD				dwOption_;			//!< 关键模式配置 在构造函数中设定
	HANDLE				hComm_;				//!< 串口句柄
	DWORD				dwPort_;			//!< 串口号 0为特殊串口 -1为无效 主要终于消息通知甄别
	TCHAR				szName_[64];		//!< 保存串口名称 类似COM1的字符串
	CRITICAL_SECTION	CS_;				//!< 临界互斥锁
	DCB					DCB_;				//!< 波特率，停止位，等   
	COMMTIMEOUTS		CO_;				//!< 超时结构 
	COMMPROP*			pCP_;				//!< 串口参数

	BlockBuffer			I_;					//!< 读缓冲区
	BlockBuffer			O_;					//!< 写缓冲区
	DWORD				dwInCount_;			//!< 读计数
	DWORD				dwOutCount_;		//!< 写计数

	bool				bContinue_;			//!< 线程继续运行循环标志
	DWORD				dwWaitEvent_;		//!< WaitCommEvent 的监视事件

	HANDLE				hWatchThread_;		//!< 监视辅助线程
	DWORD				hWatchThreadId_;	//!< 监视辅助线程 ID
	HANDLE				hWatchEvent_;		//!< 通知监视线程

	HANDLE				hReadThread_;		//!< 接收辅助线程 实现双工提高效率
	DWORD				hReadThreadId_;		//!< 接收辅助线程 ID
	HANDLE				hReadEvent_;		//!< 通知接收线程

	HANDLE				hWriteThread_;		//!< 发送辅助线程 实现双工提高效率
	DWORD				hWriteThreadId_;	//!< 发送辅助线程 ID
	HANDLE				hWriteEvent_;		//!< 通知发送线程

	HWND				hNotifyWnd_;		//!< 通知窗口
	DWORD				hNotifyThreadId_;	//!< 通知线程
	DWORD				dwNotifyNum_;		//!< 接受多少字节(>=dwNotifyNum_)发送通知消息

#ifndef CN_COMM_FOR_CE
	OVERLAPPED			RO_;				//!< 重叠I/O ReadFile
	OVERLAPPED			WO_;				//!< 重叠I/O WriteFile
	OVERLAPPED			EO_;				//!< 重叠I/O WaitCommEvent 
#endif

	//! 初始化
	virtual void Init() 
	{
		::InitializeCriticalSection(&CS_);

		memset(szName_, 0, 64*sizeof(TCHAR));

		memset(&DCB_,	0, sizeof(DCB_));
		DCB_.DCBlength	= sizeof(DCB_);

		hComm_				= INVALID_HANDLE_VALUE;
		dwPort_				= (DWORD)-1;
		dwOutCount_			= 0;
		dwInCount_			= 0;
		
		dwWaitEvent_		= CN_COMM_WAIT_EVENT;
		hWatchThread_		= NULL;
		hReadThread_		= NULL;
		hWriteThread_		= NULL;
		hWatchThreadId_		= 0;
		hReadThreadId_		= 0;
		hWriteThreadId_		= 0;
		
		hNotifyWnd_			= NULL;
		dwNotifyNum_		= 0;
		hNotifyThreadId_	= 0;
		pCP_				= NULL;

	#ifndef CN_COMM_FOR_CE
		memset(&RO_, 0, sizeof(RO_));
		memset(&WO_, 0, sizeof(WO_));
		memset(&EO_, 0, sizeof(EO_));
		
		RO_.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(RO_.hEvent != NULL); 
		
		WO_.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(WO_.hEvent != NULL);
		
		EO_.hEvent= ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(EO_.hEvent != NULL); 
	#endif

		hWatchEvent_	= ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(hWatchEvent_ != NULL); 
		hReadEvent_		= ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(hReadEvent_ != NULL); 
		hWriteEvent_	= ::CreateEvent(NULL, TRUE, FALSE, NULL);
		CN_ASSERT(hWriteEvent_ != NULL); 
	} 
	//! 析构
	virtual void Destroy()
	{
	#ifndef CN_COMM_FOR_CE
		if(RO_.hEvent != NULL)
			CloseHandle(RO_.hEvent);
		
		if(WO_.hEvent != NULL)
			CloseHandle(WO_.hEvent);
		
		if(EO_.hEvent != NULL)
			CloseHandle(EO_.hEvent);
	#endif

		if(hWatchEvent_ != NULL)
			CloseHandle(hWatchEvent_);

		if(hReadEvent_ != NULL)
			CloseHandle(hReadEvent_);

		if(hWriteEvent_ != NULL)
			CloseHandle(hWriteEvent_);

		if (pCP_)
			delete[] ((BYTE*)pCP_);

		::DeleteCriticalSection(&CS_);
	}
	//! 绑定串口 
	void BindPort(DWORD dwPort)
	{
		dwPort_ = dwPort;

	#if defined(CN_COMM_FOR_CE)
		wsprintf(szName_, _T("COM%d:"), dwPort);
	#else
		wsprintf(szName_, _T("\\\\.\\COM%d"), dwPort);
	#endif
	}
	//! 打开串口 
	virtual bool OpenPort()
	{
		CN_ASSERT(!IsOpen());

		if(IsOpen())
			Close();
		
		hComm_ = ::CreateFile(
			szName_,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | (IsOverlappedMode() ? FILE_FLAG_OVERLAPPED : 0),
			NULL
			);
		
		return IsOpen();
	}
	//! 设置串口 
	virtual bool SetupPort()
	{
		if(!CN_ASSERT(IsOpen()))
			return false;

		#if defined(CN_COMM_FOR_CE)
			::SetupComm(hComm_, 4096, 4096);
		#else
			if(!CN_ASSERT(::SetupComm(hComm_, 4096, 4096)))//! 配置端口发送接收队列大小, 读4096字节, 写4096字节, 阻塞I/O模式发送队列无意义
			return false; 
		#endif
		
		if(!CN_ASSERT(::GetCommTimeouts(hComm_, &CO_)))
			return false;

		CO_.ReadIntervalTimeout			= 100;//! 配置超时结构 字符最小间隔100ms
		CO_.ReadTotalTimeoutMultiplier	= 0;
		CO_.ReadTotalTimeoutConstant	= IsOverlappedMode() ? 500 : 250;//! 读超时 重叠I/O模式下500毫秒 阻塞I/O模式下250毫秒
		CO_.WriteTotalTimeoutMultiplier = IsOverlappedMode() ? 1 : 0;
		CO_.WriteTotalTimeoutConstant	= IsOverlappedMode() ? 10000 : 250;//! 写超时 重叠I/O模式下(10000+1×字节数)毫秒 阻塞I/O模式下250毫秒

		if(!CN_ASSERT(::SetCommTimeouts(hComm_, &CO_)))
			return false; 
		
		if(!CN_ASSERT(::PurgeComm(hComm_, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR )))//! 清除端口
			return false; 
		
		return true;
	} 
	//! 将端口数据读入缓冲区的
	DWORD PortToBuffer(DWORD dwPortByteNum)
	{
		BlockBuffer::InnerLock locker(&I_);
		return dwPortByteNum ? I_.Release(ReadPort(I_.GetFreePtr(dwPortByteNum), dwPortByteNum)) : 0;
	}
	//! 将缓冲区的数据写入端口 
	DWORD BufferToPort(DWORD dwMaxSize = 1024)
	{
		if (IsOverlappedMode())
		{
			BlockBuffer::InnerLock locker(&O_);

			if (!O_.Size())
				return 0;

			DWORD dwWrite = dwMaxSize > O_.GetPtrSize() ? O_.GetPtrSize() : dwMaxSize;
			WritePort(O_.GetPtr(), dwWrite);
			return O_.Read(NULL, dwWrite);
		}
		else
		{
			BYTE btTemp[1024];
			DWORD dwLength, dwIndex = 0;

			O_.Lock();
			dwLength = O_.Read(btTemp, 1024);
			O_.Unlock();

			while ( dwIndex < dwLength )
				dwIndex += WritePort(btTemp + dwIndex, dwLength - dwIndex);

			return dwLength;
		}
	}
	/*! \param uMsg 消息 \param lParam 参数 \param bPostMode 采用PostMessage或者SendMessage发送消息*/
	//! 当窗口句柄或线程ID有效, 送出消息, 用于相应事件通知, WPARAM默认包含串口编号 
	virtual void Notify(UINT uMsg, LPARAM lParam = 0, bool bPostMode = true)
	{
		if(hNotifyWnd_ && ::IsWindow(hNotifyWnd_))//窗口句柄有效
		{
			if (bPostMode)
				PostMessage(hNotifyWnd_, uMsg, WPARAM(dwPort_), lParam);
			else
				SendMessage(hNotifyWnd_, uMsg, WPARAM(dwPort_), lParam);
		}

		if (hNotifyThreadId_)//线程ID有效
			PostThreadMessage(hNotifyThreadId_, uMsg, WPARAM(dwPort_), lParam);
	}
	//---------------------------------------threads callback-----------------------------------------------------
	//! 响应EV_RXCHAR事件 由线程回调, 虚函数可以在基层类中扩展(推荐方式)    
	virtual void OnReceive() 
	{
		Notify(ON_COM_RECEIVE);
	}
	//! 响应EV_DSR事件
	virtual void OnDSR()
	{
		Notify(ON_COM_DSR, CheckDSR() ? 1 : 0);
	}
	//! 响应EV_CTS事件
	virtual void OnCTS()
	{
		Notify(ON_COM_CTS, CheckCTS() ? 1 : 0);
	}
	//! 响应EV_BREAK事件
	virtual void OnBreak()
	{
		ClearCommBreak(hComm_);
		Notify(ON_COM_BREAK);
	}
	//! 响应EV_TXEMPTY事件
	virtual void OnTxEmpty()
	{
		Notify(ON_COM_TXEMPTY);
	}
	//! 响应EV_ERROR事件
	virtual void OnError()
	{
		Notify(ON_COM_ERROR, ClearError());
	}
	//! 响应EV_RING事件
	virtual void OnRing()
	{
		Notify(ON_COM_RING, CheckRING() ? 1 : 0);
	}
	//! 响应EV_RLSD事件
	virtual void OnRLSD()
	{
		Notify(MS_RLSD_ON, CheckRLSD() ? 1 : 0);
	}
	//! 响应EV_RXFLAG事件
	virtual void OnRxFlag()
	{
		Notify(ON_COM_RXFLAG);
	}
	//! 响应EV_POWER事件
	virtual void OnPower()
	{
		Notify(ON_COM_POWER);
	}
	//! 响应EV_RX80FULL事件
	virtual void OnRx80Full()
	{
		Notify(ON_COM_RX80FULL);
	}
	//! 响应EV_EVENT1事件
	virtual void OnEvent1()
	{
		Notify(ON_COM_EVENT1);
	}
	//! 响应EV_EVENT2事件
	virtual void OnEvent2()
	{
		Notify(ON_COM_EVENT2);
	}
	//! 响应EV_PERR事件
	virtual void OnPrintErr()
	{
		Notify(ON_COM_PERR);
	}
	//! 通用的事件处理
	virtual void HandleEvent(DWORD dwMask)
	{
		if ((dwMask & EV_RXCHAR) && !(dwOption_&EN_RX_THREAD))
		{
			DWORD dwLength = GetQueueCount(true);

			if (dwLength)
			{
				if (dwOption_ & EN_RX_BUFFER)
				{
					PortToBuffer((dwLength << 1) + 64);//多出的字节数 用来避免事件间隙的陷阱

					if (I_.Size() >= dwNotifyNum_)
						OnReceive();
				}
				else
				{
					if (dwLength >= dwNotifyNum_)
						OnReceive();
				}
			}
		}

		if (dwMask & EV_TXEMPTY)
		{
			if ((dwOption_&EN_TX_BUFFER) && !(dwOption_&EN_TX_THREAD) && O_.SafeSize())
				BufferToPort();

			OnTxEmpty();
		}

		if (dwMask & EV_RXFLAG)
			OnRxFlag();

		if (dwMask & EV_CTS)
			OnCTS();
			
		if (dwMask & EV_DSR)
			OnDSR();
			
		if (dwMask & EV_RING)
			OnRing();
			
		if (dwMask & EV_RLSD)
			OnRLSD();
			
		if (dwMask & EV_BREAK)
			OnBreak();
			
		if (dwMask & EV_ERR)
			OnError();

	#ifdef CN_COMM_FOR_CE
		if (dwMask & EV_POWER)
			OnPower();
	#endif
	}
	
#ifndef CN_COMM_FOR_CE
	//! 监视线程用于重叠I/O模型 
	virtual DWORD OverlappedModel()
	{
		if(!CN_ASSERT(IsOpen()))
			return 1;

		if(!CN_ASSERT(::SetCommMask(hComm_, dwWaitEvent_)))
			return 1;

		for(DWORD dwMask = 0, dwLength; bContinue_ && IsOpen(); dwMask = 0)
		{
			if (GetQueueCount(true))//! 等待事件前会先扫描队列是否还有未处理数据 如果有模拟一个EV_RXCHAR事件 避免事件通知的陷阱
				dwMask = EV_RXCHAR, Sleep(10);
			else
			{
				if(!::WaitCommEvent(hComm_, &dwMask, &EO_))
				{
					if(::GetLastError() == ERROR_IO_PENDING)
						::GetOverlappedResult(hComm_, &EO_, &dwLength, TRUE);
					else
					{
						Sleep(10);
						continue;
					}
				}
			}
			
			if(dwMask == 0)
			{
				if ((dwOption_&EN_TX_BUFFER) && !(dwOption_&EN_TX_THREAD) && O_.SafeSize())
					BufferToPort();

				continue;
			}
			
			HandleEvent(dwMask);
		}//for

		return 0;
	}
#endif
	//! 监视线程用于阻塞I/O模型 
	virtual DWORD NonoverlappedModel()
	{
		if(!CN_ASSERT(IsOpen()))
			return 1;

		for (DWORD dwReturn; bContinue_ && IsOpen();)
		{
			dwReturn = WaitForSingleObject(hWatchEvent_, 50);

			if (!bContinue_) 
				break;

			switch (dwReturn)
			{
			case WAIT_OBJECT_0:
				while ((dwOption_&EN_TX_BUFFER) && O_.SafeSize())
					BufferToPort();

				ResetEvent(hWatchEvent_);

				break;

			case WAIT_TIMEOUT:

				DWORD dwLength = GetQueueCount(true);

				if (dwLength)
				{
					if (dwOption_ & EN_RX_BUFFER)
					{
						PortToBuffer(dwLength);

						if (I_.Size() >= dwNotifyNum_)
							OnReceive();
					}
					else
					{
						if (dwLength >= dwNotifyNum_)
							OnReceive();
					}
				}

				if ( (dwOption_&EN_TX_BUFFER) && O_.SafeSize() )
					BufferToPort();

				break;
			}
		}

		return 0;
	}
#ifdef CN_COMM_FOR_CE
	//! 监视线程用于WINCE的I/O模型(重叠+阻塞 也不知微软把它叫什么模式)
	virtual DWORD EmbeddedModel()
	{
		if(!CN_ASSERT(IsOpen()))
			return 1;

		if(!CN_ASSERT(::SetCommMask(hComm_, dwWaitEvent_)))
			return 1;

		for(DWORD dwMask = 0; bContinue_ && IsOpen(); dwMask = 0)
		{
			if (GetQueueCount(true))//! 等待事件前会先扫描队列是否还有未处理数据 如果有模拟一个EV_RXCHAR事件 避免事件通知的陷阱
				dwMask = EV_RXCHAR, Sleep(10);
			else
			{
				if(!::WaitCommEvent(hComm_, &dwMask, NULL))
					continue;
			}
			
			if(dwMask == 0)
			{
				if ( (dwOption_&EN_TX_BUFFER) && !(dwOption_&EN_TX_THREAD) && O_.SafeSize())
					BufferToPort();

				continue;
			}
		
			HandleEvent(dwMask);

		}//for

		return 0;
	}
#endif
	//! 用于双工处理当然您可以改变用途 
	virtual DWORD ReadModel()
	{
		while(bContinue_)
		{
			Sleep(50);

			DWORD dwLength = GetQueueCount(true);

			if (dwLength)
			{
				if (dwOption_ & EN_RX_BUFFER)
				{
					PortToBuffer(dwLength);

					if (I_.Size() >= dwNotifyNum_)
						OnReceive();
				}
				else
				{
					if (dwLength >= dwNotifyNum_)
						OnReceive();
				}
			}
		}

		return 0;
	}
	//! 用于双工处理当然您可以改变用途 
	virtual DWORD WriteModel()
	{
		CN_ASSERT(dwOption_ & EN_TX_BUFFER);

		while (bContinue_)
		{
			DWORD dwReturn = ::WaitForSingleObject(hWriteEvent_, 200);

			while(bContinue_ && O_.SafeSize())
				BufferToPort();

			if (dwReturn == WAIT_OBJECT_0)
				ResetEvent(hWatchEvent_);
		}

		return 0;
	}
	
private:  
	//! 禁止拷贝构造
	CnComm(const CnComm&);
	//! 禁止赋值函数
	CnComm &operator = (const CnComm&);

#if defined(_MT) && !defined(CN_COMM_FOR_CE)
	//! 监视线程 Watch Thread 完成监视 并根据配置决定是否完成读写
	static UINT APIENTRY WatchThreadProc(LPVOID lpPara)
#else
	//! 监视线程 Watch Thread 完成监视 并根据配置决定是否完成读写
	static DWORD WINAPI WatchThreadProc(LPVOID lpPara)
#endif
	{
	#ifdef CN_COMM_FOR_CE 
		DWORD dwCode = ((CnComm *)lpPara)->EmbeddedModel();
	#else
		DWORD dwCode = ((CnComm *)lpPara)->IsOverlappedMode() 
			? ((CnComm *)lpPara)->OverlappedModel() 
			: ((CnComm *)lpPara)->NonoverlappedModel();

		#if defined(_MT) 
			_endthreadex(dwCode);
		#endif
	#endif

		return dwCode;
	}

#if defined(_MT) && !defined(CN_COMM_FOR_CE)
	//! 双工的线程 暂时未用 估计用不着
	static UINT APIENTRY ReadThreadProc(LPVOID lpPara)
#else
	//! 双工的线程 暂时未用 估计用不着
	static DWORD WINAPI ReadThreadProc(LPVOID lpPara)
#endif
	{
		DWORD dwCode = ((CnComm *)lpPara)->ReadModel();

		#if defined(_MT) && !defined(CN_COMM_FOR_CE)
			_endthreadex(dwCode);
		#endif

		return dwCode;
	}

#if defined(_MT) && !defined(CN_COMM_FOR_CE)
	//! 双工的线程 WINCE可以采用
	static UINT APIENTRY WriteThreadProc(LPVOID lpPara)
#else 
	//! 双工的线程 WINCE可以采用
	static DWORD WINAPI WriteThreadProc(LPVOID lpPara)
#endif
	{
		DWORD dwCode = ((CnComm *)lpPara)->WriteModel();

		#if defined(_MT) && !defined(CN_COMM_FOR_CE)
			_endthreadex(dwCode);
		#endif

		return dwCode;
	}
};

#endif //! _CN_COMM_H_
