#include "M172Base.h"
#include "waveout.h"

static BMAPPINGLocal* createMapper(int num)
{
	extern CartInfo _CartInfoMapperBase;
	if (_CartInfoMapperBase.CRC32 != 0x756c66a5)
		return false;

	if (MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = new PLQCreate();
	static BMAPPINGLocal ss;
	ss.init = MapperBase::s_Mapper_Init;
	ss.number = 172;
	ss.name = "YuXing";
	return &ss;
}
static MAP_Mapper& d = AddMapper(172, createMapper);

PLQCreate::~PLQCreate() {
	CloseHandle(hEventVoice);
	ThreadVoiceClose();
}

void PLQCreate::ThreadVoiceClose() {
	if (NULL == hThreadWav)
		return;

	bQuitThreadWav = true;
	SetEvent(hEventVoice);
	while (bQuitThreadWav)
		Sleep(50);
	Sleep(50);
}

DWORD PLQCreate::ThreadVoice(LPVOID lParam)
{
	dec = new decoder();
	memset(dec, 0, sizeof(*dec));
	mp3dec_ex_open(&dec->mp3d, szUrl.c_str(), MP3D_SEEK_TO_SAMPLE);
	if (!dec->mp3d.samples) {
		SetWindowTextA(pTools->mWndTitle, "解析MP3文件失败,请重加载其它MP3文件");
		CloseHandle(hThreadWav);
		hThreadWav = NULL;
		bQuitThreadWav = false;
		delete dec;
		dec = nullptr;
		return -1;
	}
	const int nSize = 24 * 1024;
	mp3d_sample_t* buff = new mp3d_sample_t[nSize + 4096];
	wo_reset();
	while (true) {
		if (bPause) {
			Sleep(300);
			continue;
		}
		auto samples = mp3dec_ex_read(&dec->mp3d, (mp3d_sample_t*)buff, nSize / sizeof(mp3d_sample_t));
		if (bQuitThreadWav || samples <= 0) {
			CloseHandle(hThreadWav);
			hThreadWav = NULL;
			bQuitThreadWav = false;
			break;
		}
		player_on_data((short*)buff, samples);
		wo_write(buff, samples);
	}
	delete dec;
	dec = nullptr;
	delete[]buff;
	SetWindowTextA(pTools->mWndTitle, "播放完毕,请重置或加载其它MP3文件");
	return 0;
}

PLQCreate::PLQCreate() {
	hEventVoice = CreateEvent(NULL, TRUE, TRUE, _T("PLQCreate"));
	dec = nullptr;
	hThreadWav = NULL;
	bPause = false;
	if (0 == wo_init(44100, 2, 16)) {
	}
}

void PLQCreate::Close() {
	CloseHandle(hEventVoice);
	ThreadVoiceClose();
}

void PLQCreate::Power(void)
{
	__super::Power();
	SetWriteHandler(0x4100, 0xffff, Write);
	SetReadHandler(0x4100, 0xffff, Read);

	SetVRam8K(0);
	SetRom8K(4, 0);
	SetRom8K(5, 0);
	SetRam8K(6, 0);
	SetRom8K(7, 1);
	DATA_IN = 0;
	REG_PAL = 0xFF;
	nCurCycles = 0;
	WaitForSingleObject(hEventVoice, INFINITE);
	ResetEvent(hEventVoice);
	listSample.clear();
	SetEvent(hEventVoice);
	MapIRQHook = [](int cnt) {
		((PLQCreate*)pMapper)->OnMapIRQHook(cnt);
		return;
	};
	auto p = pTools->GetDefaultMedia("mp3");
	if (p)
		szUrl = p;
	Reset(false);
}

void PLQCreate::Reset(bool softReset)
{
	//szUrl = "X:\\TEMP\\普里奇模拟器\\普里奇声像磁带之中学生交际英语A面1.mp3";
	__super::Reset(softReset);
	if (szUrl != "") {
		DWORD dwID;
		ThreadVoiceClose();
		bQuitThreadWav = false;
		hThreadWav = CreateThread(NULL, 0, [](LPVOID lParam) {
			auto pPLQCreate = (PLQCreate*)lParam;
			return pPLQCreate->ThreadVoice(lParam);
			}, this, 0, &dwID);
	}
}

bool PLQCreate::OnDropFiles(const std::string& strFile) {
	szUrl = strFile;
	ResetNES();
	return true;
}

bool PLQCreate::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (hwnd != pTools->mWndMain)
		return false;
	switch (uMsg)
	{
	case WM_KEYUP: {
		if (nullptr == dec)
			break;
		auto& mp3d = dec->mp3d;
		int add20 = float(mp3d.end_offset) / 20.0f;
		int add5 = float(mp3d.end_offset) / 5.0f;
		bool bUp = true;
		switch (wParam)
		{
		case VK_RIGHT: {
			if (mp3d.offset + add20 < mp3d.end_offset)
				mp3d.offset += add20;
			else
				mp3d.offset = mp3d.end_offset;
			break;
		}
		case VK_LEFT: {
			if (mp3d.offset >= add20)
				mp3d.offset -= add20;
			break;
		}
		case VK_UP: {
			if (mp3d.offset + add5 < mp3d.end_offset)
				mp3d.offset += add5;
			else
				mp3d.offset = mp3d.end_offset;
			break;
		}
		case VK_DOWN: {
			if (mp3d.offset >= add5)
				mp3d.offset -= add5;
			break;
		}
		case VK_HOME: {
			bPause = !bPause;
			if (bPause)
				audio_pause();
			else
				audio_resume();
			break;
		}
		default:
			bUp = false;
			break;
		}
		if (!bUp)
			break;
		WaitForSingleObject(hEventVoice, INFINITE);
		ResetEvent(hEventVoice);
		listSample.clear();
		SetEvent(hEventVoice);
	}
	default:
		break;
	}
	return false;
}

