/*******************************************************************************
 *
 *  LPC-10 D6 Synthesizer
 *
 *  Author:  <87430545@qq.com>
 *
 *  Create:  May/6/2021 by fanoble
 *
 *******************************************************************************
 */
#include <string.h>

#define LPC_ORDER 10
#define LPC_SAMPLES_PER_FRAME 200 // speed

#define LPC_FRAC_BITS 15 // using Q15

typedef struct tag_lpc_frame {
	short energy;
	short pitch;
	short k[LPC_ORDER]; // K1-K10
} LPC_FRAME;

typedef struct tag_lpc_synth {
	LPC_FRAME frame_prev; // 0
	LPC_FRAME frame_curr; // 2
	LPC_FRAME frame_next; // 1

	short need_interp;
	short random_seed;
	short curr_pitch;
	short sample_index; // from 0 to LPC_SAMPLES_PER_FRAME - 1

	short x[LPC_ORDER + 1]; // filter buf
	short synth_out; // output sample

	// for bit reader
	int bits_left;
	unsigned short data_cache;
	int bitstream_length;
	const unsigned char* in_stream;
} LPC_SYNTH;

//------------------------------------------------------------------------------

#pragma warning (disable:4305)

static const short lpc_gain_tab[] =
{
	0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0700, 0x0B00,
	0x1100, 0x1A00, 0x2900, 0x3F00, 0x5500, 0x7000, 0x7F00, 0x0000
};

static const short lpc_pitch_tab[] =
{
	0x0000,	0x0100, 0x0104, 0x0108, 0x0110, 0x0114, 0x0118, 0x011C,
	0x0124, 0x0128, 0x012C, 0x0134, 0x0138, 0x0140, 0x0144, 0x014C,
	0x0150, 0x0158, 0x015C, 0x0164, 0x016C, 0x0170, 0x0178, 0x0180,
	0x0184, 0x018C, 0x0194, 0x019C, 0x01A4, 0x01AC, 0x01B4, 0x01BC,
	0x01C4, 0x01CC, 0x01D4, 0x01DC, 0x01E4, 0x01F0, 0x01F8, 0x0200,
	0x020C, 0x0214, 0x021C, 0x0228, 0x0230, 0x023C, 0x0248, 0x0250,
	0x025C, 0x0268, 0x0274, 0x0280, 0x028C, 0x0298, 0x02A4, 0x02B0,
	0x02BC, 0x02C8, 0x02D4, 0x02E4, 0x02F0, 0x0300, 0x030C, 0x031C,
	0x0328, 0x0338, 0x0348, 0x0358, 0x0368, 0x0378, 0x0388, 0x0398,
	0x03A8, 0x03BC, 0x03CC, 0x03DC, 0x03F0, 0x0404, 0x0414, 0x0428,
	0x043C, 0x0450, 0x0464, 0x0478, 0x0490, 0x04A4, 0x04BC, 0x04D0,
	0x04E8, 0x0500, 0x0514, 0x052C, 0x0548, 0x0560, 0x0578, 0x0594,
	0x05AC, 0x05C8, 0x05E4, 0x0600, 0x061C, 0x0638, 0x0654, 0x0674,
	0x0690, 0x06B0, 0x06D0, 0x06F0, 0x0710, 0x0734, 0x0754, 0x0778,
	0x079C, 0x07C0, 0x07E4, 0x0808, 0x082C, 0x0854, 0x087C, 0x08A4,
	0x08CC, 0x08F8, 0x0920, 0x094C, 0x0978, 0x09A4, 0x09D0, 0x0A00,
};

