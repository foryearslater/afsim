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
#include "ScriptBrowserDockWidget.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

#include "VaObserver.hpp"
#include "WkArrayWidget.hpp"
#include "WkPermissions.hpp"
#include "WkPlatform.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfSelectorWidgets.hpp"
#include "WkfTrack.hpp"
#include "WkfVtkEnvironment.hpp"

WkScriptBrowser::DockWidget::DockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mSimInterfacePtr(simEnv.GetScriptSimInterface())
   , mScriptPrefix("WARLOCK_")
   , mGlobalTopLevelItem(nullptr)
   , mArgWidget(nullptr)
{
   mUi.setupUi(this);

   connect(&wkfEnv, &wkf::Environment::PlatformSelectionChanged, this, &DockWidget::PlatformSelectionChanged);

   connect(mUi.treeWidget, &QTreeWidget::itemSelectionChanged, this, &DockWidget::TreeWidgetItemSelectionChanged);
   connect(mUi.executePushButton, &QPushButton::clicked, this, &DockWidget::ExecutePushButtonClicked);

   connect(mSimInterfacePtr.get(),
           &warlock::ScriptSimInterface::ReturnValueReady,
           this,
           &DockWidget::ReturnValueReadyCB,
           Qt::QueuedConnection);
   connect(mSimInterfacePtr.get(),
           &warlock::ScriptSimInterface::RemotePlatformUpdated,
           this,
           &DockWidget::RemotePlatformUpdatedCB,
           Qt::QueuedConnection);

   ClearScriptInfo();
   ClearReturnValue();
}

void WkScriptBrowser::DockWidget::SimulationStarting()
{
   mScriptsWaitingForReturnValues.clear();
   PopulateGlobalTableWidget();
}

void WkScriptBrowser::DockWidget::RemotePlatformUpdatedCB(const QString& aPlatformName)
{
   if (wkfEnv.IsPlatformSelected(aPlatformName.toStdString()))
   {
      auto* platform = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(aPlatformName.toStdString()));
      if (platform)
      {
         // If we received the list of scripts for a platform and it is still selected, update the display.
         PopulatePlatformTableWidget(aPlatformName);
      }
   }
}

void WkScriptBrowser::DockWidget::ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue)
{
   // if aScriptId is in the set of scripts I have commanded
   if (mScriptsWaitingForReturnValues.count(aScriptId) > 0)
   {
      mScriptsWaitingForReturnValues.erase(aScriptId);

      QTreeWidgetItem* item = mUi.treeWidget->currentItem();
      if (item != nullptr)
      {
         std::string platformName = item->data(0, PLATFORM_NAME).toString().toStdString();

         // if the same platform is selected when the return value is received
         if (aReturnValue.platformName ==
             platformName) // platform script with currently selected is the same platform (or "" == "" for global)
         {
            QString          scriptName = QString::fromStdString(aReturnValue.scriptName);
            QTreeWidgetItem* item       = mUi.treeWidget->currentItem();
            // if the same script is still selected when the return value is received
            if (item->data(0, SCRIPT_NAME).toString() == scriptName)
            {
               mUi.returnValueLineEdit->setText(warlock::script::UtScriptDataToString(aReturnValue.returnValue).join("; "));
            }
         }
      }
   }
}

void WkScriptBrowser::DockWidget::ExecutePushButtonClicked()
{
   QString scriptName   = sender()->property("script").toString();
   QString platformName = sender()->property("platform").toString();

   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = platformName.toStdString();

   if (platformName != "")
   {
      auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(platformName);
      if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
      {
         QMessageBox::warning(nullptr,
                              "Script Not Executed",
                              "User does not have permissions to modify platform " +
                                 QString::fromStdString(platformPtr->GetName()) + ".");
         return;
      }
      else
      {
         mSimInterfacePtr->GetPlatformScriptData(scriptInstance.platformName, scriptName.toStdString(), scriptInstance.script);
      }
   }
   else
   {
      mSimInterfacePtr->GetGlobalScriptData(scriptName.toStdString(), scriptInstance.script);
   }
   for (QList<DataWidget*>::const_iterator iter = mScriptArgWidgets.begin(); iter != mScriptArgWidgets.end(); ++iter)
   {
      scriptInstance.args.push_back((*iter)->GetScriptArg());
      (*iter)->CancelSelection();
   }
   mScriptsWaitingForReturnValues.insert(mSimInterfacePtr->ExecuteScript(scriptInstance));
}

