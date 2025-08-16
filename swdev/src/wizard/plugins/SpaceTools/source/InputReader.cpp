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

#include "InputReader.hpp"

#include <stack>
#include <stdexcept>

#include <QMessageBox>

#include "InputUtils.hpp"
#include "OrbitalMissionModel.hpp"
#include "OrbitalSequenceToModel.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtInputFile.hpp"
#include "UtTextDocument.hpp"
#include "UtUnitTypes.hpp"
#include "WsfDateTime.hpp"
#include "WsfMoverTypes.hpp"
#include "WsfOrbitalMissionSequence.hpp"
#include "WsfPM_Mover.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxy.hpp"
#include "WsfScenario.hpp"
#include "WsfSpaceMoverBase.hpp"

//! Retrieve a list of all space-domain platforms.
//!
//! This method examines the input stack and discovers all platforms that
//! have movers derived from WSF_SPACE_MOVER_BASE and, if required, have a TLE
//!
//! @param aRequireTLE - Check to require only TLE space platforms
//! \return - The list of names of all space-domain platforms.
QStringList SpaceTools::InputReader::GetAllSpacePlatforms(bool aRequireTLE)
{
   QStringList retval{};

   WsfPProxy* proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   if (proxyPtr)
   {
      WsfPM_Root        root(proxyPtr);
      WsfPM_PlatformMap map = root.platforms();
      for (auto i = map.begin(); i != map.end(); ++i)
      {
         WsfPM_Platform platNode = map.AtT(i);
         if (IsSpacePlatform(platNode, aRequireTLE))
         {
            retval << QString::fromStdString(i->first);
         }
      }
   }
   return retval;
}

bool SpaceTools::InputReader::IsSpacePlatform(const std::string& aPlatformName)
{
   return IsSpacePlatform(GetPlatformProxy(aPlatformName));
}

//! Return if the given platform is in the space-domain, and, if required, has a TLE
bool SpaceTools::InputReader::IsSpacePlatform(const WsfPM_Platform& aPlatformNode, bool aRequireTLE)
{
   WsfPM_Mover moverNode = aPlatformNode.Mover();
   return ((moverNode.IsOfType("WSF_SPACE_MOVER_BASE")) &&
           (!aRequireTLE || moverNode.GetValue().GetAttr("has_tle").GetBoolValue()));
}

//! Get the mission model for the named platform.
//!
//! This method constructs an OrbitalMissionModel for the platform with the given
//! @p aPlatformName. If there is no such platform, or the named platform has
//! no defined mission sequence, then this method will return a mission model containing
//! only the overall sequence event with a default execution condition.
//!
//! \param aPlatformName - The name of the platform of interest.
//! \returns             - The orbital mission model for the platform.
std::unique_ptr<wsfg::spaceg::OrbitalMissionModel>
SpaceTools::InputReader::GetPlatformMissionModel(const std::string& aPlatformName) const
{
   std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> retvalPtr{};
   auto                                               moverPtr = GetPlatformSpaceMover(aPlatformName);
   if (moverPtr)
   {
      auto sequencePtr = ut::clone(moverPtr->GetMissionEvents());
      retvalPtr        = GetModelFromSequence(*sequencePtr);
   }
   return retvalPtr;
}

//! Get the space mover for the given platform.
//!
//! This will examine the input stack and generate the space mover for the platform
//! names @p aPlatformName. If there is no such platform, this will return nullptr.
//! In the case of malformed input, this will throw an exception.
//!
//! \param aPlatformName - The name of the platform in question.
//! \returns             - The space mover defined by the input for the given platform; nullptr
//!                        if there is no such platform.
std::unique_ptr<WsfSpaceMoverBase> SpaceTools::InputReader::GetPlatformSpaceMover(const std::string& aPlatformName) const
{
   std::unique_ptr<WsfSpaceMoverBase> retvalPtr{};

   WsfPM_Platform platNode = InputUtils::GetPlatformProxy(aPlatformName);
   if (IsSpacePlatform(platNode))
   {
      // Create the space mover instance.
      std::string moverType = platNode.Mover().GetTypeName();
      retvalPtr.reset(dynamic_cast<WsfSpaceMoverBase*>(mScenario.GetMoverTypes().Find(moverType)->Clone()));

      // Collect the inherited types of this platform.
      std::stack<WsfPM_Platform> nodes{};
      while (platNode.IsValid())
      {
         nodes.push(platNode);
         platNode = platNode.GetInherited();
      }

      // Process the mover input for each of the inherited types, and build up the full mover
      // configuration.
      bool first{true};
      while (!nodes.empty())
      {
         WsfPM_Mover moverNode = nodes.top().Mover();
         if (moverNode.IsValid())
         {
            // Get the mover text.
            std::string moverText = moverNode.GetSourceText();
            if (!moverText.empty())
            {
               UtInput input{};
               input.PushInputString(moverText);

               try
               {
                  ProcessMoverInput(input, retvalPtr.get(), first);
                  first = false;
               }
               catch (std::exception& aErr)
               {
                  QMessageBox::information(nullptr,
                                           QString("Input Error"),
                                           QString::fromStdString(aErr.what()),
                                           QMessageBox::Ok);
                  retvalPtr.reset(nullptr);
                  break;
               }
            }
         }
         nodes.pop();
      }
   }

   return retvalPtr;
}

