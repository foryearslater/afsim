// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SCRIPTBREAKPOINTCONTROL_HPP
#define SCRIPTBREAKPOINTCONTROL_HPP

#include <memory>

#include <QDockWidget>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include "Signals.hpp"
#include "UtQtUiPointer.hpp"
#include "WsfScriptDebugger.hpp"
#include "ui_ScriptBreakpointControl.h"

class QTableView;
class ScriptDebugger;

namespace SimulationManager
{
class BreakpointSortProxyModel : public QSortFilterProxyModel
{
public:
   explicit BreakpointSortProxyModel(QWidget* aParent)
      : QSortFilterProxyModel{aParent} {};
   ~BreakpointSortProxyModel() override = default;

protected:
   bool lessThan(const QModelIndex& aLhs, const QModelIndex& aRhs) const override;
};

class ScriptBreakpointControl : public QDockWidget
{
   Q_OBJECT

public:
   explicit ScriptBreakpointControl(QWidget* aParent);
   ~ScriptBreakpointControl() override = default;
   void ActiveDebuggerChanged(WsfScriptDebugger* aProjectPtr);

public slots:

   void DeleteBreakpoint();
   void IncrementHitCount(const wizard::ScriptDebugger::Breakpoint* aBreakpointPtr);
   void ResetHitCounts(wizard::ScriptDebugger* aDebugger, bool aIsSessionActive);
   void DisplayContextMenu(const QPoint& aPos);
   void UpdateToggleButton(int aToggleState);

protected slots:

   void BreakpointsChanged();
   void ItemChanged(QStandardItem* aItemPtr);
   void SelectedBreakpoint(const QModelIndex& aIndex);
   void EditCondition();

protected:
   Ui::ScriptBreakpointControlUi               mUi;
   ut::qt::UiPointer<QStandardItemModel>       mModelPtr;
   ut::qt::UiPointer<BreakpointSortProxyModel> mProxyModelPtr;
   WsfScriptDebugger*                          mDebuggerPtr;

private:
   std::unique_ptr<UtCallback> mDebugSessionCbPtr;
   UtCallbackHolder            mCallbacks;
   bool                        mToggleLocked;
};
} // namespace SimulationManager
#endif
