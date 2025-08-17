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

#ifndef TOOLUPDATEEVENT_HPP
#define TOOLUPDATEEVENT_HPP

#include "Tool.hpp"
#include "WeaponToolsExport.hpp"
#include "WsfEvent.hpp"

class WT_EXPORT ToolUpdateEvent : public WsfEvent
{
public:
   ToolUpdateEvent(double aStartTime, Tool* aToolPtr);
   EventDisposition Execute() override;

private:
   Tool* mToolPtr;
};

#endif
