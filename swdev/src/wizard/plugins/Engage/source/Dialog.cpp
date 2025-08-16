// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Dialog.hpp"

#include <algorithm>

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QResource>
#include <QTextStream>

#include "OutputItem.hpp"
#include "OutputRateDialog.hpp"
#include "OutputRateItem.hpp"
#include "RunOutputDialog.hpp"
#include "TableRow.hpp"
#include "UtPath.hpp"

// constructors
Engage::Dialog::Dialog(QWidget* parent /* = 0*/, Qt::WindowFlags flags /* = 0*/)
   : QDialog(parent, flags)
{
   // Setup the Main Window
   mUi.setupUi(this);

   connect(mUi.mCreateFileButton, &QPushButton::clicked, this, &Dialog::CreateFileCB);
   connect(mUi.mRunOutputAddButton, &QPushButton::clicked, this, &Dialog::AddRunOutputCB);
   connect(mUi.mAddOutputRateButton, &QPushButton::clicked, this, &Dialog::AddOutputRateCB);
   connect(mUi.mRemoveOutputRateButton, &QPushButton::clicked, this, &Dialog::RemoveOutputRateCB);
   connect(mUi.mEventOutputAddButton, &QPushButton::clicked, this, &Dialog::AddEventCB);
   connect(mUi.mEventOutputRemoveButton, &QPushButton::clicked, this, &Dialog::RemoveEventCB);
   connect(mUi.mTargetBehaviorComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &Dialog::TargetBehaviorChangedCB);
   connect(mUi.mSiteBehaviorComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &Dialog::SiteBehaviorChangedCB);
   connect(mUi.mAddFlightPathButton, &QPushButton::clicked, this, &Dialog::AddTargetFlightPathCB);
   connect(mUi.mRemoveFlightPathButton, &QPushButton::clicked, this, &Dialog::RemoveTargetFlightPathCB);
   connect(mUi.mAddFlightRouteButton, &QPushButton::clicked, this, &Dialog::AddTargetFlightRouteCB);
   connect(mUi.mRemoveFlightRouteButton, &QPushButton::clicked, this, &Dialog::RemoveTargetFlightRouteCB);
   connect(mUi.mSiteAddButton, &QPushButton::clicked, this, &Dialog::AddSiteCB);
   connect(mUi.mSiteRemoveButton, &QPushButton::clicked, this, &Dialog::RemoveSiteCB);
   connect(mUi.mRootPathButton, &QPushButton::clicked, this, &Dialog::BrowseRootCB);
   connect(mUi.mRootPathLineEdit, &QLineEdit::textChanged, this, &Dialog::RootPathEditedCB);
   connect(mUi.mPkOutputPathButton, &QPushButton::clicked, this, &Dialog::SelectPkDirectoryCB);

   UtPath workingDir(".");
   workingDir.Up();

   QString resourcePath = QString("./wizard_plugins/wizard_plugins/ViPluginEngage/rcc/engage.rcc");

   if (!QResource::registerResource(resourcePath))
   {
      // couldn't find resource file
   }

   // In the old code Wizard-Engage would save off the paths
   // launcherPath, targetPath, trackerPath, weaponPath, rootPath
   // but only restore the rootPath.  This seems useless so we
   // skipped it here.
   /*   mCallbacks += ViSettings::ReadingSettings.Connect(&ViEngageMainWindow::LoadSettings, this);
      mCallbacks += ViSettings::WritingSettings.Connect(&ViEngageMainWindow::SaveSettings, this);*/

   // initially disable Pk table generation tab based on initial ui view
   QWidget* pkTableWidget = mUi.mScenarioSettingsTab->widget(2);
   pkTableWidget->setDisabled(true);

   /*   LoadSettings(ViSettings::GetInstance().GetSettings());*/
}

