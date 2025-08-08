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

#ifndef PHASERWEAPON_HPP
#define PHASERWEAPON_HPP

// Base class
#include "WsfImplicitWeapon.hpp"

// WSF
#include "WsfTrack.hpp"

//! Implements a PHASed Energy Rectification device (PHASER).
class PhaserWeapon : public WsfImplicitWeapon
{
   public:

      //! Constructor
      explicit PhaserWeapon(WsfScenario& aScenario);

      //! Virtual destructor
      ~PhaserWeapon() noexcept override = default;

      //! @name Framework methods
      //@{
      WsfWeapon* Clone() const override;
      bool ProcessInput(UtInput& aInput) override;
      //@}

      //! @name Firing method
      //@{
      FireResult Fire(double              aSimTime,
                      const FireTarget&   aTarget,
                      const FireOptions&  aSettings) override;
      //@}

   protected:

      //! Copy Constructor; used by clone
      PhaserWeapon(const PhaserWeapon& aSrc);
      PhaserWeapon& operator=(const PhaserWeapon& aSrc);

   private:

      bool FireUpdate(double          aSimTime,
                      WsfStringId     aTargetName);
      void FireComplete(double          aSimTime,
                        WsfStringId     aTargetName);
      void DisplayEngagement(WsfStringId  aTargetName,
                             bool         aErase      = false);

      //! FireUpdateEvent is executed at a regular interval
      //! to apply damage from the phaser to the target
      class FireUpdateEvent : public WsfEvent
      {
         public:
            FireUpdateEvent();
            
            ~FireUpdateEvent() noexcept override = default;
            
            EventDisposition Execute() override;

            // EXERCISE 2 TASK 1
            double        mFireTimeLeft;
            size_t        mPlatformIndex;
            PhaserWeapon* mWeaponPtr;
            WsfStringId   mTargetName;

            bool           mComplete;
      };

      //! Phaser effects are applied at this discrete time interval; units are in seconds
      double   mFireIntegrationInterval;
      //! Each time the phaser fires, it keeps the beam on the target for this much time in seconds
      double   mFireDuration;
      //! Display firing and lethality data using WsfDraw
      bool     mDisplayEngagements;
};
#endif
