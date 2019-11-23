#include <stdio.h>
#include "glm/glm.hpp"	//	*glut.hより先に定義する必要がある
#include "glut.h"

#include "font.h"
#include "Rect.h"
#include "audio.h"

using namespace glm;

ivec2 windowSize = { 800,600 };	//	ウィンドウのサイズを定義

bool keys[256];		//	どのキーが押されているかを保持する

Rect field;

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

	//	ティーポットモデルに対する変換
	glMatrixMode(GL_MODELVIEW);		//	モデルビュー行列モードに切り替え
	glLoadIdentity();				//	前回の射影行列が残らないように行列の初期化

	glColor3ub(0xff, 0xff, 0xff);	//	枠の色
	glRectf(						//	枠を描画
		field.m_position.x - 8, 0,
		field.m_position.x + field.m_size.x + 8, windowSize.y);

	glColor3ub(0x00, 0x00, 0x00);	//	フィールドの色の指定
	field.draw();					//	フィールドの描画

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
	//glFlush();			//	シングルバッファの場合
}

//	アップデートみたいなもの
void idle(void)
{

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

	float frameHeight = 16;
	float frameSize = windowSize.y - frameHeight;	//	正方形のサイズ(y軸に合わせる)
	
	field.m_size = ivec2(frameSize,frameSize);		//	正方形のフィールド(四角形)のサイズを指定

	field.m_position = ivec2(			//	正方形のフィールド(四角形)の位置を指定
		(windowSize.x - field.m_size.x) / 2,
		frameHeight);
}

void keybord(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1b:			//	Escapeキーで終了
		exit(0);
		break;
	case 'a':
		glutReshapeWindow(windowSize.x + 1, windowSize.y);
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
	//glutMotionFunc(motion);				//	マウスがクリックされた状態の移動イベントを取得

	glutMainLoop();							//	処理をglutに委託する(コールバック系はこのメソッドより前に書く)


	return 0;
}