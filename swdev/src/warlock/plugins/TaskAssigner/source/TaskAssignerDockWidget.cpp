// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TaskAssignerDockWidget.hpp"

#include <QIcon>
#include <QPixmap>

#include "TaskAssignerDataContainer.hpp"
#include "TaskAssignerSimCommands.hpp"
#include "TaskAssignerSimInterface.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfTaskResource.hpp"
#include "WsfWeaponTaskResource.hpp"

// ============================================================================
WkTaskAssigner::DockWidget::DockWidget(SimInterface*   aSimInterfacePtr,
                                       DataContainer&  aData,
                                       QWidget*        aParentPtr,
                                       Qt::WindowFlags aFlags)
   : QDockWidget(aParentPtr, aFlags)
   , mData(aData)
   , mPendingUpdate(false)
   , mSimInterfacePtr(aSimInterfacePtr)
{
   mUi.setupUi(this);
   Reset();
   Connect();
   Disable();
}

// ============================================================================
void WkTaskAssigner::DockWidget::Update()
{
   if (mPendingUpdate)
   {
      // Reset the dialog
      Reset();

      // If the platform of interest is valid
      if (mData.IsValid())
      {
         // SELF
         const std::string& selfName = mData.GetCommandChainData().mSelf.Name();
         mUi.mAssignerName->setText(selfName.c_str());
         if (IsAssignerValid() && IsTrackValid())
         {
            SetChecked(mUi.mAssignerIconLabel);
         }

         // Task Processor
         mUi.mTaskProcessorComboBox->clear();
         const auto* taskProcessorData = mData.GetTaskProcessorData(selfName);
         if (taskProcessorData && !taskProcessorData->empty())
         {
            for (const auto& taskProc : *taskProcessorData)
            {
               mUi.mTaskProcessorComboBox->addItem(QString::fromStdString(taskProc.Name() + " : " + taskProc.Type()));
            }
         }
         else
         {
            mUi.mTaskProcessorComboBox->addItem(QIcon::fromTheme("alert_red"), "No Processors");
         }
         mUi.mTaskProcessorComboBox->setCurrentIndex(0);

         // Command Chains
         mUi.mCommandChainComboBox->clear();
         if (!mData.GetCommandChainData().mCommandChainNames.empty())
         {
            for (const auto& commandChainName : mData.GetCommandChainData().mCommandChainNames)
            {
               mUi.mCommandChainComboBox->addItem(QString::fromStdString(commandChainName));
            }
         }
         mUi.mCommandChainComboBox->setCurrentIndex(0);

         // Populate the communication devices
         const auto& commData = mData.GetCommunicationDevicesData();
         for (const auto& comm : commData)
         {
            mUi.mCommsComboBox->addItem(QString::fromStdString(comm.Name() + " : " + comm.Type()));
         }

         // Reset the flag
         mPendingUpdate = false;
      }
      mUi.mCommsComboBox->setCurrentIndex(0);
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::AssigneeSelected(QTreeWidgetItem* aTreeItemPtr, int aColumn)
{
   Disable();

   if (aTreeItemPtr != nullptr)
   {
      // Get the name of the select tree item
      bool        selfSelected(false);
      std::string selectedItemName = aTreeItemPtr->text(1).toStdString();
      if (selectedItemName == "SELF")
      {
         selectedItemName = mData.GetPlatformOfInterest();
         selfSelected     = true;
      }

      // Updated selected platform
      mUi.mAssigneeName->setText(QString::fromStdString(selectedItemName));
      SetChecked(mUi.mAssigneeIconLabel);

      // Jammers
      mUi.mJammerComboBox->clear();
      const auto* jammerData = mData.GetJammerData(selectedItemName);
      if (jammerData && !jammerData->empty())
      {
         for (const auto& jammer : *jammerData)
         {
            mUi.mJammerComboBox->addItem(QString::fromStdString(jammer.Name() + " : " + jammer.Type()));
         }
         mUi.mTaskToolBox->setItemIcon(1, QIcon::fromTheme("checkbox_checked"));
      }
      else
      {
         mUi.mJammerComboBox->addItem("No Jammers");
         mUi.mBeamNumberSpinBox->setValue(0);
         mUi.mBeamNumberSpinBox->setMaximum(0);
      }
      mUi.mJammerComboBox->setCurrentIndex(0);

      // Processors
      mUi.mProcessorComboBox->clear();
      const auto* processorData = mData.GetProcessorData(selectedItemName);
      if (processorData && !processorData->empty())
      {
         // Always add blank as the first entry
         mUi.mProcessorComboBox->addItem("[Optional]");
         for (const auto& proc : *processorData)
         {
            mUi.mProcessorComboBox->addItem(QString::fromStdString(proc.Name() + " : " + proc.Type()));
         }
         mUi.mTaskToolBox->setItemIcon(3, QIcon::fromTheme("checkbox_checked"));
      }
      else
      {
         mUi.mProcessorComboBox->addItem("No Processors");
      }
      mUi.mProcessorComboBox->setCurrentIndex(0);

      // Sensors
      mUi.mSensorComboBox->clear();
      mUi.mSensorModeComboBox->clear();
      const auto* sensorData = mData.GetSensorData(selectedItemName);
      if (sensorData && !sensorData->empty())
      {
         for (const auto& sensor : *sensorData)
         {
            mUi.mSensorComboBox->addItem(QString::fromStdString(sensor.Name() + " : " + sensor.Type()));
         }
         mUi.mTaskToolBox->setItemIcon(2, QIcon::fromTheme("checkbox_checked"));
      }
      else
      {
         mUi.mSensorComboBox->addItem("No Sensors");
         mUi.mSensorModeComboBox->addItem("No Modes");
      }
      mUi.mSensorComboBox->setCurrentIndex(0);

      // Weapons
      mUi.mWeaponComboBox->clear();
      const auto& weaponData = mData.GetWeaponData(selectedItemName);
      if (weaponData && !weaponData->empty())
      {
         for (const auto& weapon : *weaponData)
         {
            mUi.mWeaponComboBox->addItem(QString::fromStdString(weapon.Name() + " : " + weapon.Type()), weapon.mID);
         }
         mUi.mWeaponQuantitySpinBox->setValue(1);

         auto id = mUi.mWeaponComboBox->currentData().toUInt();
         for (auto& weapon : *weaponData)
         {
            if (weapon.mID == id)
            {
               mUi.mWeaponQuantitySpinBox->setMaximum((int)weapon.mQuantity);
               break;
            }
         }
         mUi.mTaskToolBox->setItemIcon(0, QIcon::fromTheme("checkbox_checked"));
      }
      else
      {
         mUi.mWeaponComboBox->addItem("No Weapons");
         mUi.mWeaponQuantitySpinBox->setValue(0);
         mUi.mWeaponQuantitySpinBox->setMaximum(0);
      }
      mUi.mWeaponComboBox->setCurrentIndex(0);

      Enable(selfSelected);

      // Check to see if assignment conditions have been met
      CheckAssignmentConditions();
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::Connect()
{
   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
   connect(&mData, &DataContainer::PlatformDeleted, this, &DockWidget::PlatformDeleted);
   connect(mUi.mCommandChainComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &DockWidget::CommandChainComboBoxChanged);
   connect(mUi.mTreeWidget, &QTreeWidget::itemClicked, this, &DockWidget::AssigneeSelected);
   connect(mUi.mFilterLineEdit, &QLineEdit::textChanged, this, &DockWidget::FilterChanged);
   connect(mUi.mFilterComboBox, &QComboBox::currentTextChanged, this, &DockWidget::FilterChanged);
   connect(mUi.mTrackSelectorPushButton, &wkf::TrackSelector::Selected, this, &DockWidget::TrackSelected);
   connect(mUi.mTrackIdLineEdit, &QLineEdit::textEdited, this, &DockWidget::TrackTextEdited);
   connect(mUi.mTaskToolBox, &QToolBox::currentChanged, this, &DockWidget::TaskToolBoxChanged);
   connect(mUi.mSensorComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &DockWidget::SensorComboBoxChanged);
   connect(mUi.mWeaponComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &DockWidget::WeaponComboBoxChanged);
   connect(mUi.mFireAssignPushButton, &QPushButton::clicked, this, &DockWidget::AssignButtonPressed);
   connect(mUi.mJamAssignPushButton, &QPushButton::clicked, this, &DockWidget::AssignButtonPressed);
   connect(mUi.mTrackAssignPushButton, &QPushButton::clicked, this, &DockWidget::AssignButtonPressed);
   connect(mUi.mCustomAssignPushButton, &QPushButton::clicked, this, &DockWidget::AssignButtonPressed);
}

// ============================================================================
void WkTaskAssigner::DockWidget::Disable()
{
   mUi.mTaskToolBox->setItemIcon(0, QIcon::fromTheme("checkbox_unchecked"));
   mUi.mTaskToolBox->setItemIcon(1, QIcon::fromTheme("checkbox_unchecked"));
   mUi.mTaskToolBox->setItemIcon(2, QIcon::fromTheme("checkbox_unchecked"));
   mUi.mTaskToolBox->setItemIcon(3, QIcon::fromTheme("checkbox_unchecked"));

   mUi.mCommsComboBox->setEnabled(false);

   mUi.mProcessorComboBox->setEnabled(false);

   mUi.mSensorComboBox->setEnabled(false);
   mUi.mSensorModeComboBox->setEnabled(false);

   mUi.mWeaponComboBox->setEnabled(false);
   mUi.mWeaponQuantitySpinBox->setEnabled(false);

   mUi.mFireAssignPushButton->setEnabled(false);
   mUi.mJamAssignPushButton->setEnabled(false);
   mUi.mTrackAssignPushButton->setEnabled(false);
}

// ============================================================================
void WkTaskAssigner::DockWidget::Enable(bool aSelf)
{
   // If remote assignment then enable the comms
   if (!aSelf)
   {
      mUi.mCommsComboBox->setEnabled(true);
   }

   mUi.mProcessorComboBox->setEnabled(true);

   mUi.mSensorComboBox->setEnabled(true);
   mUi.mSensorModeComboBox->setEnabled(true);

   mUi.mWeaponComboBox->setEnabled(true);
   mUi.mWeaponQuantitySpinBox->setEnabled(true);
}

// ============================================================================
void WkTaskAssigner::DockWidget::Reset()
{
   // Assigner
   SetChecked(mUi.mAssignerIconLabel, false);

   mUi.mAssignerName->setText("No Platform Selected");
   mUi.mTaskProcessorComboBox->clear();
   mUi.mTaskProcessorComboBox->addItem(QIcon::fromTheme("alert_red"), "No Processors");
   mUi.mCommandChainComboBox->clear();
   QString temp = QString::fromStdString("default");
   mUi.mCommandChainComboBox->addItem(temp);

   // Assignees
   SetChecked(mUi.mAssigneeIconLabel, false);
   mUi.mAssigneeName->setText("No Platform Selected");
   mUi.mTreeWidget->clear();
   mUi.mTreeWidget->resizeColumnToContents(0);

   // Track
   mUi.mTrackIdLineEdit->setText("[Required]");

   // Task Type
   mUi.mTaskTypeLineEdit->setText("[Optional]");

   // Comms
   mUi.mCommsComboBox->clear();
   mUi.mCommsComboBox->addItem("[Default]");

   // Clear the assignee specific data
   mUi.mJammerComboBox->clear();
   mUi.mJammerComboBox->addItem("No Jammers");

   mUi.mProcessorComboBox->clear();
   mUi.mProcessorComboBox->addItem("No Processors");

   mUi.mSensorComboBox->clear();
   mUi.mSensorComboBox->addItem("No Sensors");
   mUi.mSensorModeComboBox->clear();
   mUi.mSensorModeComboBox->addItem("No Modes");

   mUi.mWeaponComboBox->clear();
   mUi.mWeaponComboBox->addItem("No Weapons");
   mUi.mWeaponQuantitySpinBox->setValue(0);
   mUi.mWeaponQuantitySpinBox->setMaximum(0);

   // Disable controls
   Disable();
}

// ============================================================================
bool WkTaskAssigner::DockWidget::IsAssignerValid()
{
   return ((mUi.mAssignerName->text() != "No Platform Selected") &&
           (mUi.mTaskProcessorComboBox->currentText() != "No Processors"));
}

// ============================================================================
bool WkTaskAssigner::DockWidget::IsAssigneeValid()
{
   return (mUi.mAssigneeName->text() != "No Platform Selected");
}

// ============================================================================
bool WkTaskAssigner::DockWidget::IsTaskValid()
{
   // Check each task type
   bool validWeaponTask =
      ((mUi.mTaskToolBox->currentIndex() == 0) && (mUi.mWeaponComboBox->currentText() != "No Weapons") &&
       (mUi.mWeaponQuantitySpinBox->value() > 0));
   bool validJammerTask =
      ((mUi.mTaskToolBox->currentIndex() == 1) && (mUi.mJammerComboBox->currentText() != "No Jammers"));
   bool validSensorTask =
      ((mUi.mTaskToolBox->currentIndex() == 2) && (mUi.mSensorComboBox->currentText() != "No Sensors"));
   bool validCustomTask = (mUi.mTaskToolBox->currentIndex() == 3);

   return (validWeaponTask || validJammerTask || validSensorTask || validCustomTask);
}

// ============================================================================
bool WkTaskAssigner::DockWidget::IsTrackValid()
{
   bool ok;
   int  trackID = mUi.mTrackIdLineEdit->text().toInt(&ok);
   return ((mUi.mTrackIdLineEdit->text() != "[Required]") && (ok && (trackID > 0)));
}

// ============================================================================
void WkTaskAssigner::DockWidget::AssignButtonPressed()
{
   auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(mData.GetPlatformOfInterest());
   if (platformPtr)
   {
      if (!warlock::HasPermissionToControlPlatform(*platformPtr))
      {
         QMessageBox::warning(nullptr,
                              "Script Not Executed",
                              "User does not have permissions to modify platform " +
                                 QString::fromStdString(platformPtr->GetName()) + ".");
         return;
      }
   }

   // Get the task type, if specified
   std::string taskType = mUi.mTaskTypeLineEdit->text().toStdString();
   if (taskType == "[Optional]")
   {
      taskType = "";
   }

   // Get the assignee; default to self
   std::string assignee = mData.GetPlatformOfInterest();

   // Get the name of the select tree item; should be only single selection
   QList<QTreeWidgetItem*> itemList = mUi.mTreeWidget->selectedItems();
   if (!itemList.empty())
   {
      std::string selectedItemName = itemList[0]->text(1).toStdString();
      if (selectedItemName != "SELF")
      {
         assignee = selectedItemName;
      }
   }

   // Get the task processor name
   QStringList taskProcessorList = mUi.mTaskProcessorComboBox->currentText().split(':');
   std::string taskProcessorName = taskProcessorList.at(0).trimmed().toStdString();

   // Get the comm name, if specified
   std::string commName = mUi.mCommsComboBox->currentText().toStdString();
   if (commName == "[Default]")
   {
      commName = "";
   }

   // Create a task
   if (mUi.mTaskToolBox->currentIndex() == 0) // Fire
   {
      if (mUi.mWeaponComboBox->currentText() != "No Weapons")
      {
         WsfWeaponTaskResource resource; // Resource info
         QStringList           weaponList = mUi.mWeaponComboBox->currentText().split(':');
         resource.mNameId                 = weaponList.at(0).trimmed().toStdString(); // - Weapon name
         resource.mCount                  = mUi.mWeaponQuantitySpinBox->value();      // - Weapon count
         mSimInterfacePtr->AddSimCommand(
            ut::make_unique<MilAssignWeaponTaskCommand>(taskProcessorName,                    // Task processor
                                                        mData.GetPlatformOfInterest(),        // Assigner
                                                        mUi.mTrackIdLineEdit->text().toInt(), // Track
                                                        taskType,                             // Task Type
                                                        resource,                             // Sensor resource
                                                        assignee,                             // Assignee
                                                        commName));                           // Communication Device
      }
   }
   else if (mUi.mTaskToolBox->currentIndex() == 1) // Jam
   {
      if (mUi.mJammerComboBox->currentText() != "No Jammers")
      {
         WsfJammerTaskResource resource;
         QStringList           jammerList = mUi.mJammerComboBox->currentText().split(':');
         resource.mNameId                 = jammerList.at(0).trimmed().toStdString(); // - Jammer name
         bool   ok;
         double frequency = mUi.mFrequencyLineEdit->text().toDouble(&ok);
         if (!ok)
         {
            frequency = 0.0;
         }
         resource.mFrequency = frequency;
         double bandWidth    = mUi.mBandwidthLineEdit->text().toDouble(&ok);
         if (!ok)
         {
            bandWidth = 0.0;
         }
         resource.mBandwidth     = bandWidth;                                    // - Jammer bandwidth
         resource.mBeamNumber    = mUi.mBeamNumberSpinBox->value();              // - Jammer beamNumber
         resource.mTechniqueName = mUi.mTechniqueLineEdit->text().toStdString(); // - Jammer technique name

         mSimInterfacePtr->AddSimCommand(
            ut::make_unique<MilAssignJammerTaskCommand>(taskProcessorName,                    // Task processor
                                                        mData.GetPlatformOfInterest(),        // Assigner
                                                        mUi.mTrackIdLineEdit->text().toInt(), // Track
                                                        taskType,                             // Task Type
                                                        resource,                             // Jammer resource
                                                        assignee,                             // Assignee
                                                        commName));                           // Communication Device
      }
   }
   else if (mUi.mTaskToolBox->currentIndex() == 2) // Track
   {
      if (mUi.mSensorComboBox->currentText() != "No Sensors")
      {
         WsfTaskResource resource(cTASK_RESOURCE_TYPE_SENSOR); // Resource info
         QStringList     sensorList = mUi.mSensorComboBox->currentText().split(':');
         resource.mNameId           = sensorList.at(0).trimmed().toStdString();                // - Sensor name
         resource.mModeNameId       = mUi.mSensorModeComboBox->currentText().toStdString();    // - Sensor mode
         mSimInterfacePtr->AddSimCommand(ut::make_unique<AssignTaskCommand>(taskProcessorName, // Task processor
                                                                            mData.GetPlatformOfInterest(), // Assigner
                                                                            mUi.mTrackIdLineEdit->text().toInt(), // Track
                                                                            taskType,   // Task Type
                                                                            resource,   // Sensor resource
                                                                            assignee,   // Assignee
                                                                            commName)); // Communication Device
      }
   }
   else if (mUi.mTaskToolBox->currentIndex() == 3) // Custom
   {
      if ((mUi.mProcessorComboBox->currentText() == "No Processors") ||
          (mUi.mProcessorComboBox->currentText() == "[Optional]"))
      {
         mSimInterfacePtr->AddSimCommand(ut::make_unique<AssignTaskCommand>(taskProcessorName, // Task processor
                                                                            mData.GetPlatformOfInterest(), // Assigner
                                                                            mUi.mTrackIdLineEdit->text().toInt(), // Track
                                                                            taskType,          // Task Type
                                                                            WsfTaskResource(), // Empty resource
                                                                            assignee,          // Assignee
                                                                            commName));        // Communication Device
      }
      else
      {
         WsfTaskResource resource(cTASK_RESOURCE_TYPE_PROCESSOR); // Resource info
         QStringList     procList = mUi.mProcessorComboBox->currentText().split(':');
         resource.mNameId         = procList.at(0).trimmed().toStdString();                    // - Processor name
         mSimInterfacePtr->AddSimCommand(ut::make_unique<AssignTaskCommand>(taskProcessorName, // Task processor
                                                                            mData.GetPlatformOfInterest(), // Assigner
                                                                            mUi.mTrackIdLineEdit->text().toInt(), // Track
                                                                            taskType,   // Task Type
                                                                            resource,   // Processor resource
                                                                            assignee,   // Assignee
                                                                            commName)); // Communication Device
      }
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::CheckAssignmentConditions()
{
   // Check all conditions
   if (IsAssignerValid() && IsAssigneeValid() && IsTrackValid() && IsTaskValid())
   {
      if (mUi.mTaskToolBox->currentIndex() == 0)
      {
         mUi.mFireAssignPushButton->setEnabled(true);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 1)
      {
         mUi.mJamAssignPushButton->setEnabled(true);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 2)
      {
         mUi.mTrackAssignPushButton->setEnabled(true);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 3)
      {
         mUi.mCustomAssignPushButton->setEnabled(true);
      }
   }
   else
   {
      if (mUi.mTaskToolBox->currentIndex() == 0)
      {
         mUi.mFireAssignPushButton->setEnabled(false);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 1)
      {
         mUi.mJamAssignPushButton->setEnabled(false);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 2)
      {
         mUi.mTrackAssignPushButton->setEnabled(false);
      }
      else if (mUi.mTaskToolBox->currentIndex() == 3)
      {
         mUi.mCustomAssignPushButton->setEnabled(false);
      }
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::CommandChainComboBoxChanged(int aIndex)
{
   if (!mData.IsValid())
   {
      return;
   }

   // Clear the tree
   mUi.mTreeWidget->clear();
   const auto& commandChainData = mData.GetCommandChainData();

   // SELF
   const auto& self = commandChainData.mSelf;
   mUi.mTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << " "
                                                                      << "SELF" << self.Type().c_str()));

   if (aIndex >= 0)
   {
      // Get the name of the command chain
      std::string commandChainName = mUi.mCommandChainComboBox->currentText().toStdString();
      {
         // Commander
         const auto& commanders = commandChainData.mCommanders;
         const auto& citer      = commanders.find(commandChainName);
         if (citer != commanders.end())
         {
            const auto& commander = citer->second;
            if (self.Name() != commander.Name())
            {
               mUi.mTreeWidget->addTopLevelItem(
                  new QTreeWidgetItem(QStringList() << "C" << commander.Name().c_str() << commander.Type().c_str()));
            }
         }
      }
      {
         const auto& subordinates = commandChainData.mSubordinates;
         const auto& siter        = subordinates.find(commandChainName);
         if (siter != subordinates.end())
         {
            // Populate the subordinates
            const auto& subsData = siter->second;
            if (!subsData.empty())
            {
               for (const auto& sub : subsData)
               {
                  mUi.mTreeWidget->addTopLevelItem(
                     new QTreeWidgetItem(QStringList() << "S" << sub.Name().c_str() << sub.Type().c_str()));
               }
            }
         }
      }
      {
         // Populate peers
         const auto& peers = commandChainData.mPeers;
         const auto& piter = peers.find(commandChainName);
         if (piter != peers.end())
         {
            const auto& peersData = piter->second;
            if (!peersData.empty())
            {
               for (const auto& peer : peersData)
               {
                  auto* item = new QTreeWidgetItem(QStringList() << "P" << peer.Name().c_str() << peer.Type().c_str());
                  mUi.mTreeWidget->addTopLevelItem(item);
               }
            }
         }
      }
   }

   mUi.mTreeWidget->resizeColumnToContents(0);
   mUi.mTreeWidget->resizeColumnToContents(1);
   mUi.mTreeWidget->resizeColumnToContents(2);
}

// ============================================================================
void WkTaskAssigner::DockWidget::FilterChanged()
{
   QTreeWidgetItemIterator iter(mUi.mTreeWidget);
   while (QTreeWidgetItem* item = (*iter))
   {
      item->setHidden(!ItemPassesVisibilityFilter(item));
      ++iter;
   }
   mUi.mTreeWidget->resizeColumnToContents(0);
   mUi.mTreeWidget->resizeColumnToContents(1);
   mUi.mTreeWidget->resizeColumnToContents(2);
}

// ============================================================================
bool WkTaskAssigner::DockWidget::ItemPassesVisibilityFilter(const QTreeWidgetItem* aItem)
{
   bool visible = false;

   // This assumes the first option is "All" thus -1 is all columns.
   int     column = mUi.mFilterComboBox->currentIndex() - 1;
   QString text   = mUi.mFilterLineEdit->text();

   // Check if text == "" first, since this function will most commonly be called with an empty filter, therefore we
   // wont waste processing time.
   if (text.isEmpty())
   {
      visible = true;
   }
   else
   {
      // if column == -1, search all column for the filter text
      if (column == -1)
      {
         for (int i = 0; i < mUi.mTreeWidget->columnCount(); ++i)
         {
            if (aItem->text(i).contains(text, Qt::CaseInsensitive))
            {
               visible = true;
               break; // Exit as soon as we know it should be visible
            }
         }
      }
      else // Check only the specified column's text
      {
         if (aItem->text(column).contains(text, Qt::CaseInsensitive))
         {
            visible = true;
         }
      }
   }
   return visible;
}

// ============================================================================
void WkTaskAssigner::DockWidget::PlatformDeleted(const std::string& aPlatformName)
{
   QList<QTreeWidgetItem*> items = mUi.mTreeWidget->findItems(aPlatformName.c_str(), Qt::MatchExactly, 1);
   for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      delete mUi.mTreeWidget->takeTopLevelItem(mUi.mTreeWidget->indexOfTopLevelItem(*iter));
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      mData.SetPlatformOfInterest(aPlatformPtr->GetName());
      mSimInterfacePtr->SetPlatformOfInterest(aPlatformPtr->GetName());
   }
   else
   {
      mData.SetPlatformOfInterest("");
      mSimInterfacePtr->SetPlatformOfInterest("");
   }
   mPendingUpdate = true;
}

// ============================================================================
void WkTaskAssigner::DockWidget::SensorComboBoxChanged(int aIndex)
{
   // Get the name of the select tree item; should be only single selection
   QList<QTreeWidgetItem*> itemList = mUi.mTreeWidget->selectedItems();
   if (!itemList.empty())
   {
      std::string selectedItemName = itemList[0]->text(1).toStdString();
      if (selectedItemName == "SELF")
      {
         selectedItemName = mData.GetPlatformOfInterest();
      }

      // Clear the mode combo box
      mUi.mSensorModeComboBox->clear();

      const auto* sensorData = mData.GetSensorData(selectedItemName);
      if (sensorData)
      {
         // Get the selected sensor index
         int index = mUi.mSensorComboBox->currentIndex();
         if ((index >= 0) && ((size_t)index < sensorData->size()))
         {
            if (!(*sensorData)[index].mModes.empty())
            {
               for (const auto& mode : (*sensorData)[index].mModes)
               {
                  mUi.mSensorModeComboBox->addItem(QString::fromStdString(mode));
               }
            }
         }
      }

      // If no sensor modes were added to the combo box, put "No Modes" in the combo box
      if (mUi.mSensorModeComboBox->count() == 0)
      {
         mUi.mSensorModeComboBox->addItem("No Modes");
      }
   }
}

// ============================================================================
void WkTaskAssigner::DockWidget::SetChecked(QLabel* aLabel, bool aChecked)
{
   QString imageName;
   if (aChecked)
   {
      imageName = QString(":/icons/%1/64x64/%2.png").arg(QIcon::themeName(), "checkbox_checked");
   }
   else
   {
      imageName = QString(":/icons/%1/64x64/%2.png").arg(QIcon::themeName(), "checkbox_unchecked");
   }

   QImage image = QImage(imageName).scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
   aLabel->setPixmap(QPixmap::fromImage(image));
}

// ============================================================================
void WkTaskAssigner::DockWidget::TaskToolBoxChanged(int aIndex)
{
   CheckAssignmentConditions();
}

// ============================================================================
void WkTaskAssigner::DockWidget::TrackSelected(wkf::Track* aTrack)
{
   if (aTrack)
   {
      mUi.mTrackIdLineEdit->setText(QString::number(aTrack->GetTrackId().GetLocalTrackId()));
   }
   else
   {
      mUi.mTrackIdLineEdit->setText("[Required]");
   }

   if (IsAssignerValid() && IsTrackValid())
   {
      SetChecked(mUi.mAssignerIconLabel);
   }
   else
   {
      SetChecked(mUi.mAssignerIconLabel, false);
   }

   CheckAssignmentConditions();
}

// ============================================================================
void WkTaskAssigner::DockWidget::TrackTextEdited(const QString& aText)
{
   if (IsAssignerValid() && IsTrackValid())
   {
      SetChecked(mUi.mAssignerIconLabel);
   }
   else
   {
      SetChecked(mUi.mAssignerIconLabel, false);
   }
   CheckAssignmentConditions();
}

// ============================================================================
void WkTaskAssigner::DockWidget::WeaponComboBoxChanged(int aIndex)
{
   // Get the name of the select tree item; should be only single selection
   QList<QTreeWidgetItem*> itemList = mUi.mTreeWidget->selectedItems();
   if (!itemList.empty())
   {
      std::string selectedItemName = itemList[0]->text(1).toStdString();
      if (selectedItemName == "SELF")
      {
         selectedItemName = mData.GetPlatformOfInterest();
      }

      auto id = mUi.mWeaponComboBox->currentData().toUInt();

      const auto* weaponData = mData.GetWeaponData(selectedItemName);
      if (weaponData)
      {
         for (const auto& weapon : *weaponData)
         {
            if (weapon.mID == id)
            {
               mUi.mWeaponQuantitySpinBox->setMaximum((int)weapon.mQuantity);
               break;
            }
         }
      }
   }
}