void Engage::Dialog::PopulateLists(QString aDirPath, std::vector<const QString*>* aCurList)
{
   QDir curDir(aDirPath);
   if (curDir.exists())
   {
      QFileInfoList dirFileList = curDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
      for (auto&& fileIter : dirFileList)
      {
         const QFileInfo& curInfo = fileIter;
         if (curInfo.isDir())
         {
            PopulateLists(curInfo.absoluteFilePath(), aCurList);
         }
         else if (curInfo.isFile())
         {
            QFile curFile(curInfo.absoluteFilePath());
            if (!curFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
               return;
            }

            while (!curFile.atEnd())
            {
               QString curLine(curFile.readLine());
               curLine             = curLine.trimmed();
               bool somethingAdded = false;
               if (curLine.startsWith("platform_type"))
               {
                  QStringList                           lineList = curLine.split(" ", QString::SkipEmptyParts);
                  QString                               curType  = lineList[1];
                  std::vector<const QString*>::iterator foundType = std::find(aCurList->begin(), aCurList->end(), curType);
                  if (foundType == aCurList->end())
                  {
                     QString* curStr = new QString(curType);
                     aCurList->push_back(curStr);
                     somethingAdded = true;
                  }
               }
               // Add any found radar signatures...
               else if (curLine.startsWith("radar_signature"))
               {
                  QStringList                           lineList = curLine.split(" ", QString::SkipEmptyParts);
                  QString                               curSig   = lineList[1];
                  std::vector<const QString*>::iterator foundSig =
                     std::find(mRadarSignatureList.begin(), mRadarSignatureList.end(), curSig);
                  if (foundSig == mRadarSignatureList.end())
                  {
                     QString* curStr = new QString(curSig);
                     mRadarSignatureList.push_back(curStr);
                     somethingAdded = true;
                  }
               }
               // Add any Icons
               else if (curLine.startsWith("icon"))
               {
                  QStringList                           lineList = curLine.split(" ", QString::SkipEmptyParts);
                  QString                               curIcon  = lineList[1];
                  std::vector<const QString*>::iterator foundIcon = std::find(mIconList.begin(), mIconList.end(), curIcon);
                  if (foundIcon == mIconList.end())
                  {
                     QString* curStr = new QString(curIcon);
                     mIconList.push_back(curStr);
                     somethingAdded = true;
                  }
               }
               // Add any sides
               else if (curLine.startsWith("side"))
               {
                  QStringList                           lineList = curLine.split(" ", QString::SkipEmptyParts);
                  QString                               curSide  = lineList[1];
                  std::vector<const QString*>::iterator foundSide = std::find(mSideList.begin(), mSideList.end(), curSide);
                  if (foundSide == mSideList.end())
                  {
                     QString* curStr = new QString(curSide);
                     mSideList.push_back(curStr);
                     somethingAdded = true;
                  }
               }
               if (somethingAdded)
               {
                  std::vector<const QString*>::iterator foundInclude =
                     std::find(mIncludeFiles.begin(), mIncludeFiles.end(), curFile.fileName());
                  if (foundInclude == mIncludeFiles.end())
                  {
                     QString* curStr = new QString(curFile.fileName());
                     mIncludeFiles.push_back(curStr);
                  }
               }
            }
         }
      }
   }
}

