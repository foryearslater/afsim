// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "BM_SimEvent.hpp"

#include "BM_Plugin.hpp"
#include "BM_RuleSets.hpp"

void WkBM::SimInitializingEvent::Process(Plugin& aPlugin)
{
   aPlugin.mPlatformData.Clear();

   aPlugin.mPlatformData.AddSides(std::move(mSides));
   aPlugin.mPlatformData.AddTypes(std::move(mTypes));
   aPlugin.mPlatformData.AddCategories(std::move(mCategories));
}

void WkBM::PlatformAddedEvent::Process(Plugin& aPlugin)
{
   auto& data       = aPlugin.mPlatformData.AddPlatform(mIndex);
   data.mStartTime  = mSimTime;
   data.mIsWeapon   = mIsWeapon;
   data.mName       = std::move(mName);
   data.mTypes      = std::move(mTypes);
   data.mCategories = std::move(mCategories);
   data.mSide       = std::move(mSide);
}

void WkBM::PlatformRemovedEvent::Process(Plugin& aPlugin)
{
   aPlugin.mPlatformData.RemovePlatform(mSimTime, mIndex);
}

void WkBM::PlatformStateEvent::Process(Plugin& aPlugin)
{
   for (const auto& state : mStates)
   {
      aPlugin.mPlatformStates[state.first].Assign(state.second);
   }
}

void WkBM::AuxDataEvent::Process(Plugin& aPlugin)
{
   aPlugin.mAuxData = std::move(mAuxData);
}
