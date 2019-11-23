#pragma once

enum {
	AUDIO_WAVEFORM_PULSE_12_5,
	AUDIO_WAVEFORM_PULSE_25,
	AUDIO_WAVEFORM_PULSE_50,
	AUDIO_WAVEFORM_PULSE_75,
	AUDIO_WAVEFORM_TRIANGLE,
	AUDIO_WAVEFORM_PULSE_MAX,
};

int audioInit();
void audioWaveform(int _waveform);
void audioLength(unsigned int _millis);
void audioDecay(float _decay);
void audioSweep(float _sweep, float _freqEnd = 0);
void audioFreq(float _freq);
void audioPlay();
void audioStop();
void audioUpdate();