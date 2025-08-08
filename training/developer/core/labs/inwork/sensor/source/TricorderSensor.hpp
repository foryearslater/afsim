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

#ifndef TRICORDERSENSOR_HPP
#define TRICORDERSENSOR_HPP

#include <vector>

#include "UtScriptClassDefine.hpp"
#include "script/WsfScriptSensorClass.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorMode.hpp"
class     WsfSensorResult;

//! A standard tricorder sensor for sensing living things.
class TricorderSensor : public WsfSensor
{
   public:

      //! Constructor
      explicit TricorderSensor(WsfScenario& aScenario);

      TricorderSensor& operator=(const TricorderSensor&) = delete;

      //! Virtual destructor
      ~TricorderSensor() noexcept override = default;

      //! @name Framework methods
      //@{
      WsfSensor* Clone() const override;
      bool       Initialize(double aSimTime) override;
      bool       ProcessInput(UtInput& aInput) override;
      void       Update(double aSimTime) override;
      //@}

      bool AttemptToDetect(double             aSimTime,
                           WsfPlatform*       aTargetPtr,
                           Settings&          aSettings,
                           WsfSensorResult&   aResult) override;

      size_t GetLifeFormTypeCount(WsfStringId  aModeNameId);
      WsfStringId GetLifeFormTypeEntry(WsfStringId  aModeNameId,
                                       unsigned int aEntry);

   protected:

      //! A 'mode' of the sensor.
      class TricorderMode : public WsfSensorMode
      {
         public:
            TricorderMode();
            TricorderMode(const TricorderMode& aSrc);
            TricorderMode& operator=(const TricorderMode& aSrc);

            WsfMode* Clone() const override;
            virtual bool Initialize(double     aSimTime,
                                    WsfSensor* aSensorPtr);
            bool ProcessInput(UtInput& aInput) override;

            bool AttemptToDetect(double               aSimTime,
                                         WsfPlatform*         aTargetPtr,
                                         WsfSensor::Settings& aSettings,
                                         WsfSensorResult&     aResult) override;

            void UpdateTrack(double             aSimTime,
                                     WsfTrack*          aTrackPtr,
                                     WsfPlatform*       aTargetPtr,
                                     WsfSensorResult&   aResult) override;

            void Deselect(double aSimTime) override {}
            void Select(double aSimTime) override   {}

            void ApplyMeasurementErrors(WsfSensor::Result& aResult) override;

            double GetLifeReading(WsfPlatform* aTargetPtr);
            bool   IsLifeForm(WsfPlatform* aTargetPtr);

            // EXERCISE 2 TASK 2
            // PLACE YOUR CODE HERE
         };

      //! The sensor-specific list of modes (not valid until Initialize is called)
      //! Required by the default sensor scheduler
      std::vector<TricorderMode*>          mTricorderModeList;

      //! Copy Constructor; used by clone
      TricorderSensor(const TricorderSensor& aSrc);

      //! Get the name of the script class associated with this class.
      //! This is necessary for proper downcasts in the scripting language.
      const char* GetScriptClassName() const override;
};

//! The script interface 'class'
class ScriptTricorderSensorClass : public WsfScriptSensorClass
{
public:
   ScriptTricorderSensorClass(const std::string& aClassName,
                              UtScriptTypes*     aTypesPtr);

   ~ScriptTricorderSensorClass() noexcept override = default;

   UT_DECLARE_SCRIPT_METHOD(LifeFormTypeCount_1);
   UT_DECLARE_SCRIPT_METHOD(LifeFormTypeCount_2);

   UT_DECLARE_SCRIPT_METHOD(LifeFormTypeEntry_1);

   // EXERCISE 3 TASK 1
   // PLACE YOUR CODE HERE

};

#endif
