// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#ifndef FLIGHT_CONTROLLER_WIDGET
#define FLIGHT_CONTROLLER_WIDGET

#include <QtWidgets>
#include <QCloseEvent>
#include "FlightControllerInterface.hpp"
#include "ui_flight.h"

class FlightControllerWidget : public QMainWindow
{
   Q_OBJECT
public:

   explicit FlightControllerWidget(FlightControllerInterface* aInterface);

   ~FlightControllerWidget() override;

   void closeEvent(QCloseEvent* aEvent) override;

   void keyPressEvent(QKeyEvent* aEvent) override;
   void keyReleaseEvent(QKeyEvent* aEvent) override;
   void showPopup();


private:
   FlightControllerInterface* mInterface;
   Ui::FlightControllerMainWindow mUI;
};


#endif
