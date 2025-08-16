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

#ifndef PROJECTSETTINGS_HPP
#define PROJECTSETTINGS_HPP

#include <functional>

#include <QDialog>

#include "Project.hpp"
#include "UtMemory.hpp"
#include "UtReferenceCount.hpp"
#include "ui_ProjectSettings.h"

namespace wizard
{
class ProjectSettingsDialog : public QDialog
{
   Q_OBJECT
public:
   ProjectSettingsDialog(QWidget* aParentPtr);
   ~ProjectSettingsDialog() override = default;

   void Open(Project* aProjectPtr);

protected slots:
   void BrowseClick();
   void OkClick();
   void CancelClick();
   void ClearCommandLine();

protected:
   UtWeakReference<Project> mScenarioPtr;
   Ui::ProjectSettings      mUi;
};
} // namespace wizard
#endif
