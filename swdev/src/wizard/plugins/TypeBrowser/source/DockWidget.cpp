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

#include <set>

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QSortFilterProxyModel>

#include "CreatePlatform.hpp"
#include "Environment.hpp"
#include "FindReferencesResultDialog.hpp"
#include "Item.hpp"
#include "Model.hpp"
#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "Util.hpp"
#include "ViewTypeSummaryAction.hpp"

class TypeBrowser::ProxyModel : public QSortFilterProxyModel
{
public:
   explicit ProxyModel(Model* aModelPtr)
      : mModelPtr(aModelPtr)
   {
      setSourceModel(aModelPtr);
   }

   Model*  mModelPtr;
   QRegExp mRegEx;

   void Update()
   {
      mAcceptedItems.clear();
      Walk(mModelPtr->GetRootItem());
      invalidate();
   }

   bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override { return true; }

   bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
   {
      if (sourceModel() != mModelPtr)
         return false;
      QModelIndex idx     = mModelPtr->index(source_row, 0, source_parent);
      Item*       itemPtr = dynamic_cast<Item*>(mModelPtr->FindItem(idx));
      if (!itemPtr)
      {
         return true;
      }
      return (mAcceptedItems.find(itemPtr) != mAcceptedItems.end());
   }

private:
   int Walk(Item* aItemPtr, bool aIncludeEverything = false)
   {
      bool includeEverything = aIncludeEverything;
      if (!aItemPtr)
         return 0;
      int acceptedCount = 0;

      if (-1 != mRegEx.indexIn(aItemPtr->mName))
      {
         includeEverything = true;
      }
      int count = aItemPtr->GetAbstractItemCount();
      for (int i = 0; i < count; ++i)
      {
         Item* childPtr = dynamic_cast<Item*>(aItemPtr->GetAbstractItem(i));
         acceptedCount += Walk(childPtr, includeEverything);
      }
      if (acceptedCount == 0)
      {
         if (includeEverything || -1 != mRegEx.indexIn(aItemPtr->mName))
         {
            acceptedCount = 1;
         }
         else
         {
            acceptedCount = 0;
         }
      }
      if (acceptedCount > 0)
      {
         mAcceptedItems.insert(aItemPtr);
      }
      return acceptedCount;
   }

   std::set<Item*> mAcceptedItems;
};

namespace
{
class SearchInputEventFilter : public QObject
{
public:
   explicit SearchInputEventFilter(TypeBrowser::DockWidget* aParentPtr)
      : QObject(aParentPtr)
      , mParentPtr(aParentPtr)
   {
   }
   TypeBrowser::DockWidget* mParentPtr;

   bool eventFilter(QObject* obj, QEvent* event) override
   {
      if (event->type() == QEvent::KeyPress)
      {
         QKeyEvent* evt = (QKeyEvent*)event;
         switch (evt->key())
         {
         case Qt::Key_Down:
            mParentPtr->NextSearchResult(1);
            evt->accept();
            return true;
            break;
         case Qt::Key_Up:
            mParentPtr->NextSearchResult(-1);
            evt->accept();
            return true;
            break;
         case Qt::Key_Enter:
         case Qt::Key_Return:
            mParentPtr->SelectSearchResult();
            evt->accept();
            return true;
            break;
         case Qt::Key_Escape:
            mParentPtr->ClearSearch();
            evt->accept();
            return true;
            break;
         }
      }
      return QObject::eventFilter(obj, event);
   }
};
} // namespace


