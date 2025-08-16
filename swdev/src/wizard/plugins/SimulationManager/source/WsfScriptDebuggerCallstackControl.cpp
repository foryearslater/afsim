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

#include "WsfScriptDebuggerCallstackControl.hpp"

#include <QApplication>
#include <QTreeView>
#include <QVBoxLayout>

#include "Signals.hpp"
#include "WsfScriptDebugger.hpp"

SimulationManager::WsfScriptDebuggerCallstackControl::WsfScriptDebuggerCallstackControl(QWidget* aParent)
   : QDockWidget(aParent)
{
   mDebuggerPtr = nullptr;
   setObjectName("ScriptCallstackControl");
   setWindowTitle("Script Callstack");
   setWidget(new QWidget(this));
   widget()->setLayout(new QVBoxLayout(widget()));
   mViewPtr = new QTreeView(widget());
   widget()->layout()->addWidget(mViewPtr);
   mModelPtr = new QStandardItemModel(this);
   mViewPtr->setModel(mModelPtr);
   mCallbacks +=
      wizSignals->DebugCallstackUpdated.Connect(&WsfScriptDebuggerCallstackControl::DebugCallstackUpdated, this);
   QStringList header;
   header.push_back(tr("Script"));
   header.push_back(tr("File"));
   mModelPtr->setHorizontalHeaderLabels(header);

   connect(mViewPtr, &QTreeView::doubleClicked, this, &WsfScriptDebuggerCallstackControl::selectedFrame);
   mCallbacks += wizSignals->DebugAtBreak.Connect(&WsfScriptDebuggerCallstackControl::DebugAtBreak, this);
   mCallbacks +=
      wizSignals->DebugCallStackFrameSwitch.Connect(&WsfScriptDebuggerCallstackControl::DebugCallStackFrameSwitch, this);
}

void SimulationManager::WsfScriptDebuggerCallstackControl::DebugCallstackUpdated(wizard::ScriptDebugger* aDebugger)
{
   mDebuggerPtr                                                 = aDebugger;
   const std::vector<WsfScriptDebugger::CallstackEntry>& cstack = aDebugger->Callstack();
   int                                                   rows   = mModelPtr->rowCount();
   if (rows > (int)cstack.size())
   {
      mModelPtr->removeRows((int)cstack.size(), rows - (int)cstack.size());
   }
   while (mModelPtr->rowCount() < (int)cstack.size())
   {
      QList<QStandardItem*> row;
      row.push_back(new QStandardItem());
      row.push_back(new QStandardItem());
      mModelPtr->appendRow(row);
   }

   for (size_t i = 0; i < cstack.size(); ++i)
   {
      const WsfScriptDebugger::CallstackEntry& e = cstack[i];
      QString                                  location, functionName;
      if (e.mSourcePtr)
      {
         location = QString::fromStdString(e.mSourcePtr->GetFileName());
         location += " ";
         location += QString::number(e.mLineNumber);
      }
      WsfScriptDebugger::ScriptInfo* scriptInfoPtr = mDebuggerPtr->FindScriptInfo(e.mScriptHandle);
      if (scriptInfoPtr)
      {
         functionName = QString::fromStdString(scriptInfoPtr->mScriptName);
      }
      mModelPtr->item((int)i, 0)->setEditable(false);
      mModelPtr->item((int)i, 0)->setText(functionName);
      mModelPtr->item((int)i, 1)->setEditable(false);
      mModelPtr->item((int)i, 1)->setText(location);
      setItemCurrent((int)i, i + 1 == cstack.size());
   }
}

void SimulationManager::WsfScriptDebuggerCallstackControl::selectedFrame(const QModelIndex& aIndex)
{
   mDebuggerPtr->SelectStackFrame(aIndex.row());
}

void SimulationManager::WsfScriptDebuggerCallstackControl::DebugAtBreak(wizard::ScriptDebugger*, bool) {}
void SimulationManager::WsfScriptDebuggerCallstackControl::DebugCallStackFrameSwitch(wizard::ScriptDebugger*, int aFrameIndex)
{
   int rows = mModelPtr->rowCount();
   for (int i = 0; i < rows; ++i)
   {
      setItemCurrent(i, i == aFrameIndex || (aFrameIndex == -1 && i == (rows - 1)));
   }
}

void SimulationManager::WsfScriptDebuggerCallstackControl::setItemCurrent(int aIndex, bool aIsCurrent)
{
   if (aIsCurrent)
   {
      QFont currentFont;
      currentFont.setBold(true);
      QBrush currentBrush(Qt::darkGreen);
      mModelPtr->item(aIndex, 0)->setFont(currentFont);
      mModelPtr->item(aIndex, 0)->setForeground(currentBrush);
   }
   else
   {
      QBrush currentBrush(qApp->palette().color(QPalette::Text));
      mModelPtr->item(aIndex, 0)->setFont(QFont());
      mModelPtr->item(aIndex, 0)->setForeground(currentBrush);
   }
}
