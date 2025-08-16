// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WSFDRAWSIMEVENTS_HPP
#define WSFDRAWSIMEVENTS_HPP

#include <string>
#include <vector>

#include "WkSimInterface.hpp"
#include "WsfDraw.hpp"
#include "WsfDrawObject.hpp"

namespace WkWsfDraw
{
class WsfDrawEvent : public warlock::SimEvent
{
public:
   virtual void Process(WsfDrawObject& aDrawObject) = 0;
};

class SimInitializingEvent : public WsfDrawEvent
{
public:
   SimInitializingEvent()
      : WsfDrawEvent()
   {
   }

   void Process(WsfDrawObject& aDrawObject) override { aDrawObject.Clear(); }
};

class DrawEvent : public WsfDrawEvent
{
public:
   DrawEvent(double aSimTime, const std::string& aLayer, const std::vector<WsfDraw::DrawEvent>& aEvents)
      : WsfDrawEvent()
      , mSimTime(aSimTime)
      , mLayer(aLayer)
      , mEvents(aEvents)
   {
   }

   void Process(WsfDrawObject& aDrawObject) override
   {
      aDrawObject.ProcessDrawEvents(mSimTime, QString::fromStdString(mLayer), mEvents);
   }

private:
   double                                mSimTime;
   const std::string                     mLayer;
   const std::vector<WsfDraw::DrawEvent> mEvents;
};
} // namespace WkWsfDraw

#endif