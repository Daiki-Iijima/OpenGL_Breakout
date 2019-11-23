#include "Ball.h"
#include "glut.h"


void Ball::update()
{
	m_lastposition = m_position;
	m_position += m_speed;
}
void Ball::draw()
{
	glPushMatrix();	//	行列を保存
	{
		glTranslatef(m_position.x, m_position.y,0);	//	位置更新
		glScalef(m_radius, m_radius, 0);						//	サイズを変更

		glutSolidSphere(
			1,		//	半径
			16,		//	ディティール
			16		//	ディティール
		);
	}
	glPopMatrix();	//	保存していたものを適用
}