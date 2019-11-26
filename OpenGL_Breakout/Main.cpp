#include <stdio.h>
#include "glm/glm.hpp"	//	*glut.hより先に定義する必要がある
#include "glut.h"

#include "font.h"
#include "Rect.h"
#include "audio.h"
#include "Ball.h"
#include "Paddle.h"

using namespace glm;

#define PADDLE_DEFAULT_WIDTH 48			//	パドルの幅

#define BLOCK_HEIGHT		 12			//	1ブロックの高さ
#define BLOCK_COULUM_MAX	 14			//	列
#define BLOCK_ROW_MAX		 8			//	行

#define BALL_X_SPEED_MAX	 2			//	ボールがパドルに当たった時のX方向の最大スピード

#define FONT_HEIGHT			 32			//	フォントの高さ
#define FONT_WEIGHT			 4			//	フォント太さ

#define SE_WAIT_MAX			 6			//	SEの最大待ち回数

#define TURN_MAX			 3			//	残機数の最大値
enum
{
	LEVEL_DEFAULT,
	LEVEL_HIT_4,
	LEVEL_HIT_12,
	LEVEL_HIT_ORANGE,
	LEVEL_HIT_RED,
	LEVEL_HIT_MAX,
};

ivec2 windowSize = { 800,600 };	//	ウィンドウのサイズを定義

bool keys[256];		//	どのキーが押されているかを保持する

Rect field;

Ball ball = { 8 };		//	8は半径の長さ

Paddle paddle = { PADDLE_DEFAULT_WIDTH };

Rect blocks[BLOCK_ROW_MAX][BLOCK_COULUM_MAX];

int turn = 1;
int score;

int seCount;
int seWait;		//	次のSEを鳴らすまでの待機時間

int level;		//	現在のレベル

bool started;	//	ゲームの開始フラグ

int wait;		//	ゲーム開始からボールが動くまでの待機時間

float powerTbl[] =
{
	3,		//	LEVEL_DEFAULT
	4,		//	LEVEL_HIT_4
	5,		//	LEVEL_HIT_12
	6,		//	LEVEL_HIT_ORANGE
	7,		//	LEVEL_HIT_RED
};

//	残りのブロックの個数を数える
int getBlockCount()
{
	int n = 0;

	for (int i = 0; i < BLOCK_ROW_MAX; i++)
	{
		for (int j = 0; j < BLOCK_COULUM_MAX; j++)
		{
			if (!blocks[i][j].isDead)
				n++;
		}
	}

	return n;
}

void gameOver()
{
	started = false;
	paddle.m_width = field.m_size.x;			//	パドルをフィールドと同じ幅に設定
	paddle.m_position.x = field.m_position.x;	//	位置をフィールド内に指定


	//	===	ボール初期設定 ===
	ball.m_lastposition.y =						//	ボールの初期位置はy座標だけ初期化
		ball.m_position.y = field.m_position.y + field.m_size.y / 2;

	ball.m_speed = vec2(1, 1);			//	ボールのスピード
	ball.m_power = 1;
	//	======================

	level = LEVEL_DEFAULT;
}

