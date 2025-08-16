// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef COMMVISSIMEVENTS_HPP
#define COMMVISSIMEVENTS_HPP

#include "WkSimInterface.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"

namespace WkCommVis
{
class CommVisEvent : public warlock::SimEvent
{
public:
   virtual void Process(wkf::DataContainer& aData) = 0;
};

class SimInitializedEvent : public CommVisEvent
{
public:
   SimInitializedEvent() = default;
   void Process(wkf::DataContainer& aData) override;
};

class SimulationCompleteEvent : public CommVisEvent
{
public:
   void Process(wkf::DataContainer& aData) override;
};

class CommVisUpdateEvent : public CommVisEvent
{
public:
   CommVisUpdateEvent(const wkf::CommEvent& aCommEvent);
   void Process(wkf::DataContainer& aData) override;

private:
   wkf::CommEvent mCommEvent;
};
} // namespace WkCommVis

#endif
