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

#ifndef TIMEHANDLER_HPP
#define TIMEHANDLER_HPP

#include "UtCalendar.hpp"

namespace SpaceTools
{

namespace TimeHandler
{
enum
{
   cOUTPUT_EPOCH,
   cOUTPUT_TIME_NOW,
   cOUTPUT_DATE_TIME
};
void       UpdateStartDateTime(const UtCalendar& aNewCalendar, int aOutputType);
UtCalendar GetLatestEpoch();
}; // namespace TimeHandler

} // namespace SpaceTools

#endif // SATELLITEINSERTERSETHANDLER