//	描画が必要になったら
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);	//	色情報をリセットする

	//	アスペクト比が変わらないようにする
	glMatrixMode(GL_PROJECTION);	//	射影モードを変更する
	glLoadIdentity();				//	前回の射影行列が残らないように行列の初期化
	gluOrtho2D(						//	2次元空間を定義(Ortho:正射影)
		0, windowSize.x,			//	left,right
		windowSize.y, 0				//	bottom,top
	);

	glMatrixMode(GL_MODELVIEW);		//	モデルビュー行列モードに切り替え
	glLoadIdentity();				//	前回の射影行列が残らないように行列の初期化

	glColor3ub(0xff, 0xff, 0xff);	//	枠の色
	glRectf(						//	枠を描画
		field.m_position.x - 8, 0,
		field.m_position.x + field.m_size.x + 8, windowSize.y);

	glColor3ub(0x00, 0x00, 0x00);	//	フィールドの色の指定
	field.draw();					//	フィールドの描画

	if (wait <= 0)
	{
		glColor3ub(0xff, 0xff, 0xff);	//	ボールの色を指定
		ball.draw();
	}

	glColor3ub(0x00, 0xff, 0xff);	//	パドルの色を指定
	paddle.draw();					//	パドルの描画

	unsigned char colors[][3] = {
		{0xff,0x00,0x00},
		{0xff,0x80,0x00},
		{0x00,0xff,0x00},
		{0xff,0xff,0x00}
	};

	for (int i = 0; i < BLOCK_ROW_MAX; i++)
	{
		for (int j = 0; j < BLOCK_COULUM_MAX; j++)
		{
			if (blocks[i][j].isDead)
				continue;

			int colorIdx = i / 2;						//	色を2行ごとに変える
			unsigned char *color = colors[colorIdx];	//	色配列から色を選択
			glColor3ub(color[0], color[1], color[2]);	//	色配列から取得した色を指定

			glRectfv(									//	描画(drawを使わないのはブロックごとの隙間を1ピクセル分開けたいから)	
				(GLfloat*)&(blocks[i][j].m_position + vec2(1, 1)),
				(GLfloat*)&(blocks[i][j].m_position + blocks[i][j].m_size - vec2(1, 1))
			);
		}
	}


	//	======= 文字列の描画(font.cpp) ======
	{
		glColor3ub(0xff, 0xff, 0xff);	//	文字の色を指定

		fontBegin();
		{
			fontSetHeight(FONT_HEIGHT);
			fontSetWeight(FONT_WEIGHT);

			fvec2 pos = field.m_position;

			//	== 1p ==
			fontSetPosition(pos.x, pos.y);
			fontDraw("%d", turn);

			//	== 2p ==
			fontSetPosition(pos.x + field.m_size.x / 2, pos.y);
			fontDraw("1");

			//	== 1p ==
			pos.y += fontGetHeight() + fontGetWeight();	//	改行した高さを計算
			pos.x += fontGetWidth();					//	1文字分ずらす
			fontSetPosition(pos.x, pos.y);				//	位置設定
			{
				static unsigned int frame;
				if (started && (++frame / 10) % 2 == 0)	//	点滅処理
					;
				else
					fontDraw("%03d", score);			//	スコアの描画
			}

			//	== 2p ==
			fontSetPosition(pos.x + field.m_size.x / 2, pos.y);		//	2p用のスコア描画位置を設定
			fontDraw("000");										//	2p用のスコアの描画

			pos.y += fontGetHeight() + fontGetWeight();

			//	デバッグ表示
			fontSetHeight(16);
			fontSetWeight(2);

			pos.y += BLOCK_HEIGHT * BLOCK_ROW_MAX;
			pos.x = field.m_position.x;
			fontSetPosition(pos.x, pos.y);				//	位置設定
			fontDraw("seCount:%d\n", seCount);
			fontDraw("seWait:%d\n", seWait);
			fontDraw("level:%d\n", level);
			fontDraw("blockCount:%d\n", getBlockCount());

		}
		fontEnd();
	}
	//	=====================================

	glutSwapBuffers();	//	ダブルバッファの表と裏を切り替える(スワップする)
}

