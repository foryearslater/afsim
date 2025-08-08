// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WARLOCK_TRAINING_PREFOBJECT_HPP
#define WARLOCK_TRAINING_PREFOBJECT_HPP

#include "WkfPrefObject.hpp"

namespace WarlockTraining
{
   //! This represents the data that the preferences widget modifies.
   //! @note A default constructed PrefData is used for the default preferences.
   //! Use default member initializers (as used here), and/or a default constructor.
   struct PrefData
   {
      bool mDisplayAltitude = true;
      bool mDisplayHeading  = true;
   };

   //! This provides an interface to load, save, and apply changes to the preferences.
   //! Inheriting from wkf::PrefObjectT<T> tells the class what type of data it is dealing with.
   class PrefObject : public wkf::PrefObjectT<PrefData>
   {
      // This line is required for Qt signals to be properly emitted from this class.
      Q_OBJECT

   public:
      static constexpr const char* cNAME = "WarlockTrainingPreferences";

      PrefObject(QObject* aParent = nullptr);
      ~PrefObject() override = default;

      //! Applies changes to the PrefData to the rest of the application.
      //! Called internally AFTER SetPreferenceDataP().
      void Apply() override;

      //! Sets the current preferences.
      //! If applying changes to every preference would cause a significant delay,
      //! this function is where flags can be set to indicate which preferences need re-applied.
      //! @param aPrefData This is the new preferences.
      void SetPreferenceDataP(const PrefData& aPrefData) override;

      //! Reads the current preferences from a file.
      //! @param aSettings This is the interface to the file where preference data is stored.
      //! @returns The settings read.
      PrefData ReadSettings(QSettings& aSettings) const override;

      //! Saves the current preferences to a file.
      //! @param aSettings This is the interface to the file where preference data is stored.
      void SaveSettingsP(QSettings& aSettings) const override;

   signals:
      //! Used by Apply() to only update the display when the preferences have actually changed.
      //! @param aPreferences This is the current preferences.
      //! @note In practice, one could have multiple such functions to update each non-trivial setting.
      void Changed(const PrefData& aPreferences);

   private:
      //! Represents whether the preferences have changed.
      bool mPreferencesChanged = false;
   };
}

Q_DECLARE_METATYPE(WarlockTraining::PrefData)

#endif
