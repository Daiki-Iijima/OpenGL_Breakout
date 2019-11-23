#include <stdio.h>
#include <alc.h>
#include <al.h>
#include <time.h>

#include "audio.h"

#pragma comment(lib,"OpenAL32.lib")			//	openalのライブラリをインポート

#define DEFAULT_GAIN (.1f)	//	デフォルトの音量
#define DEFAULT_FREQ (440)	//	デフォルトの周波数

static ALuint sid;							//	音源ID
ALuint buffers[AUDIO_WAVEFORM_PULSE_MAX];	//	波形を保存するバッファ
static int waveform;

static unsigned int length;	//	音の長さ
static unsigned int start;	//	音が鳴り始めた時間
static float gain;			//	現在の音量
static float decay;			//	音の減衰率
static float sweep;			//	音のピッチ変化率

static float freqStart = DEFAULT_FREQ;	//	再生開始時の音階
static float freq;						//	現在の音階
static float freqEnd;					//	目標(上限下限)の音階




int audioInit()
{
	ALCdevice* device = alcOpenDevice(NULL);	//	デバイスを開いて、デバイスのポインタを取得
	if (device == NULL)
		return 1;								//	うまくいかなかった場合1

	ALCcontext* context = alcCreateContext(		//	コンテキストを作成
		device,									//	一つ上で取得したデバイス
		NULL);
	if (context == NULL)
		return 1;								//	うまくいかなかった場合1

	alcMakeContextCurrent(context);				//	現在のコンテキストを上で作成したコンテキストに設定


	alGenBuffers(								//	波形データを管理するバッファを作成
		AUDIO_WAVEFORM_PULSE_MAX,				//	バッファーを作成する数
		buffers);								//	バッファーのポインターを返す

	//	==========	パルス波を作成 ==========
	unsigned char pulse[][8] = {						//	パルス波を作成(8なのは8ビットで使うから)
		{0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00},		//	(1:7)12.5%のパルス波 (AUDIO_WAVEFORM_PULSE_12_5)
		{0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00},		//	(2:6)20%のパルス波 (AUDIO_WAVEFORM_PULSE_25)
		{0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00},		//	(4:4)50%のパルス波 (AUDIO_WAVEFORM_PULSE_50)
		{0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00},		//	(4:7)75%のパルス波 (AUDIO_WAVEFORM_PULSE_75)
	};

	for (int i = AUDIO_WAVEFORM_PULSE_12_5; i <= AUDIO_WAVEFORM_PULSE_75; i++)
	{
		ALsizei size = sizeof pulse[0];				//	pulse変数1つの配列の長さを取得
		alBufferData(								//	パルス波データのセット
			buffers[i],								//	波形を保存するバッファ
			AL_FORMAT_MONO8,						//	フォーマット
			pulse[i],								//	波形データ
			size,									//	波形データのサイズ
			size * 440);							//	周波数(440:ラの音)
	}
	//	=====================================


	//	==========	三角波を作成 ==========
	unsigned char triangle[] = {
		0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00,
		0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
	};

	alBufferData(								//	三角波データのセット
		buffers[AUDIO_WAVEFORM_TRIANGLE],		//	波形を保存するバッファ
		AL_FORMAT_MONO8,						//	フォーマット
		triangle,								//	波形データ
		sizeof triangle,						//	波形データのサイズ
		sizeof triangle * DEFAULT_FREQ);		//	周波数(音の高さ)

	//	===================================

	alGenSources(		//	音の再生をするためのソースを作成
		1,				//	ソースの数
		&sid);			//	ソースのIDアドレス

	alSourcei(			//	ループを許可する
		sid,			//	上で作ったsidを設定
		AL_LOOPING,		//	パラメーター(AL_LOOPING : ループするか)
		true);			//	ループを許可

	alSourcef(			//	音量を調整
		sid,			//	上で作ったsidを設定
		AL_GAIN,		//	パラメーター(AL_GAIN : 音量)
		.1f);			//	0.1f


	return 0;
}

void audioWaveform(int _waveform)
{
	waveform = _waveform;
}

void audioLength(unsigned int _millis)
{
	length = _millis;
}

void audioDecay(float _decay)
{
	decay = _decay;
}

void audioSweep(float _sweep, float _freqEnd)
{
	sweep = _sweep;
	freqEnd = _freqEnd;
}

void audioFreq(float _freq)
{
	freqStart = _freq;
}

void audioPlay()
{
	alSourcef(							//	音量を調整
		sid,							//	sid
		AL_GAIN,						//	パラメーター(AL_GAIN : 音量)
		gain = DEFAULT_GAIN);			//	デフォルト値を設定しつつ(0.1f)gainにも最初の値として保存

	freq = freqStart;					//	開始周波数を設定

	alSourcef(							//	ピッチを設定
		sid,							//	sid
		AL_PITCH,						//	パラメーター(AL_PITCH : ピッチ)
		freq / DEFAULT_FREQ);			//	周波数 / デフォルト周波数 = ピッチ

	alSourcei(							//	バッファーをセットする
		sid,							//	sid
		AL_BUFFER,						//	パラメーター(AL_BUFFER : バッファーを設定する)
		buffers[waveform]);				//	waveformの現在の番号のパルス波を設定

	alSourcePlay(sid);					//	再生
	start = clock();					//	再生した時刻を保存
}

void audioStop()
{
	alSourceStop(sid);	//	停止
}

void audioUpdate()
{
	if ((length > 0) && (clock() - start >= length))
	{
		audioStop();
	}

	if ((decay != 00) && (decay < 1))
	{
		alSourcef(							//	音量を調整
			sid,							//	上で作ったsidを設定
			AL_GAIN,						//	パラメーター(AL_GAIN : 音量)
			gain *= decay);					//	現在の音量から減衰率をかけた値を入れる
	}

	if (sweep != 0)
	{
		freq *= sweep;
		if (freqEnd != 0) {
			if (
				(sweep >= 1 && freq >= freqEnd) ||		//	高くなる場合
				(sweep <= 1 && freq <= freqEnd)			//	低くなる場合
				)
			{
				audioStop();
			}
		}
		alSourcef(							//	ピッチを設定
			sid,							//	sid
			AL_PITCH,						//	パラメーター(AL_PITCH : ピッチ)
			freq / DEFAULT_FREQ);			//	周波数 / デフォルト周波数 = ピッチ
	}
}