//	アップデートみたいなもの
void idle(void)
{
	if (started && seCount > 0)
	{
		if (--seWait <= 0)
		{
			seCount--;
			seWait = SE_WAIT_MAX;

			audioStop();
			audioFreq(440 / 2);
			audioPlay();

		}
	}
	if (wait <= 0) {
		ball.m_power = powerTbl[level];	//	ボールのレベルを設定

		ball.update();

		//	===	ボールの当たり判定 ===
		if ((ball.m_position.y < field.m_position.y))//	上端
		{
			ball.m_position = ball.m_lastposition;
			ball.m_speed.y *= -1;

			if (started)
			{
				audioStop();
				audioFreq(440);
				audioPlay();
			}
		}

		if (ball.m_position.y >= field.m_position.y + field.m_size.y)	//	下端(1機失ったとき)
		{
			turn++;
			wait = 60 * 3;

			level = LEVEL_DEFAULT;

			//	===	ボール初期設定 ===
			ball.m_lastposition.y =						//	ボールの初期位置はy座標だけ初期化
				ball.m_position.y = field.m_position.y + field.m_size.y / 2;

			ball.m_speed = vec2(1, 1);					//	ボールのスピード
			ball.m_power = 1;
			//	======================
		}

		if ((ball.m_position.x >= field.m_position.x + field.m_size.x) ||
			(ball.m_position.x < field.m_position.x))//	左右端
		{
			ball.m_position = ball.m_lastposition;
			ball.m_speed.x *= -1;

			if (started)
			{
				audioStop();
				audioFreq(440);
				audioPlay();
			}
		}
		//	===========================

		//	===== パドル当たり判定 =====

		if (paddle.intersectBall(ball))
		{
			ball.m_position = ball.m_lastposition;
			ball.m_speed.y *= -1;

			if (started)
			{
				//	=== ボールの反射角度変化 ===
				float paddleCenterX = paddle.m_position.x + paddle.m_width / 2;
				float sub = ball.m_position.x - paddleCenterX;
				float subMax = paddle.m_width / 2;
				ball.m_speed.x = sub / subMax * BALL_X_SPEED_MAX;
				//	============================

				audioStop();
				audioFreq(440 * 2);
				audioPlay();
			}
		}

		//	============================


		//	======= ブロックの当たり判定 =========
		for (int i = 0; i < BLOCK_ROW_MAX; i++)
		{
			for (int j = 0; j < BLOCK_COULUM_MAX; j++)
			{
				if (blocks[i][j].isDead)						//	死亡フラグが立っていたら抜ける
					continue;

				if (blocks[i][j].intersect(ball.m_position))	//	ボールトの当たり判定
				{
					if (started)
					{
						blocks[i][j].isDead = true;					//	当たったブロックを表示しないように死亡フラグを立てる

						audioStop();
						audioFreq(440 / 2);
						audioPlay();

						//	==== スコア計算 ====

						int colorIdx = 3 - (i / 2);					//	色のインデックスを逆に数えるために3から引く

						int s = 1 + 2 * colorIdx;					//	獲得できるスコアの計算

						seCount += s - 1;							//	上で一回鳴らしているから1引く
						seWait = SE_WAIT_MAX;

						score += s;

						//	====================

						//	=======	レベルアップ処理 =========
						{
							int n = getBlockCount();
							int blockCountMax = BLOCK_COULUM_MAX * BLOCK_ROW_MAX;

							if ((n <= blockCountMax - 4) && (level < LEVEL_HIT_4))		//	4個のブロックを消していたら
								level = LEVEL_HIT_4;
							if ((n <= blockCountMax - 12) && (level < LEVEL_HIT_12))	//	12個のブロックを消していたら
								level = LEVEL_HIT_12;
							if ((colorIdx == 2) && (level < LEVEL_HIT_ORANGE))			//	オレンジ色のブロックを消していたら
								level = LEVEL_HIT_ORANGE;
							if ((colorIdx == 3) && (level < LEVEL_HIT_RED))				//	赤色のブロックを消していたら
								level = LEVEL_HIT_RED;
						}
						//	==================================
					}

					ball.m_position = ball.m_lastposition;		//	反射
					ball.m_speed.y *= -1;
				}
			}
		}
		//	======================================
	}
	else
	{
		wait--;
		if (wait <= 0)
		{
			if (turn > TURN_MAX)		//	ゲームオーバー
				gameOver();
		}
	}

	audioUpdate();

	glutPostRedisplay();	//	再描画命令
}

//	ウィンドウサイズが変更されたときに呼ぶ
void reshape(int width, int height)
{
	printf("reshape: width:%d height:%d\n", width, height);

	glViewport(			//	ビューポートを更新(更新しないと指定サイズより大きくなった時に表示できずに切れてしまう)
		0, 0,			//	座標(x,y)
		width, height	//	サイズ(w,h)
	);

	windowSize = ivec2(width, height);	//	リサイズされた値でサイズ定数を書き換える


	//	=== フィールド初期設定 ===
	float frameHeight = 16;
	float frameSize = windowSize.y - frameHeight;	//	正方形のサイズ(y軸に合わせる)

	field.m_size = ivec2(frameSize, frameSize);		//	正方形のフィールド(四角形)のサイズを指定

	field.m_position = ivec2(			//	正方形のフィールド(四角形)の位置を指定
		(windowSize.x - field.m_size.x) / 2,
		frameHeight);
	//	==========================

	//	===	ボール初期設定 ===
	ball.m_lastposition =						//	ボールの初期位置
		ball.m_position = vec2(field.m_position.x, field.m_position.y + field.m_size.y / 2);

	ball.m_speed = vec2(1, 1);			//	ボールのスピード
	ball.m_power = 1;

	//	======================

	//	=== パドルの初期化 ===
	paddle.m_position = vec2(
		field.m_position.x + field.m_size.x / 2,
		field.m_position.y + field.m_size.y - 48.f);
	//	======================

	//	=== ブロックの初期化 ===
	vec2 blockSize = vec2(field.m_size.x / BLOCK_COULUM_MAX, BLOCK_HEIGHT);

	float y = field.m_position.y + (FONT_HEIGHT + FONT_WEIGHT) * 2;	//	文字2行分下にずらす
	for (int i = 0; i < BLOCK_ROW_MAX; i++)
	{
		for (int j = 0; j < BLOCK_COULUM_MAX; j++)
		{
			blocks[i][j].m_position = vec2(						//	位置を指定
				field.m_position.x + field.m_size.x * j / BLOCK_COULUM_MAX,
				y + blockSize.y * i
			);

			blocks[i][j].m_size = blockSize;					//	大きさを指定
		}
	}
	//	=======================
}

