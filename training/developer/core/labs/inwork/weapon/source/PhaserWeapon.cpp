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

#include "PhaserWeapon.hpp"

#include <sstream>

// From this Exercise
#include "PhaserLethality.hpp"

// Utilities
#include "UtSphericalEarth.hpp"

// WSF
#include "WsfAttributeContainer.hpp"
#include "WsfDraw.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeaponEngagement.hpp"

// ****************************************************************************
// Public

// ============================================================================
// Constructor and destructor
PhaserWeapon::PhaserWeapon(WsfScenario& aScenario)
   : WsfImplicitWeapon(aScenario),
     mFireIntegrationInterval(0.1),
     mFireDuration(5.0),
     mDisplayEngagements(true)
{
}

PhaserWeapon& PhaserWeapon::operator=(const PhaserWeapon& aSrc)
{
   mFireIntegrationInterval = aSrc.mFireIntegrationInterval;
   mFireDuration = aSrc.mFireDuration;
   mDisplayEngagements = aSrc.mDisplayEngagements;
   return *this;
}

// ============================================================================
//virtual
//! Return a copy of this weapon
WsfWeapon* PhaserWeapon::Clone() const
{
   return new PhaserWeapon(*this);
}

// ============================================================================
//virtual
//! Process input from the AFNES input file.
//! @param aInput    [input]  The input stream.
//! @return 'true' if the command was processed
bool PhaserWeapon::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command = aInput.GetCommand();
   if (command == "fire_duration")
   {
      aInput.ReadValueOfType(mFireDuration, UtInput::cTIME);
   }
   else if (command == "fire_integration_interval")
   {
      // EXERCISE 2 TASK 3
      // PLACE YOUR CODE HERE
   }
   else
   {
      myCommand = WsfWeapon::ProcessInput(aInput);
   }
   return myCommand;
}

// ============================================================================
//virtual
//! Fire the weapon at a specific target.
WsfWeapon::FireResult PhaserWeapon::Fire(double                         aSimTime,
                                         const WsfWeapon::FireTarget&   aTarget,
                                         const WsfWeapon::FireOptions&  aSettings)
{
   // Call base class' Fire method
   WsfWeapon::FireResult result = WsfImplicitWeapon::Fire(aSimTime, aTarget, aSettings);

   // EXERCISE 2 TASK 4a
   // Create a new fire update event (pointer)
   // Set the attributes of the event for
   // - firing platform index
   // - the firing weapon
   // PLACE YOUR CODE HERE

   // Use the target name, either directly or from a provided track
   if (!aTarget.mTargetName.empty())  // mTargetName may change to a UtStringId for AFSIM 1.10
   {
      eventPtr->mTargetName = aTarget.mTargetName;
   }
   else if (aTarget.mTrackPtr != nullptr)
   {
      eventPtr->mTargetName = aTarget.mTrackPtr->GetTargetName();
   }
   else
   {
      result.mSuccess = false;
   }

   if (result.mSuccess)
   {
      // Set the attributes of the event for
      // - the fire time left (initially, set to the fire duration)
      // - the time to fire the event (after one fire integration interval)
      // EXERCISE 2 TRAINING TASK 4b
      // PLACE YOUR CODE HERE

      // Insert information into the replay file so that firing events will be graphically
      // illustrated during replays.
      if (mDisplayEngagements)
      {
         DisplayEngagement(eventPtr->mTargetName);
      }

      GetSimulation()->AddEvent(std::move(eventPtr));
   }

   return result;
}

// ****************************************************************************
// Protected

// ============================================================================
// Copy Constructor

//!Copy constructor used by Clone()
PhaserWeapon::PhaserWeapon(const PhaserWeapon& aSrc)
   : WsfImplicitWeapon(aSrc),
     mFireIntegrationInterval(aSrc.mFireIntegrationInterval),
     mFireDuration(aSrc.mFireDuration),
     mDisplayEngagements(aSrc.mDisplayEngagements)
{
}

// ****************************************************************************
// Private

