#include "Paddle.h"

#include "glut.h"

void Paddle::draw()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	{
		GLfloat range[2];				//	0:Å¬ 1:Å‘å
		glGetFloatv(					//	ƒ‰ƒCƒ“‚Ì‘¾‚³‚Ì”ÍˆÍ‚ðŽæ“¾
			GL_LINE_WIDTH_RANGE,
			range
		);

		glLineWidth(range[1]);

		glBegin(GL_LINES);
		{
			glVertex2f(m_position.x, m_position.y);
			glVertex2fv((GLfloat*)&(m_position + vec2(m_width, 0)));
		}
		glEnd();
	}
	glPopAttrib();
}

//	ƒ{[ƒ‹‚Æ‚Ì“–‚½‚è”»’è‚ð‚Æ‚é
bool Paddle::intersectBall(Ball &const _ball)
{
	if (
		(m_position.y >= _ball.m_position.y && m_position.y < _ball.m_lastposition.y) ||
		(m_position.y < _ball.m_position.y && m_position.y >= _ball.m_lastposition.y)
		)
	{
		if (
			(m_position.x < _ball.m_lastposition.x &&
				m_position.x + m_width >= _ball.m_position.x)
			)
			return true;
	}

	return false;
}