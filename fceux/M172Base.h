/*
*/
#include "MapperBase.h"
#include "Tools.h"
#include "minimp3_ex.h"

class PLQCreate : public MapperBase
{
public:
	// 44.1KHz   8.0/324/不容 8.0/330/不容  8.0/320/4   10不能大于440小于400    423<435
#define CYC_SCALE 10.0f //10
#define FS_IN_T 410.0f //653.0f //850.0f //415.0f //377
#define PLQ_BUF_SIZE 16384 * 3
#define PLQ_BUF_MASK (PLQ_BUF_SIZE - 1)
#define WAV_THRESHOLD 7000

	BYTE REG_PAL, DATA_IN;
	double	nCurCycles;
	SHORT	sPrevSample;
	bool bPause, bQuitThreadWav;
	HANDLE hThreadWav;

	std::string szUrl;
	HANDLE hEventVoice;
	std::list<short> listSample;
	typedef struct decoder
	{
		mp3dec_ex_t mp3d;
		float mp3_duration;
		float spectrum[32][2]; // for visualization
	} decoder;
	decoder* dec;

	virtual int GetPRGRamSize() { return 1024; }
	virtual int GetCHRRamSize() { return 1024; }
	virtual int GetWorkRamSize() { return 1024; }

	PLQCreate();
	void ThreadVoiceClose();
	DWORD ThreadVoice(LPVOID lParam);
	virtual ~PLQCreate();
	virtual void Power(void);
	virtual void Reset(bool softReset);
	virtual void Close();
	int player_on_data(short* pcm, int samples);
	void OnMapIRQHook(int cycles);
	virtual bool WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool OnReadRam(WORD addr, BYTE& data) override
	{
		if (addr < 0xA000 || addr >= 0xC000)
			return false;
		data = REG_PAL;
		return true;
	}
	virtual bool OnDropFiles(const std::string& strFile);
	virtual bool OnWriteRam(WORD addr, BYTE& data);
};