static const short lpc_k1_tab[] =
{
	0x8100, 0x8240, 0x8340, 0x8480, 0x85C0, 0x8700, 0x8840, 0x89C0,
	0x8B40, 0x8CC0, 0x8E40, 0x9000, 0x91C0, 0x9380, 0x9580, 0x9740,
	0x9980, 0x9B80, 0x9D80, 0x9FC0, 0xA200, 0xA440, 0xA6C0, 0xA940,
	0xAB80, 0xAE00, 0xB0C0, 0xB380, 0xB640, 0xB900, 0xBC00, 0xBF40,
	0xC240, 0xC580, 0xC8C0, 0xCC40, 0xCFC0, 0xD380, 0xD780, 0xDB40,
	0xDF40, 0xE380, 0xE7C0, 0xEC00, 0xF040, 0xF4C0, 0xF9C0, 0xFEC0,
	0x0440, 0x09C0, 0x0F40, 0x1580, 0x1C80, 0x2380, 0x2AC0, 0x3280,
	0x3A80, 0x42C0, 0x4B80, 0x5400, 0x5C40, 0x6500, 0x6E00, 0x7880
};

static const short lpc_k2_tab[] =
{
	0x8A00, 0x9800, 0xA3C0, 0xADC0, 0xB480, 0xBA80, 0xC000, 0xC500,
	0xC9C0, 0xCE40, 0xD2C0, 0xD6C0, 0xDAC0, 0xDE80, 0xE200, 0xE5C0,
	0xE940, 0xECC0, 0xF000, 0xF340, 0xF680, 0xF9C0, 0xFD00, 0x0000,
	0x0340, 0x0640, 0x0940, 0x0C40, 0x0F40, 0x1280, 0x1580, 0x1880,
	0x1B80, 0x1E80, 0x2180, 0x24C0, 0x27C0, 0x2AC0, 0x2DC0, 0x30C0,
	0x3400, 0x3700, 0x3A40, 0x3D00, 0x4000, 0x4300, 0x4600, 0x4900,
	0x4C00, 0x4F40, 0x5240, 0x5540, 0x5840, 0x5B40, 0x5E00, 0x6100,
	0x63C0, 0x6680, 0x6940, 0x6C00, 0x6F00, 0x7200, 0x7640, 0x7C00 
};

static const short lpc_k3_tab[] =
{
	0x8B00, 0x9A00, 0xA200, 0xA900, 0xAF00, 0xB500, 0xBB00, 0xC000,
	0xC500, 0xCA00, 0xCF00, 0xD400, 0xD900, 0xDE00, 0xE200, 0xE700,
	0xEC00, 0xF100, 0xF600, 0xFB00, 0x0100, 0x0700, 0x0D00, 0x1400,
	0x1A00, 0x2200, 0x2900, 0x3200, 0x3B00, 0x4500, 0x5300, 0x6D00 
};

static const short lpc_k4_tab[] =
{
	0x9400, 0xB000, 0xC200, 0xCB00, 0xD300, 0xD900, 0xDF00, 0xE500,
	0xEA00, 0xEF00, 0xF400, 0xF900, 0xFE00, 0x0300, 0x0700, 0x0C00,
	0x1100, 0x1500, 0x1A00, 0x1F00, 0x2400, 0x2900, 0x2E00, 0x3300,
	0x3800, 0x3E00, 0x4400, 0x4B00, 0x5300, 0x5A00, 0x6400, 0x7400
};

static const short lpc_k5_tab[] =
{
	0xA300, 0xC500, 0xD400, 0xE000, 0xEA00, 0xF300, 0xFC00, 0x0400,
	0x0C00, 0x1500, 0x1E00, 0x2700, 0x3100, 0x3D00, 0x4C00, 0x6600
};

static const short lpc_k6_tab[] =
{
	0xAA00, 0xD700, 0xE700, 0xF200, 0xFC00, 0x0500, 0x0D00, 0x1400,
	0x1C00, 0x2400, 0x2D00, 0x3600, 0x4000, 0x4A00, 0x5500, 0x6A00
};

static const short lpc_k7_tab[] =
{
	0xA300, 0xC800, 0xD700, 0xE300, 0xED00, 0xF500, 0xFD00, 0x0500,
	0x0D00, 0x1400, 0x1D00, 0x2600, 0x3100, 0x3C00, 0x4B00, 0x6700
};

static const short lpc_k8_tab[] =
{
	0xC500, 0xE400, 0xF600, 0x0500, 0x1400, 0x2700, 0x3E00, 0x5800
};