void Engage::Dialog::CreateFileCB()
{
   QString fileName = QFileDialog::getSaveFileName(this, "Save File As", ".", "*.txt");
   QFile   writeFile(fileName);
   if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      return;
   }
   QTextStream out(&writeFile);

   //*************SCENARIO SETUP************************//
   if (mUi.mThreadCountLineEdit->text() != "")
   {
      out << "thread_count " << mUi.mThreadCountLineEdit->text() << "\n\n";
   }
   if (mUi.mRandomSeedLineEdit->text() != "")
   {
      out << "random_seed " << mUi.mRandomSeedLineEdit->text() << "\n\n";
   }
   if (mUi.mFrameTimeLineEdit->text() != "")
   {
      out << "frame_time " << mUi.mFrameTimeLineEdit->text() << "\n";
   }
   if (mUi.mEndTimeLineEdit->text() != "")
   {
      out << "end_time " << mUi.mEndTimeLineEdit->text() << "\n\n";
   }

   //*************INCLUDES************************//
   for (const auto& curInclude : mIncludeFiles)
   {
      out << "include " << GetRelativePath(fileName, QString(*curInclude)) << "\n";
   }
   out << "\n";

   //*************PLATFORMS************************//
   // LAUNCHERS
   std::vector<TableRow*> launcherTableRows = mUi.mLauncherTableWidget->GetRows();
   for (auto&& rowIter : launcherTableRows)
   {
      TableRow*    curLauncherItem = rowIter;
      TableRowData rowData         = curLauncherItem->GetRowData();
      out << "platform_type LAUNCHER_TYPE " << rowData.mName << "\n";
      out << "   weapon " << rowData.mWeapon.toLower() << " " << rowData.mWeapon << "\n";
      out << "      quantity " << rowData.mWeaponQuantity << "\n";
      out << "   end_weapon\n";
      out << "end_platform_type\n\n";

      // TRACKER
      out << "platform_type TRACKER_TYPE " << rowData.mTrackerType << "\n";
      out << "   side " << rowData.mSide.toLower() << "\n";
      out << "   script_variables\n";
      out << "      weapon_name = \"" << rowData.mWeapon.toLower() << "\";\n";
      out << "   end_script_variables\n";
      out << "end_platform_type\n\n";
   }
   // TARGET
   std::vector<TableRow*> targetTableRows = mUi.mTargetTableWidget->GetRows();
   for (auto&& rowIter : targetTableRows)
   {
      TableRow*    curTargetItem = rowIter;
      TableRowData rowData       = curTargetItem->GetRowData();
      out << "platform_type TARGET_TYPE " << rowData.mName << "\n";
      out << "   side " << rowData.mSide.toLower() << "\n";
      if (rowData.mIcon != "")
      {
         out << "   icon " << rowData.mIcon << "\n";
      }
      out << "   radar_signature " << rowData.mRadarSignature << "\n";
      out << "end_platform_type\n\n";
   }

   //*************OUTPUT RATE************************//
   if (mUi.mOutputRateTableWidget->rowCount() > 0)
   {
      for (auto& curItem : mOutputRateItems)
      {
         out << "output_rate " << curItem.GetFileName() << "\n";
         for (const auto& curVar : curItem.GetItems())
         {
            out << "   " << curVar.mTime << " ";
            if (curVar.mTimeUnits != "")
            {
               out << curVar.mTimeUnits << " ";
            }
            if (curVar.mPeriod != "")
            {
               out << curVar.mPeriod << " ";
               if (curVar.mPeriodUnits != "")
               {
                  out << curVar.mPeriodUnits;
               }
            }
            out << "\n";
         }

         out << "end_output_rate\n\n";
      }
   }

   //*************RUN INFORMATION************************//
   out << "run\n";

   QStringList filePathList = fileName.split("/");
   QString     baseFileName = filePathList[filePathList.size() - 1].split(".")[0];
   if (mUi.mEventFileCB->isChecked())
   {
      out << "   event_output_file_base_name " << baseFileName << "\n";
   }
   if (mUi.mRecordFileCB->isChecked())
   {
      out << "   record_file_base_name " << baseFileName << "\n";
   }

   // CENTER LOCATION
   if (mUi.mCenterLocationGroupBox->isChecked())
   {
      out << "   center_location " << mUi.mCenterLocationLatLineEdit->text() << " "
          << mUi.mCenterLocationLonLineEdit->text() << "\n\n";
   }

   //*************TARGET TAB************************//
   // TARGET GRID;
   QString targetBehaviorStr = mUi.mTargetBehaviorComboBox->currentText();
   bool    allowWrite        = mUi.mTargetBehaviorComboBox->isEnabled();

   // Only output target grid if selected, and site grid has not been selected (and thus disabled it)
   if (targetBehaviorStr == "Grid" && allowWrite)
   {
      // verify that the fields are filled out before proceeding
      if (mUi.mTargetDownRangeFrom->text() != "" && mUi.mTargetDownRangeTo->text() != "" &&
          mUi.mTargetDownRangeBy->text() != "" && mUi.mTargetCrossRangeFrom->text() != "" &&
          mUi.mTargetCrossRangeTo->text() != "" && mUi.mTargetCrossRangeBy->text() != "" &&
          mUi.mTargetAltitudeFrom->text() != "" && mUi.mTargetAltitudeTo->text() != "" &&
          mUi.mTargetAltitudeBy->text() != "" && mUi.mTargetSpeed->text() != "")
      {
         out << "   target_grid\n";
         out << "      down_range  from " << mUi.mTargetDownRangeFrom->text() << " to "
             << mUi.mTargetDownRangeTo->text() << " by " << mUi.mTargetDownRangeBy->text()
             << "\n"; // NAC+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      cross_range from " << mUi.mTargetCrossRangeFrom->text() << " to "
             << mUi.mTargetCrossRangeTo->text() << " by " << mUi.mTargetCrossRangeBy->text()
             << "\n"; // NAC+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      altitude    from " << mUi.mTargetAltitudeFrom->text() << " to " << mUi.mTargetAltitudeTo->text()
             << " by " << mUi.mTargetAltitudeBy->text()
             << "\n"; // NAC+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      speed " << mUi.mTargetSpeed->text()
             << "\n"; // NAC+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "   end_target_grid\n\n";
      }
   }
   else if (targetBehaviorStr == "Simple Path" && allowWrite)
   {
      out << "   simple_path\n";
      out << "      x " << mUi.mSimplePathXPosLineEdit->text() << "\n";
      out << "      y " << mUi.mSimplePathYPosLineEdit->text() << "\n";
      out << "      z " << mUi.mSimplePathZPosLineEdit->text() << "\n";
      out << "      speed " << mUi.mSimplePathSpeedLineEdit->text() << "\n";
      out << "      heading " << mUi.mSimplePathHeadingLineEdit->text() << "\n";
      out << "   end_simple_path\n";
   }
   else if (targetBehaviorStr == "Flight Path" && allowWrite)
   {
      if (mUi.mFlightPathTableWidget->rowCount() > 0)
      {
         out << "   flight_path\n";
         for (int i = 0; i < mUi.mFlightPathTableWidget->rowCount(); i++)
         {
            out << "      ";
            for (int j = 0; j < mUi.mFlightPathTableWidget->columnCount(); j++)
            {
               out << ((QLineEdit*)mUi.mFlightPathTableWidget->cellWidget(i, j))->text() << " ";
            }
            out << "\n";
         }
         out << "   end_flight_path\n";
      }
   }
   else if (targetBehaviorStr == "Flight Route" && allowWrite)
   {
      if (mUi.mFlightRouteTableWidget->rowCount() > 0)
      {
         out << "   flight_route\n";
         for (int i = 0; i < mUi.mFlightRouteTableWidget->rowCount(); i++)
         {
            out << "      ";
            for (int j = 0; j < mUi.mFlightRouteTableWidget->columnCount(); j++)
            {
               out << ((QLineEdit*)mUi.mFlightRouteTableWidget->cellWidget(i, j))->text() << " ";
            }
            out << "\n";
         }
         out << "   end_flight_route\n";
      }
   }
   else if (targetBehaviorStr == "Use Target Path" && allowWrite)
   {
      out << "   use_target_path\n";
   }

   //**********PK TABLE GENERATION TAB*************//
   if (mUi.mPkEnableCheckBox->isChecked() && mUi.mPkEnableCheckBox->isEnabled())
   {
      QString altSiteName   = mUi.mPkSiteNameLineEdit->text();
      QString altTargetName = mUi.mPkTargetNameLineEdit->text();
      QString pkPath        = mUi.mPkOutputPathLineEdit->text();
      QString pkLengthUnit  = mUi.mPkLengthComboBox->currentText();
      QString pkSpeedUnit   = mUi.mPkSpeedComboBox->currentText();
      QString pkFlush;
      if (mUi.mPkPeriodicFlushCheckBox->isChecked())
      {
         pkFlush = "true";
      }
      else
      {
         pkFlush = "false";
      }

      out << "   enable_Pk_table_generation true"
          << "\n";

      if (altTargetName != "")
      {
         out << "   Pk_table_target_type " << altTargetName << "\n";
      }
      if (altSiteName != "")
      {
         out << "   Pk_table_site_type " << altSiteName << "\n";
      }

      out << "   Pk_table_output_directory " << pkPath << "\n";
      out << "   enable_Pk_table_periodic_flush " << pkFlush << "\n";
      out << "   Pk_output_length_units " << pkLengthUnit << "\n";
      out << "   Pk_output_speed_units " << pkSpeedUnit << "\n";
      out << "   \n";
   }


   //*************SITE TAB************************//
   // SITE LAYDOWN
   QString siteBehaviorStr = mUi.mSiteBehaviorComboBox->currentText();

   if (siteBehaviorStr == "Custom Laydown")
   {
      if (mUi.mSiteTableWidget->rowCount() > 0)
      {
         out << "   sites\n";
         for (int i = 0; i < mUi.mSiteTableWidget->rowCount(); i++)
         {
            out << "      xyz " << ((QLineEdit*)mUi.mSiteTableWidget->cellWidget(i, 0))->text() << " ";
            out << ((QLineEdit*)mUi.mSiteTableWidget->cellWidget(i, 1))->text() << " ";
            out << ((QLineEdit*)mUi.mSiteTableWidget->cellWidget(i, 2))->text() << "\n";
            out << "      heading " << ((QLineEdit*)mUi.mSiteTableWidget->cellWidget(i, 3))->text() << "\n";
            out << "      speed " << ((QLineEdit*)mUi.mSiteTableWidget->cellWidget(i, 4))->text() << "\n";
         }
         out << "   end_sites\n";
      }
   }
   else if (siteBehaviorStr == "Grid")
   {
      // verify that the fields are filled out before proceeding
      if (mUi.mSiteDownRangeFrom->text() != "" && mUi.mSiteDownRangeTo->text() != "" && mUi.mSiteDownRangeBy->text() != "" &&
          mUi.mSiteCrossRangeFrom->text() != "" && mUi.mSiteCrossRangeTo->text() != "" &&
          mUi.mSiteCrossRangeBy->text() != "" && mUi.mSiteAltitudeFrom->text() != "" &&
          mUi.mSiteAltitudeTo->text() != "" && mUi.mSiteAltitudeBy->text() != "" && mUi.mSiteSpeed->text() != "")
      {
         out << "   site_grid\n";
         out << "      down_range  from " << mUi.mSiteDownRangeFrom->text() << " to " << mUi.mSiteDownRangeTo->text()
             << " by " << mUi.mSiteDownRangeBy->text()
             << "\n"; // KWW+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      cross_range from " << mUi.mSiteCrossRangeFrom->text() << " to " << mUi.mSiteCrossRangeTo->text()
             << " by " << mUi.mSiteCrossRangeBy->text()
             << "\n"; // KWW+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      target_altitude    from " << mUi.mSiteAltitudeFrom->text() << " to "
             << mUi.mSiteAltitudeTo->text() << " by " << mUi.mSiteAltitudeBy->text()
             << "\n"; // KWW+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "      target_speed " << mUi.mSiteSpeed->text()
             << "\n"; // KWW+ WIZARD_TODO assumes user is putting units (km, etc) into the line edit along with the number
         out << "   end_site_grid\n\n";
      }
   }

   // PERFORM FLYOUTS
   if (mUi.mPerformFlyoutsCB->isChecked())
   {
      out << "   perform_flyouts true\n\n";
   }

   // REPETITION COUNT
   if (mUi.mRepetitionCountLineEdit->text().toInt() != 1)
   {
      out << "   repetition_count " << mUi.mRepetitionCountLineEdit->text() << "\n\n";
   }

   // OUTPUT
   for (const auto& curItem : mOutputItems)
   {
      out << "   output\n";
      out << "      file " << curItem.GetFileName() << "\n";
      out << "      phase " << curItem.GetPhase().toLower() << "\n";
      std::vector<OutputVariable*> itemVars = curItem.GetItems();
      if (!itemVars.empty())
      {
         out << "      items\n";
         for (const auto& curVar : itemVars)
         {
            out << "         variable " << curVar->mName << " ";
            if (curVar->mUnits != "")
            {
               out << "units " << curVar->mUnits << " ";
            }
            if (curVar->mFormat != "")
            {
               out << "format \"" << curVar->mFormat << "\"";
            }
            out << "\n";
         }
         out << "      end_items\n";
      }
      if (curItem.GetAddEventOutput())
      {
         out << "      event_output true\n";
      }
      if (curItem.GetAddSummaryOutput())
      {
         out << "      summary_output true\n";
      }
      out << "   end_output\n";
   }

   out << "end_run\n";

   // EVENT OUTPUT
   if (mUi.mTimeFormatLineEdit->text() != "" || mUi.mLatLonFormatLineEdit->text() != "" ||
       mUi.mPrintTrackInMessageCheckBox->isChecked() || mUi.mEventOutputTableWidget->rowCount() > 0)
   {
      out << "event_output\n";
      if (mUi.mTimeFormatLineEdit->text() != "")
      {
         out << "   time_format " << mUi.mTimeFormatLineEdit->text() << "\n";
      }
      if (mUi.mLatLonFormatLineEdit->text() != "")
      {
         out << "   lat_lon_format " << mUi.mLatLonFormatLineEdit->text() << "\n";
      }
      if (mUi.mPrintTrackInMessageCheckBox->isChecked())
      {
         out << "   print_track_in_message true"
             << "\n";
      }
      out << "\n"; // begin enable/disable section
      for (int i = 0; i < mUi.mEventOutputTableWidget->rowCount(); i++)
      {
         QLineEdit* eventLineEdit  = dynamic_cast<QLineEdit*>(mUi.mEventOutputTableWidget->cellWidget(i, 0));
         QComboBox* enableComboBox = dynamic_cast<QComboBox*>(mUi.mEventOutputTableWidget->cellWidget(i, 1));
         if (eventLineEdit && enableComboBox)
         {
            QString enableStr = "enable";
            if (enableComboBox->currentText() == "Disable")
            {
               enableStr = "disable";
            }
            out << "   " << enableStr << " " << eventLineEdit->text() << "\n";
         }
      }
      out << "end_event_output\n";
   }
   writeFile.close();
}

