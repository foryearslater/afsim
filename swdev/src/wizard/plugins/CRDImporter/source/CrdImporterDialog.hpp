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
#ifndef CRDIMPORTERDIALOG_HPP
#define CRDIMPORTERDIALOG_HPP

#include <iostream>

#include <QDialog>
#include <QThread>

#include "CrdFileImporter.hpp"
#include "ui_CrdImporterDialog.h"

namespace StatusAndResults
{
class Dialog;
}

namespace CrdImporter
{
class Dialog : public QDialog
{
   Q_OBJECT

public:
   Dialog(QWidget* aParent = nullptr);
   ~Dialog() override;

   void done(int r) override;

signals:
   void Operate(const QString&);

public slots:
   void DeleteEntry();
   void DeleteSelectedEntries();

private:
   void HandleResults(const QString&);

   void                      AddPushButtonClicked();
   void                      GetSelectedFolder();
   void                      CloseDialog();
   void                      BrowsePushButtonClicked();
   void                      ImportPushButtonClicked();
   void                      dropEvent(QDropEvent* event) override;
   void                      dragEnterEvent(QDragEnterEvent* event);
   void                      mouseMoveEvent(QMouseEvent* event);
   QThread                   mWorkThread;
   CrdFileImporter           mCrdImporter;
   StatusAndResults::Dialog* mStatusDialogPtr;
   QString                   mOpenFileLocationForFileDialog;
   QString                   mOpenFolderLocationForFileDialog;
   // QString openOutputFolderLocationForFileDialog;
   Ui::CrdImporterDialog mUI;
   bool                  mOutputFolderChanged;
   bool                  mAddFolderChanged;
};
} // namespace CrdImporter
#endif
