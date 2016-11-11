#include "Weapon_RocketLauncher.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
RocketLauncher::RocketLauncher(Raven_Bot*   owner):

                      Raven_Weapon(type_rocket_launcher,
                                   script->GetInt("RocketLauncher_DefaultRounds"),
                                   script->GetInt("RocketLauncher_MaxRoundsCarried"),
                                   script->GetDouble("RocketLauncher_FiringFreq"),
                                   script->GetDouble("RocketLauncher_IdealRange"),
                                   script->GetDouble("Rocket_MaxSpeed"),
                                   owner)
{
    //setup the vertex buffer
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -3),
                                           Vector2D(6, -3),
                                           Vector2D(6, -1),
                                           Vector2D(15, -1),
                                           Vector2D(15, 1),
                                           Vector2D(6, 1),
                                           Vector2D(6, 3),
                                           Vector2D(0, 3)
                                           };
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
  InitializeFuzzyModule();

}


//------------------------------ ShootAt --------------------------------------
//-----------------------------------------------------------------------------
inline void RocketLauncher::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire off a rocket!
    m_pOwner->GetWorld()->AddRocket(m_pOwner, pos);

    m_iNumRoundsLeft--;

    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RocketLauncher_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double RocketLauncher::GetDesirability(double DistToTarget)
{
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
    m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

  return m_dLastDesirabilityScore;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void RocketLauncher::InitializeFuzzyModule()
{
  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

  FzSet& Target_VeryClose = DistToTarget.AddLeftShoulderSet("Target_VeryClose",0,10,20);
  FzSet& Target_Close = DistToTarget.AddTriangularSet("Target_Close", 10, 20, 150);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 20, 150, 300);
  FzSet& Target_Far = DistToTarget.AddTriangularSet("Target_Far", 150, 300, 600);
  FzSet& Target_VeryFar = DistToTarget.AddRightShoulderSet("Target_VeryFar",300,600,1000);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability"); 
  FzSet& Indispensable = Desirability.AddRightShoulderSet("Indispensable", 90, 95, 100);
  FzSet& VeryDesirable = Desirability.AddTriangularSet("VeryDesirable", 50, 90, 95);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 10, 50, 90);
  FzSet& NotSoDesirable = Desirability.AddTriangularSet("NotSoDesirable", 5, 10, 50);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 5, 10);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  double max = this->m_iMaxRoundsCarried;
  FzSet& Ammo_Tons = AmmoStatus.AddRightShoulderSet("Ammo_Tons", 2*max/4, 3*max/4, max);
  FzSet& Ammo_Loads = AmmoStatus.AddTriangularSet("Ammo_Loads", 3*max/10, 2*max/4, 3*max/4);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 3 * max / 20, 3 * max / 10, 2 * max / 4);
  FzSet& Ammo_QuiteOkay = AmmoStatus.AddTriangularSet("Ammo_QuiteOkay", 0, 3 * max / 20, 3 * max / 10);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 3 * max / 20);


  m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Tons), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_QuiteOkay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryClose, Ammo_Low), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Tons), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), NotSoDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), NotSoDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_QuiteOkay), NotSoDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Tons), Indispensable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), Indispensable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Indispensable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_QuiteOkay), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Tons), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_QuiteOkay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), NotSoDesirable);

  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Tons), NotSoDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_QuiteOkay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_VeryFar, Ammo_Low), Undesirable);
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void RocketLauncher::Render()
{
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->RedPen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}