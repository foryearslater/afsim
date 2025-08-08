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

#include "LatinumComponent.hpp"

#include "ComponentTypesRegistration.hpp"
#include "ComponentRoles.hpp"
#include "UtInput.hpp"
#include "UtScriptTypes.hpp"
#include "WsfComponentFactory.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "UtMemory.hpp"

// =================================================================================================

LatinumComponent::LatinumComponent()
   : WsfPlatformComponent()
   , WsfObject()
   , mQuantity(0)
{
   SetName("Latinum");
}

// =================================================================================================
LatinumComponent::LatinumComponent(const LatinumComponent& aSrc)
   : WsfPlatformComponent(aSrc)
   , WsfObject(aSrc)
   , mQuantity(aSrc.mQuantity)
{
}

// =================================================================================================
LatinumComponent& LatinumComponent::operator=(const LatinumComponent& aSrc)
{
   mQuantity = aSrc.mQuantity;
   return *this;
}

// =================================================================================================
namespace
{
   UT_DEFINE_SCRIPT_METHOD_EXT(WsfPlatform, Latinum, 0, "Latinum", "")
   {
      // Note this script method will only return a valid pointer if the platform has a "shields" component.
      LatinumComponent* lPtr = aObjectPtr->template GetComponent<LatinumComponent>();

      // EXERCISE 4 TASK 4
      // invoke aReturnVal.SetPointer to set the return value to be the pointer to the LatinumComponent
      // the argument to SetPointer is a new UtScriptRef which is constructed with the pointer lptr
      // and the variable aReturnClassPtr
      // PLACE YOUR CODE HERE

   }
}

// =================================================================================================
void LatinumComponent::RegisterScriptMethods(UtScriptTypes & aScriptTypes)
{
   aScriptTypes.AddClassMethod("WsfPlatform", ut::make_unique<Latinum>());
}

// =================================================================================================
void LatinumComponent::RegisterScriptTypes(UtScriptTypes& aScriptTypes)
{
   aScriptTypes.Register(ut::make_unique<WsfScriptLatinumComponentClass>("Latinum", &aScriptTypes));
}

// =================================================================================================
const int * LatinumComponent::GetComponentRoles() const
{
   // EXERCISE 2 TASK 4
   // Define the set of roles as a static array of ints.
   // Initialize this array to consist of 'this' component role
   // (cWSF_COMPONENT_LATINUM),  and the null component (cWSF_COMPONENT_NULL).
   // PLACE YOUR CODE HERE
   
   
   return roles;
}

// =================================================================================================
// virtual
WsfComponent* LatinumComponent::CloneComponent() const
{
   return new LatinumComponent(*this);
}

// =================================================================================================
// virtual
void* LatinumComponent::QueryInterface(int aRole)
{
   // EXERCISE 2 TASK 5
   // Return a properly cast pointer according to the given role.
   // If the given role is no one supported by GetComponentRoles
   // (or if it is cWSF_COMPONENT_NULL), return zero.
   // PLACE YOUR CODE HERE


}

// =================================================================================================
//virtual
bool LatinumComponent::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "quantity")
   {
      aInput.ReadValue(mQuantity);
   }
   else
   {
      myCommand = false;
   }
   return myCommand;
}

// =================================================================================================
WsfScriptLatinumComponentClass::WsfScriptLatinumComponentClass(const std::string & aClassName,
                                                               UtScriptTypes*      aScriptTypesPtr)
   : UtScriptClass(aClassName, aScriptTypesPtr)
{
   SetClassName("Latinum");

   AddMethod(ut::make_unique<Quantity>());

   // EXERCISE 4 TASK 2
   // PLACE YOUR CODE HERE
   
}

