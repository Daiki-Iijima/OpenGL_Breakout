#include <stdio.h>
#include "glm/glm.hpp"	//	*glut.h����ɒ�`����K�v������
#include "glut.h"

#include "font.h"
#include "Rect.h"
#include "audio.h"
#include "Ball.h"
#include "Paddle.h"

using namespace glm;

#define PADDLE_DEFAULT_WIDTH 48			//	�p�h���̕�

#define BLOCK_HEIGHT		 12			//	1�u���b�N�̍���
#define BLOCK_COULUM_MAX	 14			//	��
#define BLOCK_ROW_MAX		 8			//	�s

#define BALL_X_SPEED_MAX	 2			//	�{�[�����p�h���ɓ�����������X�����̍ő�X�s�[�h

#define FONT_HEIGHT			 32			//	�t�H���g�̍���
#define FONT_WEIGHT			 4			//	�t�H���g����

#define SE_WAIT_MAX			 6			//	SE�̍ő�҂���

#define TURN_MAX			 3			//	�c�@���̍ő�l
enum
{
	LEVEL_DEFAULT,
	LEVEL_HIT_4,
	LEVEL_HIT_12,
	LEVEL_HIT_ORANGE,
	LEVEL_HIT_RED,
	LEVEL_HIT_MAX,
};

ivec2 windowSize = { 800,600 };	//	�E�B���h�E�̃T�C�Y���`

bool keys[256];		//	�ǂ̃L�[��������Ă��邩��ێ�����

Rect field;

Ball ball = { 8 };		//	8�͔��a�̒���

Paddle paddle = { PADDLE_DEFAULT_WIDTH };

Rect blocks[BLOCK_ROW_MAX][BLOCK_COULUM_MAX];

int turn = 1;
int score;

int seCount;
int seWait;		//	����SE��炷�܂ł̑ҋ@����

int level;		//	���݂̃��x��

bool started;	//	�Q�[���̊J�n�t���O

int wait;		//	�Q�[���J�n����{�[���������܂ł̑ҋ@����

float powerTbl[] =
{
	3,		//	LEVEL_DEFAULT
	4,		//	LEVEL_HIT_4
	5,		//	LEVEL_HIT_12
	6,		//	LEVEL_HIT_ORANGE
	7,		//	LEVEL_HIT_RED
};

//	�c��̃u���b�N�̌��𐔂���
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
	paddle.m_width = field.m_size.x;			//	�p�h�����t�B�[���h�Ɠ������ɐݒ�
	paddle.m_position.x = field.m_position.x;	//	�ʒu���t�B�[���h���Ɏw��


	//	===	�{�[�������ݒ� ===
	ball.m_lastposition.y =						//	�{�[���̏����ʒu��y���W����������
		ball.m_position.y = field.m_position.y + field.m_size.y / 2;

	ball.m_speed = vec2(1, 1);			//	�{�[���̃X�s�[�h
	ball.m_power = 1;
	//	======================

	level = LEVEL_DEFAULT;
}

