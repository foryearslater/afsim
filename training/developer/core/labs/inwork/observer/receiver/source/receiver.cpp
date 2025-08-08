// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

// Test client
// This is a genio (AFSIM libaray)-based
// alternative to use of python and "receiver.py"
// to receive and display packets sent from
// the observer exercise.
// Any packets received are echoed to stdout.

#include "UtLog.hpp"

#include "GenBufIManaged.hpp"
#include "GenNetIO.hpp"

int main(int argc, char* argv[])
{
   int localPort = 19240;

   // Use GenIO to read packets from the network.
   // Create a GenNetIO object to read local data from a port using UDP.
   std::unique_ptr<GenNetIO> clientPtr(GenNetIO::Create(localPort, GenNetIO::cOPT_UDP));

   while(true)
   {
      int received = clientPtr->Receive(1000);
      char data[2048];
      while (received > 0)
      {
         int status = clientPtr->GetRecvBuffer()->GetRaw(data, received);
         ut::log::info() << "Received: " << data;
         if (status > 0)   // packet was read and displayed successfully
         {
            received = 0;  // try to receive more packets
         }
      }
   }

   return 0;
}
