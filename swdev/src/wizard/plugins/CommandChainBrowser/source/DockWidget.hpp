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

#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include <memory>

#include <QDockWidget>
#include <QSet>

#include "Model.hpp"
#include "UtCallbackHolder.hpp"
#include "ui_CommandChainBrowser.h"

namespace wizard
{
class Editor;
class Project;
} // namespace wizard

namespace CommandChain
{
class Item;

class DockWidget : public QDockWidget
{
public:
   explicit DockWidget(QWidget* aParent);
   ~DockWidget() override = default;

   const wizard::Project* Project() const { return mProjectPtr; }
   Item*                  FindItem(const QModelIndex& aIndex);
   void                   ContextEvent(const QModelIndex& aIndex, QPoint aPos);
   void                   TryCopy();
   static wizard::Editor* GetEditor(const WsfPProxyNode& aNode, UtTextDocumentRange& aRangePtr);

private:
   void ProjectOpened(wizard::Project* aProjectPtr);
   void ProjectClosed(wizard::Project* aProjectPtr);
   void URL_Process(const QUrl& aUrl);
   void PopulateTeams();
   void PopulateCommandChains();
   void ItemActivated(const QModelIndex& aIndex);
   void FilterTeamChanged(const QString& aTeamText);
   void FilterCommandChainChanged(const QString& aCommandChain);
   void ClearSearch();
   void SetProject(wizard::Project* aProjectPtr);
   void UpdateSelection();
   bool OpenLocation(Item* aItemPtr);
   void CollapseChildren(const QModelIndex& aIndex);
   void ExpandChildren(const QModelIndex& aIndex);
   void ShowPlatformInfo(const WsfPM_Platform& aPlatform, const QPoint& aPos);
   void AddToNewCommandChain(const std::string& aPlatformName);
   void showEvent(QShowEvent*) override;
   void hideEvent(QHideEvent*) override;

   Ui::CommandChainBrowserUi mUi;
   wizard::Project*          mProjectPtr;
   std::unique_ptr<Model>    mCommandChainModelPtr;
   QSet<QString>             mCommandChainNames;
   UtCallbackHolder          mCallbacks;
   QString                   mShowChainName;
   QString                   mShowPlatformName;

   const char* cALL_SIDES  = "All Sides";
   const char* cALL_CHAINS = "All Chains";
};
} // namespace CommandChain

#endif
