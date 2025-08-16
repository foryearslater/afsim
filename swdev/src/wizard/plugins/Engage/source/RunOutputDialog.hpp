// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef RUNOUTPUTDIALOG_HPP
#define RUNOUTPUTDIALOG_HPP

#include <QDialog>

#include "ui_EngageRunOutputDialog.h"

namespace Engage
{
class OutputItem;

class RunOutputDialog : public QDialog
{
public:
   RunOutputDialog(OutputItem& aItem, QWidget* aParent = nullptr);
   RunOutputDialog(const OutputItem&) = delete;
   RunOutputDialog(RunOutputDialog&&) = delete;
   RunOutputDialog& operator=(const RunOutputDialog&) = delete;
   RunOutputDialog& operator=(RunOutputDialog&&) = delete;
   ~RunOutputDialog() override                   = default;

   void accept() override;
   void AddVariableCB();
   void RemoveVariableCB();

private:
   Ui::RunOutputDialog mUi;
   OutputItem&         mOutputItem;
};
} // namespace Engage

#endif
