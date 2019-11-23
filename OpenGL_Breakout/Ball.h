#pragma once

#include "glm/glm.hpp"

using namespace glm;

struct Ball
{
	float m_radius;			//	ボールのサイズ(半径)
	vec2 m_lastposition;	//	1フレーム前の位置
	vec2 m_position;		//	現在のフレームの位置
	vec2 m_speed;			//	速度

	void update();			//	計算
	void draw();			//	描画
};