void Engage::Dialog::AddRunOutputCB()
{
   OutputItem      newItem;
   RunOutputDialog runDialog(newItem);
   if (runDialog.exec())
   {
      mOutputItems.push_back(newItem);
      int curRow = mUi.mRunOutputTableWidget->rowCount();
      mUi.mRunOutputTableWidget->insertRow(curRow);
      QTableWidgetItem* nameItem  = new QTableWidgetItem(newItem.GetFileName());
      QTableWidgetItem* phaseItem = new QTableWidgetItem(newItem.GetPhase());
      mUi.mRunOutputTableWidget->setItem(curRow, 0, nameItem);
      mUi.mRunOutputTableWidget->setItem(curRow, 1, phaseItem);
      QPushButton* removeItemButton = new QPushButton("Remove", mUi.mRunOutputTableWidget);
      removeItemButton->setFixedWidth(50);

      QPushButton* editItemButton = new QPushButton("Edit", mUi.mRunOutputTableWidget);

      QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
      buttonBox->addButton(removeItemButton, QDialogButtonBox::ActionRole);
      buttonBox->addButton(editItemButton, QDialogButtonBox::ActionRole);
      mUi.mRunOutputTableWidget->setCellWidget(curRow, 2, buttonBox);
      connect(editItemButton, &QPushButton::clicked, this, &Dialog::EditRunOutputCB);
      connect(removeItemButton, &QPushButton::clicked, this, &Dialog::DeleteRunOutputCB);
   }
}

