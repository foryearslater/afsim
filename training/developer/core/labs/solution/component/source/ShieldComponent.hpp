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

#ifndef SHIELDCOMPONENT_HPP
#define SHIELDCOMPONENT_HPP

#include <map>
#include <string>

#include "ComponentRoles.hpp"

#include "UtCallbackHolder.hpp"

#include "WsfComponent.hpp"
#include "comm/WsfComm.hpp"
#include "WsfObject.hpp"
#include "WsfPlatformPart.hpp"
#include "script/WsfScriptPlatformPartClass.hpp"

class     WsfPlatform;

class ShieldComponent : public WsfPlatformPart
{
   public:
      explicit ShieldComponent(WsfScenario&  aScenario);
      ShieldComponent(const ShieldComponent& aSrc);
      ShieldComponent& operator=(const ShieldComponent& aSrc);
      ~ShieldComponent() noexcept override = default;

      bool Initialize(double aSimTime) override;
      double GetUpdateInterval() const override               { return mUpdateInterval; }
      void SetUpdateInterval(double aUpdateInterval) override { mUpdateInterval = aUpdateInterval; }
      double GetStrength() const { return mStrength; }

      void MessageReceived(double            aSimTime,
                           wsf::comm::Comm*          aXmtrPtr,
                           wsf::comm::Comm*          aRcvrPtr,
                           const WsfMessage& aMessage,
                           wsf::comm::Result&  aResult);

      //! @name Component infrastructure methods.
      //@{
      WsfStringId   GetComponentName()  const override { return GetNameId(); }
      WsfComponent* CloneComponent()    const override;
      const int*    GetComponentRoles() const override;
      void*         QueryInterface(int aRole) override;
      //@}

      // Framework methods.
      //@{
      WsfObject* Clone()           const override;
      bool ProcessInput(UtInput& aInput) override;
      void Update(double aSimTime)       override;
      //@}

      static void RegisterScriptTypes(UtScriptTypes& aScriptTypes);
      static void RegisterScriptMethods(UtScriptTypes& aScriptTypes);

   protected:
   private:

      double           mUpdateInterval;
      double           mInitialStrength;
      double           mRechargeRate;
      double           mStrength;
      double           mLastUpdateTime;
      UtCallbackHolder mCallbacks;
};

//! The script interface 'class' for WsfWeaponTask
class WsfScriptShieldComponentClass : public WsfScriptPlatformPartClass
{
   public:
      WsfScriptShieldComponentClass(const std::string& aClassName,
                                    UtScriptTypes*     aScriptTypesPtr);

      UT_DECLARE_SCRIPT_METHOD(Strength);
};

// EXERCISE 2 TASK 1a
// Register the shield component type using macro defined in WsfComponentRoles.hpp
WSF_DECLARE_COMPONENT_ROLE_TYPE(ShieldComponent, cWSF_COMPONENT_SHIELDS)

#endif
