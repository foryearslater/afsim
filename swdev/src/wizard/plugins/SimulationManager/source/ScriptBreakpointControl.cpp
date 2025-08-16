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

#include "ScriptBreakpointControl.hpp"

#include <set>

#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>

#include "ProjectWorkspace.hpp"
#include "UtQtMemory.hpp"

namespace SimulationManager
{
enum Column
{
   cCOL_BREAKPOINT  = 0,
   cCOL_LINE_NUMBER = 1,
   cCOL_CONDITION   = 2,
   cCOL_HIT_COUNT   = 3
};

class ScriptBreakpointControl_ViewEventFilter : public QObject
{
public:
   QTableView*              mView;
   ScriptBreakpointControl* mControlPtr;
   ScriptBreakpointControl_ViewEventFilter(ScriptBreakpointControl* aControlPtr, QTableView* aView)
      : QObject(aView)
      , mControlPtr(aControlPtr)
   {
      mView = aView;
   }
   bool eventFilter(QObject* o, QEvent* e) override
   {
      if (e->type() == QEvent::KeyPress)
      {
         QKeyEvent* ke = (QKeyEvent*)e;
         if (ke->key() == Qt::Key_Delete && ke->modifiers() == 0)
         {
            QAbstractItemModel* model = mView->model();
            if (!model)
            {
               return false;
            }

            QModelIndex row = mView->currentIndex();
            if (row.isValid())
            {
               mControlPtr->DeleteBreakpoint();
               return true;
            }
         }
      }
      return false;
   }
};

class BreakpointItem : public QStandardItem
{
public:
   wizard::ScriptDebugger::Breakpoint mBreak;
   explicit BreakpointItem(const wizard::ScriptDebugger::Breakpoint& aBreakpoint)
   {
      setEditable(false);
      setCheckable(true);
      Update(aBreakpoint);
   }
   void Update(const wizard::ScriptDebugger::Breakpoint& aBreakpoint)
   {
      if (!(mBreak == aBreakpoint))
      {
         mBreak = aBreakpoint;
         UtPath  path(aBreakpoint.mPath);
         QString location = QString::fromStdString(path.GetFileName());
         setText(location);
         setCheckState(aBreakpoint.IsEnabled() ? Qt::Checked : Qt::Unchecked);
      }
      setEnabled(!aBreakpoint.mToggleLocked);
   }
   bool operator<(const BreakpointItem& aRhs) const
   {
      std::string lhsFilename = UtPath{mBreak.mPath}.GetFileName();
      std::string rhsFilename = UtPath{aRhs.mBreak.mPath}.GetFileName();
      if (lhsFilename == rhsFilename)
      {
         return mBreak.GetLine() < aRhs.mBreak.GetLine();
      }
      else
      {
         return lhsFilename < rhsFilename;
      }
   }
};

class LineNumberItem : public QStandardItem
{
public:
   wizard::ScriptDebugger::Breakpoint mBreak;

   explicit LineNumberItem(const wizard::ScriptDebugger::Breakpoint& aBreak)
   {
      setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      setEditable(false);
      Update(aBreak);
   }

   void Update(const wizard::ScriptDebugger::Breakpoint& aBreak)
   {
      mBreak = aBreak;
      setText(QString::number(mBreak.GetLine() + 1)); // lines are zero-indexed
   }
};

class HitCountItem : public QStandardItem
{
public:
   HitCountItem()
   {
      setEditable(false);
      setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      UpdateText();
   }

   void Increment()
   {
      ++mHitCount;
      UpdateText();
   }

   void Reset()
   {
      mHitCount = 0;
      UpdateText();
   }

private:
   int mHitCount = 0;

   void UpdateText() { setText(QString::number(mHitCount)); }
};

class ConditionItem : public QStandardItem
{
public:
   wizard::ScriptDebugger::Breakpoint mBreak;

   explicit ConditionItem(const wizard::ScriptDebugger::Breakpoint& aBreak)
   {
      setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      Update(aBreak);
   };

   void Update(const wizard::ScriptDebugger::Breakpoint& aBreak)
   {
      mBreak = aBreak;
      if (!mBreak.mCondition.empty() && mBreak.mCondition != text().toStdString())
      {
         setText(QString::fromStdString(aBreak.mCondition));
      }
   }
};
} // namespace SimulationManager

