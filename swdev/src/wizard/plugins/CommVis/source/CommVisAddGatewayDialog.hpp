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

#ifndef COMMVISADDGATEWAYDIALOG_HPP
#define COMMVISADDGATEWAYDIALOG_HPP

#include <QDialog>

#include "CommVisEditDialog.hpp"
#include "ui_CommVisAddGatewayDialog.h"

class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisAddGatewayDialog : public QDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   explicit CommVisAddGatewayDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~CommVisAddGatewayDialog() = default;

   void Initialize();
   void PlatformChanged();
   void RemotePlatformChanged();
   void RouterChanged();

   void done(int r) override;

private:
   void ToggleLineEdits(bool aEnabled);
   void AddGateway();
   void EditRouter(const UtTextDocumentRange& aDocRange);
   void EditComm(const UtTextDocumentRange& aDocRange);
   void AddEditCommToPlatform(const UtTextDocumentRange& aDocRange);
   void AddEditRouterToPlatform(const UtTextDocumentRange& aDocRange);

   const QString               GetAttributeString(int aTabSize) const override { return ""; }
   Ui::CommVisAddGatewayDialog mUI;
};
} // namespace WizCommVis

#endif