void WkScriptBrowser::DockWidget::TreeWidgetItemSelectionChanged()
{
   ClearScriptInfo();
   ClearReturnValue();

   QList<QTreeWidgetItem*> items = mUi.treeWidget->selectedItems();
   for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      // If selected item is not a top level item
      if ((*iter)->parent() != nullptr)
      {
         warlock::script::Data data;

         QString scriptName = (*iter)->data(0, SCRIPT_NAME).toString();
         // user data contains platform name, which is empty if a global script
         QString platformName = (*iter)->data(0, PLATFORM_NAME).toString();

         if (platformName == "")
         {
            mSimInterfacePtr->GetGlobalScriptData(scriptName.toStdString(), data);
         }
         else
         {
            mSimInterfacePtr->GetPlatformScriptData(platformName.toStdString(), scriptName.toStdString(), data);
         }
         PopulateScrollArea(data, platformName);
      }
   }
}


void WkScriptBrowser::DockWidget::PlatformSelectionChanged(const wkf::PlatformList& aSelectedPlatforms,
                                                           const wkf::PlatformList& aUnselectedPlatforms)
{
   for (const auto& platform : aSelectedPlatforms)
   {
      auto* warlockPlatform = dynamic_cast<warlock::Platform*>(platform);
      if (warlockPlatform != nullptr)
      {
         if (warlockPlatform->GetXIO_Controlled())
         {
            mSimInterfacePtr->RequestXIO_ScriptInfo(warlockPlatform->GetIndex());
         }
         PopulatePlatformTableWidget(QString::fromStdString(warlockPlatform->GetName()));
      }
   }

   for (const auto& platform : aUnselectedPlatforms)
   {
      // We only allow a single script to be selected at once
      QList<QTreeWidgetItem*> items = mUi.treeWidget->selectedItems();
      if (!items.empty())
      {
         QString platformName = items.at(0)->data(0, PLATFORM_NAME).toString();
         // If the currently selected script belong to the same platform that is being deselected, then clear displays
         if (platformName.toStdString() == platform->GetName())
         {
            ClearScriptInfo();
            ClearReturnValue();
         }
      }

      RemovePlatformTree(QString::fromStdString(platform->GetName()));
   }
}

void WkScriptBrowser::DockWidget::RemovePlatformTree(const QString& aPlatformName)
{
   if (mPlatformTopLevelItemMap.count(aPlatformName) > 0)
   {
      mUi.treeWidget->removeItemWidget(mPlatformTopLevelItemMap.at(aPlatformName), 0);
      delete mPlatformTopLevelItemMap.at(aPlatformName);
      mPlatformTopLevelItemMap.erase(aPlatformName);
   }
}

void WkScriptBrowser::DockWidget::ClearScriptInfo()
{
   if (mArgWidget != nullptr)
   {
      delete mArgWidget;
      mArgWidget = nullptr;
   }
   mScriptArgWidgets.clear(); // widgets deleted during deletion of mArgWidget
   mUi.scrollArea->setVisible(false);
   mUi.scriptNameLabel->setVisible(false);
}

void WkScriptBrowser::DockWidget::ClearReturnValue()
{
   mUi.returnValueLineEdit->setText("");
   SetShowReturnValue(false);
}

void WkScriptBrowser::DockWidget::SetShowReturnValue(bool aShow)
{
   mUi.returnValueLabel->setVisible(aShow);
   mUi.returnValueLineEdit->setVisible(aShow);
}