TypeBrowser::DockWidget::DockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mProjectPtr(nullptr)
   , mTypeModelPtr(nullptr)
   , mFilterProxyPtr(nullptr)
{
   setObjectName("TypeBrowser");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::FullParseComplete, this, &DockWidget::OnFullParse);
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::ActiveProjectChanged,
           this,
           &DockWidget::OnActiveProjectChanged);

   mUi.typeView->mParentPtr         = this;
   mUi.typeViewFiltered->mParentPtr = this;
   mUi.typeView->setExpandsOnDoubleClick(false);
   mUi.typeView->setTextElideMode(Qt::ElideRight);
   mUi.typeViewFiltered->setTextElideMode(Qt::ElideRight);
   // Make view more compact
   mUi.typeView->setIndentation(10);
   mUi.typeViewFiltered->setIndentation(10);
   setWindowTitle(widget()->windowTitle());

   mUi.filterText->setAutoCompletion(false);
   mUi.filterText->installEventFilter(new SearchInputEventFilter(this));

   // mUi.typeView->setSelectionBehavior(QAbstractItemView::SelectRows);
   mUi.typeView->setHeaderHidden(true);
   mUi.typeView->setDragEnabled(true);
   // mUi.typeViewFiltered->setSelectionBehavior(QAbstractItemView::SelectRows);
   mUi.typeViewFiltered->setHeaderHidden(true);
   mUi.typeViewFiltered->setDragEnabled(true);

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &DockWidget::ProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &DockWidget::ProjectClosed);

   connect(mUi.typeView, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.typeViewFiltered, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.filterText, &QComboBox::editTextChanged, this, &DockWidget::FilterChanged);
   connect(mUi.clearFilterBn, &QToolButton::clicked, this, &DockWidget::ClearSearch);
   connect(mUi.showUnusedTypes, &QCheckBox::clicked, this, &DockWidget::showUnusedTypesToggled);

   mUi.stackedWidget->setCurrentIndex(0);

   mTypeModelPtr = new Model();
   mTypeModelPtr->setParent(this);
   mUi.typeView->setModel(mTypeModelPtr);

   SetProject(wizard::Project::Instance());
}

void TypeBrowser::DockWidget::NextSearchResult(int aDirection)
{
   mUi.typeViewFiltered->MoveCursor(aDirection);
}

bool TypeBrowser::DockWidget::SelectSearchResult()
{
   bool        opened = false;
   QModelIndex idx    = mUi.typeView->currentIndex();
   if (mFilterProxyPtr && idx.isValid())
   {
      idx = mFilterProxyPtr->mapToSource(idx);
   }
   return opened;
}

bool TypeBrowser::DockWidget::IsFilteredItem(QModelIndex aIndex)
{
   if (!mFilterProxyPtr)
      return false;
   QModelIndex srcIndex = mFilterProxyPtr->mapToSource(mUi.typeViewFiltered->currentIndex());
   Item*       itemPtr  = dynamic_cast<Item*>(mTypeModelPtr->FindItem(srcIndex));
   if (itemPtr != nullptr)
   {
      if (!itemPtr->GetAbstractItemCount())
      {
         return true;
      }
   }
   return false;
}

void TypeBrowser::DockWidget::ContextEvent(const QModelIndex& aIndex, QPoint aPos)
{
   if (!mProjectPtr)
      return;
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

TypeBrowser::Item* TypeBrowser::DockWidget::FindItem(const QModelIndex& aIndex) const
{
   QModelIndex idx = aIndex;
   if (mFilterProxyPtr && aIndex.model() == mFilterProxyPtr)
   {
      idx = mFilterProxyPtr->mapToSource(aIndex);
   }
   return dynamic_cast<Item*>(mTypeModelPtr->FindItem(idx));
}

void TypeBrowser::DockWidget::showEvent(QShowEvent* e)
{
   mTypeModelPtr->Hibernate(false);
}

void TypeBrowser::DockWidget::TryCopy()
{
   if (mUi.typeView->hasFocus())
   {
      QMimeData* mime = nullptr;
      if (mFilterProxyPtr)
      {
         mime = mFilterProxyPtr->mimeData(mUi.typeViewFiltered->selectionModel()->selectedIndexes());
      }
      else
      {
         mime = mTypeModelPtr->mimeData(mUi.typeView->selectionModel()->selectedIndexes());
      }
      if (mime)
      {
         QApplication::clipboard()->setMimeData(mime);
      }
   }
}

void TypeBrowser::DockWidget::ClearSearch()
{
   mUi.filterText->clearEditText();
}

void TypeBrowser::DockWidget::OnFullParse(wizard::Project*)
{
   if (!isVisible())
   {
      mTypeModelPtr->Hibernate(true);
   }
   else
   {
      mTypeModelPtr->Hibernate(false);
   }
   // mTypeModelPtr->Update(aProjectPtr);

   // if (mFilterProxyPtr) mFilterProxyPtr->Update();
   ///   RebuildTitle();
}

void TypeBrowser::DockWidget::OnActiveProjectChanged(wizard::Project* aProjectPtr)
{
   if (aProjectPtr && aProjectPtr->ParseIsUpToDate())
   {
      OnFullParse(aProjectPtr);
   }
}

void TypeBrowser::DockWidget::ProjectOpened(wizard::Project* aProjectPtr)
{
   SetProject(aProjectPtr);
}

void TypeBrowser::DockWidget::ProjectClosed(wizard::Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr)
   {
      SetProject(nullptr);
   }
}