SimulationManager::ScriptBreakpointControl::ScriptBreakpointControl(QWidget* aParent)
   : QDockWidget{aParent}
   , mModelPtr{ut::qt::make_qt_ptr<QStandardItemModel>(0, 4, this)}
   , mProxyModelPtr{ut::qt::make_qt_ptr<BreakpointSortProxyModel>(this)}
{
   setObjectName("ScriptBreakpointControl");
   mDebuggerPtr  = nullptr;
   mToggleLocked = false;
   setWidget(ut::qt::make_qt_ptr<QWidget>(this));
   mUi.setupUi(widget());
   setWindowTitle(widget()->windowTitle());

   mUi.toggleAll->setIcon(QIcon::fromTheme("stop_sign"));
   mUi.editCondition->setIcon(QIcon::fromTheme("conditional_debug"));
   mUi.deleteBreakpoint->setIcon(QIcon::fromTheme("delete"));
   mUi.deleteAllBreakpoints->setIcon(QIcon::fromTheme("delete_all"));

   // name
   auto* nameHeaderItem = new QStandardItem("Name");
   nameHeaderItem->setTextAlignment(Qt::AlignLeft);
   mModelPtr->setHorizontalHeaderItem(cCOL_BREAKPOINT, nameHeaderItem);
   // line number
   auto* lineNumItem = new QStandardItem("Line No");
   lineNumItem->setTextAlignment(Qt::AlignLeft);
   mModelPtr->setHorizontalHeaderItem(cCOL_LINE_NUMBER, lineNumItem);
   // hit count
   auto* hitCountHeaderItem = new QStandardItem("Hit Count");
   hitCountHeaderItem->setTextAlignment(Qt::AlignLeft);
   mModelPtr->setHorizontalHeaderItem(cCOL_HIT_COUNT, hitCountHeaderItem);
   // condition
   auto* conditionHeaderItem = new QStandardItem("Condition");
   conditionHeaderItem->setTextAlignment(Qt::AlignLeft);
   mModelPtr->setHorizontalHeaderItem(cCOL_CONDITION, conditionHeaderItem);

   mProxyModelPtr->sort(cCOL_BREAKPOINT, Qt::AscendingOrder);
   mProxyModelPtr->setSourceModel(mModelPtr.data());

   mUi.view->setModel(mProxyModelPtr.data());
   mUi.view->sortByColumn(cCOL_BREAKPOINT, Qt::AscendingOrder);
   mUi.view->horizontalHeader()->setSectionsMovable(true);

   mUi.view->installEventFilter(new ScriptBreakpointControl_ViewEventFilter(this, mUi.view));

   connect(wizSignals, &wizard::Signals::BreakpointsChanged, this, &ScriptBreakpointControl::BreakpointsChanged);
   connect(wizSignals, &wizard::Signals::BreakpointHit, this, &ScriptBreakpointControl::IncrementHitCount);
   connect(mUi.deleteBreakpoint, &QToolButton::clicked, this, &ScriptBreakpointControl::DeleteBreakpoint);
   connect(mModelPtr.data(), &QStandardItemModel::itemChanged, this, &ScriptBreakpointControl::ItemChanged);
   connect(mUi.view, &QTableView::activated, this, &ScriptBreakpointControl::SelectedBreakpoint);
   connect(mUi.view, &QTableView::customContextMenuRequested, this, &ScriptBreakpointControl::DisplayContextMenu);
   connect(mUi.editCondition, &QToolButton::clicked, this, &ScriptBreakpointControl::EditCondition);

   mDebugSessionCbPtr = wizSignals->DebugSessionActive.Connect(&ScriptBreakpointControl::ResetHitCounts, this);
}

bool SimulationManager::BreakpointSortProxyModel::lessThan(const QModelIndex& aLhs, const QModelIndex& aRhs) const
{
   // if hit count, easy to return quickly
   QVariant lhsData = sourceModel()->data(aLhs);
   QVariant rhsData = sourceModel()->data(aRhs);
   if (aLhs.column() == cCOL_HIT_COUNT)
   {
      return lhsData.toInt() < rhsData.toInt();
   }
   else if (aLhs.column() == cCOL_BREAKPOINT)
   {
      // sort by filename/line number
      auto* sourceModelPtr = dynamic_cast<QStandardItemModel*>(sourceModel());
      if (sourceModelPtr)
      {
         auto* lhsItemPtr = dynamic_cast<BreakpointItem*>(sourceModelPtr->itemFromIndex(aLhs));
         auto* rhsItemPtr = dynamic_cast<BreakpointItem*>(sourceModelPtr->itemFromIndex(aRhs));
         if (lhsItemPtr)
         {
            return *lhsItemPtr < *rhsItemPtr;
         }
      }
   }

   // fallback string filename comparison
   return lhsData.toString() < rhsData.toString();
}

