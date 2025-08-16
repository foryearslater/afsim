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

#ifndef DEMOMODESIMEVENTS_HPP
#define DEMOMODESIMEVENTS_HPP

#include "WkSimInterface.hpp"

namespace WkDemoMode
{
class Plugin;

class DemoModeEvent : public warlock::SimEvent
{
public:
   virtual void Process(Plugin& aPlugin) = 0;
};

class SimCompleteEvent : public DemoModeEvent
{
public:
   SimCompleteEvent()
      : DemoModeEvent()
   {
   }

   void Process(Plugin& aPlugin) override;
};
} // namespace WkDemoMode

#endif
