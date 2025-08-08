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

#include "ShieldTypes.hpp"

#include <sstream>

#include "ComponentTypesRegistration.hpp"
#include "ShieldTypes.hpp"

#include "WsfComponentFactory.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"

namespace
{
   //! Component factory to process platform input.
   class ShieldComponentFactory : public WsfComponentFactory<WsfPlatform>
   {
      public:
         bool ProcessAddOrEditCommand(UtInput&     aInput,
                                      WsfPlatform& aPlatform,
                                      bool         aIsAdding) override
         {
            ShieldTypes& types(ShieldTypes::Get(GetScenario()));
            return types.LoadUnnamedComponentWithoutEdit(aInput, aPlatform, aIsAdding, cWSF_COMPONENT_SHIELDS);
         }

         bool ProcessDeleteCommand(UtInput&     aInput,
                                   WsfPlatform& aPlatform) override
         {
            ShieldTypes& types(ShieldTypes::Get(GetScenario()));
            return types.DeleteUnnamedComponent(aInput, aPlatform, cWSF_COMPONENT_SHIELDS);
         }
   };
}

// =================================================================================================
//! Return a modifiable reference to the type list associated with the specified scenario.
ShieldTypes& ShieldTypes::Get(WsfScenario& aScenario)
{
   ComponentTypesRegistration& ctr = (ComponentTypesRegistration&)(aScenario.GetExtension("shield_types"));
   return ctr.GetShieldTypes();
}

// =================================================================================================
//! Return a const reference to the type list associated with the specified scenario.
const ShieldTypes& ShieldTypes::Get(const WsfScenario& aScenario)
{
   const ComponentTypesRegistration& ctr = (const ComponentTypesRegistration&)(aScenario.GetExtension("shield_types"));
   return ctr.GetShieldTypes();
}

// =================================================================================================
ShieldTypes::ShieldTypes(WsfScenario& aScenario)
   : WsfObjectTypeList<ShieldComponent>(aScenario, cREDEFINITION_ALLOWED, "shields")
{
   //SetSingularBaseType();  // Should be allowable in the future; currently not allowed due to need for default constrcutor.
   
   aScenario.RegisterComponentFactory(ut::make_unique<ShieldComponentFactory>());  // Allows for definition inside
   // platform, platform_type blocks.

   Add("WSF_SHIELDS", ut::make_unique<ShieldComponent>(aScenario));  // Dummy type "WSF_SHIELDS"
   // explicitly referenced in the input.
}
