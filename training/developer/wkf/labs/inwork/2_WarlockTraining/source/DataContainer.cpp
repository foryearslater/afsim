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

#include "DataContainer.hpp"

WarlockTraining::PlatformData WarlockTraining::DataContainer::GetPlatformData(const std::string& aPlatformName) const
{
   auto iter = mData.find(aPlatformName);
   if (iter != mData.end())
   {
      return iter->second;
   }
   return PlatformData();
}

void WarlockTraining::DataContainer::SetData(const std::map<std::string, PlatformData>& aData)
{
   // EXERCISE 1 TASK 1c
   // Store aData and emit the DataChanged() signal
   // PLACE YOUR CODE HERE 
}
