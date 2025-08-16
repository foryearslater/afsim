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
#ifndef DialogBuilderDockWidget_HPP
#define DialogBuilderDockWidget_HPP

#include <QDockWidget>
#include <QPushButton>

#include "DialogBuilderDialogDefinition.hpp"
#include "DialogBuilderScriptInputsDockWidget.hpp"
#include "WkScriptSimInterface.hpp"
#include "WkfAction.hpp"
#include "WkfPlatform.hpp"
#include "ui_DialogBuilderDockWidget.h"

namespace WkDialogBuilder
{
// This is the class that implements the dock widget defined by a user.
class DockWidget : public QDockWidget
{
   using ScriptInterfacePtr = std::shared_ptr<warlock::ScriptSimInterface>;

public:
   DockWidget(const DialogDef&        aDialogDefinition,
              ScriptInterfacePtr      aSimInterfacePtr,
              ScriptInputsDockWidget* aPromptDockWidget,
              QWidget*                parent = nullptr,
              Qt::WindowFlags         f      = Qt::WindowFlags());
   ~DockWidget() override;

   void                SetDefinition(const DialogDef& aDialogDefinition);
   QList<wkf::Action*> GetActions() const { return mActions; }

private:
   void PlatformSelectionChanged(const wkf::PlatformList& aSelected, const wkf::PlatformList& aUnselected);
   void UpdatePlatformSelection();

   void RemotePlatformUpdatedCB(const std::string& aPlatformName);
   void ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue);

   void Clear();

   QVariant GetValue(const QString& aArgType, const QVariant& aData);
   bool     BuildScriptInstance(const ScriptDefinition& aDefinition, warlock::script::Instance& aScriptInstance);
   void     ExecuteScriptInstance(warlock::script::Instance& aInstance);
   void     PromptUserForArguments(const QString& aDisplayName, warlock::script::Instance& aInstance);
   void     HandleButtonClicked();
   void     HandleScriptExecution(int aId);

   ScriptInterfacePtr mSimInterfacePtr;

   QList<QPushButton*>             mButtons;
   std::map<int, ScriptDefinition> mScriptData;
   std::set<int>                   mScriptsWaitingForReturnValues;
   QList<wkf::Action*>             mActions;
   ScriptInputsDockWidget*         mPromptDockWidget;
   int                             mUniqueID{1};
   std::vector<std::string>        mClampedPlatformList;

   Ui::DialogBuilderDockWidget mUI;

   static QPoint mResultsDialogPosition;
   static bool   mResultsDialogPositionValid;
};
} // namespace WkDialogBuilder
#endif
