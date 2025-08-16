// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef NetworkBrowserSimEvents_HPP
#define NetworkBrowserSimEvents_HPP

#include "NetworkBrowserDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkNetworkBrowser
{
class NetworkEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aData) = 0;
};

class InitialEvent : public NetworkEvent
{
public:
   InitialEvent(unsigned short                                     aSiteId,
                unsigned short                                     aAppId,
                unsigned short                                     aExerciseId,
                const std::vector<DataContainer::DisConnection>&   aDisConnections,
                const std::map<int, DataContainer::XioConnection>& aXioConnections)
      : mSiteId(aSiteId)
      , mApplicationId(aAppId)
      , mExerciseId(aExerciseId)
      , mDisConnections(aDisConnections)
      , mXioConnections(aXioConnections)
   {
   }

   void Process(DataContainer& aData) override
   {
      aData.SetInfo(mSiteId, mApplicationId, mExerciseId, mDisConnections, mXioConnections);
   }

private:
   unsigned short                              mSiteId;
   unsigned short                              mApplicationId;
   unsigned short                              mExerciseId;
   std::vector<DataContainer::DisConnection>   mDisConnections;
   std::map<int, DataContainer::XioConnection> mXioConnections;
};
} // namespace WkNetworkBrowser
#endif