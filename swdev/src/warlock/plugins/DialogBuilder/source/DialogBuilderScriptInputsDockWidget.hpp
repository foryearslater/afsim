// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DialogBuilderScriptPromptDockWidget_HPP
#define DialogBuilderScriptPromptDockWidget_HPP

#include <QDockWidget>

#include "ui_DialogBuilderScriptInputsDockWidget.h"

namespace WkDialogBuilder
{
// This is the class that implements the dock widget defined by a user.
class ScriptInputsDockWidget : public QDockWidget
{
public:
   ScriptInputsDockWidget(QWidget* aParentPtr = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());

   void AddUserPrompt(QWidget* aPromptPtr, const QString& aName, bool aShowReturnValue);
   void RemoveUserPrompt();

private:
   QWidget*                                mPromptPtr{nullptr};
   bool                                    mPreviouslyVisible{false};
   Ui::DialogBuilderScriptInputsDockWidget mUI;
};
} // namespace WkDialogBuilder
#endif
