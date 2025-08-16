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

#ifndef COMMVISADDNETWORKDIALOG_HPP
#define COMMVISADDNETWORKDIALOG_HPP

#include <QDialog>

#include "CommVisEditDialog.hpp"
#include "ui_CommVisAddNetworkDialog.h"

class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisAddNetworkDialog : public QDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   explicit CommVisAddNetworkDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~CommVisAddNetworkDialog() = default;

   void Initialize();
   void FileChanged();
   void BrowseFiles();

   void done(int r) override;

private:
   void ToggleLineEdits(bool aEnabled);
   void AddNetwork();

   const QString               GetAttributeString(int aTabSize) const override;
   Ui::CommVisAddNetworkDialog mUI;
};
} // namespace WizCommVis

#endif
