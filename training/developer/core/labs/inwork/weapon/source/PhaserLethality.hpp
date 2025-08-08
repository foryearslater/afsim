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

#ifndef PHASER_LETHALITY_HPP
#define PHASER_LETHALITY_HPP

class UtInput;

class WsfPlatform;
class WsfScenario;
#include "WsfWeaponEffects.hpp"
class WsfWeaponEngagement;

/** A simple weapon effect calculating the lethal effect from a
    Phaser (PHASed Energy Rectification) weapon.  This effect
    first removes "shields", then "armor", then kills the target
    when both have been reduced to zero.
*/
class PhaserLethality : public WsfWeaponEffects
{
   public:
      explicit PhaserLethality(WsfScenario& aScenario);
      PhaserLethality(const PhaserLethality& aLethality);

      PhaserLethality& operator=(const PhaserLethality&) = delete;

      ~PhaserLethality() noexcept override = default;

      WsfWeaponEffects* Clone() const override
      {
         return new PhaserLethality(*this);
      }

      bool ProcessInput(UtInput& aInput) override;

      bool Initialize(double                     aSimTime,
                      const WsfWeaponEngagement* aEngagementPtr) override;

   protected:

      void ApplyEffectTo(double       aSimTime,
                         WsfPlatform* aTargetPtr) override;

   private:

      //! For each second of beam on target, the shield is reduced by this amount
      double   mShieldDamageRate;

      //! Once the shield is gone, the armor is reduced by this amount.
      double   mArmorDamageRate;

      //! The last time we applied damage to the target
      double   mLastUpdateTime;
};
#endif
