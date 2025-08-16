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

#ifndef WSFSCRIPTDEBUGGERCALLSTACKCONTROL_HPP
#define WSFSCRIPTDEBUGGERCALLSTACKCONTROL_HPP
#include <QDockWidget>
#include <QStandardItemModel>

#include "ScriptDebugger.hpp"
#include "UtCallbackHolder.hpp"

class QStandardItemModel;
class QTreeView;

namespace SimulationManager
{
class WsfScriptDebugger;
class WsfScriptDebuggerCallstackControl : public QDockWidget
{
   Q_OBJECT

public:
   explicit WsfScriptDebuggerCallstackControl(QWidget* aParent);

   void selectedFrame(const QModelIndex& aIndex);
   void DebugAtBreak(wizard::ScriptDebugger*, bool);

private:
   void DebugCallstackUpdated(wizard::ScriptDebugger* aDebugger);
   void DebugCallStackFrameSwitch(wizard::ScriptDebugger*, int);
   void setItemCurrent(int aIndex, bool aIsCurrent);

   QStandardItemModel* mModelPtr;
   QTreeView*          mViewPtr;

   wizard::ScriptDebugger* mDebuggerPtr;
   UtCallbackHolder        mCallbacks;
};
} // namespace SimulationManager
#endif
