#pragma once
#include "Raven_Bot_Class.h"
#include <vector>
#include <iosfwd>
#include <map>

#include "game/MovingEntity.h"
#include "misc/utils.h"
#include "Raven_TargetingSystem.h"
class Bot_Leader :
	public Raven_Bot_Class
{
private:
	double m_dMaxSpeedBot;

	void          UpdateMovement();

public:
	Bot_Leader(Raven_Game* world, Vector2D pos);
	virtual ~Bot_Leader();

	void         Update();
};

