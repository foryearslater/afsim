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

#include "ToolPrefWidget.hpp"

#include <QFileDialog>
#include <QMenu>

#include "UtPath.hpp"
#include "UtProcess.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::ToolPrefWidget::ToolPrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<ToolPrefObject>(parent)
{
   mUi.setupUi(this);

   mUi.mAdd->setIcon(QIcon::fromTheme("add"));
   mUi.mRemove->setIcon(QIcon::fromTheme("remove"));
   mUi.alias->setDisabled(true);
   mUi.path->setDisabled(true);
   mUi.cmdLine->setDisabled(true);
   mUi.working->setDisabled(true);
   mUi.types->setDisabled(true);

   connect(mUi.mAdd, &QPushButton::clicked, this, &ToolPrefWidget::Add);
   connect(mUi.mRemove, &QPushButton::clicked, this, &ToolPrefWidget::Remove);
   connect(mUi.toolList, &QListWidget::itemClicked, this, &ToolPrefWidget::ToolClicked);
   connect(mUi.working, &QLineEdit::textEdited, this, &ToolPrefWidget::DataChanged);
   connect(mUi.cmdLine, &QLineEdit::textEdited, this, &ToolPrefWidget::DataChanged);
   connect(mUi.types, &QLineEdit::textEdited, this, &ToolPrefWidget::DataChanged);
   connect(mUi.cmdLineVars, &QPushButton::clicked, this, &ToolPrefWidget::CommandLineVariableMenuRequested);
   connect(mUi.workingVars, &QPushButton::clicked, this, &ToolPrefWidget::WorkingPathVariableMenuRequested);
}

void wizard::ToolPrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void wizard::ToolPrefWidget::ReadPreferenceData(const ToolPrefData& aPrefData)
{
   mUi.toolList->clear();

   for (auto&& it : aPrefData.mToolList)
   {
      QListWidgetItem* item = new QListWidgetItem(it.first, mUi.toolList);
      item->setData(Qt::UserRole, it.second.mAlias);
      item->setData(Qt::UserRole + 1, it.second.mPath);
      item->setData(Qt::UserRole + 2, it.second.mWorking);
      item->setData(Qt::UserRole + 3, it.second.mCmdLine);
      item->setData(Qt::UserRole + 4, it.second.mExtensions);
   }
}

void wizard::ToolPrefWidget::WritePreferenceData(ToolPrefData& aPrefData)
{
   aPrefData.mToolList.clear();

   int c = mUi.toolList->count();
   for (int i = 0; i < c; ++i)
   {
      QListWidgetItem* item             = mUi.toolList->item(i);
      aPrefData.mToolList[item->text()] = ToolDef(item->data(Qt::UserRole).toString(),
                                                  item->data(Qt::UserRole + 1).toString(),
                                                  item->data(Qt::UserRole + 3).toString(),
                                                  item->data(Qt::UserRole + 2).toString(),
                                                  item->data(Qt::UserRole + 4).toStringList());
   }
}

void wizard::ToolPrefWidget::Add()
{
#ifdef _WIN32
   QString file = QFileDialog::getOpenFileName(this, "Choose tool executable", QString(), "Executable (*.exe);;");
#else
   QString   file = QFileDialog::getOpenFileName(this, "Choose tool executable", QString(), "Executable (*);;");
#endif

   if (!file.isEmpty())
   {
      QFileInfo fi(file);

      QString alias   = fi.completeBaseName();
      QString path    = fi.filePath();
      QString working = fi.path();
      QString cmdLine = "";
      QString extensions;

      alias = FindAvailableAlias(alias);

      if (alias == "vespa")
      {
         if (QMessageBox::Yes == QMessageBox::question(wkfEnv.GetMainWindow(),
                                                       "Load CME Tools?",
                                                       "Would you like to load other CME tools?",
                                                       QMessageBox::Yes,
                                                       QMessageBox::No))
         {
            PopulateCME_Tools(fi.path());
            return;
         }
      }
      QListWidgetItem* item = new QListWidgetItem(mUi.toolList);
      item->setText(alias);
      item->setData(Qt::UserRole, alias);
      item->setData(Qt::UserRole + 1, path);
      item->setData(Qt::UserRole + 2, working);
      item->setData(Qt::UserRole + 3, cmdLine);
      item->setData(Qt::UserRole + 4, extensions);
      mUi.toolList->addItem(item);
   }
}

void wizard::ToolPrefWidget::Remove()
{
   QListWidgetItem* item = mUi.toolList->currentItem();
   if (item)
   {
      delete item;
      mUi.alias->setText("");
      mUi.alias->setDisabled(true);
      mUi.path->setText("");
      mUi.path->setDisabled(true);
      mUi.cmdLine->setText("");
      mUi.cmdLine->setDisabled(true);
      mUi.working->setText("");
      mUi.working->setDisabled(true);
      mUi.types->setText("");
      mUi.types->setDisabled(true);
   }
}

