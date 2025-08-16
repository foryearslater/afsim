// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMBROWSERSIMCOMMANDS_HPP
#define PLATFORMBROWSERSIMCOMMANDS_HPP

#include <QList>

#include "WkSimInterface.hpp"

namespace WkPlatformBrowser
{
class DeletePlatformsCommand : public warlock::SimCommand
{
public:
   DeletePlatformsCommand(QList<std::string> aPlatforms)
      : SimCommand(true)
      , mPlatformList(aPlatforms)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   QList<std::string> mPlatformList;
};
} // namespace WkPlatformBrowser
#endif