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
#ifndef PLATFORMMOVEMENTDOCKWIDGET_HPP
#define PLATFORMMOVEMENTDOCKWIDGET_HPP

#include <QDockWidget>
#include <QMutex>

#include "UtUnitTypes.hpp"
#include "VaCallbackHolder.hpp"
#include "WkfPlatform.hpp"

class WkUnitsObject;
#include "PlatformMovementSimInterface.hpp"
#include "RouteBrowserInterface.hpp"
#include "WkPlatform.hpp"
#include "ui_PlatformMovementDockWidget.h"

namespace WkPlatformMovement
{
class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   DockWidget(SimInterface*          aSimInterfacePtr,
              RouteBrowserInterface* aRouteInterface,
              QWidget*               parent = nullptr,
              Qt::WindowFlags        f      = Qt::WindowFlags());
   ~DockWidget() override = default;

   void Update();

signals:
   int RouteComboBoxSelected(const QString& aRouteName);

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatform);

   void LocationStarted();
   void LocationTerminated();
   void LocationChanged(double aLatitude, double aLongitude);
   void LocationSelected(double aLatitude, double aLongitude, double aAltitude, bool aAltitudeValid);

   void GoToAltitude();
   void GoToLocation();
   void GoToSpeed();
   void TurnToHeading();
   void FollowRoute();
   void DelayRoute();
   void ReturnToRoute();

   void AltitudeRateOfChangeToggled(bool aChecked);
   void LocationAltitudeToggled(bool aChecked);
   void SpeedLinearAccelerationToggled(bool aChecked);
   void HeadingRadialAccelerationToggled(bool aChecked);

   void RouteComboBoxActivatedHandler(const QString& aRouteName);

   warlock::Platform* mEntityPtr;

   double mLatitudeBeforeSelection;
   double mLongitudeBeforeSelection;

   RouteBrowserInterface*         mRouteInterfacePtr;
   SimInterface*                  mSimInterfacePtr;
   Ui::PlatformMovementDockWidget mUI;
};
} // namespace WkPlatformMovement
#endif // PLATFORMMOVEMENTDOCKWIDGET_HPP