void keybord(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1b:			//	Escapeキーで終了
		exit(0);
		break;
	}

	keys[key] = true;	//	キーが押された
}

void keybordUp(unsigned char key, int x, int y)
{
	keys[key] = false;	//	キーが離された
}

void passiveMotion(int _x, int _y)
{
	printf("passiveMotion: x%d:y%d \n", _x, _y);

	//	===	パドルをマウスで操作できるように ===
	paddle.m_position.x = _x;
	paddle.m_position.x = max(paddle.m_position.x, field.m_position.x);
	paddle.m_position.x = min(paddle.m_position.x, field.m_position.x + field.m_size.x - paddle.m_width);
	// =========================================
}

void mouse(int button, int state, int x, int y)
{
	if ((!started) && (state == GLUT_DOWN))
	{
		//	===	ボール初期設定 ===
		ball.m_lastposition.y =						//	ボールの初期位置はy座標だけ初期化
			ball.m_position.y = field.m_position.y + field.m_size.y / 2;

		ball.m_speed = vec2(1, 1);					//	ボールのスピード
		ball.m_power = 1;
		//	======================

		started = true;

		for (int i = 0; i < BLOCK_ROW_MAX; i++)
		{
			for (int j = 0; j < BLOCK_COULUM_MAX; j++)
			{
				blocks[i][j].isDead = false;
			}
		}

		turn = 1;
		score = 0;

		level = LEVEL_DEFAULT;
		paddle.m_width = PADDLE_DEFAULT_WIDTH;

		wait = 60.f * 3;		//	待機時間を設定
	}
}

int main(int argc, char *argv[])
{
	audioInit();
	audioWaveform(AUDIO_WAVEFORM_PULSE_50);
	audioDecay(.9f);

	glutInit(&argc, argv);

	glutInitDisplayMode(GL_DOUBLE);			//	ダブルバッファを使用する(やらない場合シングルバッファ)

	glutInitWindowPosition(640, 0);			//	Window位置(やらなくてもいい)
	glutInitWindowSize(windowSize.x, windowSize.y);			//	Window大きさ(やらなくてもいい)

	glutCreateWindow("†Breakout†");		//	Windowのタイトル設定
	glutDisplayFunc(display);				//	描画が必要になったら呼ばれるコールバックの設定

	glutIdleFunc(idle);						//	GLUTの手が空いた時に呼ばれるコールバックの設定

	glutReshapeFunc(reshape);				//	Windowのサイズが変わったら呼ばれるコールバックの設定
	glutKeyboardFunc(keybord);				//	キーボードイベントを取得
	glutIgnoreKeyRepeat(GL_TRUE);			//	キーボードの押しっぱなし状態を無効にすることをTrueにする
	glutKeyboardUpFunc(keybordUp);			//	キーボードが離されたときイベント

	glutPassiveMotionFunc(passiveMotion);	//	マウスの移動イベントを取得
	glutMouseFunc(mouse);						//	マウスのクリックイベントを取得
	reshape(windowSize.x, windowSize.y);	//	初期化のために強制的に一回呼ぶ

	gameOver();								//	最初はデモ画面(ゲームオーバー画面)

	glutMainLoop();							//	処理をglutに委託する(コールバック系はこのメソッドより前に書く)


	return 0;
}