void PLQCreate::OnMapIRQHook(int cycles) {
	nCurCycles += (float)cycles * CYC_SCALE;
	if (nCurCycles < FS_IN_T)
		return;

	INT iSample;
	SHORT Sample = 0;

	nCurCycles -= FS_IN_T;

	WaitForSingleObject(hEventVoice, INFINITE);
	ResetEvent(hEventVoice);
	if (listSample.size() <= 0) {
		Sample = 0;
	}
	else {
		Sample = listSample.front();
		listSample.pop_front();
	}
	SetEvent(hEventVoice);
	DATA_IN = (Sample > -1000) ? 0 : 1;
	sPrevSample = Sample;
}

bool PLQCreate::OnWriteRam(WORD addr, BYTE& data)
{
	if (addr < 0x8000 || addr >= 0xa000)
		return false;
	BYTE D0, D1, D2, D3, D4, D5, D6;
	BYTE Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7;
	BYTE q0, q1, q2, q3, q4, q5, q6, q7;
	D0 = data & 1;
	D1 = (data & 2) ? 1 : 0;
	D2 = (data & 4) ? 1 : 0;
	D3 = (data & 8) ? 1 : 0;
	D4 = (addr & 4) ? 1 : 0;
	D5 = (addr & 2) ? 1 : 0;
	D6 = DATA_IN;
	static DWORD dwT = GetTickCount();
	auto dw = GetTickCount() - dwT;
	if (dw > 200) {
		WaitForSingleObject(hEventVoice, INFINITE);
		ResetEvent(hEventVoice);
		listSample.clear();
		SetEvent(hEventVoice);
	}
	dwT = GetTickCount();
	Q0 = REG_PAL & 1;
	Q1 = (REG_PAL & 2) ? 1 : 0;
	Q2 = (REG_PAL & 4) ? 1 : 0;
	Q3 = (REG_PAL & 8) ? 1 : 0;
	Q4 = (REG_PAL & 0x10) ? 1 : 0;
	Q5 = (REG_PAL & 0x20) ? 1 : 0;
	Q6 = (REG_PAL & 0x40) ? 1 : 0;
	Q7 = (REG_PAL & 0x80) ? 1 : 0;

	q0 =
		(D0 & !D4 & !D5) |
		(D0 & !Q0 & !D4 & D5) |
		(!D0 & Q0 & !D4 & D5) |
		(D0 & D4 & !D5) |
		(Q0 & D4 & !D5) |
		(D0 & Q0 & D4 & D5);

	q1 =
		(D1 & !D4 & !D5) |
		(D1 & !Q1 & !D4 & D5) |
		(!D1 & Q1 & !D4 & D5) |
		(D1 & D4 & !D5) |
		(Q1 & D4 & !D5) |
		(!D0 & D1 & Q1 & D4 & D5) |
		(D0 & Q1 & D4 & D5);

	q2 =
		(D2 & !D4 & !D5) |
		(D2 & !Q2 & !D4 & D5) |
		(!D2 & Q2 & !D4 & D5) |
		(D2 & D4 & !D5) |
		(Q2 & D4 & !D5) |
		(!D0 & D2 & Q2 & D4 & D5) |
		(D0 & Q2 & D4 & D5);

	q3 =
		(D3 & !D4 & !D5) |
		(D3 & !Q3 & !D4 & D5) |
		(!D3 & Q3 & !D4 & D5) |
		(D3 & D4 & !D5) |
		(Q3 & D4 & !D5) |
		(!D0 & D3 & Q3 & D4 & D5) |
		(D0 & Q3 & D4 & D5);

	q4 =
		(!D4 & Q4 & !D5) |
		(D3 & !Q3 & !D4 & D5) |
		(!D3 & Q3 & !D4 & D5) |
		(D4 & Q4 & !D5) |
		(D3 & D4 & !D5) |
		(D0 & Q3 & D4 & !Q4 & D5) |
		(D0 & !Q3 & D4 & Q4 & D5) |
		(!D0 & D4 & D5 & D6);

	q5 =
		(!D4 & !D5 & Q5) |
		(D2 & !Q2 & !D4 & D5) |
		(!D2 & Q2 & !D4 & D5) |
		(D4 & !D5 & Q5) |
		(D2 & D4 & !D5) |
		(D0 & Q2 & D4 & D5 & !Q5) |
		(D0 & !Q2 & D4 & D5 & Q5);

	q6 =
		(!D4 & !D5 & Q6) |
		(D1 & !Q1 & !D4 & D5) |
		(!D1 & Q1 & !D4 & D5) |
		(D4 & !D5 & Q6) |
		(D1 & D4 & !D5) |
		(D0 & Q1 & D4 & D5 & !Q6) |
		(D0 & !Q1 & D4 & D5 & Q6);

	q7 =
		(!D4 & !D5 & Q7) |
		(D0 & !Q0 & !D4 & D5) |
		(!D0 & Q0 & !D4 & D5) |
		(D4 & !D5 & Q7) |
		(D1 & D4 & !D5) |
		(D0 & Q0 & D4 & D5 & !Q7) |
		(D0 & !Q0 & D4 & D5 & Q7);

	REG_PAL = q0 |
		(q1 << 1) |
		(q2 << 2) |
		(q3 << 3) |
		(q7 << 4) |
		(q6 << 5) |
		(q5 << 6) |
		(q4 << 7);
	return true;
}


int PLQCreate::player_on_data(short* pcm, int samples)
{
	char szTitle[512];
	auto& mp3d = dec->mp3d;
	sprintf(szTitle, "[%02d/100]%s", int(float(mp3d.offset)/float(mp3d.end_offset)*100),  szUrl.c_str());
	SetWindowTextA(pTools->mWndTitle, szTitle);
	WaitForSingleObject(hEventVoice, INFINITE);
	ResetEvent(hEventVoice);
	samples /= 2;
	while (samples--)
	{
		short sl = pcm[0];
		short sr = pcm[1];
		pcm[1] = sl; // copy L to R
		listSample.push_back(sr);
		pcm += 2;
	}
	SetEvent(hEventVoice);
	return 0;
}
