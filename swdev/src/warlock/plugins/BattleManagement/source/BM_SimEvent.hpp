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
#ifndef BM_SIM_EVENT_HPP
#define BM_SIM_EVENT_HPP

#include <utility>
#include <vector>

#include "BM_RuleSets.hpp"
#include "WkSimInterface.hpp"
#include "statistics/WkfBM_PlatformDatabase.hpp"

namespace WkBM
{
class Plugin;

class BMSimEvent : public warlock::SimEvent
{
public:
   virtual void Process(Plugin& aPlugin) = 0;
};

class SimInitializingEvent : public BMSimEvent
{
public:
   void Process(Plugin& aPlugin) override;

   QStringList mSides;
   QStringList mTypes;
   QStringList mCategories;
};

class PlatformAddedEvent : public BMSimEvent
{
public:
   void Process(Plugin& aPlugin) override;

   unsigned    mIndex;
   double      mSimTime;
   QString     mName;
   QStringList mTypes;
   QStringList mCategories;
   QString     mSide;
   bool        mIsWeapon;
};

class PlatformRemovedEvent : public BMSimEvent
{
public:
   void Process(Plugin& aPlugin) override;

   unsigned mIndex;
   double   mSimTime;
};

class PlatformStateEvent : public BMSimEvent
{
public:
   void Process(Plugin& aPlugin) override;

   std::vector<std::pair<unsigned, PlatformState>> mStates;
};

//! Key = platform index
//! Value = aux data keys and values
using AuxDataMap = std::map<unsigned, std::map<std::string, QVariant>>;

class AuxDataEvent final : public BMSimEvent
{
public:
   void Process(Plugin& aPlugin) override;

   AuxDataMap mAuxData;
};
} // namespace WkBM

#endif
