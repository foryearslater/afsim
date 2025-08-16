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

#ifndef OUTPUTRATEITEM_HPP
#define OUTPUTRATEITEM_HPP

#include <vector>

#include <QString>

namespace Engage
{
struct OutputRateVariable
{
   QString mName;
   QString mTime;
   QString mTimeUnits;
   QString mPeriod;
   QString mPeriodUnits;
};

class OutputRateItem
{
public:
   OutputRateItem();
   ~OutputRateItem() = default;

   QString                          GetFileName() const { return mFileName; }
   std::vector<OutputRateVariable>& GetItems() { return mItems; }

   void SetFileName(const QString& aFileName) { mFileName = aFileName; }

   void ClearItems();
   void AddItem(const QString& aName,
                const QString& aTime,
                const QString& aTimeUnits,
                const QString& aPeriod,
                const QString& aPeriodUnits);

private:
   QString                         mFileName;
   QString                         mTime;
   std::vector<OutputRateVariable> mItems;
};
} // namespace Engage

#endif
