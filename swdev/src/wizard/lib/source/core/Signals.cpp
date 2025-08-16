// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "Signals.hpp"

#include <QApplication>
#include <QMessageBox>

#include "Project.hpp"
#include "ProjectWorkspace.hpp"

wizard::Signals* wizSignals = new wizard::Signals(qApp);

void wizard::Signals::applyProxy()
{
   if (gEditMode == Vi::cEDIT_MODE_VISUAL)
   {
      if (Project::Instance())
      {
         if (Project::Instance()->GetStartupFiles().empty())
         {
            QMessageBox::information(
               nullptr,
               "No Files",
               "Cannot save changes from visual mode until the project has at least one startup file.");
         }
         else
         {
            gEditMode = Vi::cEDIT_MODE_SWITCHING_TO_TEXT;
            wizSignals->emitEditModeSwitch(gEditMode);
            ProjectWorkspace::Instance()->BeginSerialize();
         }
      }
      else
      {
         gEditMode = Vi::cEDIT_MODE_SWITCHING_TO_TEXT;
         wizSignals->emitEditModeSwitch(gEditMode);
      }
   }
}

void wizard::Signals::revertVisualModeChanges()
{
   if (gEditMode == Vi::cEDIT_MODE_VISUAL)
   {
      Project::Instance()->Proxy()->UndoPlatformRenames();
      gEditMode = Vi::cEDIT_MODE_TEXT;
      wizSignals->emitEditModeSwitch(gEditMode);
      ProjectWorkspace::Instance()->ReparseAll();
   }
}

void wizard::Signals::switchToEditModeP()
{
   gEditMode = Vi::cEDIT_MODE_VISUAL;
   wizSignals->emitEditModeSwitch(gEditMode);
}

void wizard::Signals::handleProjectClose()
{
   if (gEditMode != Vi::cEDIT_MODE_TEXT)
   {
      gEditMode = Vi::cEDIT_MODE_TEXT;
      wizSignals->emitEditModeSwitch(gEditMode);
   }
}

void wizard::Signals::editProxy()
{
   if (gEditMode != Vi::cEDIT_MODE_VISUAL)
   {
      switchToEditModeP();
   }
}
