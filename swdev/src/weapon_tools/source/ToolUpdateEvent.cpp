// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ToolUpdateEvent.hpp"

#include "WsfSimulation.hpp"

WsfEvent::EventDisposition ToolUpdateEvent::Execute()
{
   double t0 = GetTime();
   mToolPtr->Update(t0);
   double t1 = t0 + mToolPtr->FrameStep();
   SetTime(t1);
   return cRESCHEDULE;
}

ToolUpdateEvent::ToolUpdateEvent(double aStartTime, Tool* aToolPtr)
   : WsfEvent(aStartTime)
   , mToolPtr(aToolPtr)
{
}
