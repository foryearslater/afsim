// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "OutputRateItem.hpp"

Engage::OutputRateItem::OutputRateItem()
   : mFileName("")
{
}

void Engage::OutputRateItem::ClearItems()
{
   mItems.clear();
}

void Engage::OutputRateItem::AddItem(const QString& aName,
                                     const QString& aTime,
                                     const QString& aTimeUnits,
                                     const QString& aPeriod,
                                     const QString& aPeriodUnits)
{
   OutputRateVariable newItem;
   newItem.mName        = aName;
   newItem.mTime        = aTime;
   newItem.mTimeUnits   = aTimeUnits;
   newItem.mPeriod      = aPeriod;
   newItem.mPeriodUnits = aPeriodUnits;

   mItems.push_back(newItem);
}