void WkScriptBrowser::DockWidget::PopulateGlobalTableWidget()
{
   // If the global item exist, it must be from a previous simulation, so delete it
   if (mGlobalTopLevelItem != nullptr)
   {
      delete mGlobalTopLevelItem;
      mGlobalTopLevelItem = nullptr;
   }

   mGlobalTopLevelItem = new QTreeWidgetItem();
   mGlobalTopLevelItem->setText(0, "Global");
   mUi.treeWidget->addTopLevelItem(mGlobalTopLevelItem);

   bool globalScriptsAdded = false;

   warlock::script::DataMap data;
   mSimInterfacePtr->GetAllGlobalScriptData(data);
   for (const auto& scriptPair : data)
   {
      QString scriptName = QString::fromStdString(scriptPair.second.name);

      if (IsWarlockScript(scriptName))
      {
         // If there is at least 1 Warlock script present, don't show instruction on how to make scripts appear
         mUi.emptyMessageLabel->hide();

         auto* item = new QTreeWidgetItem();
         item->setText(0, RemoveWarlockPrefix(scriptName));
         item->setData(0, PLATFORM_NAME, ""); // empty name indicates Global script
         item->setData(0, SCRIPT_NAME, scriptName);

         for (const auto& type : scriptPair.second.argTypes)
         {
            if (!warlock::script::IsValidType(type))
            {
               item->setFlags(Qt::NoItemFlags);
               item->setToolTip(0, "Invalid script arguments");
               break;
            }
         }

         mGlobalTopLevelItem->addChild(item);

         globalScriptsAdded = true;
      }
   }

   if (globalScriptsAdded)
   {
      mUi.treeWidget->addTopLevelItem(mGlobalTopLevelItem);
   }
   else
   {
      // if no items were added to the global item, then delete it
      delete mGlobalTopLevelItem;
      mGlobalTopLevelItem = nullptr;
   }

   mUi.treeWidget->expandAll();
}

void WkScriptBrowser::DockWidget::PopulatePlatformTableWidget(const QString& aPlatformName)
{
   // If the node exist already, then repopulate it as data may have changed.
   // This happened when an XIO request for data is being handled.
   RemovePlatformTree(aPlatformName);

   // We are guaranteed that there is not a QTreeWidgetItem for aPlatform name, due to the call
   //  to RemovePlatformTree(), therefore we need to create a new item and populate it
   auto* item = new QTreeWidgetItem();
   item->setText(0, aPlatformName);
   mUi.treeWidget->addTopLevelItem(item);
   mPlatformTopLevelItemMap[aPlatformName] = item;

   warlock::script::DataMap data;
   if (mSimInterfacePtr->GetAllPlatformScriptData(aPlatformName.toStdString(), data))
   {
      for (const auto& scriptPair : data)
      {
         QString scriptName = QString::fromStdString(scriptPair.second.name);
         if (IsWarlockScript(scriptName))
         {
            // If there is at least 1 Warlock script present, don't show instruction on how to make scripts appear
            mUi.emptyMessageLabel->hide();

            // Check to see if there are any invalid script arguments
            QString tooltip;
            for (const auto& type : scriptPair.second.argTypes)
            {
               if (!warlock::script::IsValidType(type))
               {
                  tooltip = "Invalid script arguments";
                  break;
               }
            }

            auto* item = new QTreeWidgetItem();
            item->setText(0, RemoveWarlockPrefix(scriptName));
            item->setData(0, PLATFORM_NAME, aPlatformName);
            item->setData(0, SCRIPT_NAME, scriptName);

            if (!tooltip.isEmpty())
            {
               item->setFlags(Qt::NoItemFlags);
               item->setToolTip(0, tooltip);
            }

            mPlatformTopLevelItemMap.at(aPlatformName)->addChild(item);
         }
      }
      mUi.treeWidget->expandAll();
   }
}

