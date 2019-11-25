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
#define BLOCK_COULUM_MAX	 14			//	列
#define BLOCK_ROW_MAX		 8			//	行
#define BALL_X_SPEED_MAX	 8			//	ボールがパドルに当たった時のX方向の最大スピード


ivec2 windowSize = { 800,600 };	//	ウィンドウのサイズを定義

bool keys[256];		//	どのキーが押されているかを保持する

Rect field;

Ball ball = { 8 };		//	8は半径の長さ

Paddle paddle = { PADDLE_DEFAULT_WIDTH };

Rect blocks[BLOCK_ROW_MAX][BLOCK_COULUM_MAX];

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

	glColor3ub(0xff, 0xff, 0xff);	//	ボールの色を指定
	ball.draw();

	glColor3ub(0x00, 0xff, 0xff);	//	パドルの色を指定
	paddle.draw();					//	パドルの描画

	unsigned char colors[][3] = {
		{0xff,0x00,0x00},
		{0x00,0x80,0x00},
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
	fontBegin();
	fontSetHeight(FONT_DEFAULT_HEIGHT);

	float y = fontGetWeight();
	fontSetPosition(windowSize.x / 2 + windowSize.x / 2, y);
	fontSetWeight(fontGetWeightMax());
	fontDraw("");
	fontEnd();
	//	=====================================

	glutSwapBuffers();	//	ダブルバッファの表と裏を切り替える(スワップする)
}

//	アップデートみたいなもの
void idle(void)
{
	ball.update();

	//	===	ボールの当たり判定 ===
	if (
		(ball.m_position.y >= field.m_position.y + field.m_size.y) ||
		(ball.m_position.y < field.m_position.y))//	上下端
	{
		ball.m_position = ball.m_lastposition;
		ball.m_speed.y *= -1;
	}

	if ((ball.m_position.x >= field.m_position.x + field.m_size.x) ||
		(ball.m_position.x < field.m_position.x))//	左右端
	{
		ball.m_position = ball.m_lastposition;
		ball.m_speed.x *= -1;
	}
	//	===========================

	//	===== パドル当たり判定 =====

	if (paddle.intersectBall(ball))
	{
		ball.m_position = ball.m_lastposition;
		ball.m_speed.y *= -1;

		float paddleCenterX = paddle.m_position.x + paddle.m_width / 2;
		float sub = ball.m_position.x - paddleCenterX;
		float subMax = paddle.m_width / 2;

		ball.m_speed.x = sub / subMax * BALL_X_SPEED_MAX;

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
				blocks[i][j].isDead = true;					//	当たったブロックを表示しないように死亡フラグを立てる

				ball.m_position = ball.m_lastposition;		//	反射
				ball.m_speed.y *= -1;
			}
		}
	}
	//	======================================
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

	ball.m_speed = vec2(1, 1) * 4.0f;			//	ボールのスピード

	//	======================

	//	=== パドルの初期化 ===
	paddle.m_position = vec2(
		field.m_position.x + field.m_size.x / 2,
		field.m_position.y + field.m_size.y - 48.f);
	//	======================

	//	=== ブロックの初期化 ===
	vec2 blockSize = vec2(field.m_size.x / BLOCK_COULUM_MAX, 12);

	float y = field.m_position.y + 64.f;
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

int main(int argc, char *argv[])
{
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

	reshape(windowSize.x, windowSize.y);	//	初期化のために強制的に一回呼ぶ

	glutMainLoop();							//	処理をglutに委託する(コールバック系はこのメソッドより前に書く)


	return 0;
}