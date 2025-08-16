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
#include "DockWidget.hpp"

#include <QMenu>

#include "CreatePlatform.hpp"
#include "FindReferencesResultDialog.hpp"
#include "Item.hpp"
#include "Model.hpp"
#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "UtQt.hpp"
#include "ViewTypeSummaryAction.hpp"
#include "WkfVtkEnvironment.hpp"

PlatformBrowser::DockWidget::DockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mProjectPtr(nullptr)
   , mPlatformModelPtr(nullptr)
   , mSavedExpandState()
   , mExpandStateIsSaved(false)
{
   connect(&wkfEnv, &wkf::Environment::PlatformSelectionChanged, this, &DockWidget::PlatformSelectionChangedHandler);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProxyUpdated, this, &DockWidget::OnProxyUpdated);
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::ActiveProjectChanged,
           this,
           &DockWidget::OnActiveProjectChanged);

   setObjectName("PlatformBrowser");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());
   mUi.platformView->mParentPtr = this;
   mUi.platformView->setExpandsOnDoubleClick(false);
   mUi.platformView->setTextElideMode(Qt::ElideRight);
   // Make view more compact
   mUi.platformView->setIndentation(10);
   setWindowTitle(widget()->windowTitle());

   mUi.filterText->setAutoCompletion(false);

   mUi.platformView->setSelectionBehavior(QAbstractItemView::SelectRows);
   mUi.platformView->setHeaderHidden(true);
   mUi.platformView->setDragEnabled(true);

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &DockWidget::ProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &DockWidget::ProjectClosed);

   connect(mUi.platformView, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.filterText, &QComboBox::editTextChanged, this, &DockWidget::FilterChanged);
   connect(mUi.clearFilterBn, &QToolButton::clicked, this, &DockWidget::ClearSearch);

   mPlatformModelPtr = new Model;
   mPlatformModelPtr->setParent(this);
   mUi.platformView->setModel(mPlatformModelPtr);

   connect(mUi.platformView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DockWidget::ViewSelectionChanged);
}

bool PlatformBrowser::DockWidget::SelectSearchResult()
{
   bool opened = false;
   return opened;
}

void PlatformBrowser::DockWidget::ContextEvent(const QModelIndex& aIndex, QPoint aPos)
{
   if (!mProjectPtr)
   {
      return;
   }
   wizard::ParseResults* results = mProjectPtr->WaitForParseResults();
   Item*                 itemPtr = FindItem(aIndex);

   if (results && itemPtr)
   {
      const WsfPProxyNode proxyNode = itemPtr->GetProxyNode();

      if (proxyNode.IsValid() && !proxyNode.GetPath().Empty())
      {
         QMenu    menu;
         QAction* copyAction = new QAction("Copy", &menu);
         connect(copyAction, &QAction::triggered, this, &DockWidget::TryCopy);
         menu.addAction(new wizard::ViewTypeSummaryAction(proxyNode, &menu));
         emit wizSignals->ActionsForNodeRequested(proxyNode, &menu, this);
         menu.addSeparator();
         if (proxyNode.IsOfType("Platform"))
         {
            menu.addAction(new wizard::CreatePlatform(proxyNode, this, &menu));
         }
         menu.addAction(copyAction);
         menu.exec(aPos);
      }
   }
}

PlatformBrowser::Item* PlatformBrowser::DockWidget::FindItem(const QModelIndex& aIndex) const
{
   QModelIndex idx = aIndex;
   return dynamic_cast<Item*>(mPlatformModelPtr->FindItem(idx));
}

void PlatformBrowser::DockWidget::RebuildTitle()
{
   if ((nullptr != mPlatformModelPtr) && (nullptr != mPlatformModelPtr->GetRootItem()))
   {
      int numPlatforms = (int)mPlatformModelPtr->GetRootItem()->GetNumChildren();

      setWindowTitle(QString("Platform Browser (") + QString::number(numPlatforms) + ")");
   }
}

void PlatformBrowser::DockWidget::ClearSearch()
{
   mUi.filterText->clearEditText();
}

void PlatformBrowser::DockWidget::OnActiveProjectChanged(wizard::Project* aProjectPtr)
{
   if (aProjectPtr && aProjectPtr->GetParseResults())
   {
      OnProxyUpdated(nullptr);
   }
}

void PlatformBrowser::DockWidget::TryCopy()
{
   if (mUi.platformView->hasFocus())
   {
      QModelIndexList indices = mUi.platformView->selectionModel()->selectedIndexes();
      if (indices.isEmpty())
      {
         mPlatformModelPtr->mimeData(indices);
      }
   }
}

void PlatformBrowser::DockWidget::ProjectOpened(wizard::Project* aProjectPtr)
{
   SetProject(aProjectPtr);
}

void PlatformBrowser::DockWidget::ProjectClosed(wizard::Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr)
   {
      SetProject(nullptr);
   }
}

