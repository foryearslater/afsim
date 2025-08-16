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

#include "SatelliteInserter.hpp"

#include "SatelliteInserterDialog.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

SpaceTools::SatelliteInserter::SatelliteInserter()
   : mPrefWidgetPtr(new PrefWidget())
   , mDialogPtr(new SatelliteInserterDialog(wkfEnv.GetMainWindow(), mPrefWidgetPtr->GetPreferenceObject()))
{
}

//! Display the Satellite Inserter dialog.
void SpaceTools::SatelliteInserter::ShowDialog()
{
   if (mDialogPtr)
   {
      mDialogPtr->Show();
   }
}

SpaceTools::SatelliteInserter::~SatelliteInserter()
{
   delete mPrefWidgetPtr;
}

//! Handles closing the current project
void SpaceTools::SatelliteInserter::OnProjectClosed()
{
   if (mDialogPtr != nullptr)
   {
      mDialogPtr->OnProjectClosed();
   }
}

//! Handles opening a project
void SpaceTools::SatelliteInserter::OnProjectOpened()
{
   mDialogPtr->OnProjectOpened();
}