// =================================================================================================
//! UT_DEFINE_SCRIPT_METHOD streamlines the class definition syntax for InterfaceMethods.
//! It uses a double-dispatch technique to typecast the application object from a void*
//! to the appropriate type.  It also checks the size of the argument list and throws
//! an assert if there's an inconsistency.
//! CLASS     - The derived UtScriptClass name (e.g. WsfScriptPlatformClass).
//! OBJ_TYPE  - The type of the application object (e.g. WsfPlatform).
//! METHOD    - The derived UtScriptClass::InterfaceMethod name (e.g. GetPlatform).
//! NUM_ARGS  - The required number of arguments in the argument list.
//! RET_TYPE  - The type of the return argument as a string.
//! ARG_TYPES - A string containing a comma separated list of types. Use an empty string
//!             if there are no arguments.

//! Retrieve the quantity (number of bars) of latinum.
UT_DEFINE_SCRIPT_METHOD(WsfScriptLatinumComponentClass, LatinumComponent, Quantity, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetQuantity());
}

//! Transfer the latinum from one platform to another.
UT_DEFINE_SCRIPT_METHOD(WsfScriptLatinumComponentClass, LatinumComponent, TransferTo, 1, "void", "WsfPlatform")
{
   WsfPlatform* receiverPlatformPtr = static_cast<WsfPlatform*>(aVarArgs[0].GetPointer()->GetAppObject());
   // EXERCISE 4 TASK 3a
   // Get this component's parent platform (use GetComponentParent).
   // PLACE YOUR CODE HERE

   // EXERCISE 4 TASK 3b
   // Clone this component, casting it to the correct object type.
   // PLACE YOUR CODE HERE

   // EXERCISE 4 TASK 3c
   // Remove the component from the existing component's parent platform (use RemoveComponent).
   // PLACE YOUR CODE HERE

   // EXERCISE 4 TASK 3d
   // Add the cloned component to the receiver platform (use AddComponent).
   // PLACE YOUR CODE HERE

}

// =================================================================================================
namespace
{
   //! Component factory to process platform input.
   class LatinumComponentFactory : public WsfComponentFactory<WsfPlatform>
   {
      public:
         bool ProcessAddOrEditCommand(UtInput&     aInput,
                                      WsfPlatform& aPlatform,
                                      bool         aIsAdding) override
         {
            return false;
         }

         bool ProcessInput(UtInput&     aInput,
                           WsfPlatform& aParent) override
         {
            std::string command;
            aInput.GetCommand(command);
            bool myCommand = false;

            if (command == "latinum")
            {
               LatinumComponent* lPtr = LatinumComponent::FindOrCreate(aParent);
               aInput.ReadCommand(command);
               myCommand = lPtr->ProcessInput(aInput);
               if (! myCommand)
               {
                  throw UtInput::BadValue(aInput);
               }
            }
            return myCommand;
         }
   };
}

// =================================================================================================
//! Find the instance of this component attached to the specified sensor.
LatinumComponent* LatinumComponent::Find(const WsfPlatform& aParent)
{
   LatinumComponent* componentPtr = nullptr;
   aParent.GetComponents().FindByRole<LatinumComponent>(componentPtr);
   return componentPtr;
}

// =================================================================================================
//! Find the instance of this component attached to the specified processor,
//! and create it if it doesn't exist.
LatinumComponent* LatinumComponent::FindOrCreate(WsfPlatform& aParent)
{
   LatinumComponent* componentPtr = Find(aParent);
   if (componentPtr == nullptr)
   {
      componentPtr = new LatinumComponent();
      aParent.AddComponent(componentPtr);
   }
   return componentPtr;
}

// =================================================================================================
//! Use the type list constructor to register itself with the scenario.
LatinumTypes::LatinumTypes(WsfScenario& aScenario)
   : WsfObjectTypeList<LatinumComponent>(aScenario, cREDEFINITION_ALLOWED, "latinum")
{
   SetSingularBaseType();

   aScenario.RegisterComponentFactory(ut::make_unique<LatinumComponentFactory>());  // Allows for definition inside
   // platform, platform_type blocks.
}
