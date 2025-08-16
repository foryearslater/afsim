// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SENSORCONTROLLERDOCKWIDGET_HPP
#define SENSORCONTROLLERDOCKWIDGET_HPP

#include <QDockWidget>
#include <QPushButton>

#include "SensorControllerDataContainer.hpp"
#include "SensorControllerSimInterface.hpp"
#include "WkPlatform.hpp"
#include "WkScriptSimInterface.hpp"
#include "ui_DockWidget.h"

namespace WkSensorController
{
// This is the class that implements the dock widget defined by a user.
class DockWidget : public QDockWidget
{
public:
   DockWidget(SimInterface*   aInterfacePtr,
              DataContainer&  aDataContainer,
              QWidget*        aParent = nullptr,
              Qt::WindowFlags aFlags  = Qt::WindowFlags());
   ~DockWidget() override = default;

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatformPtr);
   void SensorComboBoxChanged(const QString& aSensor);
   void UpdateCurrentSensor(const std::string& aPlatform, const std::string& aSensor);
   void UpdateGui(const DataContainer::Sensor& aSensor);

   void SlewComplete(const std::string& aPlatform, const std::string& aSensor);

   void CueToTrack();
   void CueToAngle();
   void TurnOn();
   void TurnOff();

   void Clear();

   SimInterface*  mInterfacePtr{nullptr};
   DataContainer& mDataContainer;
   int            mSensorId;
   Ui::DockWidget mUI;
};
} // namespace WkSensorController
#endif
