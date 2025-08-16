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

#ifndef IMPORTERDIALOG_HPP
#define IMPORTERDIALOG_HPP

#include <QDialog>
#include <QThread>

#include "ui_ImporterDialog.h"

namespace Ui
{
class ImporterDialog;
}

namespace AcoImporter
{


class Dialog : public QDialog
{
   Q_OBJECT

public:
   Dialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~Dialog() override;

   void Done(int r);

public slots:
   void DeleteEntry();
   void DeleteSelectedEntries();
   void RegisterAcoRequirements();
   void RegisterAtoRequirements();
   void UnregisterAcoRequirements();
   void UnregisterAtoRequirements();

private:
   void HandleResults(const QString& aString);

   void AddPushButtonClicked();
   void GetSelectedFolder();
   void CloseDialog();
   void BrowsePushButtonClicked();
   void ImportPushButtonClicked();
   void dropEvent(QDropEvent* aEvent) override;
   void dragEnterEvent(QDragEnterEvent* aEvent) override;
   void mouseMoveEvent(QMouseEvent* aEvent) override;

   Ui::ImporterDialog mUi;
   QString            mOpenFileLocationForFileDialog;
   QString            mOpenFolderLocationForFileDialog;

   bool        mOutputFolderChanged;
   bool        mAddFolderChanged;
   std::string mType;
};
} // namespace AcoImporter
#endif
