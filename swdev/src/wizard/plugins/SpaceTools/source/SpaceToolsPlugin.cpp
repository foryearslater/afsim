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

#include "SpaceToolsPlugin.hpp"

#include <QMenu>

#include "Astrolabe.hpp"
#include "ConstellationMaker.hpp"
#include "ProjectWorkspace.hpp"
#include "SatelliteInserter.hpp"
#include "UtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(SpaceTools::Plugin,
                          "Space Tools",
                          "Provides tools for creating space scenarios, such as the Constellation Maker and Astrolabe.",
                          "wizard")

SpaceTools::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mConstellationMakerPtr{ut::make_unique<SpaceTools::ConstellationMaker>()}
   , mAstrolabePtr{ut::make_unique<SpaceTools::Astrolabe>()}
   , mSatelliteInserterPtr{ut::make_unique<SpaceTools::SatelliteInserter>()}

{
   mConstellationMakerActionPtr =
      new wkf::Action(QIcon::fromTheme("satellite"), "Constellation Maker", wkfEnv.GetMainWindow());
   mConstellationMakerActionPtr->setEnabled(false);
   connect(mConstellationMakerActionPtr,
           &QAction::triggered,
           mConstellationMakerPtr.get(),
           &ConstellationMaker::ShowDialogFromMenu);

   mSatelliteActionPtr = new wkf::Action(QIcon::fromTheme("satellite"), "Satellite Inserter", wkfEnv.GetMainWindow());
   mSatelliteActionPtr->setEnabled(false);
   connect(mSatelliteActionPtr, &QAction::triggered, mSatelliteInserterPtr.get(), &SatelliteInserter::ShowDialog);

   QMenu* tMenuPtr = wkfEnv.GetMainWindow()->GetMenuByName("Tools");
   if (tMenuPtr)
   {
      tMenuPtr->addAction(mConstellationMakerActionPtr);
      tMenuPtr->addAction(mSatelliteActionPtr);
   }
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &Plugin::OnProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &Plugin::OnProjectClosed);
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::FullParseComplete,
           mAstrolabePtr.get(),
           &Astrolabe::OnFullParseComplete);
}

QList<wkf::Action*> SpaceTools::Plugin::GetActions() const
{
   QList<wkf::Action*> actList;
   actList << mConstellationMakerActionPtr;
   actList << mSatelliteActionPtr;
   return actList;
}

void SpaceTools::Plugin::BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr)
{
   mAstrolabePtr->BuildEntityContextMenu(aMenuPtr, aEntityPtr);
}

void SpaceTools::Plugin::OnProjectOpened(wizard::Project* aProjectPtr)
{
   mConstellationMakerActionPtr->setEnabled(true);
   mConstellationMakerPtr->OnProjectOpened(aProjectPtr);

   mAstrolabePtr->OnProjectOpened(aProjectPtr);

   mSatelliteActionPtr->setEnabled(true);
   mSatelliteInserterPtr->OnProjectOpened();
}

void SpaceTools::Plugin::OnProjectClosed(wizard::Project* aProjectPtr)
{
   mConstellationMakerActionPtr->setEnabled(false);
   mConstellationMakerPtr->OnProjectClosed(aProjectPtr);

   mAstrolabePtr->OnProjectClosed(aProjectPtr);

   mSatelliteActionPtr->setEnabled(false);
   mSatelliteInserterPtr->OnProjectClosed();
}

QList<wkf::PrefWidget*> SpaceTools::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> list;
   list.push_back(mSatelliteInserterPtr->GetPrefWidget());
   return list;
}
