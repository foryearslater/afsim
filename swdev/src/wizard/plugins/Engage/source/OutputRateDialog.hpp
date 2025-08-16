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

#ifndef OUTPUTRATEDIALOG_HPP
#define OUTPUTRATEDIALOG_HPP

#include <QDialog>

#include "ui_EngageOutputRateDialog.h"

namespace Engage
{
class OutputRateItem;

class OutputRateDialog : public QDialog
{
public:
   OutputRateDialog(OutputRateItem& aItem, QWidget* aParent = nullptr);
   OutputRateDialog(const OutputRateDialog&) = delete;
   OutputRateDialog(OutputRateDialog&&)      = delete;
   OutputRateDialog& operator=(const OutputRateDialog&) = delete;
   OutputRateDialog& operator=(OutputRateDialog&&) = delete;
   ~OutputRateDialog() override                    = default;

   void accept() override;
   void AddRateCB();
   void RemoveRateCB();

private:
   Ui::OutputRateDialog mUi;
   OutputRateItem&      mOutputRateItem;
};
} // namespace Engage

#endif
