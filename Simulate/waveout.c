/********************** BEGIN LICENSE BLOCK ************************************
 *
 * HiBy Audio Player V1.0
 * SMARTACTION CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) HiBy Music Co. Ltd 2013. All rights reserved.
 *
 * This file, and the files included with this file, is distributed and made
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * http://www.hiby.com
 *
 ********************** END LICENSE BLOCK **************************************
 *
 *  Author:  <fanoblem@gmail.com>
 *
 *  Create:   2013-11-26, by fanoble
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <windows.h>

/******************************************************************************/

#define		BUFFER_SIZE		1800
#define		BUFFER_COUNT	32

static WAVEHDR*			waveBlocks;		// pointer to our ringbuffer memory
static HWAVEOUT			hWaveOut;		// handle to the waveout device
static unsigned int		buf_write      = 0;
static unsigned int		buf_write_pos  = 0;
static int				full_buffers   = 0;
static int				buffered_bytes = 0;
static int				(*wo_idle_proc)(void* data, int code) = 0;
static void*			wo_idle_data = 0;

static unsigned int		buffer_size = BUFFER_SIZE;

static void CALLBACK waveOutProc(	HWAVEOUT	hWaveOut,
									UINT		uMsg,
									DWORD		dwInstance,
									DWORD		dwParam1,
									DWORD		dwParam2)
{
	LPWAVEHDR		pHeader = (LPWAVEHDR)dwParam1;

	if (uMsg != WOM_DONE)
		return;

//	printf("%d %d\n", full_buffers, buffered_bytes);

	if (full_buffers)
	{
		buffered_bytes -= buffer_size;
		if (buffered_bytes < 0) buffered_bytes = 0;

		--full_buffers;
	}
	else
	{
		buffered_bytes = 0;
	}
}

#if		0
// to set/get/query special features/parameters
int control(int cmd, void *arg)
{
	DWORD volume;

	waveOutGetVolume(hWaveOut, &volume);
	waveOutSetVolume(hWaveOut, volume);

	return 0;
}
#endif

// open & setup audio device
// return: 1=success 0=fail
int wo_init(int sample_rate, int channels, int sample_size)
{
	WAVEFORMATEX	wf;
	MMRESULT		result;
	DWORD			totalBufferSize;
	int				i;
	unsigned char*	buffer;

	switch(sample_rate){
		case 8000:
		case 10000:
		case 16000:
		case 32000:
		case 44100:
		case 48000:
			buffer_size = BUFFER_SIZE;
			break;
		case 88200:
		case 96000:
			buffer_size = BUFFER_SIZE * 2;
			break;
		case 176400:
		case 192000:
			buffer_size = BUFFER_SIZE * 4;
			break;
		case 352800:
		case 384000:
			buffer_size = BUFFER_SIZE * 8;
			break;
		default:
			buffer_size = BUFFER_SIZE * 4;
			break;
	}

	// fill waveformatex
	wf.wFormatTag		= WAVE_FORMAT_PCM;
	wf.nBlockAlign		= sample_size / 8 * channels;
	wf.nAvgBytesPerSec	= sample_rate * wf.nBlockAlign;
	wf.nSamplesPerSec	= sample_rate;
	wf.nChannels		= channels;
	wf.wBitsPerSample	= sample_size;
	wf.cbSize			= sizeof(wf);

	// open sound device
	// WAVE_MAPPER always points to the default wave device on the system
	result = waveOutOpen(&hWaveOut, WAVE_MAPPER, (WAVEFORMATEX*)&wf,
							(DWORD)waveOutProc, 0, CALLBACK_FUNCTION);

	if (result != MMSYSERR_NOERROR)
		return 0;

	totalBufferSize = (buffer_size + sizeof(WAVEHDR)) * BUFFER_COUNT;

	// allocate buffer memory as one big block
	buffer = (unsigned char*)malloc(totalBufferSize);
	memset(buffer, 0, totalBufferSize);

	// and setup pointers to each buffer
	waveBlocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * BUFFER_COUNT;
	for (i = 0; i < BUFFER_COUNT; i++)
	{
		waveBlocks[i].dwFlags = WHDR_DONE;
		waveBlocks[i].lpData  = (LPSTR)buffer;
		buffer += buffer_size;
	}

	buf_write      = 0;
	buf_write_pos  = 0;
	full_buffers   = 0;
	buffered_bytes = 0;

	return 1;
}

// close audio device
static void uninit(int immed)
{
	if (!immed)
	{
		while (buffered_bytes > 0)
			Sleep(50);
	}
	else
	{
		buffered_bytes = 0;
	}

	waveOutPause(hWaveOut);
//	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);

	free(waveBlocks);
}

// stop playing and empty buffers (for seeking/pause)
void wo_reset()
{
	waveOutReset(hWaveOut);

	buf_write      = 0;
	buf_write_pos  = 0;
	full_buffers   = 0;
	buffered_bytes = 0;
}

// stop playing, keep buffers (for pause)
void audio_pause()
{
	waveOutPause(hWaveOut);
}

// resume playing, after audio_pause()
void audio_resume()
{
	waveOutRestart(hWaveOut);
}

// return: how many bytes can be played without blocking
static int get_space()
{
	return BUFFER_COUNT * buffer_size - buffered_bytes;
}

static int get_holds()
{
	return buffered_bytes;
}

// writes data into buffer, based on ringbuffer code in ao_sdl.c
static int write_waveOutBuffer(unsigned char* data, int len)
{
	WAVEHDR* current;
	int len2 = 0;
	int x;
	unsigned int buf_start_write = buf_write;

	if (buffered_bytes == BUFFER_COUNT * buffer_size)
		return 0;

	while (len > 0)
	{
		current = &waveBlocks[buf_write];
		if (0 == (current->dwFlags & WHDR_DONE))
		{
			// block is full, find next!
			buf_write     = (buf_write + 1) % BUFFER_COUNT;
			buf_write_pos = 0;

			if (buf_start_write == buf_write)
				return len2;

			continue;
		}

		buf_start_write = buf_write;

		// unprepare the header if it is prepared
		if (current->dwFlags & WHDR_PREPARED)
		{
			waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
			current->dwFlags &= ~WHDR_PREPARED;
		}

		x = buffer_size - buf_write_pos;

		if (x > len) x = len;

		memcpy(current->lpData + buf_write_pos, data + len2, x);

		len2 += x;
		len  -= x;

		buffered_bytes += x;
		buf_write_pos  += x;

		// prepare header and write data to device
		current->dwBufferLength = buf_write_pos;

		waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

		if (buf_write_pos >= buffer_size)
		{
			// block is full, find next!
			buf_write     = (buf_write + 1) % BUFFER_COUNT;
			buf_write_pos = 0;
			full_buffers++;
		}
	}

	return len2;
}

int wo_open(int sample_rate)
{
	int r;

	r = wo_init(sample_rate, 1, 16);

	r = r ? 0 : -1;

	return r;
}

int wo_write(short* pcm, int size)
{
	size *= 2;

	while (size > 0)
	{
		int consumed;

		consumed = write_waveOutBuffer((unsigned char* )pcm, size);

		size -= consumed;
		pcm += consumed / 2;
	}

	return 0;
}

int wo_close()
{
	Sleep(200);
	uninit(0);
	return 0;
}

/*******************************************************************************
                           E N D  O F  F I L E
*******************************************************************************/
