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

#include "TLE_Widget.hpp"

#include <QFileDialog>
#include <QStringList>

#include "SatelliteInserterPrefWidget.hpp"
#include "SatelliteInserterSetHandler.hpp"

SpaceTools::TLE_Widget::TLE_Widget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr)
   : QWidget(aParentPtr)
   , mPrefObjectPtr(aPrefObjectPtr)
{
   mUI.setupUi(this);

   connect(mUI.mAddTLE_SetButton, &QPushButton::clicked, this, &TLE_Widget::AddTLE_File);
   connect(mUI.mRemoveTLE_Button, &QPushButton::clicked, this, &TLE_Widget::RemoveTLE_Files);
   connect(mUI.mLoadedTLE_Files, &QListWidget::itemChanged, this, &TLE_Widget::UpdateTLECheckState);
   connect(mUI.mUpdateTLEsButton, &QPushButton::clicked, this, &TLE_Widget::UpdateTLEs);
   connect(mUI.mTLE_CheckAll, &QCheckBox::clicked, this, &TLE_Widget::CheckAllTLEs);
   connect(mUI.mLoadedTLE_Files, &QListWidget::itemSelectionChanged, this, &TLE_Widget::TLE_SetSelectionChanged);

   mUI.mRemoveTLE_Button->setEnabled(false);
}

void SpaceTools::TLE_Widget::AddTLE_File()
{
   QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add TLE Set", "", QString("Text (*.txt)"));
   for (const auto& file : fileNames)
   {
      bool loaded = false;
      for (int i = 0; i < mUI.mLoadedTLE_Files->count(); ++i)
      {
         auto f = mUI.mLoadedTLE_Files->item(i)->data(Qt::ToolTipRole).toString();
         if (file == f)
         {
            loaded = true;
            break;
         }
      }
      if (!loaded)
      {
         auto itemPtr = new QListWidgetItem;
         itemPtr->setData(Qt::ToolTipRole, file);
         UtPath name(file.toStdString());
         itemPtr->setData(Qt::DisplayRole, QString::fromStdString(name.GetFileName()));
         itemPtr->setCheckState(Qt::Unchecked);
         QVariant f;
         f.setValue(PrefData::File(file));
         itemPtr->setData(Qt::UserRole, f);
         mUI.mLoadedTLE_Files->insertItem(0, itemPtr);
      }
   }
   SetTLE_CheckAll();
   SaveTLE_Files();
}

void SpaceTools::TLE_Widget::CheckAllTLEs()
{
   auto checkState(mUI.mTLE_CheckAll->checkState());
   for (int i = 0; i < mUI.mLoadedTLE_Files->count(); ++i)
   {
      mUI.mLoadedTLE_Files->item(i)->setCheckState(checkState);
   }
   SaveTLE_Files();
}

void SpaceTools::TLE_Widget::SetTLE_CheckAll()
{
   bool allChecked   = true;
   bool allUnchecked = true;
   for (int i = 0; i < mUI.mLoadedTLE_Files->count(); ++i)
   {
      if (mUI.mLoadedTLE_Files->item(i)->checkState() == Qt::Checked)
      {
         allUnchecked = false;
      }
      else
      {
         allChecked = false;
      }
      if (!allChecked && !allUnchecked)
      {
         mUI.mTLE_CheckAll->setCheckState(Qt::PartiallyChecked);
         break;
      }
   }

   if (allChecked)
   {
      mUI.mTLE_CheckAll->setChecked(true);
   }
   else if (allUnchecked)
   {
      mUI.mTLE_CheckAll->setChecked(false);
   }
}

void SpaceTools::TLE_Widget::RemoveTLE_Files()
{
   for (auto& database : mUI.mLoadedTLE_Files->selectedItems())
   {
      delete mUI.mLoadedTLE_Files->takeItem(mUI.mLoadedTLE_Files->row(database));
   }
   SaveTLE_Files();
}

void SpaceTools::TLE_Widget::TLE_SetSelectionChanged()
{
   mUI.mRemoveTLE_Button->setEnabled(!mUI.mLoadedTLE_Files->selectedItems().empty());
}

void SpaceTools::TLE_Widget::UpdateTLEs()
{
   QStringList fileList;
   for (int i = 0; i < mUI.mLoadedTLE_Files->count(); ++i)
   {
      auto itemPtr = mUI.mLoadedTLE_Files->item(i);
      auto file    = qvariant_cast<PrefData::File>(itemPtr->data(Qt::UserRole));
      if (file.mChecked)
      {
         fileList << file.mPath;
      }
   }
   if (!fileList.isEmpty())
   {
      SatelliteInserterSetHandler handler(fileList);
      handler.UpdateTLEs();
   }
}

//! Initializes the TLE widget if it hasn't previously been initialized
void SpaceTools::TLE_Widget::OnShow()
{
   if (!mIsInitialized)
   {
      Initialize();
      mIsInitialized = true;
   }
}

void SpaceTools::TLE_Widget::Initialize()
{
   auto& currentPrefs = mPrefObjectPtr->GetPreferences();
   for (auto& f : currentPrefs.mTLE_Sets)
   {
      UtPath path(f.mPath.toStdString());
      if (!path.Exists())
      {
         continue;
      }
      auto itemPtr = new QListWidgetItem;
      itemPtr->setData(Qt::ToolTipRole, f.mPath);
      itemPtr->setData(Qt::DisplayRole, QString::fromStdString(path.GetFileName()));
      itemPtr->setCheckState(f.mChecked ? Qt::Checked : Qt::Unchecked);
      QVariant file;
      file.setValue(f);
      itemPtr->setData(Qt::UserRole, file);

      mUI.mLoadedTLE_Files->addItem(itemPtr);
   }
   SetTLE_CheckAll();
}

void SpaceTools::TLE_Widget::SaveTLE_Files()
{
   std::vector<SpaceTools::PrefData::File> tleFiles;
   for (int i = 0; i < mUI.mLoadedTLE_Files->count(); i++)
   {
      auto itemPtr = mUI.mLoadedTLE_Files->item(i);
      tleFiles.push_back(qvariant_cast<PrefData::File>(itemPtr->data(Qt::UserRole)));
   }
   mPrefObjectPtr->SaveTLE_Sets(tleFiles);
}


void SpaceTools::TLE_Widget::UpdateTLECheckState(QListWidgetItem* aItemPtr)
{
   auto f     = qvariant_cast<PrefData::File>(aItemPtr->data(Qt::UserRole));
   f.mChecked = (aItemPtr->checkState() == Qt::Checked) ? true : false;
   QVariant file;
   file.setValue(f);
   aItemPtr->setData(Qt::UserRole, file);
   SetTLE_CheckAll();
   SaveTLE_Files();
}
