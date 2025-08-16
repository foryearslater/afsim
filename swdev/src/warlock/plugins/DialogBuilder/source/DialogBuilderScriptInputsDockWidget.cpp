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
#include "DialogBuilderScriptInputsDockWidget.hpp"

WkDialogBuilder::ScriptInputsDockWidget::ScriptInputsDockWidget(QWidget*        aParentPtr /*= nullptr*/,
                                                                Qt::WindowFlags aFlags /*= Qt::WindowFlags()*/)
   : QDockWidget(aParentPtr, aFlags)
   , mUI()
{
   mUI.setupUi(this);
   mUI.scriptNameLabel->setVisible(false);
   mUI.scrollArea->setVisible(false);
}

void WkDialogBuilder::ScriptInputsDockWidget::AddUserPrompt(QWidget* aPromptPtr, const QString& aName, bool aShowReturnValue)
{
   mPreviouslyVisible = isVisible();
   mUI.scriptNameLabel->setText(aName);
   mUI.scriptNameLabel->setVisible(true);
   mUI.scrollArea->setVisible(true);
   RemoveUserPrompt();
   auto vLayout = dynamic_cast<QVBoxLayout*>(mUI.scrollAreaWidgetContents->layout());
   if (vLayout != nullptr)
   {
      mPromptPtr = aPromptPtr;
      vLayout->insertWidget(0, aPromptPtr);
   }
   show();
}

void WkDialogBuilder::ScriptInputsDockWidget::RemoveUserPrompt()
{
   auto vLayout = dynamic_cast<QVBoxLayout*>(mUI.scrollAreaWidgetContents->layout());
   if (mPromptPtr != nullptr && vLayout != nullptr)
   {
      vLayout->removeWidget(mPromptPtr);
      delete mPromptPtr;
      mPromptPtr = nullptr;
   }

   if (!mPreviouslyVisible)
   {
      close();
   }
}
