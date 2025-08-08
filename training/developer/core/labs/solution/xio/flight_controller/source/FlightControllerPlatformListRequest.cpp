// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#include "FlightControllerPlatformListRequest.hpp"
#include "FlightControllerInterface.hpp"

#include "UtLog.hpp"

//! static instantiation
FlightControllerPlatformListRequest::Platforms FlightControllerPlatformListRequest::sPlatforms;

FlightControllerPlatformListRequest::FlightControllerPlatformListRequest(WsfXIO_Connection* aConnectionPtr)
   : WsfXIO_PlatformListRequest(aConnectionPtr)
{
}

//virtual
//! When a platform is added to the the simulation, save the information locally.
//! When a platform is removed from the simulation, remove it from the list.
void FlightControllerPlatformListRequest::HandlePlatformList(WsfXIO_PlatformListUpdatePkt& aPkt)
{
   FlightControllerInterface::PlatformListPktReceived();

   if (!aPkt.mPlatformsAdded.empty())
   {
      // there is a new list of platforms so clear the list before processing the new list
      sPlatforms.clear();
   }

   // EXERCISE 3 TASK 1
   // Add new platform information
   // Iterate over aPkt.mPlatformsAdded using a Platforms::iterator
   // Copy Platform structures from mPlatformsAdded to the Static Class Variable, sPlatforms
   for (const auto& added : aPkt.mPlatformsAdded)
   {
      auto out = ut::log::info() << "Platform Added.";
      out.AddNote() << "Index: " << added.mIndex;
      out.AddNote() << "Name: " << added.mName;
      
      sPlatforms.push_back(added);
   }

   // Remove information if a platform is removed
   for (auto deleted : aPkt.mPlatformsDeleted)
   {
      { // RAII block
         auto out = ut::log::info() << "Platform deleted.";
         out.AddNote() << "Index: " << deleted;
      }
      
      Platforms::iterator platformsIter = sPlatforms.begin();
      while (platformsIter != sPlatforms.end())
      {
         if (platformsIter->mIndex == deleted)
         {
            platformsIter = sPlatforms.erase(platformsIter);
            break;
         }
         else
         {
            ++platformsIter;
         }
      }
   }
}