void Engage::Dialog::AddOutputRateCB()
{
   int curRow = mUi.mOutputRateTableWidget->rowCount();
   mUi.mOutputRateTableWidget->insertRow(curRow);

   OutputRateItem   newItem;
   OutputRateDialog runDialog(newItem);
   if (runDialog.exec())
   {
      mOutputRateItems.push_back(newItem);
      QTableWidgetItem* nameItem = new QTableWidgetItem(newItem.GetFileName());
      mUi.mOutputRateTableWidget->setItem(curRow, 0, nameItem);
      QPushButton* removeItemButton = new QPushButton("Remove", mUi.mOutputRateTableWidget);
      removeItemButton->setFixedWidth(50);

      QPushButton* editItemButton = new QPushButton("Edit", mUi.mOutputRateTableWidget);

      QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
      buttonBox->addButton(removeItemButton, QDialogButtonBox::ActionRole);
      buttonBox->addButton(editItemButton, QDialogButtonBox::ActionRole);
      mUi.mOutputRateTableWidget->setCellWidget(curRow, 1, buttonBox);
      connect(editItemButton, &QPushButton::clicked, this, &Dialog::EditOutputRateCB);
      connect(removeItemButton, &QPushButton::clicked, this, &Dialog::DeleteOutputRateCB);
   }
}

void Engage::Dialog::RemoveOutputRateCB()
{
   int curRow = mUi.mOutputRateTableWidget->currentRow();
   if (curRow != -1)
   {
      mUi.mOutputRateTableWidget->removeRow(curRow);
   }
}

