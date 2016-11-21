#pragma once
#include "Raven_Bot_Class.h"
class Bot_Sniper :
	public Raven_Bot_Class
{
public:
	Bot_Sniper(Raven_Game* world, Vector2D pos) : Raven_Bot_Class(world, pos){ m_iMaxHealth /= 2; m_dPrecision *= 2; m_dMaxSpeedBot /= 2; };
	virtual ~Bot_Sniper();
};
