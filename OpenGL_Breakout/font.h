#pragma once

#define FONT_DEFAULT_HEIGHT (100.f)				//	フォント高さ(OpenGLのストロークフォントのサイズが100)
#define FONT_DEFAULT_WIDTH (100.f)				//	フォント幅

//	わかりやすいようにfontを枕につける

void fontBegin();	//	フォント描画開始
void fontEnd();		//	フォント描画終了

void fontSetPosition(float _x, float _y);		//	フォント位置
void fontSetHeight(float _size);				//	フォントの高さを設定
float fontGetHeight();							//	フォントの高さを取得
float fontGetWidth();							//	フォントの幅を取得

float fontGetWeightMin();						//	設定できるフォントの一番細い太さ
float fontGetWeightMax();						//	設定できるフォントの一番太い太さ

void fontSetWeight(float _weight);				//	フォントの太さを変更
float fontGetWeight();							//	フォントの太さを取得


//void fontSetColor(unsigned char _red, unsigned char _green, unsigned char _blue);	//フォントカラー
void fontDraw(const char *_format, ...);		//	フォントを描画する