void PlatformBrowser::DockWidget::ItemActivated(const QModelIndex& aIndex)
{
   if (!mPlatformModelPtr)
   {
      return;
   }
   Item* itemPtr = FindItem(aIndex);
   if (nullptr != itemPtr)
   {
      OpenLocation(itemPtr);
   }
}

void PlatformBrowser::DockWidget::FilterChanged(const QString& aFilterText)
{
   if (!aFilterText.isEmpty() && !mExpandStateIsSaved)
   {
      mExpandStateIsSaved = true;
      mSavedExpandState.clear();
      UtQtGetExpandedItems(*mUi.platformView, mUi.platformView->rootIndex(), mSavedExpandState);
      mUi.platformView->expandAll();
   }
   else if (aFilterText.isEmpty())
   {
      if (mExpandStateIsSaved)
      {
         UtQtSetExpandedItems(*mUi.platformView, mUi.platformView->rootIndex(), mSavedExpandState);
      }
      mSavedExpandState.clear();
      mExpandStateIsSaved = false;
   }
   mUi.platformView->mSearchRegex = QRegExp(aFilterText, Qt::CaseInsensitive, QRegExp::FixedString);
   mUi.platformView->FilterUpdateLater();

   RebuildTitle();
}

void PlatformBrowser::DockWidget::OnProxyUpdated(wizard::ProxyMerge* aProxyMerge)
{
   if (!isVisible())
   {
      return Hibernate();
   }
   RebuildTitle();
}

bool PlatformBrowser::DockWidget::OpenLocation(Item* aItemPtr)
{
   if (!aItemPtr || !mProjectPtr || !mProjectPtr->TinyParseResults())
   {
      return false;
   }
   wizard::ParseResultsTiny* resultsPtr = mProjectPtr->TinyParseResults();
   WsfPProxyIndex*           indexPtr   = resultsPtr->ProxyIndex();

   bool opened = false;

   if (indexPtr)
   {
      WsfPProxyPath                    endPath = aItemPtr->GetProxyPath();
      std::vector<UtTextDocumentRange> ranges;
      resultsPtr->FindDefinitionStartLocationsExtended(endPath, ranges);
      wizard::FindReferencesResultDialog::GoToLocationList(mProjectPtr, ranges);
   }
   return opened;
}

void PlatformBrowser::DockWidget::SetProject(wizard::Project* aProjectPtr)
{
   Hibernate();
   mProjectPtr = aProjectPtr;
   ClearSearch();
}

// This browser is no longer being shown and contains stale data.  Stop updating until visible again.
void PlatformBrowser::DockWidget::Hibernate() {}

void PlatformBrowser::DockWidget::showEvent(QShowEvent* e)
{
   if (mProjectPtr && mProjectPtr->GetParseResults())
   {
      OnProxyUpdated(nullptr);
   }
   QDockWidget::showEvent(e);
}

void PlatformBrowser::DockWidget::hideEvent(QHideEvent* e)
{
   Hibernate();
   QDockWidget::hideEvent(e);
}

void PlatformBrowser::DockWidget::PlatformSelectionChangedHandler(const wkf::PlatformList& aSelectedPlatforms,
                                                                  const wkf::PlatformList& aUnselectedPlatforms)
{
   if (mEntitySelectionMutex.tryLock())
   {
      for (const auto& platform : aSelectedPlatforms)
      {
         SetPlatformSelected(QString::fromStdString(platform->GetName()), true);
      }
      for (const auto& platform : aUnselectedPlatforms)
      {
         SetPlatformSelected(QString::fromStdString(platform->GetName()), false);
      }
      mEntitySelectionMutex.unlock();
   }
}

void PlatformBrowser::DockWidget::SetPlatformSelected(const QString& aName, bool aSelect)
{
   QModelIndexList list =
      mPlatformModelPtr->match(mPlatformModelPtr->index(0, 0, QModelIndex()), Qt::DisplayRole, aName, 1, Qt::MatchExactly);
   if (!list.empty())
   {
      const QModelIndex& idx = list.first();
      mUi.platformView->selectionModel()->select(idx,
                                                 aSelect ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
      mUi.platformView->scrollTo(idx);
   }
}

void PlatformBrowser::DockWidget::ViewSelectionChanged(const QItemSelection& aSelected, const QItemSelection& aDeselected)
{
   if (mEntitySelectionMutex.tryLock())
   {
      QModelIndexList sil = mUi.platformView->selectionModel()->selectedIndexes();

      wkf::PlatformList selectedList;
      for (auto&& sel : sil)
      {
         Item* item = dynamic_cast<Item*>(mPlatformModelPtr->FindItem(sel));
         if (item->GetNodeType() == Item::NodeType::cPLATFORM)
         {
            wkf::Platform* platform = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(item->mName.c_str()));
            if (platform != nullptr)
            {
               selectedList.push_back(platform);
            }
         }
      }
      wkfEnv.SetPlatformsSelected(selectedList);

      mEntitySelectionMutex.unlock();
   }
}
