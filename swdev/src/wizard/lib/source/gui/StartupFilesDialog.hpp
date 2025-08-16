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

#ifndef STARTUPFILESDIALOG_HPP
#define STARTUPFILESDIALOG_HPP

#include <vector>

#include <QDialog>

#include "UtPath.hpp"
#include "ui_StartupFilesDialog.h"

namespace wizard
{
class StartupFilesDialog : public QDialog
{
public:
   StartupFilesDialog(QWidget* aParent, const std::vector<UtPath>& aStartupFiles, const UtPath& aProjectDir);
   ~StartupFilesDialog() override = default;
   void                       accept() override;
   const std::vector<UtPath>& startupFiles() const;

protected:
   void showEvent(QShowEvent*) override;

   Ui::StartupFileDialog mUI;
   std::vector<UtPath>   mStartupFiles;
   UtPath                mProjectDir;
};
} // namespace wizard
#endif
