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

#include "ShieldComponent.hpp"

#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtIntersectMesh.hpp"
#include "UtUnitTypes.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "UtRandom.hpp"
#include "UtScriptTypes.hpp"
#include "WsfAttributeContainer.hpp"
#include "observer/WsfCommObserver.hpp"
#include "WsfComponentRoles.hpp"
#include "WsfMessage.hpp"
#include "WsfPlatform.hpp"
#include "processor/WsfProcessor.hpp"
#include "WsfSimulation.hpp"

ShieldComponent::ShieldComponent(WsfScenario&  aScenario)
   : WsfPlatformPart(aScenario, cCOMPONENT_ROLE<ShieldComponent>()),
     mUpdateInterval(-1.0),
     mInitialStrength(0.0),
     mRechargeRate(0.0),
     mStrength(0.0),
     mLastUpdateTime(0.0)
{
   SetName("shields");
}

ShieldComponent::ShieldComponent(const ShieldComponent& aSrc)
   : WsfPlatformPart(aSrc),
     mUpdateInterval(aSrc.mUpdateInterval),
     mInitialStrength(aSrc.mInitialStrength),
     mRechargeRate(aSrc.mRechargeRate),
     mStrength(aSrc.mStrength),
     mLastUpdateTime(aSrc.mLastUpdateTime)
{
}

ShieldComponent& ShieldComponent::operator=(const ShieldComponent& aSrc)
{
   mUpdateInterval = aSrc.mUpdateInterval;
   mInitialStrength = aSrc.mInitialStrength;
   mRechargeRate = aSrc.mRechargeRate;
   mStrength = aSrc.mStrength;
   mLastUpdateTime = aSrc.mLastUpdateTime;
   return *this;
}

void ShieldComponent::MessageReceived(double             aSimTime,
                                      wsf::comm::Comm*   aXmtrPtr,
                                      wsf::comm::Comm*   aRcvrPtr,
                                      const WsfMessage&  aMessage,
                                      wsf::comm::Result& aResult)
{
   std::string type = aMessage.GetSubType();
   if (type == "DROP_SHIELDS")
   {
      { // RAII block
         auto out = ut::log::info() << "Turning off shield component.";
         out.AddNote() << "T = " << aSimTime;
         out.AddNote() << "Platform: " << GetPlatform()->GetName();
         out.AddNote() << "Shield: " << GetName();
      }
      
      GetPlatform()->GetSimulation()->TurnPartOff(aSimTime, this);
   }
}

bool ShieldComponent::Initialize(double aSimTime)
{
   if (InitiallyTurnedOn())
   {
      GetSimulation()->TurnPartOn(aSimTime, this);
   }
   mStrength = mInitialStrength;
   mCallbacks.Add(WsfObserver::MessageReceived(GetSimulation()).Connect(&ShieldComponent::MessageReceived, this));
   return WsfPlatformPart::Initialize(aSimTime);
}

void ShieldComponent::Update(double aSimTime)
{
   if (mStrength < mInitialStrength)
   {
      mStrength = std::min(mInitialStrength, mStrength + mRechargeRate * (aSimTime - mLastUpdateTime));
   }
   mLastUpdateTime = aSimTime;
}

namespace
{
   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, Shields, 0, "Shields", "")
   {
      // Note the following will only return a valid pointer if the platform has a "shields" component.
      ShieldComponent* ppPtr = aObjectPtr->template GetComponent<ShieldComponent>();
      aReturnVal.SetPointer(new UtScriptRef(ppPtr, aReturnClassPtr));
   }
}

void ShieldComponent::RegisterScriptMethods(UtScriptTypes & aScriptTypes)
{
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<Shields>());
}

void ShieldComponent::RegisterScriptTypes(UtScriptTypes& aScriptTypes)
{
   aScriptTypes.Register(ut::make_unique<WsfScriptShieldComponentClass>("Shields", &aScriptTypes));
}

//virtual
WsfObject* ShieldComponent::Clone() const
{
   return new ShieldComponent(*this);
}

// virtual
WsfComponent* ShieldComponent::CloneComponent() const
{
   return new ShieldComponent(*this);
}

// virtual
const int* ShieldComponent::GetComponentRoles() const
{
   // EXERCISE 2 TASK 6
   // Define the set of roles as a static array of ints.
   // Initialize this array to consist of 'this' component role
   // (cWSF_COMPONENT_SHIELDS), platform part (cWSF_COMPONENT_PLATFORM_PART),
   // and the null component (cWSF_COMPONENT_NULL).
   static const int roles[] = { cWSF_COMPONENT_SHIELDS,
                                cWSF_COMPONENT_PLATFORM_PART,
                                cWSF_COMPONENT_NULL
                              };
   return roles;
}

// virtual
void* ShieldComponent::QueryInterface(int aRole)
{
   // Exercise 2 TASK 7
   // Return a properly cast pointer according to the given role.
   // If the given role is not one supported by GetCompoenntRoles
   // (or if it is cWSF_COMPONENT_NULL), return zero.
   if (aRole == cWSF_COMPONENT_SHIELDS)
   {
      return this;
   }
   else if (aRole == cWSF_COMPONENT_PLATFORM_PART)
   {
      return (static_cast<WsfPlatformPart*>(this));
   }
   return nullptr;
}

//virtual
bool ShieldComponent::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "initial_strength")
   {
      aInput.ReadValueOfType(mInitialStrength, UtInput::cENERGY);
   }
   else if (command == "recharge_rate")
   {
      aInput.ReadValueOfType(mRechargeRate, UtInput::cPOWER);
   }
   else
   {
      myCommand = WsfPlatformPart::ProcessInput(aInput);
   }
   return myCommand;
}

WsfScriptShieldComponentClass::WsfScriptShieldComponentClass(const std::string & aClassName,
                                                             UtScriptTypes*      aScriptTypesPtr)
   : WsfScriptPlatformPartClass(aClassName, aScriptTypesPtr)
{
   SetClassName("Shields");

   AddMethod(ut::make_unique<Strength>());
}

UT_DEFINE_SCRIPT_METHOD(WsfScriptShieldComponentClass, ShieldComponent, Strength, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetStrength());
}