//	�`�悪�K�v�ɂȂ�����
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);	//	�F�������Z�b�g����

	//	�A�X�y�N�g�䂪�ς��Ȃ��悤�ɂ���
	glMatrixMode(GL_PROJECTION);	//	�ˉe���[�h��ύX����
	glLoadIdentity();				//	�O��̎ˉe�s�񂪎c��Ȃ��悤�ɍs��̏�����
	gluOrtho2D(						//	2������Ԃ��`(Ortho:���ˉe)
		0, windowSize.x,			//	left,right
		windowSize.y, 0				//	bottom,top
	);

	glMatrixMode(GL_MODELVIEW);		//	���f���r���[�s�񃂁[�h�ɐ؂�ւ�
	glLoadIdentity();				//	�O��̎ˉe�s�񂪎c��Ȃ��悤�ɍs��̏�����

	glColor3ub(0xff, 0xff, 0xff);	//	�g�̐F
	glRectf(						//	�g��`��
		field.m_position.x - 8, 0,
		field.m_position.x + field.m_size.x + 8, windowSize.y);

	glColor3ub(0x00, 0x00, 0x00);	//	�t�B�[���h�̐F�̎w��
	field.draw();					//	�t�B�[���h�̕`��

	if (wait <= 0)
	{
		glColor3ub(0xff, 0xff, 0xff);	//	�{�[���̐F���w��
		ball.draw();
	}

	glColor3ub(0x00, 0xff, 0xff);	//	�p�h���̐F���w��
	paddle.draw();					//	�p�h���̕`��

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

			int colorIdx = i / 2;						//	�F��2�s���Ƃɕς���
			unsigned char *color = colors[colorIdx];	//	�F�z�񂩂�F��I��
			glColor3ub(color[0], color[1], color[2]);	//	�F�z�񂩂�擾�����F���w��

			glRectfv(									//	�`��(draw���g��Ȃ��̂̓u���b�N���Ƃ̌��Ԃ�1�s�N�Z�����J����������)	
				(GLfloat*)&(blocks[i][j].m_position + vec2(1, 1)),
				(GLfloat*)&(blocks[i][j].m_position + blocks[i][j].m_size - vec2(1, 1))
			);
		}
	}


	//	======= ������̕`��(font.cpp) ======
	{
		glColor3ub(0xff, 0xff, 0xff);	//	�����̐F���w��

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
			pos.y += fontGetHeight() + fontGetWeight();	//	���s�����������v�Z
			pos.x += fontGetWidth();					//	1���������炷
			fontSetPosition(pos.x, pos.y);				//	�ʒu�ݒ�
			{
				static unsigned int frame;
				if (started && (++frame / 10) % 2 == 0)	//	�_�ŏ���
					;
				else
					fontDraw("%03d", score);			//	�X�R�A�̕`��
			}

			//	== 2p ==
			fontSetPosition(pos.x + field.m_size.x / 2, pos.y);		//	2p�p�̃X�R�A�`��ʒu��ݒ�
			fontDraw("000");										//	2p�p�̃X�R�A�̕`��

			pos.y += fontGetHeight() + fontGetWeight();

			//	�f�o�b�O�\��
			fontSetHeight(16);
			fontSetWeight(2);

			pos.y += BLOCK_HEIGHT * BLOCK_ROW_MAX;
			pos.x = field.m_position.x;
			fontSetPosition(pos.x, pos.y);				//	�ʒu�ݒ�
			fontDraw("seCount:%d\n", seCount);
			fontDraw("seWait:%d\n", seWait);
			fontDraw("level:%d\n", level);
			fontDraw("blockCount:%d\n", getBlockCount());

		}
		fontEnd();
	}
	//	=====================================

	glutSwapBuffers();	//	�_�u���o�b�t�@�̕\�Ɨ���؂�ւ���(�X���b�v����)
}

