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
#ifndef STATUSANDRESULTSDIALOG_HPP
#define STATUSANDRESULTSDIALOG_HPP

#include <QDialog>

#include "CrdFileImporter.hpp"
#include "CrdImporterDialog.hpp"
#include "ui_StatusAndResultsDialog.h"

namespace StatusAndResults
{
class Dialog : public QDialog
{
public:
   Dialog(QWidget* aParent = nullptr);
   ~Dialog() override = default;

   void SetCrdImporter(CrdImporter::CrdFileImporter& aCrdImporter, CrdImporter::Dialog* aDialog);
   void SetOutputTree(const std::map<std::string, std::vector<std::string>>& aInputAndOutputMap);
   void done(int r) override;
   void SetStatus(const QString& aStatus);

   void ConversionStarted();

private:
   void CancelButtonClicked();
   void CloseDialog();

   CrdImporter::CrdFileImporter* mImporter;
   CrdImporter::Dialog*          mImporterDialog;
   Ui::StatusAndResultsDialog    mUI;
};
} // namespace StatusAndResults
#endif
