// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ToolManager.hpp"

#include <algorithm>
#include <iostream>

#include "ATA_LaunchComputerGenerator.hpp"
#include "ATG_LAR_And_LC_Generator.hpp"
#include "AirToAirLaunchComputerGenerator.hpp"
#include "BallisticLaunchComputerGenerator.hpp"
#include "BallisticMissileLaunchComputerGenerator.hpp"
#include "OrbitalLaunchComputerGenerator.hpp"
#include "SAM_LaunchComputerGenerator.hpp"
#include "Tool.hpp"
#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtMemory.hpp"
#include "WsfScenario.hpp"

//! Method of inquiry to see if all registered Tools are complete,
//! and the simulation in progress may be terminated.
bool ToolManager::AllToolsDone()
{
   bool done = true;
   if (mRunningToolsStatus.empty())
   {
      // No Tools are registered, of course we are done!
   }
   else
   {
      ToolsStatus::iterator iter = mRunningToolsStatus.begin();
      for (; iter != mRunningToolsStatus.end(); ++iter)
      {
         if (!iter->second)
         {
            // Found a Tool that is not done yet,
            // so we have to continue to let him finish
            done = false;
            break;
         }
      }
   }
   return done;
}

ToolManager::ToolManager(WsfScenario& aScenario)
   : mRunningToolsStatus()
   , mTypeList(aScenario, "tool")
{
}

void ToolManager::AddedToScenario()
{
   AddBaseTypes();
}

ToolManager::~ToolManager()
{
   ToolsStatus::iterator iter = mRunningToolsStatus.begin();
   for (; iter != mRunningToolsStatus.end();)
   {
      // Get confirmation this is okay... ???
      delete iter->first;                     // deallocate the Tool memory
      iter = mRunningToolsStatus.erase(iter); // remove the pair we no longer need (iterates to the next item).
   }
}

//! Method to indicate that a new Tool is registering to begin its work.
//!   @param aToolPtr Pointer to the registering Tool instance.
//!   @returns Success
bool ToolManager::Register(Tool* aToolPtr)
{
   // Need to check to make sure he is not already added...
   ToolsStatus::iterator iter = mRunningToolsStatus.begin();
   for (; iter != mRunningToolsStatus.end(); ++iter)
   {
      if (iter->first == aToolPtr)
      {
         ut::log::warning() << "Registered Tool attempted to re-register again. Ignored.";
         return false;
      }
   }

   // He was not found, so add him in:
   mRunningToolsStatus.push_back(std::pair<Tool*, bool>(aToolPtr, false));
   return true;
}

//! Non-static method to indicate that a Tool has completed its work, and is un-registering.
//!   @param aToolPtr Pointer to the already registered Tool instance, which is now done.
//!   @return If true, the instance was found Registered, and set UnRegistered.
bool ToolManager::UnRegister(Tool* aToolPtr)
{
   ToolsStatus::iterator iter = mRunningToolsStatus.begin();
   for (; iter != mRunningToolsStatus.end(); ++iter)
   {
      if (iter->first == aToolPtr)
      {
         iter->second = true;
         return true;
      }
   }
   ut::log::warning() << "Tool tried to Un-register and no match was found.";
   return false;
}

void ToolManager::AddBaseTypes()
{
   mTypeList.Add("ATA_LAUNCH_COMPUTER_GENERATOR", ut::make_unique<ATA_LaunchComputerGenerator>(GetScenario()));
   mTypeList.Add("ATG_LAR_AND_LC_GENERATOR", ut::make_unique<ATG_LAR_And_LC_Generator>(GetScenario()));
   mTypeList.Add("AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR", ut::make_unique<AirToAirLaunchComputerGenerator>(GetScenario()));
   mTypeList.Add("BALLISTIC_LAUNCH_COMPUTER_GENERATOR", ut::make_unique<BallisticLaunchComputerGenerator>(GetScenario()));
   mTypeList.Add("BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR",
                 ut::make_unique<BallisticMissileLaunchComputerGenerator>(GetScenario()));
   mTypeList.Add("ORBITAL_LAUNCH_COMPUTER_GENERATOR", ut::make_unique<OrbitalLaunchComputerGenerator>(GetScenario()));
   mTypeList.Add("SAM_LAUNCH_COMPUTER_GENERATOR", ut::make_unique<SAM_LaunchComputerGenerator>(GetScenario()));
}

// virtual
bool ToolManager::ProcessInput(UtInput& aInput)
{
   bool myCommand = false;

   if (aInput.GetCommand() == "tool")
   {
      myCommand = true;
      UtInputBlock inputBlock(aInput);
      std::string  toolType;
      inputBlock.GetInput().ReadValue(toolType);

      // The code is now in place for a standard tool <toolName> <toolType> usage.
      // But have not done this, as it would break some existing input files.
      // At some point, will make the switch.
      // string toolName;
      // inputBlock.GetInput().ReadValue(toolName);
      Tool* typePtr = mTypeList.Find(toolType);
      if (typePtr == nullptr)
      {
         throw UtInput::BadValue(inputBlock.GetInput(), "Tool type " + toolType + " not found.");
      }

      Tool* toolPtr = typePtr->Clone();
      if (toolPtr == nullptr)
      {
         throw UtInput::BadValue(inputBlock.GetInput(), "Unknown Tool Type: " + toolType);
      }

      std::string lowerName;
      for (std::string::size_type i = 0; i != toolType.size(); ++i)
      {
         // Make the instance name a lower case version of the type name:
         lowerName.push_back(tolower(toolType[i]));
      }

      if (toolType == lowerName)
      {
         throw UtInput::BadValue(
            inputBlock.GetInput(),
            "By convention, type names are upper case, instances are lower case. Required for uniqueness.");
      }
      else
      {
         toolPtr->SetName(lowerName);
         // Process the tool-specific commands within the tool/end_tool block.
         while (inputBlock.ReadCommand())
         {
            if (!toolPtr->ProcessInput(inputBlock.GetInput()))
            {
               throw UtInput::UnknownCommand(inputBlock.GetInput());
            }
         }
      }

      if (!Register(toolPtr))
      {
         throw UtInput::BadValue(aInput, "Could not re-register a duplicate named tool type.");
      }
   }

   return myCommand;
}

bool ToolManager::Initialize(WsfSimulation& aSimulation)
{
   bool success = true;

   ToolsStatus::iterator iter = mRunningToolsStatus.begin();

   if (iter == mRunningToolsStatus.end())
   {
      ut::log::warning() << "No Tools have been instantiated for this simulation.";
   }

   for (; iter != mRunningToolsStatus.end(); ++iter)
   {
      if (!iter->first->Initialize(aSimulation))
      {
         auto out = ut::log::error() << "Tool did not initialize for this simulation.";
         out.AddNote() << "Tool: " << iter->first->GetName();
         UnRegister(iter->first);
         success = false;
      }
   }

   return success;
}

ToolManager* ToolManager::Find(WsfScenario& aScenario)
{
   return static_cast<ToolManager*>(aScenario.FindExtension("weapon_tools"));
}