//	�A�b�v�f�[�g�݂����Ȃ���
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
		ball.m_power = powerTbl[level];	//	�{�[���̃��x����ݒ�

		ball.update();

		//	===	�{�[���̓����蔻�� ===
		if ((ball.m_position.y < field.m_position.y))//	��[
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

		if (ball.m_position.y >= field.m_position.y + field.m_size.y)	//	���[(1�@�������Ƃ�)
		{
			turn++;
			wait = 60 * 3;

			level = LEVEL_DEFAULT;

			//	===	�{�[�������ݒ� ===
			ball.m_lastposition.y =						//	�{�[���̏����ʒu��y���W����������
				ball.m_position.y = field.m_position.y + field.m_size.y / 2;

			ball.m_speed = vec2(1, 1);					//	�{�[���̃X�s�[�h
			ball.m_power = 1;
			//	======================
		}

		if ((ball.m_position.x >= field.m_position.x + field.m_size.x) ||
			(ball.m_position.x < field.m_position.x))//	���E�[
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

		//	===== �p�h�������蔻�� =====

		if (paddle.intersectBall(ball))
		{
			ball.m_position = ball.m_lastposition;
			ball.m_speed.y *= -1;

			if (started)
			{
				//	=== �{�[���̔��ˊp�x�ω� ===
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


		//	======= �u���b�N�̓����蔻�� =========
		for (int i = 0; i < BLOCK_ROW_MAX; i++)
		{
			for (int j = 0; j < BLOCK_COULUM_MAX; j++)
			{
				if (blocks[i][j].isDead)						//	���S�t���O�������Ă����甲����
					continue;

				if (blocks[i][j].intersect(ball.m_position))	//	�{�[���g�̓����蔻��
				{
					if (started)
					{
						blocks[i][j].isDead = true;					//	���������u���b�N��\�����Ȃ��悤�Ɏ��S�t���O�𗧂Ă�

						audioStop();
						audioFreq(440 / 2);
						audioPlay();

						//	==== �X�R�A�v�Z ====

						int colorIdx = 3 - (i / 2);					//	�F�̃C���f�b�N�X���t�ɐ����邽�߂�3�������

						int s = 1 + 2 * colorIdx;					//	�l���ł���X�R�A�̌v�Z

						seCount += s - 1;							//	��ň��炵�Ă��邩��1����
						seWait = SE_WAIT_MAX;

						score += s;

						//	====================

						//	=======	���x���A�b�v���� =========
						{
							int n = getBlockCount();
							int blockCountMax = BLOCK_COULUM_MAX * BLOCK_ROW_MAX;

							if ((n <= blockCountMax - 4) && (level < LEVEL_HIT_4))		//	4�̃u���b�N�������Ă�����
								level = LEVEL_HIT_4;
							if ((n <= blockCountMax - 12) && (level < LEVEL_HIT_12))	//	12�̃u���b�N�������Ă�����
								level = LEVEL_HIT_12;
							if ((colorIdx == 2) && (level < LEVEL_HIT_ORANGE))			//	�I�����W�F�̃u���b�N�������Ă�����
								level = LEVEL_HIT_ORANGE;
							if ((colorIdx == 3) && (level < LEVEL_HIT_RED))				//	�ԐF�̃u���b�N�������Ă�����
								level = LEVEL_HIT_RED;
						}
						//	==================================
					}

					ball.m_position = ball.m_lastposition;		//	����
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
			if (turn > TURN_MAX)		//	�Q�[���I�[�o�[
				gameOver();
		}
	}

	audioUpdate();

	glutPostRedisplay();	//	�ĕ`�施��
}

//	�E�B���h�E�T�C�Y���ύX���ꂽ�Ƃ��ɌĂ�
void reshape(int width, int height)
{
	printf("reshape: width:%d height:%d\n", width, height);

	glViewport(			//	�r���[�|�[�g���X�V(�X�V���Ȃ��Ǝw��T�C�Y���傫���Ȃ������ɕ\���ł����ɐ؂�Ă��܂�)
		0, 0,			//	���W(x,y)
		width, height	//	�T�C�Y(w,h)
	);

	windowSize = ivec2(width, height);	//	���T�C�Y���ꂽ�l�ŃT�C�Y�萔������������


	//	=== �t�B�[���h�����ݒ� ===
	float frameHeight = 16;
	float frameSize = windowSize.y - frameHeight;	//	�����`�̃T�C�Y(y���ɍ��킹��)

	field.m_size = ivec2(frameSize, frameSize);		//	�����`�̃t�B�[���h(�l�p�`)�̃T�C�Y���w��

	field.m_position = ivec2(			//	�����`�̃t�B�[���h(�l�p�`)�̈ʒu���w��
		(windowSize.x - field.m_size.x) / 2,
		frameHeight);
	//	==========================

	//	===	�{�[�������ݒ� ===
	ball.m_lastposition =						//	�{�[���̏����ʒu
		ball.m_position = vec2(field.m_position.x, field.m_position.y + field.m_size.y / 2);

	ball.m_speed = vec2(1, 1);			//	�{�[���̃X�s�[�h
	ball.m_power = 1;

	//	======================

	//	=== �p�h���̏����� ===
	paddle.m_position = vec2(
		field.m_position.x + field.m_size.x / 2,
		field.m_position.y + field.m_size.y - 48.f);
	//	======================

	//	=== �u���b�N�̏����� ===
	vec2 blockSize = vec2(field.m_size.x / BLOCK_COULUM_MAX, BLOCK_HEIGHT);

	float y = field.m_position.y + (FONT_HEIGHT + FONT_WEIGHT) * 2;	//	����2�s�����ɂ��炷
	for (int i = 0; i < BLOCK_ROW_MAX; i++)
	{
		for (int j = 0; j < BLOCK_COULUM_MAX; j++)
		{
			blocks[i][j].m_position = vec2(						//	�ʒu���w��
				field.m_position.x + field.m_size.x * j / BLOCK_COULUM_MAX,
				y + blockSize.y * i
			);

			blocks[i][j].m_size = blockSize;					//	�傫�����w��
		}
	}
	//	=======================
}

void keybord(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1b:			//	Escape�L�[�ŏI��
		exit(0);
		break;
	}

	keys[key] = true;	//	�L�[�������ꂽ
}

void keybordUp(unsigned char key, int x, int y)
{
	keys[key] = false;	//	�L�[�������ꂽ
}

void passiveMotion(int _x, int _y)
{
	printf("passiveMotion: x%d:y%d \n", _x, _y);

	//	===	�p�h�����}�E�X�ő���ł���悤�� ===
	paddle.m_position.x = _x;
	paddle.m_position.x = max(paddle.m_position.x, field.m_position.x);
	paddle.m_position.x = min(paddle.m_position.x, field.m_position.x + field.m_size.x - paddle.m_width);
	// =========================================
}

void mouse(int button, int state, int x, int y)
{
	if ((!started) && (state == GLUT_DOWN))
	{
		//	===	�{�[�������ݒ� ===
		ball.m_lastposition.y =						//	�{�[���̏����ʒu��y���W����������
			ball.m_position.y = field.m_position.y + field.m_size.y / 2;

		ball.m_speed = vec2(1, 1);					//	�{�[���̃X�s�[�h
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

		wait = 60.f * 3;		//	�ҋ@���Ԃ�ݒ�
	}
}

int main(int argc, char *argv[])
{
	audioInit();
	audioWaveform(AUDIO_WAVEFORM_PULSE_50);
	audioDecay(.9f);

	glutInit(&argc, argv);

	glutInitDisplayMode(GL_DOUBLE);			//	�_�u���o�b�t�@���g�p����(���Ȃ��ꍇ�V���O���o�b�t�@)

	glutInitWindowPosition(640, 0);			//	Window�ʒu(���Ȃ��Ă�����)
	glutInitWindowSize(windowSize.x, windowSize.y);			//	Window�傫��(���Ȃ��Ă�����)

	glutCreateWindow("��Breakout��");		//	Window�̃^�C�g���ݒ�
	glutDisplayFunc(display);				//	�`�悪�K�v�ɂȂ�����Ă΂��R�[���o�b�N�̐ݒ�

	glutIdleFunc(idle);						//	GLUT�̎肪�󂢂����ɌĂ΂��R�[���o�b�N�̐ݒ�

	glutReshapeFunc(reshape);				//	Window�̃T�C�Y���ς������Ă΂��R�[���o�b�N�̐ݒ�
	glutKeyboardFunc(keybord);				//	�L�[�{�[�h�C�x���g���擾
	glutIgnoreKeyRepeat(GL_TRUE);			//	�L�[�{�[�h�̉������ςȂ���Ԃ𖳌��ɂ��邱�Ƃ�True�ɂ���
	glutKeyboardUpFunc(keybordUp);			//	�L�[�{�[�h�������ꂽ�Ƃ��C�x���g

	glutPassiveMotionFunc(passiveMotion);	//	�}�E�X�̈ړ��C�x���g���擾
	glutMouseFunc(mouse);						//	�}�E�X�̃N���b�N�C�x���g���擾
	reshape(windowSize.x, windowSize.y);	//	�������̂��߂ɋ����I�Ɉ��Ă�

	gameOver();								//	�ŏ��̓f�����(�Q�[���I�[�o�[���)

	glutMainLoop();							//	������glut�Ɉϑ�����(�R�[���o�b�N�n�͂��̃��\�b�h���O�ɏ���)


	return 0;
}