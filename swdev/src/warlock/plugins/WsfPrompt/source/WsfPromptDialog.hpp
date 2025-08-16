// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PROMPTDIALOG_HPP
#define PROMPTDIALOG_HPP

#include <QDialog>

#include "WkScriptSimInterface.hpp"
#include "WkfMainWindow.hpp"
#include "WsfPromptSimInterface.hpp"
#include "WsfScriptPrompt.hpp"
#include "ui_PromptDialog.h"

namespace WkWsfPrompt
{
class PromptDialog : public QDialog
{
public:
   PromptDialog(wkf::MainWindow*                        aParent,
                const WsfScriptPromptClass::PromptData& aPromptData,
                ScriptSimInterfacePtr                   aScriptSimInterfacePtr);

   ~PromptDialog() override = default;

protected:
   void showEvent(QShowEvent* aShowEvent) override;
   void closeEvent(QCloseEvent* aCloseEvent) override;

private:
   void ButtonClicked(const std::string& aScriptName);

   Ui::PromptDialog      mUI;
   ScriptSimInterfacePtr mScriptSimInterfacePtr;
   std::string           mPlatformName;
   bool                  mBlock;
};
} // namespace WkWsfPrompt

#endif