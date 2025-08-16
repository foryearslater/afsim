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

#include "WsfScriptDebuggerWatchControl.hpp"

#include <QKeyEvent>
#include <QTreeView>
#include <QVBoxLayout>

#include "UtQtAbstractTreeModel.hpp"
#include "UtQtMemory.hpp"
#include "WsfDebugValueModel.hpp"
#include "WsfScriptDebugger.hpp"

namespace SimulationManager
{
class WsfScriptWatchModel : public UtQtAbstractModel
{
public:
   QVariant headerData(int section, Qt::Orientation orientation, int role) const override
   {
      if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
      {
         return UtQtAbstractModel::headerData(section, orientation, role);
      }
      switch (section)
      {
      case 0:
         return "Variable";
      case 1:
         return "Value";
      case 2:
         return "Type";
      default:
         return QVariant();
      }
   }
   int columnCount(const QModelIndex& parent) const override { return 3; }
};

class WsfScriptDebuggerWatchControl_ViewEventFilter : public QObject
{
public:
   QTreeView* mView;
   explicit WsfScriptDebuggerWatchControl_ViewEventFilter(QTreeView* aView)
      : QObject(aView)
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
               model->removeRow(row.row(), model->parent(row));
               return true;
            }
         }
      }
      return false;
   }
};
} // namespace SimulationManager

SimulationManager::WsfScriptDebuggerWatchControl::WsfScriptDebuggerWatchControl(QWidget* aParent)
   : QDockWidget(aParent)
{
   mDebuggerPtr = nullptr;
   setObjectName("ScriptWatchControl");
   setWindowTitle("Script Watches");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());
   mUi.view->setUniformRowHeights(true); // this is needed to avoid a crash/bug in Qt
   mUi.view->installEventFilter(new WsfScriptDebuggerWatchControl_ViewEventFilter(mUi.view));
   mUi.addWatchButton->setIcon(QIcon::fromTheme("add_circular"));
   mUi.deleteWatchButton->setIcon(QIcon::fromTheme("remove"));
   mUi.deleteAllWatchesButton->setIcon(QIcon::fromTheme("delete_all"));

   // widget()->setLayout(new QVBoxLayout(widget()));

   // mViewPtr = new QTreeView(widget());
   // widget()->layout()->addWidget(mViewPtr);
   // mModelPtr = new WsfScriptWatchModel;
   mModelPtr = nullptr;
   // mViewPtr->setModel(mModelPtr);

   connect(mUi.addWatchButton,
           &QToolButton::clicked,
           this,
           static_cast<void (WsfScriptDebuggerWatchControl::*)()>(&WsfScriptDebuggerWatchControl::addNewWatch));
   connect(mUi.deleteWatchButton,
           &QToolButton::clicked,
           this,
           static_cast<void (WsfScriptDebuggerWatchControl::*)()>(&WsfScriptDebuggerWatchControl::DeleteWatch));
   connect(mUi.deleteAllWatchesButton,
           &QToolButton::clicked,
           this,
           static_cast<void (WsfScriptDebuggerWatchControl::*)()>(&WsfScriptDebuggerWatchControl::DeleteAllWatches));
}

void SimulationManager::WsfScriptDebuggerWatchControl::ActiveDebuggerChanged(WsfScriptDebugger* aDebuggerPtr)
{
   if (aDebuggerPtr)
   {
      mDebuggerPtr = aDebuggerPtr;

      if (mModelPtr)
      {
         // model still exists from last time debugger was run, need to clean up
         // keep user watches
         if (mInvRootPtr && mWatchRootPtr)
         {
            mInvRootPtr->RemoveAbstractItem(mWatchRootPtr);
         }

         mUi.view->setModel(nullptr);
         delete mModelPtr;

         // cleaning up model also cleans up these ptrs
         mInvRootPtr         = nullptr;
         mLocalWatchRootPtr  = nullptr;
         mGlobalWatchRootPtr = nullptr;
      }
      mModelPtr = ut::qt::make_qt_ptr<WsfScriptWatchModel>();
      mModelPtr->setParent(this);

      mInvRootPtr = ut::qt::make_qt_ptr<UtQtAbstractModelItem>();
      mModelPtr->SetRoot(mInvRootPtr);

      // user watches
      if (mWatchRootPtr == nullptr)
      {
         auto* dmd          = ut::qt::make_qt_ptr<WsfDebugValueModelData>(mDebuggerPtr);
         dmd->mRootName     = "Watch";
         auto* nullQueryPtr = new WsfDebugValueModelQueryData(dmd, true);
         mWatchRootPtr      = ut::qt::make_qt_ptr<WsfDebugValueModelNode>(nullQueryPtr);
      }
      mInvRootPtr->AddAbstractItem(mWatchRootPtr);

      // local watches
      auto* localsRootData  = ut::qt::make_qt_ptr<WsfDebugValueModelData>(mDebuggerPtr);
      auto* localsRootQuery = new WsfDebugValueModelQueryData(localsRootData, true);
      mLocalWatchRootPtr    = ut::qt::make_qt_ptr<WsfDebugValueModelNodeLocals>(localsRootQuery);
      mInvRootPtr->AddAbstractItem(mLocalWatchRootPtr);

      // global watches
      auto* globalsRootData            = ut::qt::make_qt_ptr<WsfDebugValueModelData>(mDebuggerPtr);
      globalsRootData->mCanEditQueries = false;
      globalsRootData->mRootName       = "Globals";
      auto* globalNullQueryPtr         = new WsfDebugValueModelQueryData(globalsRootData, true);
      mGlobalWatchRootPtr              = ut::qt::make_qt_ptr<WsfDebugValueModelNode>(globalNullQueryPtr);
      mInvRootPtr->AddAbstractItem(mGlobalWatchRootPtr);

      //       {
      //          WsfDebugValueModelQueryData* globalQuery = new
      //          WsfDebugValueModelQueryData(mGlobalWatchRoot->mRootData->mDataPtr);
      //
      //          WsfDebugValueModelNode* queryNode = new WsfDebugValueModelNode(globalQuery);
      //          globalQuery->mQuery.mContext = 0;
      //          globalQuery->mQuery.mStackFrame = -3;
      //          globalQuery->mQuery.mVariableName = "Simulation";
      //          mGlobalWatchRoot->AddAbstractItem(queryNode);
      //       }

      mUi.view->setModel(mModelPtr);
   }
   else
   {
      // keep user watches
      mInvRootPtr->RemoveAbstractItem(mWatchRootPtr);

      mUi.view->setModel(nullptr);
      delete mModelPtr;
      mModelPtr = nullptr;

      mLocalWatchRootPtr  = nullptr;
      mGlobalWatchRootPtr = nullptr;

      mDebuggerPtr = nullptr;
   }
}