void Engage::Dialog::AddEventCB()
{
   int curRow = mUi.mEventOutputTableWidget->rowCount();
   mUi.mEventOutputTableWidget->insertRow(curRow);
   QLineEdit* eventNameLineEdit = new QLineEdit(mUi.mEventOutputTableWidget);
   mUi.mEventOutputTableWidget->setCellWidget(curRow, 0, eventNameLineEdit);

   QComboBox*  enableComboBox = new QComboBox(mUi.mEventOutputTableWidget);
   QStringList enableList;
   enableList.push_back("Enable");
   enableList.push_back("Disable");
   enableComboBox->addItems(enableList);
   mUi.mEventOutputTableWidget->setCellWidget(curRow, 1, enableComboBox);
   eventNameLineEdit->setFocus();
}

void Engage::Dialog::RemoveEventCB()
{
   int curRow = mUi.mEventOutputTableWidget->currentRow();
   if (curRow != -1)
   {
      mUi.mEventOutputTableWidget->removeRow(curRow);
   }
}

void Engage::Dialog::TargetBehaviorChangedCB(int aIndex)
{
   mUi.mTargetBehaviorWidget->setCurrentIndex(aIndex);
}

void Engage::Dialog::SiteBehaviorChangedCB(int aIndex)
{
   mUi.mSiteBehaviorWidget->setCurrentIndex(aIndex);

   // Disable or enable the target grid based on user selection
   if (aIndex == 0) // User defined site input
   {
      // enable all other forms of target input
      for (int i = 0; i < mUi.mTargetBehaviorWidget->count(); ++i)
      {
         QWidget* targetGridWidget = mUi.mTargetBehaviorWidget->widget(i);
         targetGridWidget->setDisabled(false);
      }

      // Disable Pk Table Generation
      QWidget* pkTableWidget = mUi.mScenarioSettingsTab->widget(2); // Pk table tab
      pkTableWidget->setDisabled(true);
   }
   else if (aIndex == 1) // Site grid
   {
      // disable all other forms of target input
      for (int i = 0; i < mUi.mTargetBehaviorWidget->count(); ++i)
      {
         QWidget* targetGridWidget = mUi.mTargetBehaviorWidget->widget(i);
         targetGridWidget->setDisabled(true);
      }

      // Enable Pk Table Generation
      QWidget* pkTableWidget = mUi.mScenarioSettingsTab->widget(2); // Pk table tab
      pkTableWidget->setDisabled(false);
   }

   // WIZARD_TODO
}

void Engage::Dialog::AddTargetFlightPathCB()
{
   int        curRow             = mUi.mFlightPathTableWidget->rowCount();
   QLineEdit* timeLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* xPosLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* yPosLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* zPosLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* posUnitsLE         = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* xVelLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* yVelLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* zVelLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* velUnitsLE         = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* yawLE              = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* pitchLE            = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* rollLE             = new QLineEdit(mUi.mFlightPathTableWidget);
   QLineEdit* orientationUnitsLE = new QLineEdit(mUi.mFlightPathTableWidget);

   mUi.mFlightPathTableWidget->insertRow(curRow);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 0, timeLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 1, xPosLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 2, yPosLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 3, zPosLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 4, posUnitsLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 5, xVelLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 6, yVelLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 7, zVelLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 8, velUnitsLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 9, yawLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 10, pitchLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 11, rollLE);
   mUi.mFlightPathTableWidget->setCellWidget(curRow, 12, orientationUnitsLE);
}

void Engage::Dialog::RemoveTargetFlightPathCB()
{
   int curRow = mUi.mFlightPathTableWidget->currentRow();
   if (curRow != -1)
   {
      mUi.mFlightPathTableWidget->removeRow(curRow);
   }
}

void Engage::Dialog::AddTargetFlightRouteCB()
{
   int        curRow     = mUi.mFlightRouteTableWidget->rowCount();
   QLineEdit* xPosLE     = new QLineEdit(mUi.mFlightRouteTableWidget);
   QLineEdit* yPosLE     = new QLineEdit(mUi.mFlightRouteTableWidget);
   QLineEdit* zPosLE     = new QLineEdit(mUi.mFlightRouteTableWidget);
   QLineEdit* posUnitsLE = new QLineEdit(mUi.mFlightRouteTableWidget);
   QLineEdit* speedLE    = new QLineEdit(mUi.mFlightRouteTableWidget);

   mUi.mFlightRouteTableWidget->insertRow(curRow);
   mUi.mFlightRouteTableWidget->setCellWidget(curRow, 0, xPosLE);
   mUi.mFlightRouteTableWidget->setCellWidget(curRow, 1, yPosLE);
   mUi.mFlightRouteTableWidget->setCellWidget(curRow, 2, zPosLE);
   mUi.mFlightRouteTableWidget->setCellWidget(curRow, 3, posUnitsLE);
   mUi.mFlightRouteTableWidget->setCellWidget(curRow, 4, speedLE);
}

void Engage::Dialog::RemoveTargetFlightRouteCB()
{
   int curRow = mUi.mFlightRouteTableWidget->currentRow();
   if (curRow != -1)
   {
      mUi.mFlightRouteTableWidget->removeRow(curRow);
   }
}

