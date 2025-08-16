// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ACESDISPLAY_EVENTS_HPP
#define ACESDISPLAY_EVENTS_HPP

#include "WkSimInterface.hpp"
#include "WsfPlatform.hpp"
#include "aces/WkfAcesDisplayDataContainer.hpp"

namespace WkAcesDisplay
{
class Plugin;

class AcesEvent : public warlock::SimEvent
{
public:
   AcesEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }
   virtual void Process(wkf::AcesDisplay::DataContainer& aState) = 0;
};

class SimulationInitializingEvent : public AcesEvent
{
public:
   SimulationInitializingEvent()
      : AcesEvent(false)
   {
   }

   void Process(wkf::AcesDisplay::DataContainer& aData) override;
};

// This updates the ACES data container
class UpdateAcesDataEvent : public AcesEvent
{
public:
   UpdateAcesDataEvent(const std::map<size_t, wkf::AcesDisplay::AcesData>& aData)
      : AcesEvent(true)
      , mData(aData)
   {
   }
   void Process(wkf::AcesDisplay::DataContainer& aData) override;

private:
   std::map<size_t, wkf::AcesDisplay::AcesData> mData;
};

// This checks for a platform comment and adds it
// to the map of comments, to display at the bottom right
// of the ACES Display
class CommentAddEvent : public AcesEvent
{
public:
   CommentAddEvent(double aTime, const size_t aPlatformIndex, const std::string& aMessage)
      : AcesEvent(true)
      , mCreationTime(aTime)
      , mPlatformIndex(aPlatformIndex)
      , mMessage(aMessage)
   {
   }
   void Process(wkf::AcesDisplay::DataContainer& aData) override;

private:
   double      mCreationTime;
   size_t      mPlatformIndex;
   std::string mMessage;
};

// This adds a platform that has a Situation Awareness
// processor to the list, so that an ACES Display may
// be opened on it
class PlatformWithSAP_AddedEvent : public AcesEvent
{
public:
   PlatformWithSAP_AddedEvent(const size_t aPlatformIndex)
      : AcesEvent(false)
      , mPlatformIndex(aPlatformIndex)
   {
   }
   void Process(wkf::AcesDisplay::DataContainer& aData) override;

private:
   size_t mPlatformIndex;
};

// This creates an EntityTruth object for a platform
// when it is added and adds it to the data container.
class PlatformAddedEvent : public AcesEvent
{
public:
   PlatformAddedEvent(WsfPlatform& aPlatform)
      : AcesEvent(false)
      , mPlatform(aPlatform)
   {
   }
   void Process(wkf::AcesDisplay::DataContainer& aData) override;

private:
   WsfPlatform& mPlatform;
};

// This removes an EntityTruth object for a platform
// when it is deleted
class PlatformDeletedEvent : public AcesEvent
{
public:
   PlatformDeletedEvent(const WsfPlatform& aPlatform)
      : AcesEvent(false)
      , mPlatform(aPlatform)
   {
   }
   void Process(wkf::AcesDisplay::DataContainer& aData) override;

private:
   const WsfPlatform& mPlatform;
};
} // namespace WkAcesDisplay

#endif // ACESDISPLAY_EVENTS_HPP
