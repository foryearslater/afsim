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

#ifndef COMMVISADDROUTERDIALOG_HPP
#define COMMVISADDROUTERDIALOG_HPP

#include <QDialog>

#include "CommVisEditDialog.hpp"
#include "ui_CommVisAddRouterDialog.h"
class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisAddRouterDialog : public QDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   explicit CommVisAddRouterDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~CommVisAddRouterDialog() = default;

   void Initialize();
   void PlatformChanged();

   void done(int r) override;

private:
   void ToggleLineEdits(bool aEnabled);
   void ToggleCheckBoxes(bool aEnabled);
   void AddRouter();

   const QString              GetAttributeString(int aTabSize) const override;
   Ui::CommVisAddRouterDialog mUI;
};
} // namespace WizCommVis

#endif
