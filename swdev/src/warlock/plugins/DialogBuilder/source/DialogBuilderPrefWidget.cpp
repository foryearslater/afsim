// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "DialogBuilderPrefWidget.hpp"

#include "DialogBuilderDialogDefinition.hpp"
#include "DialogBuilderMainDialog.hpp"
#include "DialogBuilderPrefObject.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPluginManager.hpp"

WkDialogBuilder::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUi.setupUi(this);

   connect(mUi.newPushButton, &QPushButton::clicked, this, &PrefWidget::AddAction);
   connect(mUi.editPushButton, &QPushButton::clicked, this, &PrefWidget::EditAction);
   connect(mUi.deletePushButton, &QPushButton::clicked, this, &PrefWidget::DeleteAction);
}

void WkDialogBuilder::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.dialogs.clear();
   aPrefData.dialogs = mDialogMap;
}

void WkDialogBuilder::PrefWidget::RestoreDefaults()
{
   bool restore = true;

   if (!mDialogMap.empty())
   {
      int button = QMessageBox::warning(
         this,
         "Preferences",
         "Dialog Builder\n\nAre you sure you want to delete all created dialogs by restoring defaults?",
         QMessageBox::Yes,
         QMessageBox::No);
      if (button == QMessageBox::No)
      {
         restore = false;
      }
   }

   if (restore)
   {
      BasePrefWidget::RestoreDefaults();
   }
}

void WkDialogBuilder::PrefWidget::SetBannedNames(MainDialog& dialog, QString aExcludedName)
{
   QStringList list;
   for (auto i : mDialogMap.toStdMap())
   {
      if (i.first != aExcludedName)
      {
         list.push_back(QString(i.first).remove(QRegularExpression("\\s")));
      }
   }
   for (const auto& pluginIt : wkfEnv.GetPluginManager()->GetLoadedPlugins())
   {
      list.push_back(QString(pluginIt.second->GetName()).remove(QRegularExpression("\\s")));
   }
   dialog.SetBannedNames(list);
}

void WkDialogBuilder::PrefWidget::AddAction()
{
   MainDialog dialog(this);
   SetBannedNames(dialog);
   DialogDef definition;
   if (dialog.Execute(definition))
   {
      mDialogMap.insert(definition.name, definition);
      PopulateWidgets();

      // Set the new item as the selected item
      auto items = mUi.listWidget->findItems(definition.name, Qt::MatchExactly);
      if (!items.empty())
      {
         mUi.listWidget->setItemSelected(*items.begin(), true);
         mUi.listWidget->setCurrentItem(*items.begin());
      }
   }
}

void WkDialogBuilder::PrefWidget::DeleteAction()
{
   auto item = mUi.listWidget->currentItem();
   if (item)
   {
      mDialogMap.remove(item->text());
      PopulateWidgets();
   }
}

void WkDialogBuilder::PrefWidget::EditAction()
{
   auto item = mUi.listWidget->currentItem();
   if (item && mDialogMap.count(item->text()) > 0)
   {
      MainDialog dialog(this);
      SetBannedNames(dialog, item->text());
      DialogDef definition = mDialogMap.value(item->text());
      if (dialog.Execute(definition))
      {
         mDialogMap.remove(item->text());
         mDialogMap.insert(definition.name, definition);
         PopulateWidgets();

         // Set the modified item as the selected item
         auto items = mUi.listWidget->findItems(definition.name, Qt::MatchExactly);
         if (!items.empty())
         {
            mUi.listWidget->setItemSelected(*items.begin(), true);
            mUi.listWidget->setCurrentItem(*items.begin());
         }
      }
   }
}

void WkDialogBuilder::PrefWidget::PopulateWidgets()
{
   // clear and populate the list widget
   mUi.listWidget->clear();
   for (auto d : mDialogMap.toStdMap())
   {
      mUi.listWidget->addItem(d.first);
   }

   // If not empty, set the first item to the selected item.
   if (mUi.listWidget->item(0) != nullptr)
   {
      mUi.listWidget->setItemSelected(mUi.listWidget->item(0), true);
      mUi.listWidget->setCurrentItem(mUi.listWidget->item(0));
   }
}

void WkDialogBuilder::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mDialogMap.clear();
   mDialogMap = aPrefData.dialogs;
   MainDialog dialog(this);
   SetBannedNames(dialog);
   PopulateWidgets();
}
