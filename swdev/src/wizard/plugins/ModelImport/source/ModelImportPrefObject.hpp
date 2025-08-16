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
#ifndef MODELIMPORT_PREFOBJECT_HPP
#define MODELIMPORT_PREFOBJECT_HPP

#include <QDateTime>

#include "WkfPrefObject.hpp"

namespace ModelImport
{
//! Contains the state of the preferences.
struct PreferenceData
{
   QString mPath        = QString();
   QString mMetaFile    = "importData.json";
   QString mImportsFile = "imports/imports.txt";
   QString mErrorsFile  = "importErrors.log";
   QString mDisplayMode = "Category List";
};


class PrefObject : public wkf::PrefObjectT<PreferenceData>
{
   Q_OBJECT

public:
   PrefObject(QObject* aParent = nullptr);

   //! Applies the current preferences stored.
   virtual void Apply() override;

   //! Indicates that something has changed that requires action when Apply() is called.
   void SetUpdateFlag();

signals:
   void Update();
   void UpdateDisplay();

private:
   //! Updates the preferences.
   //! @param aPrefData The new set of preferences.
   void SetPreferenceDataP(const PreferenceData& aPrefData) override;

   //! Reads the preferences from a file.
   //! @param aSettings The settings file to read from.
   PreferenceData ReadSettings(QSettings& aSettings) const override;

   //! Stores the preferences to a file.
   //! @param aSettings The settings file to write to.
   void SaveSettingsP(QSettings& aSettings) const override;

   bool mPreferencesChanged = false;
};
} // namespace ModelImport

Q_DECLARE_METATYPE(ModelImport::PreferenceData)

#endif