void SimulationManager::ScriptBreakpointControl::BreakpointsChanged()
{
   if (!mDebuggerPtr)
   {
      return;
   }

   std::vector<WsfScriptDebugger::Breakpoint> breaks = mDebuggerPtr->AllBreakpoints();
   for (int i = 0; i < static_cast<int>(breaks.size()); ++i)
   {
      const WsfScriptDebugger::Breakpoint& bp          = breaks[i];
      bool                                 updatedItem = false;
      while (mModelPtr->rowCount() > i)
      {
         auto* bpItemPtr = dynamic_cast<BreakpointItem*>(mModelPtr->item(i, cCOL_BREAKPOINT));
         if (bpItemPtr->mBreak.mId == bp.mId)
         {
            auto* conditionItemPtr = dynamic_cast<ConditionItem*>(mModelPtr->item(i, cCOL_CONDITION));
            auto* lineNumItemPtr   = dynamic_cast<LineNumberItem*>(mModelPtr->item(i, cCOL_LINE_NUMBER));
            bpItemPtr->Update(bp);
            conditionItemPtr->Update(bp);
            lineNumItemPtr->Update(bp);
            updatedItem = true;
            break;
         }
         else if (bpItemPtr->mBreak.mId < bp.mId)
         {
            // item deleted
            mModelPtr->removeRow(i);
         }
         else
         {
            break;
         }
      }
      if (!updatedItem)
      {
         auto* bpItem        = new BreakpointItem(bp);
         auto* lineNumItem   = new LineNumberItem(bp);
         auto* conditionItem = new ConditionItem(bp);
         auto* hitCountItem  = new HitCountItem;
         mModelPtr->insertRow(i, {bpItem, lineNumItem, conditionItem, hitCountItem});
      }
   }
   while (mModelPtr->rowCount() > static_cast<int>(breaks.size()))
   {
      mModelPtr->removeRow(static_cast<int>(breaks.size()));
   }

   mUi.view->resizeColumnToContents(cCOL_BREAKPOINT);
   mUi.view->resizeColumnToContents(cCOL_CONDITION);
}

void SimulationManager::ScriptBreakpointControl::IncrementHitCount(const wizard::ScriptDebugger::Breakpoint* aBreakpointPtr)
{
   if (mDebuggerPtr)
   {
      // fast return if we're not at a breakpoint
      if (!mDebuggerPtr->IsAtBreak())
      {
         return;
      }

      // look for corresponding breakpoint row and increment hit count
      for (int i = 0; i < mModelPtr->rowCount(); ++i)
      {
         auto* bpItem = dynamic_cast<BreakpointItem*>(mModelPtr->item(i, cCOL_BREAKPOINT));
         if (aBreakpointPtr->mId == bpItem->mBreak.mId)
         {
            auto* hitCountItem = dynamic_cast<HitCountItem*>(mModelPtr->item(i, cCOL_HIT_COUNT));
            hitCountItem->Increment();
            break;
         }
      }
   }
}

void SimulationManager::ScriptBreakpointControl::ResetHitCounts(wizard::ScriptDebugger* aDebugger, bool aIsSessionActive)
{
   // reset the hit counters upon a new debug session so the counters persist on finishing a session, so users can review
   if (aIsSessionActive)
   {
      for (int i = 0; i < mModelPtr->rowCount(); ++i)
      {
         auto* hitCountItem = dynamic_cast<HitCountItem*>(mModelPtr->item(i, cCOL_HIT_COUNT));
         hitCountItem->Reset();
      }
   }
}

void SimulationManager::ScriptBreakpointControl::ActiveDebuggerChanged(WsfScriptDebugger* aDebuggerPtr)
{
   mDebuggerPtr = aDebuggerPtr;
   if (aDebuggerPtr)
   {
      connect(mUi.toggleAll, &QToolButton::clicked, mDebuggerPtr, &WsfScriptDebugger::ToggleAllBreakpoints);
      connect(mUi.deleteAllBreakpoints, &QToolButton::clicked, mDebuggerPtr, &WsfScriptDebugger::DeleteAllBreakpoints);
      connect(mDebuggerPtr, &WsfScriptDebugger::BreakpointsToggled, this, &ScriptBreakpointControl::UpdateToggleButton);
   }
   else
   {
      mModelPtr->clear();
      mDebuggerPtr = nullptr;
   }
}

void SimulationManager::ScriptBreakpointControl::UpdateToggleButton(int aToggleState)
{
   if (aToggleState == ToggleState::cALL_LOCKED)
   {
      mUi.toggleAll->setIcon(QIcon::fromTheme("close"));
   }
   else if (aToggleState == ToggleState::cTOGGLE_ENABLE)
   {
      mUi.toggleAll->setIcon(QIcon::fromTheme("stop_sign_red"));
   }
   else if (aToggleState == ToggleState::cTOGGLE_DISABLE)
   {
      mUi.toggleAll->setIcon(QIcon::fromTheme("stop_sign"));
   }
}

void SimulationManager::ScriptBreakpointControl::DeleteBreakpoint()
{
   if (!mDebuggerPtr)
   {
      return;
   }

   std::vector<int> breakpointIds;
   QModelIndexList  idxs = mUi.view->selectionModel()->selectedRows();
   for (const auto& idx : idxs)
   {
      if (idx.isValid())
      {
         auto* item = dynamic_cast<BreakpointItem*>(mModelPtr->item(mProxyModelPtr->mapToSource(idx).row()));
         if (item)
         {
            breakpointIds.push_back(item->mBreak.mId);
         }
      }
   }
   mDebuggerPtr->RemoveBreakpoints(breakpointIds);
}

