// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlotOptionsDialog.hpp"

#include "PlotOptionsWidget.hpp"
#include "Project.hpp"

SPLAT::PlotOptionsDialog::PlotOptionsDialog(QWidget* aParent)
   : QDialog(aParent)
   , mTargetPlatformType("")
   , mProjectDirectory(wizard::Project::Instance()->ProjectDirectory().GetNormalizedPath().c_str())
{
   mUi.setupUi(this);
   Connect();
}

void SPLAT::PlotOptionsDialog::AddWidget(const std::string& aName, PlotOptionsWidget* aWidgetPtr)
{
   mWidgetVec.push_back(aWidgetPtr);
   mUi.stackedWidget->addWidget(mWidgetVec.back());
   mWidgetIndexMap[aName] = static_cast<int>(mWidgetVec.size() - 1);
}

void SPLAT::PlotOptionsDialog::AddItem(QTreeWidgetItem* aItem, const QString& aToolTip, QTreeWidgetItem* aParentItem)
{
   if (aItem != nullptr)
   {
      if (aParentItem == nullptr)
      {
         mUi.treeWidget->addTopLevelItem(aItem);
      }
      else
      {
         aParentItem->addChild(aItem);
         aParentItem->setExpanded(true);
      }
      aItem->setToolTip(0, aToolTip);
   }
}

void SPLAT::PlotOptionsDialog::ItemSelected(QTreeWidgetItem* aTreeItem, int aColumn)
{
   if (aTreeItem != nullptr)
   {
      std::string selectedItemName = aTreeItem->data(0, Qt::UserRole).value<QString>().toStdString();
      SetWidget(selectedItemName);
   }
}

void SPLAT::PlotOptionsDialog::SetWidget(const std::string& aName)
{
   auto iter = mWidgetIndexMap.find(aName);
   if (iter != mWidgetIndexMap.end())
   {
      mUi.stackedWidget->setCurrentIndex(iter->second);
   }
}

SPLAT::PlotOptionsWidget* SPLAT::PlotOptionsDialog::GetWidget(const std::string& aName)
{
   auto iter = mWidgetIndexMap.find(aName);
   if (iter != mWidgetIndexMap.end())
   {
      return GetWidget(iter->second);
   }
   return nullptr;
}

SPLAT::PlotOptionsWidget* SPLAT::PlotOptionsDialog::GetWidget(const int aIndex)
{
   if (aIndex < static_cast<int>(mWidgetVec.size() - 1))
   {
      return mWidgetVec.at(aIndex);
   }
   return nullptr;
}

void SPLAT::PlotOptionsDialog::Connect()
{
   connect(mUi.treeWidget, &QTreeWidget::itemClicked, this, &PlotOptionsDialog::ItemSelected);
   connect(mUi.mCancel, &QPushButton::pressed, this, [this]() { close(); });
   connect(mUi.mOk, &QPushButton::pressed, this, &PlotOptionsDialog::OkClicked);
   connect(mUi.mRestoreDefaults, &QPushButton::pressed, this, &PlotOptionsDialog::RestoreDefaults);
}

void SPLAT::PlotOptionsDialog::OkClicked()
{
   bool success = true;
   // For all widgets, apply changes.
   // If all of them succeed, close Dialog
   for (auto widget : mWidgetVec)
   {
      if (!(widget->ApplyChanges()))
      {
         success = false;
         break;
      }
   }

   if (success)
   {
      close();
   }
}

void SPLAT::PlotOptionsDialog::RestoreDefaults()
{
   mWidgetVec[mUi.stackedWidget->currentIndex()]->RestoreDefaults();
}
