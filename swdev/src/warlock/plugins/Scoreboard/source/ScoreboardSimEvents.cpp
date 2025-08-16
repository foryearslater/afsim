// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ScoreboardSimEvents.hpp"

WkScoreboard::ScoreboardUpdateEvent::ScoreboardUpdateEvent(const WeaponEvent& aWeaponEvent)
   : mWeaponEvent(aWeaponEvent)
{
}

void WkScoreboard::SimulationCompleteEvent::Process(DataContainer& aData)
{
   aData.Clear();
}

void WkScoreboard::ScoreboardUpdateEvent::Process(DataContainer& aData)
{
   aData.AddScoreboardData(mWeaponEvent);
}
