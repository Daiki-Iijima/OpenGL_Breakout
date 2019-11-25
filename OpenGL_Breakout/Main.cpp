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
#define BLOCK_COULUM_MAX	 14			//	��
#define BLOCK_ROW_MAX		 8			//	�s
#define BALL_X_SPEED_MAX	 8			//	�{�[�����p�h���ɓ�����������X�����̍ő�X�s�[�h


ivec2 windowSize = { 800,600 };	//	�E�B���h�E�̃T�C�Y���`

bool keys[256];		//	�ǂ̃L�[��������Ă��邩��ێ�����

Rect field;

Ball ball = { 8 };		//	8�͔��a�̒���

Paddle paddle = { PADDLE_DEFAULT_WIDTH };

Rect blocks[BLOCK_ROW_MAX][BLOCK_COULUM_MAX];

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

	glColor3ub(0xff, 0xff, 0xff);	//	�{�[���̐F���w��
	ball.draw();

	glColor3ub(0x00, 0xff, 0xff);	//	�p�h���̐F���w��
	paddle.draw();					//	�p�h���̕`��

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
	fontBegin();
	fontSetHeight(FONT_DEFAULT_HEIGHT);

	float y = fontGetWeight();
	fontSetPosition(windowSize.x / 2 + windowSize.x / 2, y);
	fontSetWeight(fontGetWeightMax());
	fontDraw("");
	fontEnd();
	//	=====================================

	glutSwapBuffers();	//	�_�u���o�b�t�@�̕\�Ɨ���؂�ւ���(�X���b�v����)
}

//	�A�b�v�f�[�g�݂����Ȃ���
void idle(void)
{
	ball.update();

	//	===	�{�[���̓����蔻�� ===
	if (
		(ball.m_position.y >= field.m_position.y + field.m_size.y) ||
		(ball.m_position.y < field.m_position.y))//	�㉺�[
	{
		ball.m_position = ball.m_lastposition;
		ball.m_speed.y *= -1;
	}

	if ((ball.m_position.x >= field.m_position.x + field.m_size.x) ||
		(ball.m_position.x < field.m_position.x))//	���E�[
	{
		ball.m_position = ball.m_lastposition;
		ball.m_speed.x *= -1;
	}
	//	===========================

	//	===== �p�h�������蔻�� =====

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


	//	======= �u���b�N�̓����蔻�� =========
	for (int i = 0; i < BLOCK_ROW_MAX; i++)
	{
		for (int j = 0; j < BLOCK_COULUM_MAX; j++)
		{
			if (blocks[i][j].isDead)						//	���S�t���O�������Ă����甲����
				continue;

			if (blocks[i][j].intersect(ball.m_position))	//	�{�[���g�̓����蔻��
			{
				blocks[i][j].isDead = true;					//	���������u���b�N��\�����Ȃ��悤�Ɏ��S�t���O�𗧂Ă�

				ball.m_position = ball.m_lastposition;		//	����
				ball.m_speed.y *= -1;
			}
		}
	}
	//	======================================
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

	ball.m_speed = vec2(1, 1) * 4.0f;			//	�{�[���̃X�s�[�h

	//	======================

	//	=== �p�h���̏����� ===
	paddle.m_position = vec2(
		field.m_position.x + field.m_size.x / 2,
		field.m_position.y + field.m_size.y - 48.f);
	//	======================

	//	=== �u���b�N�̏����� ===
	vec2 blockSize = vec2(field.m_size.x / BLOCK_COULUM_MAX, 12);

	float y = field.m_position.y + 64.f;
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

int main(int argc, char *argv[])
{
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

	reshape(windowSize.x, windowSize.y);	//	�������̂��߂ɋ����I�Ɉ��Ă�

	glutMainLoop();							//	������glut�Ɉϑ�����(�R�[���o�b�N�n�͂��̃��\�b�h���O�ɏ���)


	return 0;
}