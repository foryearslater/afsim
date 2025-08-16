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

#ifndef SCOREBOARDSIMEVENTS_HPP
#define SCOREBOARDSIMEVENTS_HPP

#include "ScoreboardDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkScoreboard
{
class ScoreboardEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aData) = 0;
};

class SimulationCompleteEvent : public ScoreboardEvent
{
public:
   void Process(DataContainer& aData) override;
};

class ScoreboardUpdateEvent : public ScoreboardEvent
{
public:
   ScoreboardUpdateEvent(const WeaponEvent& aWeaponEvent);
   void Process(DataContainer& aData) override;

private:
   WeaponEvent mWeaponEvent;
};
} // namespace WkScoreboard

#endif