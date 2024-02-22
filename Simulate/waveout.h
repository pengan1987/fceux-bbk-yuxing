extern "C" {
	int wo_open(int sample_rate);
	int wo_write(short* pcm, int size);
	void wo_reset();
	int wo_init(int sample_rate, int channels, int sample_size);
	void audio_pause();
	void audio_resume();
}
int lpc10_d6_synth(short* pcm, int* pcm_size, const unsigned char* bs, int len);
