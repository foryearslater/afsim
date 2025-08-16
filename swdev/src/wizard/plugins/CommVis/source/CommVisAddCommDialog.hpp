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

#ifndef COMMVISADDCOMMDIALOG_HPP
#define COMMVISADDCOMMDIALOG_HPP

#include <QDialog>

#include "CommVisEditDialog.hpp"
#include "ui_CommVisAddCommDialog.h"

class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisAddCommDialog : public QDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   explicit CommVisAddCommDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~CommVisAddCommDialog() = default;

   void Initialize();

   void done(int r) override;

private:
   void ToggleLineEdits();
   void AddComm();

   const QString            GetAttributeString(int aTabSize) const override;
   Ui::CommVisAddCommDialog mUI;
};
} // namespace WizCommVis

#endif