// ============================================================================
//! Called every integration interval while firing, this method updates the
//! current engagement and checks to see whether the target is still valid
//! and whether it is masked by the horizon.
//! @return 'false' if the target platform is no longer able to be fired upon
bool PhaserWeapon::FireUpdate(double          aSimTime,
                              WsfStringId     aTargetName)
{
   bool repeatFireIsNeeded = false;
   WsfPlatform* targetPtr = GetSimulation()->GetPlatformByName(aTargetName);

   if (targetPtr != nullptr)
   {
      double wpnLat, wpnLon, wpnAlt;
      GetLocationLLA(wpnLat, wpnLon, wpnAlt);
      double tgtLat, tgtLon, tgtAlt;
      targetPtr->GetLocationLLA(tgtLat, tgtLon, tgtAlt);

      // Verify the target isn't masked by the earth's horizon
      if (! UtSphericalEarth::MaskedByHorizon(wpnLat, wpnLon, wpnAlt, tgtLat, tgtLon, tgtAlt, 1.0))
      {
         repeatFireIsNeeded = true;

         //Update the engagement.  It will apply damage and destroy any targets.
         GetEngagement()->Update(aSimTime);

         if (mDisplayEngagements)
         {
            DisplayEngagement(aTargetName);
         }
      }
   }
   return repeatFireIsNeeded;
}

// ============================================================================
//! Called when the phaser is done firing at a target.
void PhaserWeapon::FireComplete(double          aSimTime,
                                WsfStringId     aTargetName)
{
   if (mDisplayEngagements)
   {
      DisplayEngagement(aTargetName, true);  // Erase any engagement data
   }

   // Call base class' method to automatically terminate the engagement.
   WsfImplicitWeapon::CeaseFire(aSimTime);
}

// ****************************************************************************
// ****************************************************************************
// Definition for nested class

//! Execute the FireUpdateEvent
WsfEvent::EventDisposition PhaserWeapon::FireUpdateEvent::Execute()
{
   EventDisposition disposition = cRESCHEDULE;
   if (mComplete)
   {
      mWeaponPtr->FireComplete(GetTime(), mTargetName);
      disposition = cDELETE;
   }
   else
   {
      mComplete = true;

      // Ensure the firing platform is still alive during the engagement
      if (GetSimulation()->PlatformExists(mPlatformIndex))
      {
         bool repeat = mWeaponPtr->FireUpdate(GetTime(), mTargetName);
         if (repeat)
         {
            // EXERCISE 2 TASK 5a
            // Compute the remaining fire time
            // PLACE YOUR CODE HERE

            if (mFireTimeLeft > 0.0)
            {
               // EXERCISE 2 TASK 5b
               // Set the time the event will next execute to the end of the fire time
               // or the next integration update time, whichever is first.
               // PLACE YOUR CODE HERE

               mComplete = false;
            }
         }
      }
      if (mComplete)
      {
         // Set the last time the event will execute to a slightly later
         // time.  This is a common practice; in this case it allows the
         // WsfDraw erase command to execute properly.
         SetTime(GetTime() + 0.001);
      }
   }
   return disposition;
}

//! Draw a line and display information from the weapon to the target.
//! Note: This can now be done completely in script.
void PhaserWeapon::DisplayEngagement(WsfStringId  aTargetName,
                                     bool         aErase)  // = false
{
   // Create a unique ID based on the weapon name and track ID
   // which is used to erase the line later
   std::stringstream ss;
   ss << GetPlatform()->GetName() << "." << GetName() << "." << aTargetName;
   WsfDraw d(*GetSimulation());
   d.Erase(ss.str());

   if (! aErase)  // draw
   {
      WsfPlatform* targetPtr = GetSimulation()->GetPlatformByName(aTargetName);
      if (targetPtr != nullptr)
      {
         d.SetId(ss.str());
         d.SetColor(1.0, 0, 0);

         // a single line between two platforms
         d.BeginLines();
         d.Vertex(*GetPlatform());
         d.Vertex(*targetPtr);
         d.End();

         if (targetPtr->GetAuxData().AttributeExists("phaser_shields"))
         {
             double shieldValue = targetPtr->GetAuxData().GetDouble("phaser_shields");
             double armorValue  = targetPtr->GetAuxData().GetDouble("phaser_armor");
             ss.str("");  // Clear the stringstream
             ss << (unsigned)shieldValue << " : " << (unsigned)armorValue;
             d.SetTextSize(10);
             if (shieldValue > 0.0)
             {
                d.SetColor(0.7, 0.7, 0.1);
             }
             else
             {
                d.SetColor(0.9, 0.1, 0.1);
             }
             d.BeginText(ss.str());
             d.Vertex(*targetPtr);
             d.End();
         }
      }
   }
}

//! FireUpdateEvent constructor
PhaserWeapon::FireUpdateEvent::FireUpdateEvent()
   : mFireTimeLeft(0)
   , mPlatformIndex(0)
   , mWeaponPtr(nullptr)
   , mComplete(false)
{
}
