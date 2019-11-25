#pragma once

#include "glm/glm.hpp"

using namespace glm;

struct Rect {
	vec2 m_position;
	vec2 m_size;

	bool isDead;	//	Breakoutê—p‚Ì€–S”»’èƒtƒ‰ƒO

	Rect();
	Rect(vec2 const& _position, vec2 const& _size);
	void draw();	//	•`‰æ
	bool intersect(vec2 const& _point);	//	“n‚³‚ê‚½ˆÊ’u‚Æ‚Ì“–‚½‚è”»’è
	bool intersect(Rect const& _rect);	//	“n‚³‚ê‚½RectŒ^‚Æ‚Ì“–‚½‚è”»’è
	
};