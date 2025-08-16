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

#ifndef INPUTREADER_HPP
#define INPUTREADER_HPP

#include <memory>

#include <QStringList>

#include "UtCalendar.hpp"
class UtInput;
class UtInputBlock;

class WsfOrbitalMissionSequence;
#include "WsfPM_Platform.hpp"
class WsfScenario;
class WsfSpaceMoverBase;

namespace wsfg
{
namespace spaceg
{

class OrbitalMissionModel;

} // namespace spaceg
} // namespace wsfg

namespace SpaceTools
{

//! This object is responsible for creating an OrbitalMissionModel instance from the input stack.
class InputReader
{
public:
   explicit InputReader(const WsfScenario& aScenario)
      : mScenario(aScenario)
   {
   }

   static QStringList GetAllSpacePlatforms(bool aRequireTLE = false);
   static bool        IsSpacePlatform(const std::string& aPlatformName);
   static bool        IsSpacePlatform(const WsfPM_Platform& aPlatformNode, bool aRequireTLE = false);

   std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> GetPlatformMissionModel(const std::string& aPlatformName) const;
   std::unique_ptr<WsfSpaceMoverBase>                 GetPlatformSpaceMover(const std::string& aPlatformName) const;
   static UtCalendar                                  GetStartDateAndTime();
   static UtCalendar GetPlatformCreationTime(const std::string& aPlatformName, const UtCalendar& aDateTime);

private:
   static std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> GetModelFromSequence(WsfOrbitalMissionSequence& aSequence);
   static WsfPM_Platform                                     GetPlatformProxy(const std::string& aPlatformName);

   static void ProcessMoverInput(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr, bool aFirst);
   static void ProcessAddMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr);
   static void ProcessMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr, bool aReadType);
   static void ProcessEditMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr);
   static void ProcessMoverBlockContents(UtInputBlock& aBlock, WsfSpaceMoverBase* aMoverPtr);

   const WsfScenario& mScenario;
};

} // namespace SpaceTools

#endif // INPUTREADER_HPP