void WkScriptBrowser::DockWidget::PopulateScrollArea(const warlock::script::Data& aData, QString aPlatformName)
{
   QString scriptName = QString::fromStdString(aData.name);
   mUi.scriptNameLabel->setText(RemoveWarlockPrefix(scriptName));

   auto* groupBoxLayout = dynamic_cast<QVBoxLayout*>(mUi.scrollAreaWidgetContents->layout());
   if (groupBoxLayout != nullptr)
   {
      mArgWidget           = new QWidget();
      auto* argumentLayout = new QFormLayout();
      argumentLayout->setContentsMargins(0, 0, 0, 0);

      if (aData.argNames.size() == aData.argTypes.size()) // Should always be true
      {
         for (size_t i = 0; i < aData.argNames.size(); ++i)
         {
            QLabel*     label      = new QLabel(QString::fromStdString(aData.argNames[i]), mArgWidget);
            DataWidget* scriptData = new DataWidget(QString::fromStdString(aData.argTypes[i]), mArgWidget);
            argumentLayout->addRow(label, scriptData);
            mScriptArgWidgets.push_back(scriptData);
         }
      }

      mArgWidget->setLayout(argumentLayout);
      groupBoxLayout->insertWidget(0, mArgWidget);

      mUi.executePushButton->setProperty("script", scriptName);
      mUi.executePushButton->setProperty("platform", aPlatformName);

      mUi.scrollArea->setVisible(true);
      mUi.scriptNameLabel->setVisible(true);
      SetShowReturnValue(!warlock::script::IsVoidType(aData.returnType));
   }
}

bool WkScriptBrowser::DockWidget::IsWarlockScript(const QString& aScriptName) const
{
   return aScriptName.startsWith(mScriptPrefix);
}

QString WkScriptBrowser::DockWidget::RemoveWarlockPrefix(const QString& aScriptName) const
{
   QString retVal = aScriptName;
   if (IsWarlockScript(aScriptName))
   {
      retVal.remove(0, mScriptPrefix.length());
   }
   return retVal;
}

WkScriptBrowser::DataWidget::DataWidget(const QString& aType, QWidget* parent /*= 0*/)
   : QWidget(parent)
   , mType(warlock::script::DataTypeFromString(aType))
   , mWidget(nullptr)
   , mTypeStr(aType)
{
   auto* hLayout = new QHBoxLayout();
   hLayout->setContentsMargins(0, 0, 0, 0);

   switch (mType)
   {
   case warlock::script::cINTEGER:
   case warlock::script::cDOUBLE:
   case warlock::script::cSTRING:
   {
      mWidget = new QLineEdit(this);
      break;
   }
   case warlock::script::cBOOL:
   {
      mWidget = new QCheckBox(this);
      break;
   }
   case warlock::script::cWSF_GEOPOINT:
   {
      mWidget = new wkf::LocationWidget(this);
      break;
   }
   case warlock::script::cWSF_PLATFORM:
   {
      mWidget = new wkf::PlatformWidget(this);
      break;
   }
   case warlock::script::cWSF_TRACK:
   {
      mWidget = new wkf::TrackWidget(this);
      break;
   }
   case warlock::script::cARRAY_INTEGER:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cINTEGER);
      break;
   }
   case warlock::script::cARRAY_DOUBLE:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cDOUBLE);
      break;
   }
   case warlock::script::cARRAY_STRING:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cSTRING);
      break;
   }
   case warlock::script::cARRAY_BOOL:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cBOOL);
      break;
   }
   case warlock::script::cARRAY_WSF_GEOPOINT:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cWSF_GEOPOINT);
      break;
   }
   case warlock::script::cARRAY_WSF_PLATFORM:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cWSF_PLATFORM);
      break;
   }
   case warlock::script::cARRAY_WSF_TRACK:
   {
      mWidget = new warlock::ArrayWidget(this, warlock::script::cWSF_TRACK);
      break;
   }
   default:
      break;
   }

   if (mWidget != nullptr)
   {
      hLayout->addWidget(mWidget);
   }
   setLayout(hLayout);
}

WkScriptBrowser::DataWidget::~DataWidget()
{
   if (mWidget)
   {
      delete mWidget;
   }
}