void wizard::ToolPrefWidget::ToolClicked(QListWidgetItem* aItem)
{
   mUi.alias->setDisabled(false);
   mUi.path->setDisabled(false);
   mUi.cmdLine->setDisabled(false);
   mUi.working->setDisabled(false);
   mUi.types->setDisabled(false);
   mUi.alias->setText(aItem->data(Qt::UserRole).toString());
   mUi.path->setText(aItem->data(Qt::UserRole + 1).toString());
   mUi.working->setText(aItem->data(Qt::UserRole + 2).toString());
   mUi.cmdLine->setText(aItem->data(Qt::UserRole + 3).toString());
   mUi.types->setText(aItem->data(Qt::UserRole + 4).toStringList().join(','));
}

QString wizard::ToolPrefWidget::FindAvailableAlias(const QString& input)
{
   int check = 1;
   if (!mUi.toolList->findItems(input, Qt::MatchExactly).empty())
   {
      QString test = input + "_" + QString::number(check);
      while (!mUi.toolList->findItems(test, Qt::MatchExactly).empty())
      {
         check++;
         test = input + "_" + QString::number(check);
      }
      return test;
   }
   return input;
}

void wizard::ToolPrefWidget::DataChanged()
{
   QString                 alias = mUi.alias->text();
   QList<QListWidgetItem*> il    = mUi.toolList->findItems(alias, Qt::MatchExactly);
   if (!il.empty())
   {
      il.front()->setData(Qt::UserRole + 2, mUi.working->text());
      il.front()->setData(Qt::UserRole + 3, mUi.cmdLine->text());
      il.front()->setData(Qt::UserRole + 4, mUi.types->text().split(','));
   }
}

