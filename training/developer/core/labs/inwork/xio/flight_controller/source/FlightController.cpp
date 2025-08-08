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

#include <string>

#include "FlightControllerConstants.hpp"
#include "FlightControllerWidget.hpp"
#include "FlightControllerInterface.hpp"
#include "FlightControllerPlatformListRequest.hpp"
#include "UtWallClock.hpp"
#include "UtLog.hpp"
#include "UtSleep.hpp"

//! This is a simple main loop that Initializes and Updates the Flight Controller.
int main(int argc, char* argv[])
{
   try {
      QApplication app(argc, argv);
      auto fci = ut::make_unique<FlightControllerInterface>();
      auto mainWindow = ut::make_unique<FlightControllerWidget>(fci.get());
      mainWindow->show();

      if (argc != 2)
      {
         { // RAII block
            auto out = ut::log::fatal() << "Invalid arguments.";
            out.AddNote() << "Usage: flight_controller <config-file>";
         }
         exit(0);
      }

      std::string inputFile = argv[1];

      {
         ut::log::info() << "Initializing FlightControllerInterface.";
      }
      if (fci->Initialize(inputFile))
      {
         UtWallClock clock;
         clock.ResetClock();

         QCoreApplication::processEvents();

         if (! FlightControllerInterface::IsFirstPktReceived())
         {
            auto out = ut::log::info() << "First PlatformListUpdate packet not received.";
            out.AddNote() << "---> Attempting to get PlatformListUpdate packet.";
         }

         // attempt to get PlatformListUpdate packet if first one still not yet received
         while (! FlightControllerInterface::IsFirstPktReceived())
         {
            fci->GetPlatformList(clock.GetClock());
            UtSleep::Sleep(1.0);  // execute approximately 1 times a second
            QCoreApplication::processEvents();
            fci->AdvanceTime(clock.GetClock());
         }

         // notify console log that first PlatformListUpdate packet has now been received
         {
            auto out = ut::log::info() << "Received first PlatformListUpdate packet.";
            out.AddNote() << "Time: " << clock.GetClock();
         }

         // The following in the main loop could also be threaded.
         while (true)
         {
            QCoreApplication::processEvents();
            fci->Update(clock.GetClock());
            UtSleep::Sleep(0.01); // execute approximately 100 times a second
         }
      }

      return app.exec();
   }
   catch (FlightControllerWidgetException fcwe)
   {
      return (static_cast<int>(fcwe));
   }
}
