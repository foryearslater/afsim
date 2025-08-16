// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef CRDSTATE_HPP
#define CRDSTATE_HPP

#include "ElementBase.hpp"
#include "UtCalendar.hpp"

namespace CrdImporter
{
class CrdState : public ElementBase
{
public:
   CrdState()           = default;
   ~CrdState() override = default;

   double GetClockTime() const { return mDateTime.GetTime(); }
   void   SetClockTime(double aTimeSinceMidnight_sec) { mDateTime.SetTime(aTimeSinceMidnight_sec); }

   UtCalendar GetClockDate() const { return mDateTime; }
   void       SetClockDate(int aYear, int aMonth, int aDay) { mDateTime.SetDate(aYear, aMonth, aDay); }

private:
   UtCalendar mDateTime{};
};
} // namespace CrdImporter
#endif