void SimulationManager::WsfScriptDebuggerWatchControl::addNewWatch()
{
   if (addNewWatch(""))
   {
      mUi.view->edit(mUi.view->currentIndex());
   }
}
bool SimulationManager::WsfScriptDebuggerWatchControl::addNewWatch(const std::string& aText)
{
   if (!mWatchRootPtr)
   {
      return false;
   }
   WsfDebugValueModelQueryData* newQuery = new WsfDebugValueModelQueryData(mWatchRootPtr->mRootData->mDataPtr);

   WsfDebugValueModelNode* queryNode = new WsfDebugValueModelNode(newQuery);
   newQuery->mQuery.mContext         = nullptr;
   newQuery->mQuery.mStackFrame      = -1;
   newQuery->mQuery.mVariableName    = aText;
   mWatchRootPtr->AddAbstractItem(queryNode);

   mWatchRootPtr->mChildValues.erase(queryNode->GetItemIndex());

   QModelIndex idx = mModelPtr->GetRowIndex(*queryNode);
   mUi.view->expand(idx.parent());
   mUi.view->setCurrentIndex(idx);
   return true;
}

void SimulationManager::WsfScriptDebuggerWatchControl::DeleteWatch()
{
   if (mWatchRootPtr)
   {
      auto idx = mUi.view->currentIndex();
      mUi.view->model()->removeRow(idx.row(), idx.parent());
   }
}

void SimulationManager::WsfScriptDebuggerWatchControl::DeleteAllWatches()
{
   if (mWatchRootPtr)
   {
      mWatchRootPtr->RemoveAllAbstractItems(true);
   }
}

void SimulationManager::WsfScriptDebuggerWatchControl::setGlobalWatches(std::vector<std::string> aGlobalWatchNames)
{
   mGlobalWatchRootPtr->ForceRemoveRows(0, mGlobalWatchRootPtr->rowCount());
   for (auto&& name : aGlobalWatchNames)
   {
      WsfDebugValueModelQueryData* globalQuery = new WsfDebugValueModelQueryData(mGlobalWatchRootPtr->mRootData->mDataPtr);
      WsfDebugValueModelNode*      queryNode = new WsfDebugValueModelNode(globalQuery);
      globalQuery->mQuery.mContext           = nullptr;
      globalQuery->mQuery.mStackFrame        = -3;
      globalQuery->mQuery.mVariableName      = name;
      mGlobalWatchRootPtr->AddAbstractItem(queryNode);
   }
}

//
// void WsfScriptDebuggerWatchControl::NewPopupWatch(const std::string& aExpression, WsfScriptDebugger* aDebuggerPtr,
// QPoint aPoint)
// {
//    QTreeView* view = new QTreeView;
//    WsfScriptWatchModel* modelPtr = new WsfScriptWatchModel;
//    modelPtr->setParent(view);
//
//    UtQtAbstractModelItem* invRootPtr = new UtQtAbstractModelItem;
//    modelPtr->SetRoot(invRootPtr);
//
//    WsfDebugValueModelData* dmd = new WsfDebugValueModelData(aDebuggerPtr);
//    dmd->mRootName = "Watch";
//
//    WsfDebugValueModelQueryData* nullQueryPtr = new WsfDebugValueModelQueryData(dmd, true);
//    WsfDebugValueModelNode* watchRoot = new WsfDebugValueModelNode(nullQueryPtr);
//    invRootPtr->AddAbstractItem(watchRoot);
//
//    WsfDebugValueModelQueryData* newQuery = new WsfDebugValueModelQueryData(watchRoot->mRootData->mDataPtr);
//
//    WsfDebugValueModelNode* queryNode = new WsfDebugValueModelNode(newQuery);
//    newQuery->mQuery.mContext = 0;
//    newQuery->mQuery.mStackFrame = -1;
//    newQuery->mQuery.mVariableName = aExpression;
//    watchRoot->AddAbstractItem(queryNode);
//
//    watchRoot->mChildValues.erase(queryNode->GetItemIndex());
//
//    view->setModel(modelPtr);
//    //view->setRootIndex(modelPtr->GetRowIndex(*queryNode));
//
//    view->move(aPoint);
//    view->show();
// }
//