void TypeBrowser::DockWidget::ItemActivated(const QModelIndex& aIndex)
{
   if (!mTypeModelPtr)
      return;
   Item* itemPtr = FindItem(aIndex);
   if (nullptr != itemPtr)
   {
      OpenLocation(itemPtr);
   }
}

void TypeBrowser::DockWidget::FilterChanged(const QString& aFilterText)
{
   if (aFilterText.isEmpty())
   {
      mUi.stackedWidget->setCurrentIndex(0);
      mUi.typeViewFiltered->setModel(nullptr);
      delete mFilterProxyPtr;
      mFilterProxyPtr = nullptr;
      return;
   }
   if (!mFilterProxyPtr)
   {
      mFilterProxyPtr = new ProxyModel(mTypeModelPtr);
      mUi.typeViewFiltered->setModel(mFilterProxyPtr);
   }

   mFilterProxyPtr->mRegEx = wizard::Util::SimpleRegex(aFilterText);
   mFilterProxyPtr->Update();
   mUi.typeViewFiltered->expandAll();
   mUi.stackedWidget->setCurrentIndex(1);
}

void TypeBrowser::DockWidget::showUnusedTypesToggled()
{
   mTypeModelPtr->SetShowUnusedTypes(mUi.showUnusedTypes->isChecked());
}

bool TypeBrowser::DockWidget::OpenLocation(Item* aItemPtr)
{
   if (!mProjectPtr)
      return false;
   bool opened = false;
   if (aItemPtr)
   {
      std::vector<UtTextDocumentRange> ranges;
      wizard::ParseResults*            results = mProjectPtr->WaitForParseResults();
      if (results && results->ProxyIndex())
      {
         WsfPProxyIndexNode* nodePtr = aItemPtr->mTypeNode.FindIndexNode();
         if (nodePtr != nullptr && !nodePtr->mEntries.empty())
         {
            WsfParseNode* parseNode = nodePtr->mEntries[0].mNodePtr;
            if (parseNode)
            {
               WsfParseNode* n = parseNode->PreviousLeaf();
               if (n)
               {
                  UtTextDocumentRange range = n->GetParent()->SubtreeRange();
                  if (range.Valid())
                  {
                     ranges.push_back(range);
                  }
               }
            }
         }
      }
      if (!ranges.empty())
      {
         wizard::FindReferencesResultDialog::GoToLocationList(mProjectPtr, ranges);
      }
      else
      {
         QString page("ide://documentation/docs/");
         // Most pages are just lower case with underscores for spaces
         QString pageName = aItemPtr->mName;
         pageName         = pageName.toLower();
         pageName         = pageName.replace(" ", "_");
         QMap<QString, QString> nameToPage;
         // Explicitly provide pages for some types:
         nameToPage["ew_effect"]          = "Predefined_Electronic_Warfare_Effect_Types";
         nameToPage["ea_technique"]       = "electronic_warfare_technique";
         nameToPage["ep_technique"]       = "electronic_warfare_technique";
         nameToPage["electronic_attack"]  = "WSF_EA_TECHNIQUE";
         nameToPage["electronic_protect"] = "WSF_EP_TECHNIQUE";
         if (nameToPage.contains(pageName))
         {
            pageName = nameToPage[pageName];
         }
         page += pageName;
         page += ".html";
         wizEnv.OpenUrl(page);
      }
   }
   return opened;
}

void TypeBrowser::DockWidget::SetProject(wizard::Project* aProjectPtr)
{
   mTypeModelPtr->Hibernate(true);
   mProjectPtr = aProjectPtr;
   ClearSearch();
}