static const short lpc_k9_tab[] =
{
	0xB900, 0xDC00, 0xEC00, 0xF900, 0x0400, 0x1000, 0x1F00, 0x4500
};

static const short lpc_k10_tab[] =
{
	0xC300, 0xE600, 0xF300, 0xFD00, 0x0600, 0x1100, 0x1E00, 0x4300
};

static const short lpc_excit[] =
{
	0x00A2, 0x00AF, 0x00BA, 0x00C2, 0x00C7, 0x00C9, 0x00CA, 0x00C6,
	0x00C2, 0x00BC, 0x00B5, 0x00AD, 0x00A5, 0x009E, 0x009A, 0x0095,
	0x0095, 0x0098, 0x009F, 0x00A8, 0x00B8, 0x00CA, 0x00E3, 0x00FE,
	0x011F, 0x0141, 0x0169, 0x0191, 0x01BD, 0x01E8, 0x0216, 0x0240,
	0x026C, 0x0292, 0x02B9, 0x02D9, 0x02F8, 0x030F, 0x0325, 0x0332,
	0x033F, 0x0343, 0x0347, 0x0345, 0x0345, 0x033F, 0x033D, 0x033A,
	0x033D, 0x0341, 0x034E, 0x035F, 0x037B, 0x03A0, 0x03D2, 0x040D,
	0x0457, 0x04AD, 0x0511, 0x0582, 0x0600, 0x068A, 0x071F, 0x07BD,
	0x0864, 0x0911, 0x09C1, 0x0A74, 0x0B26, 0x0BD5, 0x0C7F, 0x0D20,
	0x0DB7, 0x0E40, 0x0EBB, 0x0F24, 0x0F7A, 0x0FBC, 0x0FE9, 0x0FFF,
	0x0FFF, 0x0FE9, 0x0FBC, 0x0F7A, 0x0F24, 0x0EBB, 0x0E40, 0x0DB7,
	0x0D20, 0x0C7F, 0x0BD5, 0x0B26, 0x0A74, 0x09C1, 0x0911, 0x0864,
	0x07BD, 0x071F, 0x068A, 0x0600, 0x0582, 0x0511, 0x04AD, 0x0457,
	0x040D, 0x03D2, 0x03A0, 0x037B, 0x035F, 0x034E, 0x0341, 0x033D,
	0x033A, 0x033D, 0x033F, 0x0345, 0x0345, 0x0347, 0x0343, 0x033F,
	0x0332, 0x0325, 0x030F, 0x02F8, 0x02D9, 0x02B9, 0x0292, 0x026C,
	0x0240, 0x0216, 0x01E8, 0x01BD, 0x0191, 0x0169, 0x0141, 0x011F,
	0x00FE, 0x00E3, 0x00CA, 0x00B8, 0x00A8, 0x009F, 0x0098, 0x0095,
	0x0095, 0x009A, 0x009E, 0x00A5, 0x00AD, 0x00B5, 0x00BC, 0x00C2,
	0x00C6, 0x00CA, 0x00C9, 0x00C7, 0x00C2, 0x00BA, 0x00AF, 0x00A2
};

//------------------------------------------------------------------------------

static void get_bits_init(LPC_SYNTH* s, const unsigned char* bs, int len)
{
	s->in_stream = bs;
	s->bitstream_length = len;

	s->data_cache = 0;
	s->bits_left = 0;
}

static unsigned char byte_rev(unsigned char a)
{
	// 76543210
	a = (a >> 4) | (a << 4); // Swap in groups of 4

	// 32107654
	a = ((a & 0xCC) >> 2) | ((a & 0x33) << 2); // Swap in groups of 2

	// 10325476
	a = ((a & 0xAA) >> 1) | ((a & 0x55) << 1); // Swap bit pairs

	// 01234567
	return a;
}

