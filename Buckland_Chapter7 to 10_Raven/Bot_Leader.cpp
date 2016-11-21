#include "Bot_Leader.h"
#include "misc/Cgdi.h"
#include "misc/utils.h"
#include "2D/Transformations.h"
#include "2D/Geometry.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_Game.h"
#include "navigation/Raven_PathPlanner.h"
#include "Raven_SteeringBehaviors.h"
#include "Raven_UserOptions.h"
#include "time/Regulator.h"
#include "Raven_WeaponSystem.h"
#include "Raven_SensoryMemory.h"

#include "Messaging/Telegram.h"
#include "Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"

#include "goals/Raven_Goal_Types.h"
#include "goals/Goal_Think.h"


#include "Debug/DebugConsole.h"


Bot_Leader::Bot_Leader(Raven_Game* world, Vector2D pos) :

	Raven_Bot_Class(world, pos)
{
	m_dMaxSpeedBot = 1;

	int mult = rand() % 3;
	int div = rand() % 3;
	while (mult == div){
		div = rand() % 3;
	}

	switch (mult){
		case 0:
			m_iMaxHealth *= 5;
			break;

		case 1:
			m_dPrecision *= 5;
			break;

		case 2:
			m_dMaxSpeedBot *= 2;
			break;
	}

	switch (div){
	case 0:
		m_iMaxHealth /= 2;
		break;

	case 1:
		m_dPrecision /= 2;
		break;

	case 2:
		m_dMaxSpeedBot /= 4;
		break;
	}

}


Bot_Leader::~Bot_Leader()
{
}

//-------------------------------- Update -------------------------------------
//
void Bot_Leader::Update()
{
	//process the currently active goal. Note this is required even if the bot
	//is under user control. This is because a goal is created whenever a user 
	//clicks on an area of the map that necessitates a path planning request.
	m_pBrain->Process();

	//Calculate the steering force and update the bot's velocity and position
	UpdateMovement();

	//if the bot is under AI control but not scripted
	if (!isPossessed())
	{
		//examine all the opponents in the bots sensory memory and select one
		//to be the current target
		if (m_pTargetSelectionRegulator->isReady())
		{
			m_pTargSys->Update();
		}

		//appraise and arbitrate between all possible high level goals
		if (m_pGoalArbitrationRegulator->isReady())
		{
			m_pBrain->Arbitrate();
		}

		//update the sensory memory with any visual stimulus
		if (m_pVisionUpdateRegulator->isReady())
		{
			m_pSensoryMem->UpdateVision();
		}

		//select the appropriate weapon to use from the weapons currently in
		//the inventory
		if (m_pWeaponSelectionRegulator->isReady())
		{
			m_pWeaponSys->SelectWeapon();
		}

		//this method aims the bot's current weapon at the current target
		//and takes a shot if a shot is possible
		m_pWeaponSys->TakeAimAndShoot();
	}
}

//------------------------- UpdateMovement ------------------------------------
//
//  this method is called from the update method. It calculates and applies
//  the steering force for this time-step.
//-----------------------------------------------------------------------------
void Bot_Leader::UpdateMovement()
{
	//calculate the combined steering force
	Vector2D force = m_pSteering->Calculate();

	//if no steering force is produced decelerate the player by applying a
	//braking force
	if (m_pSteering->Force().isZero())
	{
		const double BrakingRate = 0.8;

		m_vVelocity = m_vVelocity * BrakingRate;
	}

	//calculate the acceleration
	Vector2D accel = force / m_dMass;

	//update the velocity
	m_vVelocity += accel;
	//make sure vehicle does not exceed maximum velocity
	m_vVelocity.Truncate(m_dMaxSpeedBot);

	//update the position
	m_vPosition += m_vVelocity;

	//if the vehicle has a non zero velocity the heading and side vectors must 
	//be updated
	if (!m_vVelocity.isZero())
	{
		m_vHeading = Vec2DNormalize(m_vVelocity);

		m_vSide = m_vHeading.Perp();
	}
}
