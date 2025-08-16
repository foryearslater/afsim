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

#include "ScoreboardDialog.hpp"

#include <QMessageBox>
#include <QTextStream>

#include "WkfEnvironment.hpp"

WkScoreboard::Dialog::Dialog(DataContainer& aScoreboardData, QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
   , mScoreboardData(aScoreboardData)

{
   mUi.setupUi(this);
   mUi.mHelperButton->setIcon(QIcon::fromTheme("help"));
   SetDefaultValues();
   Connect();
   Build();
}

void WkScoreboard::Dialog::ExportData()
{
   QString fileName = wkf::getSaveFileName(this,
                                           "Save Selected Data",
                                           "",
                                           "CSV (Comma delimited) (*.csv);;Text Documents (*.txt);;All Files(*)");
   if (!fileName.isEmpty())
   {
      QFile file(fileName);
      if (!file.open(QIODevice::WriteOnly))
      {
         QMessageBox::information(this, "Unable to open file", file.errorString());
      }
      else
      {
         QTextStream out(&file);
         // branch statement because I don't empty the map when the user makes a detailed selection (only change the table)
         if (mPreviousComboString != ALL)
         {
            auto it = mData.find(mPreviousComboString.toUtf8().constData());
            out << QString::fromStdString(it->first) << "," << QString::number(it->second.totalFired) << ","
                << QString::number(it->second.totalHit) << "," << QString::number(it->second.totalKill) << ","
                << QString::number(it->second.totalMiss) << "\n";
         }
         else
         {
            for (auto& it : mData)
            {
               out << QString::fromStdString(it.first) << "," << QString::number(it.second.totalFired) << ","
                   << QString::number(it.second.totalHit) << "," << QString::number(it.second.totalKill) << ","
                   << QString::number(it.second.totalMiss) << "\n";
            }
         }
      }
   }
}

void WkScoreboard::Dialog::ShowContextMenu(const QPoint& pos)
{
   QMenu   contextMenu;
   QAction exportData("Export Currently Displayed Data");
   connect(&exportData, &QAction::triggered, this, &Dialog::ExportData);
   contextMenu.addAction(&exportData);
   contextMenu.exec(QCursor::pos());
}

// Create an instance of the entire scoreboard (modify the default values)
void WkScoreboard::Dialog::Build()
{
   mUi.mDataDisplayTable->clearContents();

   // first case shows a single specified row, 2nd case shows all rows in the filter
   if (mPreviousComboString != ALL)
   {
      std::map<std::string, FireStats>::iterator it =
         mData.find(mUi.mSelectionComboBox->currentText().toUtf8().constData());
      mUi.mDataDisplayTable->setRowCount(1);
      BuildHighlightedSelection(0, it);
   }
   else
   {
      mUi.mDataDisplayTable->setRowCount(0);
      for (auto it = mData.begin(); it != mData.end(); ++it)
      {
         ////add a row to the table for each key in the map
         int row = mUi.mDataDisplayTable->rowCount();
         mUi.mDataDisplayTable->setRowCount(row + 1);
         BuildHighlightedSelection(row, it);
      }
   }
   // format the table: stretch name column since it has longest data (user can make window bigger if names are cut off)
   mUi.mDataDisplayTable->resizeColumnsToContents();
   mUi.mDataDisplayTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
   // Create the detailed combobox
   // mUserChange distinguishes whether the combobox was changed by the user or programmatically
   mUserChange = false;
   mUi.mSelectionComboBox->clear();
   mUi.mSelectionComboBox->addItem(ALL);
   for (const auto& mComboBoxOption : mComboBoxOptions)
   {
      mUi.mSelectionComboBox->addItem(QString::fromStdString(mComboBoxOption));
   }
   // after rebuilding the combobox, highlight the user's previous selection (or All if there is none)
   mUi.mSelectionComboBox->setCurrentIndex(mUi.mSelectionComboBox->findText(mPreviousComboString));
   mUserChange = true;
}

void WkScoreboard::Dialog::BuildHighlightedSelection(int aRow, std::map<std::string, FireStats>::iterator aIt)
{
   // add associated key data to table for each new row added
   QTableWidgetItem* itemPlatform = new QTableWidgetItem(QString::fromStdString(aIt->first));
   QTableWidgetItem* itemNumFired = new QTableWidgetItem(QString::number(aIt->second.totalFired));
   QTableWidgetItem* itemNumHit   = new QTableWidgetItem(QString::number(aIt->second.totalHit));
   QTableWidgetItem* itemNumKill  = new QTableWidgetItem(QString::number(aIt->second.totalKill));
   QTableWidgetItem* itemNumMiss  = new QTableWidgetItem(QString::number(aIt->second.totalMiss));
   QTableWidgetItem* itemInFlight =
      new QTableWidgetItem(QString::number(aIt->second.totalFired - (aIt->second.totalHit + aIt->second.totalMiss)));

   itemPlatform->setFlags(itemPlatform->flags() & (~Qt::ItemIsEditable));
   itemNumFired->setFlags(itemNumFired->flags() & (~Qt::ItemIsEditable));
   itemNumHit->setFlags(itemNumHit->flags() & (~Qt::ItemIsEditable));
   itemNumKill->setFlags(itemNumKill->flags() & (~Qt::ItemIsEditable));
   itemNumMiss->setFlags(itemNumMiss->flags() & (~Qt::ItemIsEditable));
   itemInFlight->setFlags(itemInFlight->flags() & (~Qt::ItemIsEditable));

   mUi.mDataDisplayTable->setItem(aRow, 0, itemPlatform);
   mUi.mDataDisplayTable->setItem(aRow, 1, itemNumFired);
   mUi.mDataDisplayTable->setItem(aRow, 2, itemNumHit);
   mUi.mDataDisplayTable->setItem(aRow, 3, itemNumKill);
   mUi.mDataDisplayTable->setItem(aRow, 4, itemNumMiss);
   mUi.mDataDisplayTable->setItem(aRow, 5, itemInFlight);
}

void WkScoreboard::Dialog::Connect()
{
   // Set context menu policy on Dialog and on TableWidget
   setContextMenuPolicy(Qt::CustomContextMenu);
   mUi.mDataDisplayTable->setContextMenuPolicy(Qt::CustomContextMenu);

   connect(this, &QDialog::customContextMenuRequested, this, &Dialog::ShowContextMenu);
   connect(mUi.mDataDisplayTable, &QDialog::customContextMenuRequested, this, &Dialog::ShowContextMenu);

   connect(mUi.mFilterComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &Dialog::FiringFilterComboBoxChanged);
   connect(mUi.mSelectionComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &Dialog::FiringFilterDetailedComboBoxChanged);
   connect(mUi.mHelperButton, &QToolButton::clicked, this, &Dialog::HelperButtonClicked);
}

void WkScoreboard::Dialog::Update()
{
   // add new data to the map
   if (mPreviousVectorSize < mScoreboardData.GetScoreboardData().size())
   {
      // filter never changes unless we change combobox
      while (mPreviousVectorSize < mScoreboardData.GetScoreboardData().size())
      {
         switch (mCurrentFiringFilter)
         {
         case FilterOptions::Team:
            AddToMap(mScoreboardData.GetScoreboardData()[mPreviousVectorSize].firingPlatformTeam,
                     mScoreboardData.GetScoreboardData()[mPreviousVectorSize].eventType);
            break;
         case FilterOptions::PlatformType:
            AddToMap(mScoreboardData.GetScoreboardData()[mPreviousVectorSize].firingPlatformType,
                     mScoreboardData.GetScoreboardData()[mPreviousVectorSize].eventType);
            break;
         case FilterOptions::PlatformName:
            AddToMap(mScoreboardData.GetScoreboardData()[mPreviousVectorSize].firingPlatformName,
                     mScoreboardData.GetScoreboardData()[mPreviousVectorSize].eventType);
            break;
         case FilterOptions::WeaponType:
            AddToMap(mScoreboardData.GetScoreboardData()[mPreviousVectorSize].weaponType,
                     mScoreboardData.GetScoreboardData()[mPreviousVectorSize].eventType);
            break;
         }
         mPreviousVectorSize++;
      }
      Build();
   }
   // will only happen if a reset occurs and mScoreboardData vector is cleared...reset the defaults
   else if (mPreviousVectorSize > mScoreboardData.GetScoreboardData().size())
   {
      SetDefaultValues();
      Build();
   }
}

void WkScoreboard::Dialog::AddToMap(std::string aKey, WeaponEventType aWeaponEventType)
{
   auto it = mData.find(aKey);
   if (it == mData.end())
   {
      FireStats tmp = {0, 0, 0, 0};
      mData.emplace(aKey, tmp);
      it = mData.find(aKey);
   }
   switch (aWeaponEventType)
   {
   case WeaponEventType::Fire:
      it->second.totalFired++;
      break;
   case WeaponEventType::Hit:
      it->second.totalHit++;
      break;
   case WeaponEventType::Kill:
      it->second.totalKill++;
      break;
   case WeaponEventType::Miss:
      it->second.totalMiss++;
      break;
   }
   mComboBoxOptions.insert(aKey);
}


void WkScoreboard::Dialog::FiringFilterComboBoxChanged()
{
   mData.clear();
   mComboBoxOptions.clear();
   mPreviousComboString = ALL; // change the filter, go back to default value of all

   switch (static_cast<FilterOptions>(mUi.mFilterComboBox->currentData().toInt()))
   {
   case FilterOptions::Team:
      mCurrentFiringFilter = FilterOptions::Team;
      for (auto& i : mScoreboardData.GetScoreboardData())
      {
         AddToMap(i.firingPlatformTeam, i.eventType);
      }
      break;
   case FilterOptions::PlatformType:
      mCurrentFiringFilter = FilterOptions::PlatformType;
      for (auto& i : mScoreboardData.GetScoreboardData())
      {
         AddToMap(i.firingPlatformType, i.eventType);
      }
      break;
   case FilterOptions::WeaponType:
      mCurrentFiringFilter = FilterOptions::WeaponType;
      for (auto& i : mScoreboardData.GetScoreboardData())
      {
         AddToMap(i.weaponType, i.eventType);
      }
      break;
   case FilterOptions::PlatformName:
      mCurrentFiringFilter = FilterOptions::PlatformName;
      for (auto& i : mScoreboardData.GetScoreboardData())
      {
         AddToMap(i.firingPlatformName, i.eventType);
      }
      break;
   }
   Build();
}

void WkScoreboard::Dialog::FiringFilterDetailedComboBoxChanged()
{
   if (mUserChange == true)
   {
      mPreviousComboString = mUi.mSelectionComboBox->currentText();
      Build();
   }
}

void WkScoreboard::Dialog::HelperButtonClicked()
{
   QMessageBox::about(
      nullptr,
      "Scoreboard Explanation",
      QString("Fired, Hit, Miss, Kill, and In Flight are determinations made by Warlock. AFSIM reports GeometryResult "
              "codes within a WsfWeaponEngagement when a Weapon is Terminated. The charts presented on this page "
              "summarize the data provided within the WsfWeaponEngagement event. The data may become invalid when "
              "using networked simulations.\n\n") +
         "Fired is the number of WeaponFired events received.\n\n"
         "A Hit is scored when the GeometryResult code is cTARGET_IMPACT, cTARGET_PROXIMITY_ABOVE_GROUND, "
         "cTARGET_PROXIMITY_AIR_BURST, or cTARGET_PROXIMITY_GROUND_IMPACT.\n\n" +
         "A Miss is scored when the GeometryResult code is cDUD, cFAR_AWAY_ABOVE_GROUND, cFAR_AWAY_GROUND_IMPACT, "
         "cFAR_AWAY_IN_AIR, or cPART_DISABLED_OR_DESTROYED.\n\n" +
         "A Kill is scored when a hit is scored and the damage factor on the target platform is equal to 1.\n\n"
         "The In Flight count is the number fires minus the number of hit and misses.  This means a missile is counted "
         "as In Flight once a WeaponFired event is received and will stay in flight until a WeaponTerminated event is "
         "received. If no WeaponTerminated event is received (such as the missile being intercepted by another "
         "missile), no Hit or Miss will be scored for the weapon and it will count as In Flight for the rest of the "
         "simulation.");
}

void WkScoreboard::Dialog::SetDefaultValues()
{
   mCurrentFiringFilter = FilterOptions::Team;
   mPreviousVectorSize  = 0;
   mUserChange          = true;
   mPreviousComboString = ALL;
   mData.clear();
   mComboBoxOptions.clear();
   mUi.mFilterComboBox->clear();

   // Add options to filter combo box
   for (auto i : mFilterQStringMap)
   {
      mUi.mFilterComboBox->addItem(i.second, static_cast<int>(i.first));
   }
   // by default highlight team filter
   mUi.mFilterComboBox->setCurrentIndex(mUi.mFilterComboBox->findData(static_cast<int>(FilterOptions::Team)));
   // in case "Team" is not found
   mCurrentFiringFilter = static_cast<FilterOptions>(mUi.mFilterComboBox->currentData().toInt());
   // Add a default All button to detailed combo box
   mUi.mSelectionComboBox->addItem(mPreviousComboString);
}
