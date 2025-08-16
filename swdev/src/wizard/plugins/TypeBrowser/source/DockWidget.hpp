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

#include <QDockWidget>

#include "ui_TypeBrowser.h"

namespace wizard
{
class Project;
}

namespace TypeBrowser
{
class Item;
class Model;
class ProxyModel;

class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   friend class Model;

   DockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

   void NextSearchResult(int aDirection);
   bool SelectSearchResult();

   bool IsFilteredItem(QModelIndex aIndex);
   void ContextEvent(const QModelIndex& aIndex, QPoint aPos);

   Item* FindItem(const QModelIndex& aIndex) const;
   void  showEvent(QShowEvent* e) override;
public slots:

   void TryCopy();
   void ClearSearch();
   void OnFullParse(wizard::Project*);
   void OnActiveProjectChanged(wizard::Project*);

protected slots:

   void ProjectOpened(wizard::Project* aProjectPtr);
   void ProjectClosed(wizard::Project* aProjectPtr);
   void ItemActivated(const QModelIndex& aIndex);
   void FilterChanged(const QString& aFilterText);
   void showUnusedTypesToggled();

protected:
   bool OpenLocation(Item* aItemPtr);
   void SetProject(wizard::Project* aProjectPtr);

private:
   wizard::Project* mProjectPtr;
   Model*           mTypeModelPtr;
   ProxyModel*      mFilterProxyPtr;
   Ui::TypeBrowser  mUi;
};
} // namespace TypeBrowser

#endif