// bits <= 8
static short get_nbits(LPC_SYNTH* s, short bits)
{
	unsigned short data;
	
	if (s->bits_left < bits)
	{
		s->data_cache <<= 8;
		
		if (s->bitstream_length < 1)
			return -1;
		
		s->data_cache |= byte_rev(*s->in_stream);
		s->in_stream++;
		s->bitstream_length--;
		s->bits_left += 8;
	}

	data = s->data_cache >> (s->bits_left - bits);
	data &= (1 << bits) - 1;
	s->bits_left -= bits;
	
	return (short)data;
}

//------------------------------------------------------------------------------

static int lpc_get_frame(LPC_SYNTH* s, LPC_FRAME* dst, LPC_FRAME* ref)
{
	int pitch_i;
	int energy_i;
	int repeat;

	energy_i = get_nbits(s, 4);
	energy_i &= 0xf;
	if (0 == energy_i) {
		// silent
		dst->energy = 0;
		dst->pitch = 0;
		memset(dst->k, 0, sizeof(ref->k));
		return 0;
	}

	if (15 == energy_i) // end of stream
		return -1;

	repeat = get_nbits(s, 1); // repeat
	pitch_i = get_nbits(s, 7); // pitch

	// coded -> uncoded
	dst->energy = lpc_gain_tab[energy_i];
	dst->pitch  = lpc_pitch_tab[pitch_i];

	if (repeat) {
		dst->k[0] = ref->k[0];
		dst->k[1] = ref->k[1];
		dst->k[2] = ref->k[2];
		dst->k[3] = ref->k[3];
	} else {
		dst->k[0] = lpc_k1_tab[get_nbits(s, 6)]; // K1
		dst->k[1] = lpc_k2_tab[get_nbits(s, 6)]; // K2
		dst->k[2] = lpc_k3_tab[get_nbits(s, 5)]; // K3
		dst->k[3] = lpc_k4_tab[get_nbits(s, 5)]; // K4
	}

	if (0 == pitch_i) {
		// lpc unvoiced frame
		dst->k[4] = 0;
		dst->k[5] = 0;
		dst->k[6] = 0;
		dst->k[7] = 0;
		dst->k[8] = 0;
		dst->k[9] = 0;
	} else {
		// voiced
		if (repeat) {
			memcpy(dst->k, ref->k, sizeof(ref->k));
		} else {
			dst->k[4] = lpc_k5_tab[get_nbits(s, 4)]; // K5
			dst->k[5] = lpc_k6_tab[get_nbits(s, 4)]; // K6
			dst->k[6] = lpc_k7_tab[get_nbits(s, 4)]; // K7
			dst->k[7] = lpc_k8_tab[get_nbits(s, 3)]; // K8
			dst->k[8] = lpc_k9_tab[get_nbits(s, 3)]; // K9
			dst->k[9] = lpc_k10_tab[get_nbits(s, 3)]; // K10
		}
	}

	return 0;
}

static void lpc_set_interp_flag(LPC_SYNTH* s)
{
	s->need_interp = 1;

	if (s->frame_prev.energy == 0) {
		if (s->frame_next.pitch == 0) {
			if (s->frame_next.energy)
				s->need_interp = 0;
		}
	} else {
		if (s->frame_prev.pitch) {
			if (s->frame_next.pitch == 0) {
				if (s->frame_next.energy)
					s->need_interp = 0;
			}
		} else {
			if (s->frame_next.pitch)
				s->need_interp = 0;
		}
	}
}

static void lpc_synth_init(LPC_SYNTH* s)
{
	memset(s, 0, sizeof(LPC_SYNTH));

	s->random_seed = 0xFFFF;

	lpc_get_frame(s, &s->frame_prev, &s->frame_prev);
	lpc_get_frame(s, &s->frame_next, &s->frame_prev);

	s->frame_curr = s->frame_prev;

	lpc_set_interp_flag(s);
}

