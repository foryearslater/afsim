// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMPARTBROWSERSIMEVENTS_HPP
#define PLATFORMPARTBROWSERSIMEVENTS_HPP

#include <string>

#include "PlatformPartBrowserTypes.hpp"
#include "WkSimInterface.hpp"

namespace WkPlatformPartBrowser
{
class DataContainer;

class PlatformPartEvent : public warlock::SimEvent
{
public:
   PlatformPartEvent(const std::string& aPlatformName, bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
      , mPlatformName(aPlatformName)
   {
   }

   virtual void Process(DataContainer& aData) = 0;

protected:
   std::string mPlatformName;
};

class PartUpdateEvent : public PlatformPartEvent
{
public:
   PartUpdateEvent(const std::string& aPlatformName, std::vector<PlatformPart>& aParts)
      : PlatformPartEvent(aPlatformName, true)
      , mParts(aParts)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::vector<PlatformPart> mParts;
};

class PartPropertyUpdateEvent : public PlatformPartEvent
{
public:
   PartPropertyUpdateEvent(const std::string& aPlatformName,
                           unsigned int       aUniqueId,
                           PropertyName       aPropertyName,
                           QVariant           aPropertyValue)
      : PlatformPartEvent(aPlatformName)
      , mUniqueId(aUniqueId)
      , mPropertyName(aPropertyName)
      , mPropertyValue(aPropertyValue)
   {
   }

   void Process(DataContainer& aData) override;

private:
   unsigned int mUniqueId;
   PropertyName mPropertyName;
   QVariant     mPropertyValue;
};

class PartListEvent : public PlatformPartEvent
{
public:
   PartListEvent(const std::string& aPlatformName, std::vector<PlatformPart>& aParts)
      : PlatformPartEvent(aPlatformName)
      , mParts(aParts)
   {
   }

   void Process(DataContainer& aData) override;

private:
   std::vector<PlatformPart> mParts;
};
} // namespace WkPlatformPartBrowser

#endif