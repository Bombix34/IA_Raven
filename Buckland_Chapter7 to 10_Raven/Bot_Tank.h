#pragma once
#include "Raven_Bot_Class.h"
class Bot_Tank :
	public Raven_Bot_Class
{

private:

public:

	Bot_Tank::Bot_Tank(Raven_Game* world, Vector2D pos) : Raven_Bot_Class(world, pos){ m_iMaxHealth *= 2; m_dPrecision /= 2; m_dMaxSpeed /= 2; };
	virtual ~Bot_Tank();


};