void Engage::Dialog::AddSiteCB()
{
   int        curRow    = mUi.mSiteTableWidget->rowCount();
   QLineEdit* xPosLE    = new QLineEdit(mUi.mSiteTableWidget);
   QLineEdit* yPosLE    = new QLineEdit(mUi.mSiteTableWidget);
   QLineEdit* zPosLE    = new QLineEdit(mUi.mSiteTableWidget);
   QLineEdit* headingLE = new QLineEdit(mUi.mSiteTableWidget);
   QLineEdit* speedLE   = new QLineEdit(mUi.mSiteTableWidget);

   mUi.mSiteTableWidget->insertRow(curRow);
   mUi.mSiteTableWidget->setCellWidget(curRow, 0, xPosLE);
   mUi.mSiteTableWidget->setCellWidget(curRow, 1, yPosLE);
   mUi.mSiteTableWidget->setCellWidget(curRow, 2, zPosLE);
   mUi.mSiteTableWidget->setCellWidget(curRow, 3, headingLE);
   mUi.mSiteTableWidget->setCellWidget(curRow, 4, speedLE);
}

void Engage::Dialog::RemoveSiteCB()
{
   int curRow = mUi.mSiteTableWidget->currentRow();
   if (curRow != -1)
   {
      mUi.mSiteTableWidget->removeRow(curRow);
   }
}

void Engage::Dialog::BrowseRootCB()
{
   QString filePath = QFileDialog::getExistingDirectory(this, "Open Directory", ".", QFileDialog::ShowDirsOnly);
   if (!filePath.isNull())
   {
      mUi.mRootPathLineEdit->setText(filePath);
      mRootPath = filePath;
   }
}

