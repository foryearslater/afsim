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
#ifndef PROJECTBROWSERDOCKWIDGET_HPP
#define PROJECTBROWSERDOCKWIDGET_HPP

#include <set>

#include <QDockWidget>
#include <QMutex>
#include <QSettings>
#include <QTreeWidgetItem>

#include "UtCallbackHolder.hpp"
#include "UtPath.hpp"
#include "ui_ProjectBrowser.h"

namespace wizard
{
class CreateNewFile;
class Project;
} // namespace wizard

namespace ProjectBrowser
{
class Item;
class Model;
class ProxyModel;

class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   friend class View;
   DockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

   void NextSearchResult(int aDirection);
   bool SelectSearchResult();

   bool  IsFilteredItem(QModelIndex aIndex);
   void  ContextEvent(const QModelIndex& aIndex, QPoint aPos);
   Item* FindItem(const QModelIndex& aIndex);
   void  parseUpdate(bool aFullUpdate);
   void  CheckForModifications();
public slots:

   void ClearSearch();
   void FileCheck();
   void DirectorySetChanged(const std::vector<UtPath>& aDirectories);

protected slots:

   void ItemActivated(const QModelIndex& aIndex);
   void FilterChanged(const QString& aFilterText);
   void SetProject(wizard::Project* aProjectPtr);

protected:
   bool OpenFile(Item* aItemPtr);
   void CurrentChanged(const QModelIndex& aIndex);

private:
   void CollapseChildren(const QModelIndex& aIndex);
   void ExpandChildren(const QModelIndex& aIndex);

   wizard::Project*       mProjectPtr;
   Model*                 mModelPtr;
   ProxyModel*            mFilterProxyPtr;
   wizard::CreateNewFile* mNewFilePtr;
   Ui::ProjectBrowser     mUi;
   UtCallbackHolder       mUtCallbacks;
};
} // namespace ProjectBrowser
#endif
