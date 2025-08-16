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
#ifndef CREATEPLATFORMSIMCOMMANDS_HPP
#define CREATEPLATFORMSIMCOMMANDS_HPP

#include "WkSimInterface.hpp"

namespace WkCreatePlatform
{
class CreatePlatformCommand : public warlock::SimCommand
{
public:
   CreatePlatformCommand(const std::string& aName,
                         const std::string& aSide,
                         const std::string& aType,
                         double             aLat,
                         double             aLon,
                         double             aAlt,
                         double             aHeading,
                         double             aSpeed)
      : SimCommand(true)
      , mName(aName)
      , mSide(aSide)
      , mType(aType)
      , mLat(aLat)
      , mLon(aLon)
      , mAlt(aAlt)
      , mHeading(aHeading)
      , mSpeed(aSpeed)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mName;
   std::string mSide;
   std::string mType;
   double      mLat;
   double      mLon;
   double      mAlt;
   double      mHeading;
   double      mSpeed;
};
} // namespace WkCreatePlatform
#endif