void Engage::Dialog::RootPathEditedCB(const QString& aStr)
{
   // autopopulate the other fields
   mLauncherPath = aStr + "/launchers";
   mTargetPath   = aStr + "/targets";
   mTrackerPath  = aStr + "/trackers";
   mWeaponPath   = aStr + "/weapons";

   PopulateLists(mLauncherPath, &mLauncherList);
   PopulateLists(mTrackerPath, &mTrackerList);
   PopulateLists(mTargetPath, &mTargetList);
   PopulateLists(mWeaponPath, &mWeaponList);

   TableWidget* siteTableWidget = mUi.mLauncherTableWidget;
   siteTableWidget->AddRow(this);
   siteTableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

   TableWidget* targetTableWidget = mUi.mTargetTableWidget;
   targetTableWidget->AddRow(this);
   targetTableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void Engage::Dialog::SelectPkDirectoryCB()
{
   QString directoryPath = QFileDialog::getExistingDirectory(this, "Select Pk Table Output Directory", ".");
   mUi.mPkOutputPathLineEdit->setText(directoryPath);
}

void Engage::Dialog::closeEvent(QCloseEvent* event)
{
   event->accept();
}

void Engage::Dialog::DeleteRunOutputCB()
{
   auto* removeButton = dynamic_cast<QPushButton*>(sender());

   // find the button
   int foundRow = -1;
   for (int curRow = 0; curRow < mUi.mRunOutputTableWidget->rowCount(); curRow++)
   {
      for (int curCol = 0; curCol < mUi.mRunOutputTableWidget->columnCount(); curCol++)
      {
         QDialogButtonBox* buttonBox =
            dynamic_cast<QDialogButtonBox*>(mUi.mRunOutputTableWidget->cellWidget(curRow, curCol));
         if (buttonBox)
         {
            QList<QAbstractButton*> buttons = buttonBox->buttons();
            if (buttons.contains(removeButton))
            {
               foundRow = curRow;
               break;
            }
         }
      }
      if (foundRow != -1)
      {
         break;
      }
   }
   if (foundRow != -1)
   {
      QString fileName = mUi.mRunOutputTableWidget->item(foundRow, 0)->text();
      for (auto itemIter = mOutputItems.begin(); itemIter != mOutputItems.end(); ++itemIter)
      {
         const OutputItem& curItem = *itemIter;
         if (curItem.GetFileName() == fileName)
         {
            mOutputItems.erase(itemIter);
            break;
         }
      }
      mUi.mRunOutputTableWidget->removeRow(foundRow);
   }
}

void Engage::Dialog::EditRunOutputCB()
{
   auto* editButton = dynamic_cast<QPushButton*>(sender());

   // find the button
   int foundRow = -1;
   for (int curRow = 0; curRow < mUi.mRunOutputTableWidget->rowCount(); curRow++)
   {
      for (int curCol = 0; curCol < mUi.mRunOutputTableWidget->columnCount(); curCol++)
      {
         QDialogButtonBox* buttonBox =
            dynamic_cast<QDialogButtonBox*>(mUi.mRunOutputTableWidget->cellWidget(curRow, curCol));
         if (buttonBox)
         {
            QList<QAbstractButton*> buttons = buttonBox->buttons();
            if (buttons.contains(editButton))
            {
               foundRow = curRow;
               break;
            }
         }
      }
      if (foundRow != -1)
      {
         break;
      }
   }
   if (foundRow != -1)
   {
      QString fileName = mUi.mRunOutputTableWidget->item(foundRow, 0)->text();
      for (auto& curItem : mOutputItems)
      {
         if (curItem.GetFileName() == fileName)
         {
            RunOutputDialog runDialog(curItem);
            if (runDialog.exec())
            {
               QString           fileName = curItem.GetFileName();
               QTableWidgetItem* editItem = mUi.mRunOutputTableWidget->takeItem(foundRow, 0);
               editItem->setText(fileName);
               mUi.mRunOutputTableWidget->setItem(foundRow, 0, editItem);

               QString phaseName = curItem.GetPhase();
               editItem          = mUi.mRunOutputTableWidget->takeItem(foundRow, 1);
               editItem->setText(phaseName);
               mUi.mRunOutputTableWidget->setItem(foundRow, 1, editItem);
            }
            break;
         }
      }
   }
}

void Engage::Dialog::DeleteOutputRateCB()
{
   auto* removeButton = dynamic_cast<QPushButton*>(sender());

   // find the button
   int foundRow = -1;
   for (int curRow = 0; curRow < mUi.mOutputRateTableWidget->rowCount(); curRow++)
   {
      for (int curCol = 0; curCol < mUi.mOutputRateTableWidget->columnCount(); curCol++)
      {
         QDialogButtonBox* buttonBox =
            dynamic_cast<QDialogButtonBox*>(mUi.mOutputRateTableWidget->cellWidget(curRow, curCol));
         if (buttonBox)
         {
            QList<QAbstractButton*> buttons = buttonBox->buttons();
            if (buttons.contains(removeButton))
            {
               foundRow = curRow;
               break;
            }
         }
      }
      if (foundRow != -1)
      {
         break;
      }
   }
   if (foundRow != -1)
   {
      QString fileName = mUi.mOutputRateTableWidget->item(foundRow, 0)->text();
      for (auto itemIter = mOutputRateItems.begin(); itemIter != mOutputRateItems.end(); ++itemIter)
      {
         OutputRateItem curItem = *itemIter;
         if (curItem.GetFileName() == fileName)
         {
            mOutputRateItems.erase(itemIter);
            break;
         }
      }
      mUi.mOutputRateTableWidget->removeRow(foundRow);
   }
}

void Engage::Dialog::EditOutputRateCB()
{
   auto* editButton = dynamic_cast<QPushButton*>(sender());

   // find the button
   int foundRow = -1;
   for (int curRow = 0; curRow < mUi.mOutputRateTableWidget->rowCount(); curRow++)
   {
      for (int curCol = 0; curCol < mUi.mOutputRateTableWidget->columnCount(); curCol++)
      {
         QDialogButtonBox* buttonBox =
            dynamic_cast<QDialogButtonBox*>(mUi.mOutputRateTableWidget->cellWidget(curRow, curCol));
         if (buttonBox)
         {
            QList<QAbstractButton*> buttons = buttonBox->buttons();
            if (buttons.contains(editButton))
            {
               foundRow = curRow;
               break;
            }
         }
      }
      if (foundRow != -1)
      {
         break;
      }
   }
   if (foundRow != -1)
   {
      QString fileName = mUi.mOutputRateTableWidget->item(foundRow, 0)->text();
      for (auto& curItem : mOutputRateItems)
      {
         if (curItem.GetFileName() == fileName)
         {
            OutputRateDialog runDialog(curItem);
            if (runDialog.exec())
            {
               QString           fileName = curItem.GetFileName();
               QTableWidgetItem* editItem = mUi.mRunOutputTableWidget->takeItem(foundRow, 0);
               if (editItem)
               {
                  editItem->setText(fileName);
                  mUi.mRunOutputTableWidget->setItem(foundRow, 0, editItem);
               }
            }
            break;
         }
      }
   }
}

QString Engage::Dialog::GetRelativePath(QString aRootDir, QString aOtherDir)
{
   // split each string by the "/"
   QStringList rootStrList  = aRootDir.split("/");
   QStringList otherStrList = aOtherDir.split("/");

   QString rootFileName  = rootStrList.takeLast();
   QString otherFileName = otherStrList.takeLast();

   QString retStr = "";

   int samePath = 0;
   for (; samePath < rootStrList.size(); samePath++)
   {
      if (rootStrList[samePath] != otherStrList[samePath])
      {
         break;
      }
   }

   if (samePath == rootStrList.size() && samePath == otherStrList.size())
   {
      // the are in the same directory
      retStr = "./";
      retStr.append(otherFileName);
   }
   // check to see if the otherfilename is deeper within the rootfile's current directory structure
   else if (samePath == rootStrList.size())
   {
      for (; samePath < otherStrList.size(); samePath++)
      {
         retStr.append(otherStrList[samePath]);
         retStr.append("/");
      }
      retStr.append(otherFileName);
   }

   // else drill down to the root file dir, back up and move down the other file path
   else
   {
      int dirDiff = rootStrList.size() - samePath;
      for (int i = 0; i < dirDiff; i++)
      {
         retStr.append("../");
      }
      for (; samePath < otherStrList.size(); samePath++)
      {
         retStr.append(otherStrList[samePath]);
         retStr.append("/");
      }
      retStr.append(otherFileName);
   }

   return retStr;
}