void SimulationManager::ScriptBreakpointControl::ItemChanged(QStandardItem* aItemPtr)
{
   auto* bpItemPtr = dynamic_cast<BreakpointItem*>(aItemPtr);
   if (bpItemPtr)
   {
      bool            setEnabled = bpItemPtr->checkState() == Qt::Checked;
      QModelIndexList idxs       = mUi.view->selectionModel()->selectedRows();
      if (idxs.size() < 2)
      {
         mDebuggerPtr->SetBreakpointEnabled(bpItemPtr->mBreak.mId, setEnabled);
      }
      else
      {
         std::vector<int> breakpointIds;
         for (auto& idx : idxs)
         {
            auto* otherBpItemPtr = dynamic_cast<BreakpointItem*>(mModelPtr->item(mProxyModelPtr->mapToSource(idx).row()));
            if (otherBpItemPtr)
            {
               breakpointIds.push_back(otherBpItemPtr->mBreak.mId);
            }
         }
         mDebuggerPtr->SetBreakpointsEnabled(breakpointIds, setEnabled);
      }
   }

   auto* conditionItemPtr = dynamic_cast<ConditionItem*>(aItemPtr);
   if (conditionItemPtr)
   {
      mDebuggerPtr->SetBreakpointCondition(conditionItemPtr->mBreak.mId, conditionItemPtr->text().trimmed().toStdString());
   }
}

void SimulationManager::ScriptBreakpointControl::SelectedBreakpoint(const QModelIndex& aIndex)
{
   auto* itemPtr = dynamic_cast<BreakpointItem*>(mModelPtr->item(aIndex.row(), cCOL_BREAKPOINT));
   if (itemPtr)
   {
      const WsfScriptDebugger::Breakpoint* breakPoint = mDebuggerPtr->BreakpointFromId(itemPtr->mBreak.mId);
      if (breakPoint)
      {
         wizard::ProjectWorkspace::Instance()->GotoRange(breakPoint->mPath, breakPoint->GetLine());
      }
   }
}

void SimulationManager::ScriptBreakpointControl::EditCondition()
{
   QModelIndex idx = mUi.view->currentIndex();
   if (idx.isValid())
   {
      mUi.view->edit(idx.siblingAtColumn(cCOL_CONDITION));
   }
}

void SimulationManager::ScriptBreakpointControl::DisplayContextMenu(const QPoint& aPos)
{
   int                 row         = mProxyModelPtr->mapToSource(mUi.view->indexAt(aPos)).row();
   auto*               bpItemPtr   = dynamic_cast<BreakpointItem*>(mModelPtr->item(row, 0));
   WsfScriptDebugger*& debuggerPtr = mDebuggerPtr; // so lambdas can capture
   if (bpItemPtr && debuggerPtr)
   {
      QMenu   contextMenu;
      QString labelEnableDisable = (bpItemPtr->mBreak.IsEnabled() ? "Disable Breakpoint" : "Enable Breakpoint");
      QIcon   iconEnableDisable =
         (bpItemPtr->mBreak.IsEnabled() ? QIcon::fromTheme("stop_sign") : QIcon::fromTheme("stop_sign_red"));
      QAction* actionEnableDisablePtr = contextMenu.addAction(iconEnableDisable, labelEnableDisable);
      connect(actionEnableDisablePtr,
              &QAction::triggered,
              [&, debuggerPtr, bpItemPtr]()
              {
                 // flip the breakpoint
                 // the one-sided negative wording (SetEnabled vs IsDisabled) makes the boolean logic confusing
                 debuggerPtr->SetBreakpointEnabled(bpItemPtr->mBreak.mId, bpItemPtr->mBreak.IsEnabled());
              });

      QAction* actionRemovePtr = contextMenu.addAction(QIcon::fromTheme("delete"), "Remove Breakpoint");
      connect(actionRemovePtr,
              &QAction::triggered,
              [&, debuggerPtr, bpItemPtr]() { debuggerPtr->RemoveBreakpoint(bpItemPtr->mBreak.mId); });

      QAction* actionEditConditionPtr = contextMenu.addAction("Edit Condition");
      connect(actionEditConditionPtr, &QAction::triggered, [&, this]() { this->EditCondition(); });

      // for some reason Qt doesn't take TableView horizontal header height into account
      QPoint offsetPos = aPos;
      offsetPos.setY(aPos.y() + mUi.view->horizontalHeader()->height());
      contextMenu.exec(mUi.view->mapToGlobal(offsetPos));
   }
}
