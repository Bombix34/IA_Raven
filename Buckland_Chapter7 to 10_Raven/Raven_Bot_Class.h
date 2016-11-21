#pragma once
#include "Raven_Bot.h"
class Raven_Bot_Class :
	public Raven_Bot
{
protected:
	double m_dMaxSpeedBot;

public:
	Raven_Bot_Class(Raven_Game* world, Vector2D pos);
	virtual ~Raven_Bot_Class();

	double MaxSpeed()const{ return m_dMaxSpeedBot; }
};

