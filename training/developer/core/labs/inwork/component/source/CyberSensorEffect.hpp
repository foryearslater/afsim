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

#ifndef CYBERSENSOREFFECT_HPP
#define CYBERSENSOREFFECT_HPP


#include "ComponentRoles.hpp"

#include "sensor/WsfSensorComponent.hpp"
#include "WsfComponentRoles.hpp"
#include "WsfTrackId.hpp"

class     WsfScenario;
class     WsfSensor;
class     WsfTrack;

//! The EW component that will be attached to all sensor systems.
class CyberSensorEffect : public WsfSensorComponent
{
   public:

      enum Type
      {
         cUNDEFINED,
         cTRACK_PULLOFF,
         cTRACK_DROP
      };

      static void RegisterComponentFactory(WsfScenario& aScenario);
      static CyberSensorEffect* Find(const WsfSensor& aSensor);
      static CyberSensorEffect* FindOrCreate(WsfSensor& aSensor);

      CyberSensorEffect();
      CyberSensorEffect(const CyberSensorEffect& aSrc);
      CyberSensorEffect& operator=(const CyberSensorEffect& aSrc);
      ~CyberSensorEffect() noexcept override = default;

      //! @name Required interface from WsfComponent.
      //@{
      WsfComponent* CloneComponent() const override;
      WsfStringId GetComponentName() const override;
      const int* GetComponentRoles() const override;
      void* QueryInterface(int aRole) override;
      //@}

      //! @name Interface from WsfSensorComponent.
      //@{
      bool TrackerAllowTracking(double                 aSimTime,
                                const TrackerSettings& aSettings,
                                const WsfTrackId&      aRequestId,
                                size_t                 aObjectId,
                                WsfTrack*              aTrackPtr,
                                WsfSensorResult&       aResult) override;

      //@}

      void SetType(Type aType) { mType = aType; }
      void SetExploitDelay(double aExploitDelay) { mExploitDelay = aExploitDelay; }

   private:

      Type   mType;
      double mExploitTime;
      double mExploitDelay;
};

// EXERCISE 2 TASK 1c
// Register the sensor cyber effect component type using macro defined in WsfComponentRoles.hpp
// PLACE YOUR CODE HERE

#endif
