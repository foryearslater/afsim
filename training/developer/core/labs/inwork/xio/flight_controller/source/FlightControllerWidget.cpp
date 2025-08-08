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

#include <cstdlib>
#include <QtWidgets>
#include <QCloseEvent>
#include "FlightControllerConstants.hpp"
#include "FlightControllerWidget.hpp"

FlightControllerWidget::FlightControllerWidget(FlightControllerInterface* aInterface)
{
   mUI.setupUi(this);
   mInterface = aInterface;
}

FlightControllerWidget::~FlightControllerWidget()
{
   close();
}

void FlightControllerWidget::closeEvent(QCloseEvent *aEvent)
{
   close();
   FlightControllerWidgetException fcwe = 0;
   throw(fcwe);
   // if we reach this point we have a big problem, so simply exit
   exit(0);
}

void FlightControllerWidget::keyPressEvent(QKeyEvent* aEvent)
{
   if (aEvent->type() == QKeyEvent::KeyPress)
   {
      mInterface->HandleKeyPress(true, aEvent->key());
   }
}

void FlightControllerWidget::keyReleaseEvent(QKeyEvent* aEvent)
{
   if (aEvent->type() == QKeyEvent::KeyRelease)
   {
   }
}