static short lpc_reload_pitch(LPC_SYNTH* s)
{
	short curr_pitch;

	curr_pitch = s->curr_pitch;

	if (s->need_interp) {
		int i;
		int ratio;
		short* vector0;
		short* vector1;
		short* vector2;

		ratio = (s->sample_index << LPC_FRAC_BITS) / LPC_SAMPLES_PER_FRAME;

		vector0 = (short*)&s->frame_prev;
		vector1 = (short*)&s->frame_next;
		vector2 = (short*)&s->frame_curr;

		// do interp
		for (i = 0; i < sizeof(LPC_FRAME) / sizeof(short); i++)
		{
			short v;

			v = vector1[i] - vector0[i];
			v = (short)((v * ratio) >> LPC_FRAC_BITS);
			vector2[i] = v + vector0[i];
		}

		curr_pitch += s->frame_curr.pitch;
		if (curr_pitch > 0)
			return curr_pitch;
	}

	if (s->frame_curr.pitch != 0) {
		curr_pitch += s->frame_curr.pitch;
		return curr_pitch;
	}

	return 0x80;
}

#define LPC_CLAMP_P 27500 // to be fixed
#define LPC_CLAMP_N -(LPC_CLAMP_P + 1)

static void lpc_synth_do_filter(LPC_SYNTH* s)
{
	int i;
	int sample;
	short* x;
	short* k;

	x = s->x;
	k = s->frame_curr.k;
	sample = s->synth_out;

	for (i = 0; i < LPC_ORDER; i++) {
		sample -= (k[LPC_ORDER - 1 - i] * x[LPC_ORDER - 1 - i]) >> LPC_FRAC_BITS;

		if (sample > LPC_CLAMP_P)
			sample = LPC_CLAMP_P;
		else if (sample < LPC_CLAMP_N)
			sample = LPC_CLAMP_N;

		x[LPC_ORDER - i] = x[LPC_ORDER - 1 - i] +
			((sample * k[LPC_ORDER - 1 - i]) >> LPC_FRAC_BITS);
	}

	x[0] = s->synth_out = (short)sample;
}

static short random_gen(LPC_SYNTH* s)
{
	short r;
	short seed;

	r = 0;

	seed = s->random_seed << 1;

	r = ((seed >> 12) ^ (seed >> 13)) & 1;

	s->random_seed = seed | r;

	return r;
}

static short* lpc_synth_run(LPC_SYNTH* s, short* pcm_out, int* eos)
{
	int i;
	int excit;

	excit = 0;
	if (eos) *eos = 0;

	for (i = 0; i < LPC_SAMPLES_PER_FRAME; i++) {
		s->sample_index = i;
		s->curr_pitch -= 16;

		if (s->curr_pitch < 0)
			s->curr_pitch = lpc_reload_pitch(s);

		if (0 == s->frame_curr.pitch) {
			// unvoiced
			if (s->frame_curr.energy) {
				excit = random_gen(s) ? 1408 : -1408;
				excit = (excit * s->frame_curr.energy) >> LPC_FRAC_BITS;
			} else {
				// silent
				excit = 0;
			}
		} else if (s->curr_pitch >= 160) {
			excit = 0;
		} else {
			// lpc voiced excit
			excit = lpc_excit[s->curr_pitch];
			excit = (excit * s->frame_curr.energy) >> LPC_FRAC_BITS;
		}

		excit *= 8;

		// setup lpc
		s->synth_out = (short)excit;

		lpc_synth_do_filter(s);

		*pcm_out++ = s->synth_out;
	}

	// prepare for the next frame
	s->frame_prev = s->frame_next;
	s->frame_curr = s->frame_next;

	if (lpc_get_frame(s, &s->frame_next, &s->frame_prev)) {
		// end of stream
		if (eos) *eos = 1;
	}

	lpc_set_interp_flag(s);

	return pcm_out;
}

int lpc10_d6_synth(short* pcm, int* pcm_size, const unsigned char* bs, int len)
{
	LPC_SYNTH lpc;
	short* pcm_base;

	lpc_synth_init(&lpc);

	get_bits_init(&lpc, bs, len);

	pcm_base = pcm;

	for (;;) {
		int eos;

		pcm = lpc_synth_run(&lpc, pcm, &eos);
		if (eos) break;
	}

	*pcm_size = pcm - pcm_base;

	return 0;
}

/*******************************************************************************
                           E N D  O F  F I L E
*******************************************************************************/
