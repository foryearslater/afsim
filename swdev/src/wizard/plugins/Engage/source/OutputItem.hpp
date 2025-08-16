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

#ifndef OUTPUTITEM_HPP
#define OUTPUTITEM_HPP

#include <vector>

#include <QString>

namespace Engage
{
struct OutputVariable
{
   QString mName;
   QString mUnits;
   QString mFormat;
};

class OutputItem
{
public:
   OutputItem();
   ~OutputItem() = default;

   QString                            GetFileName() const { return mFileName; }
   QString                            GetPhase() const { return mPhase; }
   bool                               GetAddEventOutput() const { return mAddEventOutput; }
   bool                               GetAddSummaryOutput() const { return mAddSummaryOutput; }
   std::vector<OutputVariable*> const GetItems() const { return mItems; }

   void SetFileName(const QString& aFileName) { mFileName = aFileName; }
   void SetPhase(const QString& aPhase) { mPhase = aPhase; }
   void SetAddEventOutput(bool aVal) { mAddEventOutput = aVal; }
   void SetAddSummaryOutput(bool aVal) { mAddSummaryOutput = aVal; }

   void ClearItems();
   void AddItem(const QString& aName, const QString& aUnit, const QString& aFormat);

private:
   QString                      mFileName;
   QString                      mPhase;
   std::vector<OutputVariable*> mItems;
   bool                         mAddEventOutput;
   bool                         mAddSummaryOutput;
};
} // namespace Engage

#endif
