// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ErrorDock.hpp"

#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "TextLocationItemModel.hpp"
#include "UtQtTextDelegate.hpp"
#include "WsfParser.hpp"

ErrorList::ErrorDock::ErrorDock(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mModelPtr(nullptr)
   , mProjectPtr(nullptr)
{
   setObjectName("OutputDock");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());

   setWindowTitle(widget()->windowTitle());

   mUi.errorListView->setItemDelegateForColumn(1, new UtQtTextDelegate(mUi.errorListView));

   connect(mUi.errorListView, &QTreeView::activated, this, &ErrorDock::ItemActivated);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &ErrorDock::ProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &ErrorDock::ProjectClosed);
}

void ErrorList::ErrorDock::ProjectOpened(wizard::Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
   connect(aProjectPtr, &wizard::Project::ParseUpdatedFull, this, &ErrorDock::Update);
}

void ErrorList::ErrorDock::ProjectClosed(wizard::Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr)
   {
      mProjectPtr = nullptr;
   }
}

void ErrorList::ErrorDock::ItemActivated(const QModelIndex& aIndex)
{
   if (!mProjectPtr)
      return;
   if (mModelPtr && aIndex.row() >= 0 && aIndex.row() < (int)mModelPtr->GetLocations().size())
   {
      UtTextDocumentRange range = mModelPtr->GetLocations()[aIndex.row()];
      mProjectPtr->GotoRange(range);
   }
}

void ErrorList::ErrorDock::Update()
{
   if (!mProjectPtr || !mProjectPtr->ParseIsUpToDate())
   {
      return;
   }
   delete mModelPtr;
   mModelPtr = nullptr;
   if (!isVisible())
      return;

   std::vector<UtTextDocumentRange>   errorLocations;
   wizard::ParseResults&              results = *mProjectPtr->GetParseResults();
   const std::vector<WsfParseError*>& errors  = results.errors();
   for (auto&& it : errors)
   {
      errorLocations.push_back(it->mRange);
   }

   std::sort(errorLocations.begin(), errorLocations.end(), &ErrorDock::CompareRange);
   std::vector<UtTextDocumentRange>::iterator newEnd =
      std::unique(errorLocations.begin(), errorLocations.end(), &ErrorDock::RangeEqual);
   if (newEnd != errorLocations.end())
   {
      errorLocations.erase(newEnd, errorLocations.end());
   }
   delete mModelPtr;
   mModelPtr            = new wizard::TextLocationItemModel(errorLocations);
   mModelPtr->mShowText = true;
   mUi.errorListView->setModel(mModelPtr);

   int numErrors = mModelPtr->rowCount(QModelIndex());
   if (numErrors > 0)
   {
      setWindowTitle("Error List (" + QVariant(numErrors).toString() + ")");
   }
   else
   {
      setWindowTitle("Error List");
   }
}

void ErrorList::ErrorDock::showEvent(QShowEvent* aEventPtr)
{
   QDockWidget::showEvent(aEventPtr);
   if (mProjectPtr)
   {
      Update();
   }
}

// static
bool ErrorList::ErrorDock::CompareRange(const UtTextDocumentRange& aLhs, const UtTextDocumentRange& aRhs)
{
   if (aLhs.mSource < aRhs.mSource)
      return true;
   if (aLhs.mSource > aRhs.mSource)
      return false;
   if (aLhs.GetBegin() < aRhs.GetBegin())
      return true;
   return false;
}

// static
bool ErrorList::ErrorDock::RangeEqual(const UtTextDocumentRange& aLhs, const UtTextDocumentRange& aRhs)
{
   return (aLhs.mSource == aRhs.mSource && aLhs.GetBegin() == aRhs.GetBegin());
}