//! Get the start date and time of the simulation.
UtCalendar SpaceTools::InputReader::GetStartDateAndTime()
{
   WsfDateTime dateTime;

   UtInput                    input{};
   const std::vector<UtPath>& startupFiles = wizard::ProjectWorkspace::Instance()->GetProject()->GetStartupFiles();
   for (auto& path : startupFiles)
   {
      input.PushInput(ut::make_unique<UtInputFile>(path.GetSystemPath()));
   }

   std::string command{};
   bool        commandsLeft = true;
   while (commandsLeft)
   {
      try
      {
         commandsLeft = input.TryReadCommand(command);
         if (commandsLeft)
         {
            dateTime.ProcessInput(input);
         }
      }
      catch (UtInputFile::OpenError&)
      {
      }
   }

   return dateTime.GetStartDateAndTime();
}

//! Return the creation time of the given platform.
//!
//! This will search the platform with the given @p aPlatformName for the
//! 'creation_time' input. Note that this only makes use of a constant part
//! of the creation time input.
//!
//! \param aPlatformName - The name of the platform in question.
//! \param aDateTime     - The start date and time of the simulation.
//! \returns             - The creation date and time of the platform.
UtCalendar SpaceTools::InputReader::GetPlatformCreationTime(const std::string& aPlatformName, const UtCalendar& aDateTime)
{
   UtCalendar retval{aDateTime};

   // Note that this will only use the constant part of a 'creation_time' input.
   WsfPM_Platform platNode = InputUtils::GetPlatformProxy(aPlatformName);
   auto           ctNode   = platNode + "creationTime";
   if (ctNode.IsValid())
   {
      auto constNode = ctNode + "constant";
      if (constNode.IsValid())
      {
         auto valueNode = constNode + "value";
         if (valueNode.IsValid())
         {
            double timeInSeconds =
               static_cast<WsfProxy::Time*>(valueNode.GetValue())->GetValue().GetAsUnit(UtUnitTime::cSECONDS);
            retval.AdvanceTimeBy(timeInSeconds);
         }
      }
   }

   return retval;
}

//! Convert the given mission sequence into an Astrolabe adapted format.
//!
//! This method will convert the given @p aSequencePtr into a Astrolabe-adapted format.
//!
//! \param aSequencePtr - The mission sequence to transform.
//! \returns            - The Astrolabe-adapted mission model.
std::unique_ptr<wsfg::spaceg::OrbitalMissionModel>
SpaceTools::InputReader::GetModelFromSequence(WsfOrbitalMissionSequence& aSequence)
{
   std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> retvalPtr{nullptr};
   wsfg::spaceg::OrbitalSequenceToModel               modelMaker{};
   aSequence.Accept(&modelMaker);
   retvalPtr = std::move(modelMaker.GetModel());
   return retvalPtr;
}

//! Get the proxy node for the given platform.
WsfPM_Platform SpaceTools::InputReader::GetPlatformProxy(const std::string& aPlatformName)
{
   WsfPProxy*        proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root        rootNode{proxyPtr};
   WsfPM_PlatformMap map = rootNode.platforms();
   return map.FindT(aPlatformName);
}

//! Process mover input.
//!
//! This will handle the various cases of mover inputs, allowing the commands in the given @p aInput
//! to modify the given @p aMoverPtr. If this is the first mover block read, @p aFirst will be true.
//!
//! \param aInput    - An input object containing the input for the mover block.
//! \param aMoverPtr - The mover that will be affected by the given input.
//! \param aFirst    - true if this is the first mover block processed.
void SpaceTools::InputReader::ProcessMoverInput(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr, bool aFirst)
{
   std::string command{};
   aInput.ReadCommand(command);

   if (command == "add")
   {
      ProcessAddMoverText(aInput, aMoverPtr);
   }
   else if (command == "mover")
   {
      ProcessMoverText(aInput, aMoverPtr, aFirst);
   }
   else if (command == "edit")
   {
      ProcessEditMoverText(aInput, aMoverPtr);
   }
   else
   {
      if (!aMoverPtr->ProcessInput(aInput))
      {
         throw UtInput::UnknownCommand(aInput);
      }
   }
}

//! Process an 'add mover` block.
void SpaceTools::InputReader::ProcessAddMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr)
{
   std::string command{};
   aInput.ReadCommand(command);
   if (command != "mover")
   {
      throw std::runtime_error("Input error: 'mover' should follow 'add' in mover specifications.");
   }
   ProcessMoverText(aInput, aMoverPtr, true);
}

//! Process a 'mover' block.
//!
//! This method will handle both the case of edits to an existing mover and the declaration
//! of a new mover. If @p aReadType is true, this will assume the block takes the form
//! 'mover <type>', otherwise it is assumed that the type is omitted.
void SpaceTools::InputReader::ProcessMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr, bool aReadType)
{
   UtInputBlock block{aInput};
   if (aReadType)
   {
      std::string typeToken{};
      block.ReadCommand(typeToken);
   }
   ProcessMoverBlockContents(block, aMoverPtr);
}

//! Process an 'edit_mover' block.
void SpaceTools::InputReader::ProcessEditMoverText(UtInput& aInput, WsfSpaceMoverBase* aMoverPtr)
{
   std::string command{};
   aInput.ReadCommand(command);
   if (command != "mover")
   {
      throw std::runtime_error("Input error: 'mover' should follow 'edit' in mover specifications.");
   }
   UtInputBlock block{aInput};
   ProcessMoverBlockContents(block, aMoverPtr);
}

//! Process the contents of the block.
void SpaceTools::InputReader::ProcessMoverBlockContents(UtInputBlock& aBlock, WsfSpaceMoverBase* aMoverPtr)
{
   aBlock.ProcessInput(aMoverPtr);
}
