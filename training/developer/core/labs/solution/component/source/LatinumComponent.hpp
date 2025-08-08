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

#ifndef LATINUMCOMPONENT_HPP
#define LATINUMCOMPONENT_HPP

#include <string>

#include "ComponentRoles.hpp"

#include "UtScriptBasicTypes.hpp"

#include "WsfObject.hpp"
#include "WsfObjectTypeList.hpp"
#include "WsfComponent.hpp"

class     WsfPlatform;
class     WsfScenario;

//! An example of a simple platform component (See WsfSimplePlatformCompoent).
//! Latinum are meant to be single instances on platforms with only a defined quantity.
class LatinumComponent : public WsfPlatformComponent,
                         public WsfObject
{
   public:
      LatinumComponent();
      LatinumComponent(const LatinumComponent& aSrc);
      LatinumComponent& operator=(const LatinumComponent& aSrc);

      ~LatinumComponent() noexcept override = default;

      //! @name Component infrastructure methods.
      //@{
      WsfStringId   GetComponentName()  const override { return GetNameId(); }
      const int*    GetComponentRoles() const override;
      WsfComponent* CloneComponent()    const override;
      void*         QueryInterface(int aRole) override;
      static LatinumComponent* Find(const WsfPlatform& aParent);
      static LatinumComponent* FindOrCreate(WsfPlatform& aParent);
      //@}

      // Framework methods.
      //@{
      bool ProcessInput(UtInput& aInput) override;
      WsfObject* Clone() const override { return new LatinumComponent(*this); }
      //@}

      static void RegisterScriptTypes(UtScriptTypes& aScriptTypes);
      static void RegisterScriptMethods(UtScriptTypes& aScriptTypes);

      double GetQuantity() const { return mQuantity; }

   protected:

   private:

      double           mQuantity;
};

//! The script interface 'class' for WsfWeaponTask
class WsfScriptLatinumComponentClass : public UtScriptClass
{
   public:
      WsfScriptLatinumComponentClass(const std::string& aClassName,
                                     UtScriptTypes*     aScriptTypesPtr);

      UT_DECLARE_SCRIPT_METHOD(Quantity);

      // EXERCISE 4 TASK 1
      UT_DECLARE_SCRIPT_METHOD(TransferTo);
};

//! A type that only uses functionality of base class to register itself with the scenario.
class LatinumTypes : public WsfObjectTypeList<LatinumComponent>
{
   public:
      explicit LatinumTypes(WsfScenario& aScenario);
};

// EXERCISE 2 TASK 1b
// Register the latinum component type using macro defined in WsfComponentRoles.hpp
WSF_DECLARE_COMPONENT_ROLE_TYPE(LatinumComponent, cWSF_COMPONENT_LATINUM)

#endif