bool WkScriptBrowser::DataWidget::GetBool() const
{
   if (mType == warlock::script::cBOOL)
   {
      auto* checkBox = dynamic_cast<QCheckBox*>(mWidget);
      return checkBox->isChecked();
   }
   return false;
}

int WkScriptBrowser::DataWidget::GetInt() const
{
   if (mType == warlock::script::cINTEGER)
   {
      auto* lineEdit = dynamic_cast<QLineEdit*>(mWidget);
      return lineEdit->text().toDouble();
   }
   return 0;
}

QString WkScriptBrowser::DataWidget::GetString() const
{
   if (mType == warlock::script::cSTRING)
   {
      auto* lineEdit = dynamic_cast<QLineEdit*>(mWidget);
      return lineEdit->text();
   }
   return "";
}

double WkScriptBrowser::DataWidget::GetDouble() const
{
   if (mType == warlock::script::cDOUBLE)
   {
      auto* lineEdit = dynamic_cast<QLineEdit*>(mWidget);
      return lineEdit->text().toDouble();
   }
   return 0.0;
}

QVector<double> WkScriptBrowser::DataWidget::GetLocation() const
{
   if (mType == warlock::script::cWSF_GEOPOINT)
   {
      auto* locationWidget = dynamic_cast<wkf::LocationWidget*>(mWidget);
      return locationWidget->GetLocation();
   }
   return {};
}

QString WkScriptBrowser::DataWidget::GetPlatform() const
{
   if (mType == warlock::script::cWSF_PLATFORM)
   {
      auto* platformWidget = dynamic_cast<wkf::PlatformWidget*>(mWidget);
      return platformWidget->GetPlatformName();
   }
   return "";
}

wkf::TrackId WkScriptBrowser::DataWidget::GetTrackId() const
{
   if (mType == warlock::script::cWSF_TRACK)
   {
      auto* trackWidget = dynamic_cast<wkf::TrackWidget*>(mWidget);
      return trackWidget->GetTrackId();
   }
   return wkf::TrackId();
}

QVector<QVariant> WkScriptBrowser::DataWidget::GetArray() const
{
   if (warlock::script::IsArrayType(mType))
   {
      auto* arrayWidget = dynamic_cast<warlock::ArrayWidget*>(mWidget);
      return arrayWidget->GetArray();
   }
   return {};
}

void WkScriptBrowser::DataWidget::CancelSelection()
{
   if (warlock::script::IsArrayType(mType))
   {
      auto* arrayWidget = dynamic_cast<warlock::ArrayWidget*>(mWidget);
      if (arrayWidget != nullptr)
      {
         emit arrayWidget->CancelSelection();
      }
   }
}

warlock::script::Argument WkScriptBrowser::DataWidget::GetScriptArg()
{
   warlock::script::Argument retVal;
   retVal.type = mTypeStr.toStdString();

   switch (mType)
   {
   case warlock::script::cBOOL:
      retVal.data = GetBool();
      break;
   case warlock::script::cDOUBLE:
      retVal.data = GetDouble();
      break;
   case warlock::script::cINTEGER:
      retVal.data = GetInt();
      break;
   case warlock::script::cSTRING:
      retVal.data = GetString();
      break;
   case warlock::script::cWSF_GEOPOINT:
      retVal.data = QVariant::fromValue(GetLocation());
      break;
   case warlock::script::cWSF_PLATFORM:
      retVal.data = GetPlatform();
      break;
   case warlock::script::cWSF_TRACK:
      retVal.data = QVariant::fromValue(GetTrackId());
      break;
   case warlock::script::cARRAY_INTEGER:
   case warlock::script::cARRAY_DOUBLE:
   case warlock::script::cARRAY_STRING:
   case warlock::script::cARRAY_BOOL:
   case warlock::script::cARRAY_WSF_PLATFORM:
   case warlock::script::cARRAY_WSF_TRACK:
   case warlock::script::cARRAY_WSF_GEOPOINT:
      retVal.data = QVariant::fromValue(GetArray());
      break;
   default:
      retVal.data = 0;
   }
   return retVal;
}
