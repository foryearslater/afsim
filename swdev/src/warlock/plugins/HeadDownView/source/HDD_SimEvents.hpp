// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef HDD_SIMEVENTS_HPP
#define HDD_SIMEVENTS_HPP

#include "WkSimInterface.hpp"
#include "hdd/WkfHDD_DataContainer.hpp"

namespace HDD
{
class HDDEvent : public warlock::SimEvent
{
public:
   HDDEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }
   virtual void Process(DataContainer& aState) = 0;
};

class UpdateHDDDataEvent : public HDDEvent
{
public:
   UpdateHDDDataEvent(const DataContainer& aData)
      : HDDEvent(true)
      , mData(aData)
   {
   }
   void Process(DataContainer& aState) override;

private:
   DataContainer mData;
};
} // namespace HDD

#endif // HDD_SIMEVENTS_HPP
