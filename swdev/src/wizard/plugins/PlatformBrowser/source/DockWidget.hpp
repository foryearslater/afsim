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
#ifndef PLATFORMBROWSERDOCKWIDGET_HPP
#define PLATFORMBROWSERDOCKWIDGET_HPP

#include <QDockWidget>
#include <QMutex>
#include <QPersistentModelIndex>
#include <QSet>

#include "WkfEnvironment.hpp"
#include "ui_PlatformBrowser.h"

namespace wizard
{
class Project;
class ProxyMerge;
} // namespace wizard

namespace PlatformBrowser
{
class Item;
class Model;

class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   friend class Model;

   DockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

   bool SelectSearchResult();

   void ContextEvent(const QModelIndex& aIndex, QPoint aPos);

   Item* FindItem(const QModelIndex& aIndex) const;
   void  RebuildTitle();

public slots:

   void ClearSearch();
   void OnActiveProjectChanged(wizard::Project* aProjectPtr);
   void TryCopy();

protected slots:

   void ProjectOpened(wizard::Project* aProjectPtr);
   void ProjectClosed(wizard::Project* aProjectPtr);
   void ItemActivated(const QModelIndex& aIndex);
   void FilterChanged(const QString& aFilterText);
   void OnProxyUpdated(wizard::ProxyMerge* aProxyMerge);

protected:
   bool OpenLocation(Item* aItemPtr);

   void SetProject(wizard::Project* aProjectPtr);
   void Hibernate();

   void showEvent(QShowEvent*) override;
   void hideEvent(QHideEvent*) override;


private:
   void PlatformSelectionChangedHandler(const wkf::PlatformList& aSelectedPlatforms,
                                        const wkf::PlatformList& aUnselectedPlatforms);
   void SetPlatformSelected(const QString& aName, bool aSelect);
   void ViewSelectionChanged(const QItemSelection& aSelected, const QItemSelection& aDeselected);
   ;
   wizard::Project*    mProjectPtr;
   Model*              mPlatformModelPtr;
   Ui::PlatformBrowser mUi;

   QSet<QPersistentModelIndex> mSavedExpandState;
   bool                        mExpandStateIsSaved;

   // When a user selects a platform in the treeWidget, it sets the platform in focus in the WkfEnvironment,
   // When a platform is set in Focus in the WkfEnvironment, it sets the platform selected in the treeWidget,
   // To prevent an infinite loop from occurring, use a mutex to prevent processing when necessary.
   QMutex mEntitySelectionMutex;
};
} // namespace PlatformBrowser
#endif
