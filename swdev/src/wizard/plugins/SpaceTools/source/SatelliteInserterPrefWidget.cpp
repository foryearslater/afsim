// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "SatelliteInserterPrefWidget.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMenu>

#include "SatelliteInserterPrefObject.hpp"
#include "SpaceToolsUtil.hpp"
#include "UtPath.hpp"

SpaceTools::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<SpaceTools::PrefObject>()
{
   mUI.setupUi(this);
   connect(mUI.mAddDatabase, &QPushButton::clicked, this, &PrefWidget::AddDatabase);
   connect(mUI.mDeleteButton, &QPushButton::clicked, this, &PrefWidget::DeleteLoadedDatabases);
   connect(mUI.mLoadedDatabases, &QListWidget::itemChanged, this, &PrefWidget::UpdateCheckState);
   connect(mUI.mLoadedDatabases, &QListWidget::itemSelectionChanged, this, &PrefWidget::DatabaseSelectionChanged);
   connect(mUI.mUpdateButton, &QPushButton::clicked, this, &PrefWidget::UpdateDatabases);
   connect(mUI.mDatabaseCheckAll, &QCheckBox::clicked, this, &PrefWidget::CheckAllDatabases);

   SetDatabaseCheckAll();
}

void SpaceTools::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mLoadedDatabases.clear();

   for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
   {
      auto itemPtr  = mUI.mLoadedDatabases->item(i);
      auto database = qvariant_cast<PrefData::Database>(itemPtr->data(Qt::UserRole));
      aPrefData.mLoadedDatabases.emplace_back(database);
   }
}

void SpaceTools::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUI.mLoadedDatabases->clear();
   for (auto& d : aPrefData.mLoadedDatabases)
   {
      auto itemPtr = new QListWidgetItem;
      itemPtr->setData(Qt::ToolTipRole, d.mFile.mPath);
      itemPtr->setData(Qt::DisplayRole, GetDisplayName(d.mFile.mPath));
      itemPtr->setCheckState(d.mFile.mChecked ? Qt::Checked : Qt::Unchecked);
      QVariant database;
      database.setValue(d);
      itemPtr->setData(Qt::UserRole, database);

      mUI.mLoadedDatabases->addItem(itemPtr);
   }

   SetDatabaseCheckAll();
}

void SpaceTools::PrefWidget::UpdateCheckState(QListWidgetItem* aItemPtr)
{
   auto d           = qvariant_cast<PrefData::Database>(aItemPtr->data(Qt::UserRole));
   d.mFile.mChecked = (aItemPtr->checkState() == Qt::Checked) ? true : false;
   QVariant database;
   database.setValue(d);
   aItemPtr->setData(Qt::UserRole, database);
   SetDatabaseCheckAll();
}


QString SpaceTools::PrefWidget::GetDisplayName(const QString& aPath) const
{
   UtPath path(aPath.toStdString());
   return QString::fromStdString(path.GetFileName());
}

void SpaceTools::PrefWidget::AddDatabase()
{
   QStringList databases = QFileDialog::getOpenFileNames(this, "Add Database", "", QString("Json (*.json)"));
   for (const auto& database : databases)
   {
      bool loaded = false;
      for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
      {
         auto d = qvariant_cast<PrefData::Database>(mUI.mLoadedDatabases->item(i)->data(Qt::UserRole));
         if (database == d.mFile.mPath)
         {
            loaded = true;
            break;
         }
      }
      if (!loaded)
      {
         auto itemPtr = new QListWidgetItem;
         itemPtr->setData(Qt::ToolTipRole, database);
         itemPtr->setData(Qt::DisplayRole, GetDisplayName(database));
         itemPtr->setCheckState(Qt::Checked);
         QVariant d;
         d.setValue(PrefData::Database(Util::GetDatabase(database), database));
         itemPtr->setData(Qt::UserRole, d);
         mUI.mLoadedDatabases->addItem(itemPtr);
         mUI.mDatabaseCheckAll->setChecked(false);
      }
   }
   SetDatabaseCheckAll();
}

void SpaceTools::PrefWidget::CheckAllDatabases()
{
   if (mUI.mDatabaseCheckAll->isChecked())
   {
      for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
      {
         mUI.mLoadedDatabases->item(i)->setCheckState(Qt::Checked);
      }
   }
   else
   {
      for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
      {
         mUI.mLoadedDatabases->item(i)->setCheckState(Qt::Unchecked);
      }
   }
}

void SpaceTools::PrefWidget::SetDatabaseCheckAll()
{
   bool allChecked   = true;
   bool allUnchecked = true;
   for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
   {
      if (mUI.mLoadedDatabases->item(i)->checkState() == Qt::Checked)
      {
         allUnchecked = false;
      }
      else
      {
         allChecked = false;
      }
      if (!allChecked && !allUnchecked)
      {
         mUI.mDatabaseCheckAll->setCheckState(Qt::PartiallyChecked);
         break;
      }
   }

   if (allChecked)
   {
      mUI.mDatabaseCheckAll->setChecked(true);
   }
   else if (allUnchecked)
   {
      mUI.mDatabaseCheckAll->setChecked(false);
   }
}

void SpaceTools::PrefWidget::DeleteLoadedDatabases()
{
   for (auto& database : mUI.mLoadedDatabases->selectedItems())
   {
      delete mUI.mLoadedDatabases->takeItem(mUI.mLoadedDatabases->row(database));
   }
}

SpaceTools::FilterListWidget::FilterListWidget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr)
   : QListWidget(aParentPtr)
   , mPrefObjectPtr(aPrefObjectPtr)
{
   for (auto&& it : mPrefObjectPtr->GetSavedFilters())
   {
      addItem(it.first);
   }
}

void SpaceTools::FilterListWidget::contextMenuEvent(QContextMenuEvent* aEventPtr)
{
   QListWidgetItem* itemPtr = itemAt(aEventPtr->pos());
   if (itemPtr != nullptr)
   {
      QMenu    menu(this);
      QAction* removeFilter = new QAction("Remove Filter", &menu);
      connect(removeFilter,
              &QAction::triggered,
              [this, itemPtr]()
              {
                 mPrefObjectPtr->DeleteFilter(itemPtr->text());
                 delete itemPtr;
              });
      menu.addAction(removeFilter);
      menu.exec(mapToGlobal(aEventPtr->pos()));
   }
}

void SpaceTools::PrefWidget::DatabaseSelectionChanged()
{
   mUI.mDeleteButton->setEnabled(!mUI.mLoadedDatabases->selectedItems().empty());
}

//! Updates the currently loaded databases. If the database no longer exists, it will be removed from the list.
void SpaceTools::PrefWidget::UpdateDatabases()
{
   for (int i = 0; i < mUI.mLoadedDatabases->count(); ++i)
   {
      auto itemPtr  = mUI.mLoadedDatabases->item(i);
      auto database = itemPtr->data(Qt::ToolTipRole).toString();
      auto doc      = Util::GetDatabase(database);
      if (doc.isNull())
      {
         delete mUI.mLoadedDatabases->takeItem(i);
      }
      else
      {
         QVariant d;
         d.setValue(PrefData::Database(doc, database));
         itemPtr->setData(Qt::UserRole, d);
      }
   }
}
