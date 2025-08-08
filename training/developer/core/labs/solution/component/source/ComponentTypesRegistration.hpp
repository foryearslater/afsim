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

#ifndef COMPONENTTYPESREGISTRATION_HPP
#define COMPONENTTYPESREGISTRATION_HPP

#include "LatinumComponent.hpp"
#include "ShieldTypes.hpp"
#include "CyberSensorEffect.hpp"

#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "UtMemory.hpp"

//! Provide a way to register the new shield types list with the scenario.
class ComponentTypesRegistration : public WsfScenarioExtension
{
   public:

      ComponentTypesRegistration() = default;

      void AddedToScenario() override
      {
         // Register our custom type lists with the scenario.
         // Shields
         mShieldTypesIndex = GetScenario().GetTypeLists().size();

         auto shieldTypesPtr = ut::make_unique<ShieldTypes>(GetScenario());
         GetScenario().AddTypeList(std::move(shieldTypesPtr));

         // Latinum
         // Note we do not need to access the type list for latinum as we are only
         // using it to process input and add instances to the platform.
         GetScenario().AddTypeList(ut::make_unique<LatinumTypes>(GetScenario()));
      }

      ShieldTypes&   GetShieldTypes() const { return *static_cast<ShieldTypes*>(GetScenario().GetTypeLists().at(mShieldTypesIndex)); }

   private:

      size_t         mShieldTypesIndex;
};

#endif
