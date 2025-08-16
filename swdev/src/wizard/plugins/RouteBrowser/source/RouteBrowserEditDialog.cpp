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


#include "RouteBrowserEditDialog.hpp"

#include "map/Platform.hpp"

RouteBrowser::RouteBrowserEditDialog::RouteBrowserEditDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : wkf::RouteBrowserEditDialog(aParent, aFlags)
{
}

void RouteBrowser::RouteBrowserEditDialog::showEvent(QShowEvent* aEvent)
{
   // lock down all platforms while the route edit dialog is open.  Moving them causes too much route rebuilding and
   // leads to bad places.
   vespa::VaEntityList platformList;
   vaEnv.GetEntityListOfType<wizard::Platform>(platformList);
   for (auto&& curPlatform : platformList)
   {
      curPlatform->SetDraggable(false);
   }
}

void RouteBrowser::RouteBrowserEditDialog::hideEvent(QHideEvent* aEvent)
{
   // unlock all platforms
   vespa::VaEntityList platformList;
   vaEnv.GetEntityListOfType<wizard::Platform>(platformList);
   for (auto&& curPlatform : platformList)
   {
      curPlatform->SetDraggable(true);
   }
}
