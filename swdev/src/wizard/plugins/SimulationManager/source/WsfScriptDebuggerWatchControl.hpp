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

#ifndef WSFSCRIPTDEBUGGERWATCHCONTROL_HPP
#define WSFSCRIPTDEBUGGERWATCHCONTROL_HPP
#include <QDockWidget>
#include <QStandardItemModel>

#include "UtQtAbstractTreeModel.hpp"
#include "ui_ScriptWatchControl.h"

class QStandardItemModel;
class QTreeView;

namespace SimulationManager
{
class WsfDebugValueModelNode;
class WsfDebugValueModelNodeLocals;
class WsfScriptDebugger;
class WsfScriptWatchModel;
class WsfScriptDebuggerWatchControl : public QDockWidget
{
   Q_OBJECT

public:
   explicit WsfScriptDebuggerWatchControl(QWidget* aParent);
   ~WsfScriptDebuggerWatchControl() override = default;

   void setGlobalWatches(std::vector<std::string> aGlobalWatchNames);
   // static void NewPopupWatch(const std::string& aExpression, ViWsfScriptDebugger* aDebuggerPtr, QPoint aPoint);

public slots:

   void ActiveDebuggerChanged(WsfScriptDebugger*);
   void addNewWatch();
   bool addNewWatch(const std::string& aText);
   void DeleteWatch();
   void DeleteAllWatches();

protected:
   WsfScriptDebugger*       mDebuggerPtr;
   WsfScriptWatchModel*     mModelPtr;
   Ui::ScriptWatchControlUi mUi;

   UtQtAbstractModelItem*        mInvRootPtr{nullptr};
   WsfDebugValueModelNode*       mWatchRootPtr{nullptr};
   WsfDebugValueModelNodeLocals* mLocalWatchRootPtr{nullptr};
   WsfDebugValueModelNode*       mGlobalWatchRootPtr{nullptr};
};
} // namespace SimulationManager
#endif
