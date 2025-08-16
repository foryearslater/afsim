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

#include "OutputItem.hpp"

Engage::OutputItem::OutputItem()
   : mFileName("")
   , mPhase("")
   , mAddEventOutput(false)
   , mAddSummaryOutput(false)
{
}

void Engage::OutputItem::ClearItems()
{
   mItems.clear();
}

void Engage::OutputItem::AddItem(const QString& aName, const QString& aUnit, const QString& aFormat)
{
   OutputVariable* newItem = new OutputVariable;
   newItem->mName          = aName;
   newItem->mFormat        = aFormat;
   newItem->mUnits         = aUnit;

   mItems.push_back(newItem);
}
