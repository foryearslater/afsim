// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CyberSensorEffect.hpp"

#include "WsfComponentFactory.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSensor.hpp"
#include "WsfTrack.hpp"
#include "UtMemory.hpp"

class WsfSensorResult;

namespace
{
   class CyberSensorComponentFactory : public WsfComponentFactory<WsfSensor>
   {
      public:

         bool ProcessAddOrEditCommand(UtInput&   aInput,
                                      WsfSensor& aParent,
                                      bool       aIsAdding) override
         {
            // No add or edit commands to process
            return false;
         }

         bool ProcessInput(UtInput&   aInput,
                           WsfSensor& aParent) override
         {
            std::string command;
            aInput.GetCommand(command);
            bool myCommand = false;

            if (command == "cyber_effect")
            {
               CyberSensorEffect* cbePtr = CyberSensorEffect::FindOrCreate(aParent);
               aInput.ReadCommand(command);
               if (command == "track_pulloff")
               {
                  cbePtr->SetType(CyberSensorEffect::cTRACK_PULLOFF);
                  myCommand = true;
               }
               else if (command == "track_drop")
               {
                  cbePtr->SetType(CyberSensorEffect::cTRACK_DROP);
                  myCommand = true;
               }
               else if (command == "exploit_delay")
               {
                  // EXERCISE 3 TASK 1
                  // Create a variable which is a double,
                  // use that variable to read in teh time value (cTIME) from input, and
                  // use that variable to se the CyberSensorEffect's exploit delay
                  double delay;
                  aInput.ReadValueOfType(delay, UtInput::cTIME);
                  cbePtr->SetExploitDelay(delay);
               }
               else
               {
                  throw UtInput::UnknownCommand(aInput);
               }
            }
            return myCommand;
         }
      };
}

// =================================================================================================
CyberSensorEffect::CyberSensorEffect()
   : mType(cUNDEFINED),
     mExploitTime(-1.0),
     mExploitDelay(60.0)
{
}

// =================================================================================================
CyberSensorEffect::CyberSensorEffect(const CyberSensorEffect& aSrc)
   : mType(aSrc.mType),
     mExploitTime(aSrc.mExploitTime),
     mExploitDelay(aSrc.mExploitDelay)
{
}

CyberSensorEffect& CyberSensorEffect::operator=(const CyberSensorEffect& aSrc)
{
   mType = aSrc.mType;
   mExploitTime = aSrc.mExploitTime;
   mExploitDelay = aSrc.mExploitDelay;
   return *this;
}

// =================================================================================================
//! Register the component factory that handles input for this component.
void CyberSensorEffect::RegisterComponentFactory(WsfScenario& aScenario)
{
   aScenario.RegisterComponentFactory(ut::make_unique<CyberSensorComponentFactory>());
}

// =================================================================================================
//! Find the instance of this component attached to the specified sensor.
CyberSensorEffect* CyberSensorEffect::Find(const WsfSensor& aParent)
{
   CyberSensorEffect* componentPtr(nullptr);
   aParent.GetComponents().FindByRole<CyberSensorEffect>(componentPtr);
   return componentPtr;
}

// =================================================================================================
//! Find the instance of this component attached to the specified processor,
//! and create it if it doesn't exist.
CyberSensorEffect* CyberSensorEffect::FindOrCreate(WsfSensor& aParent)
{
   CyberSensorEffect* componentPtr = Find(aParent);
   if (componentPtr == nullptr)
   {
      componentPtr = new CyberSensorEffect;
      aParent.GetComponents().AddComponent(componentPtr);
   }
   return componentPtr;
}

// =================================================================================================
//! Create a copy of this object instance as a WsfComponent*.
WsfComponent* CyberSensorEffect::CloneComponent() const
{
   return new CyberSensorEffect(*this);
}

// =================================================================================================
//! Return the unique name for this component type.
WsfStringId CyberSensorEffect::GetComponentName() const
{
   static WsfStringId id = "WSF_CYBER_SENSOR_EFFECT";
   return id;
}

// =================================================================================================
//! Return the "roles" associated with this component.
//! Assigning a unique role to this type allows for a "RoleInterator" search.
const int* CyberSensorEffect::GetComponentRoles() const
{
   // EXERCISE 2 TASK 2
   // Define the set of roles as a static array of ints.
   // Initialize this array to consist of 'this' component role
   // (cWSF_COMPONENT_CYBER_SENSOR_EFFECT), sensor (cWSF_COMPONENT_SENSOR),
   // and the null component (cWSF_COMPONENT_NULL).
   static int roles[] = { cWSF_COMPONENT_CYBER_SENSOR_EFFECT,
                          cWSF_COMPONENT_SENSOR,
                          cWSF_COMPONENT_NULL };
   return roles;
}

// =================================================================================================
//! If this object type supports the given interface (i.e., role), return an appropriately-cast pointer.
void* CyberSensorEffect::QueryInterface(int aRole)
{
   // EXERCISE 2 TASK 3
   // Return a properly cast pointer according to the given role.
   // If the given role is no one supported by GetComponentRoles
   // (or if it is cWSF_COMPONENT_NULL), return zero.
   if (aRole == cWSF_COMPONENT_CYBER_SENSOR_EFFECT) 
   { 
      return this; 
   }
   if (aRole == cWSF_COMPONENT_SENSOR) 
   { 
      return (static_cast<WsfSensorComponent*>(this)); 
   }
   return nullptr;
}

// =================================================================================================
// virtual
bool CyberSensorEffect::TrackerAllowTracking(double                 aSimTime,
                                             const TrackerSettings& aSettings,
                                             const WsfTrackId&      aRequestId,
                                             size_t                 aObjectId,
                                             WsfTrack*              aTrackPtr,
                                             WsfSensorResult&       aResult)
{
   // If we've received a message
   // EXERCISE 3 TASK 2
   // Signal that the tracker should not allow tracking if the sensor has received the command to begin the exploit
   // Check to see that the exploit is not already occurring (mExploitTime less than zero)
   // And check to see that the sensor's aux data attribute exists, called "BEGIN_EXPLOIT"
   // If so, set the exploit time to be the current sim time plus the exploit delay time (mExploitDelayTime).
   if ((mExploitTime < 0.0) && 
      GetSensor()->GetAuxData().AttributeExists("BEGIN_EXPLOIT"))
   {
      mExploitTime = aSimTime + mExploitDelay;
   }
   return (aSimTime >= mExploitTime);
}
