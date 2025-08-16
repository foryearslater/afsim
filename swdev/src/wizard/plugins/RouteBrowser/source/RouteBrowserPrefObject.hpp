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
#ifndef ROUTEBROWSERPREFOBJECT_HPP
#define ROUTEBROWSERPREFOBJECT_HPP

class QSettings;
#include "WkfPrefObject.hpp"

namespace RouteBrowser
{
struct PrefData
{
   bool    mShowGlobalRoutes        = true;
   bool    mUseDefaultRouteName     = true;
   bool    mUseDefaultWaypointLabel = true;
   QString mDefaultRouteName        = "Route-";
   QString mDefaultWaypointLabel    = "Waypoint-";
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "RouteBrowser";
   PrefObject(QObject* parent = nullptr);

   bool           GetShowGlobalRoutes() const { return mCurrentPrefs.mShowGlobalRoutes; }
   bool           GetUsingDefaultRouteNames() const { return mCurrentPrefs.mUseDefaultRouteName; }
   const QString& GetDefaultRouteName() const { return mCurrentPrefs.mDefaultRouteName; }
   bool           GetUsingDefaultWaypointLabels() const { return mCurrentPrefs.mUseDefaultWaypointLabel; }
   const QString& GetDefaultWaypointLabel() const { return mCurrentPrefs.mDefaultWaypointLabel; }

private:
   void EmitAllSignals();

   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   bool mShowGlobalRoutesChanged = false;
};
} // namespace RouteBrowser

Q_DECLARE_METATYPE(RouteBrowser::PrefData)
#endif // ROUTEBROWSERPREFOBJECT_HPP