void wizard::ToolPrefWidget::PopulateCME_Tools(const QString& aPath)
{
   std::string p = aPath.toStdString();
#ifndef _WIN32
   QFileInfo vespafi(aPath + "/vespa");
   QFileInfo timeviewfi(aPath + "/timeview");
   QFileInfo plotviewfi(aPath + "/plotview");
   QFileInfo gritfi(aPath + "/grit");
   QFileInfo sigviewfi(aPath + "/sigview");
#else
   QFileInfo vespafi(aPath + "/vespa.exe");
   QFileInfo timeviewfi(aPath + "/timeview.exe");
   QFileInfo plotviewfi(aPath + "/plotview.exe");
   QFileInfo gritfi(aPath + "/grit.exe");
   QFileInfo sigviewfi(aPath + "/sigview.exe");
#endif
   if (vespafi.exists())
   {
      QListWidgetItem* item  = new QListWidgetItem(mUi.toolList);
      QString          alias = FindAvailableAlias("Vespa (Playback)");
      item->setText(alias);
      item->setData(Qt::UserRole, alias);
      item->setData(Qt::UserRole + 1, vespafi.filePath());
      item->setData(Qt::UserRole + 2, aPath);
      item->setData(Qt::UserRole + 3, "-ct playback:$(ITEM_PATH)");
      QStringList types;
      types << ".rep";
      item->setData(Qt::UserRole + 4, types);
      mUi.toolList->addItem(item);

      QListWidgetItem* item2  = new QListWidgetItem(mUi.toolList);
      QString          alias2 = FindAvailableAlias("Vespa (Laydown)");
      item2->setText(alias2);
      item2->setData(Qt::UserRole, alias2);
      item2->setData(Qt::UserRole + 1, vespafi.filePath());
      item2->setData(Qt::UserRole + 2, aPath);
      item2->setData(Qt::UserRole + 3, "-wsf $(ITEM_PATH)");
      QStringList types2;
      types2 << ".txt";
      item2->setData(Qt::UserRole + 4, types2);
      mUi.toolList->addItem(item2);
   }
   if (timeviewfi.exists())
   {
      QListWidgetItem* item  = new QListWidgetItem(mUi.toolList);
      QString          alias = FindAvailableAlias("Timeview (Playback)");
      item->setText(alias);
      item->setData(Qt::UserRole, alias);
      item->setData(Qt::UserRole + 1, timeviewfi.filePath());
      item->setData(Qt::UserRole + 2, aPath);
      item->setData(Qt::UserRole + 3, "-connect playback:$(ITEM_PATH)");
      QStringList types;
      types << ".rep";
      item->setData(Qt::UserRole + 4, types);
      mUi.toolList->addItem(item);

      QListWidgetItem* item2  = new QListWidgetItem(mUi.toolList);
      QString          alias2 = FindAvailableAlias("Timeview (Events)");
      item2->setText(alias2);
      item2->setData(Qt::UserRole, alias2);
      item2->setData(Qt::UserRole + 1, timeviewfi.filePath());
      item2->setData(Qt::UserRole + 2, aPath);
      item2->setData(Qt::UserRole + 3, "$(ITEM_PATH)");
      QStringList types2;
      types2 << ".evt";
      item2->setData(Qt::UserRole + 4, types2);
      mUi.toolList->addItem(item2);
   }
   if (plotviewfi.exists())
   {
      QListWidgetItem* item  = new QListWidgetItem(mUi.toolList);
      QString          alias = FindAvailableAlias("Plotview (PdMap)");
      item->setText(alias);
      item->setData(Qt::UserRole, alias);
      item->setData(Qt::UserRole + 1, plotviewfi.filePath());
      item->setData(Qt::UserRole + 2, aPath);
      item->setData(Qt::UserRole + 3, "-plot1 $(ITEM_PATH)");
      QStringList types;
      types << ".pd";
      item->setData(Qt::UserRole + 4, types);
      mUi.toolList->addItem(item);

      QListWidgetItem* item2  = new QListWidgetItem(mUi.toolList);
      QString          alias2 = FindAvailableAlias("Plotview (Engagement)");
      item2->setText(alias2);
      item2->setData(Qt::UserRole, alias2);
      item2->setData(Qt::UserRole + 1, plotviewfi.filePath());
      item2->setData(Qt::UserRole + 2, aPath);
      item2->setData(Qt::UserRole + 3, "-engage1 $(ITEM_PATH)");
      QStringList types2;
      types2 << ".loc";
      item2->setData(Qt::UserRole + 4, types2);
      mUi.toolList->addItem(item2);
   }
   if (gritfi.exists())
   {
      QListWidgetItem* item  = new QListWidgetItem(mUi.toolList);
      QString          alias = FindAvailableAlias("GRIT Behavior Tree Event View");
      item->setText(alias);
      item->setData(Qt::UserRole, alias);
      item->setData(Qt::UserRole + 1, gritfi.filePath());
      item->setData(Qt::UserRole + 2, aPath);
      item->setData(Qt::UserRole + 3, "--evt $(ITEM_PATH)");
      QStringList types;
      types << ".evt";
      item->setData(Qt::UserRole + 4, types);
      mUi.toolList->addItem(item);
   }
   if (sigviewfi.exists())
   {
      /*      QListWidgetItem* item = new QListWidgetItem(mUi.toolList);
            QString alias = FindAvailableAlias("Sigview");
            item->setText(alias);
            item->setData(Qt::UserRole, alias);
            item->setData(Qt::UserRole + 1, sigviewfi.filePath());
            item->setData(Qt::UserRole + 2, aPath);
            item->setData(Qt::UserRole + 3, "--evt $(ITEM_PATH)");
            QStringList types;
            types << ".evt";
            item->setData(Qt::UserRole + 4, types);
            mUi.toolList->addItem(item);*/ // this is not right
   }
}

void wizard::ToolPrefWidget::CommandLineVariableMenuRequested()
{
   BuildVariableMenu(mUi.cmdLine);
}

void wizard::ToolPrefWidget::WorkingPathVariableMenuRequested()
{
   BuildVariableMenu(mUi.working);
}

void wizard::ToolPrefWidget::BuildVariableMenu(QLineEdit* aLineEdit)
{
   QMenu menu(this);
   connect(menu.addAction(QString("$(ITEM_PATH) - c:\\dir\\item.txt")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(ITEM_PATH)"); });
   connect(menu.addAction(QString("$(ITEM_FILE) - item.txt")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(ITEM_FILE)"); });
   connect(menu.addAction(QString("$(ITEM_DIRECTORY) - c:\\dir\\")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(ITEM_DIRECTORY)"); });
   connect(menu.addAction(QString("$(CURRENT_LINE) - 123")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(CURRENT_LINE)"); });
   connect(menu.addAction(QString("$(CURRENT_COLUMN) - 21")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(CURRENT_COLUMN)"); });
   connect(menu.addAction(QString("$(SELECTED_TEXT)")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(SELECTED_TEXT)"); });
   connect(menu.addAction(QString("$(SCENARIO_FILES) - scenario.txt")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(SCENARIO_FILES)"); });
   connect(menu.addAction(QString("$(WSF_BIN_PATH) - c:\\wsf\\bin\\mission.exe")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(WSF_BIN_PATH)"); });
   connect(menu.addAction(QString("$(SCENARIO_DIRECTORY) - c:\\dir\\")),
           &QAction::triggered,
           [aLineEdit]() { aLineEdit->insert("$(SCENARIO_DIRECTORY)"); });

   menu.exec(QCursor::pos());
}
