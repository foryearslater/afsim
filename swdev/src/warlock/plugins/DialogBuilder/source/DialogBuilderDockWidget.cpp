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
#include "DialogBuilderDockWidget.hpp"

#include <algorithm>
#include <iterator>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>

#include "DialogBuilderResultsDialog.hpp"
#include "WkArrayWidget.hpp"
#include "WkPermissions.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatform.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WkfScenario.hpp"
#include "WkfSelectorWidgets.hpp"
#include "WkfUtils.hpp"
#include "WkfVtkEnvironment.hpp"

namespace
{
const QString cUSE_POI = "$CURRENT_PLATFORM";
}

QPoint WkDialogBuilder::DockWidget::mResultsDialogPosition      = QPoint(0, 0);
bool   WkDialogBuilder::DockWidget::mResultsDialogPositionValid = false;

WkDialogBuilder::DockWidget::DockWidget(const DialogDef&        aDialogDefinition,
                                        ScriptInterfacePtr      aSimInterfacePtr,
                                        ScriptInputsDockWidget* aPromptDockWidget,
                                        QWidget*                parent /*= nullptr*/,
                                        Qt::WindowFlags         f /*= Qt::WindowFlags()*/)
   : QDockWidget(parent, f)
   , mSimInterfacePtr(aSimInterfacePtr)
   , mPromptDockWidget(aPromptDockWidget)
   , mUI()
{
   mUI.setupUi(this);
   SetDefinition(aDialogDefinition);

   connect(&wkfEnv, &wkf::Environment::PlatformSelectionChanged, this, &DockWidget::PlatformSelectionChanged);

   connect(mSimInterfacePtr.get(),
           &warlock::ScriptSimInterface::ReturnValueReady,
           this,
           &DockWidget::ReturnValueReadyCB,
           Qt::QueuedConnection);
}

WkDialogBuilder::DockWidget::~DockWidget()
{
   auto prefObject = wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>();
   if (prefObject)
   {
      prefObject->RemoveActions(mActions);
   }
}

void WkDialogBuilder::DockWidget::SetDefinition(const DialogDef& aDialogDefinition)
{
   Clear();

   setWindowTitle(aDialogDefinition.name);

   auto* layout = dynamic_cast<QGridLayout*>(mUI.gridWidget->layout());
   if (layout)
   {
      int index = 0;
      for (auto& script : aDialogDefinition.scripts)
      {
         int  scriptId = index;
         auto button   = new QPushButton(this);
         button->setProperty("id", index);
         mScriptData[scriptId] = script;
         ++index;

         button->setAutoDefault(true);

         // Set the Button Size
         ApplyButtonSize(button, aDialogDefinition.buttonSize);

         // Set Name or Icon
         ApplyButtonNameAndIcon(button, script.displayName, script.icon);

         // Set the button color
         ApplyButtonColor(button, script.color);

         connect(button, &QPushButton::clicked, this, &DockWidget::HandleButtonClicked);

         // Create wkf::Action to enable keybindings
         wkf::Action* shortcutAction = new wkf::Action(script.displayName, button, Qt::ApplicationShortcut);
         connect(shortcutAction, &QAction::triggered, this, [&, scriptId]() { HandleScriptExecution(scriptId); });
         mActions.push_back(shortcutAction);
         // enable key bindings even when dialog is hidden: although not a parent, qt should handle the removal of the
         // action from the mainwindow
         wkfEnv.GetMainWindow()->addAction(shortcutAction);
         layout->addWidget(button, script.row, script.col);
         mButtons.push_back(button);
      }
      // Update the button states to match the platform selection state
      UpdatePlatformSelection();
      // Load in key bindings that were already saved to preferences
      wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->AddActions(mActions, aDialogDefinition.name);
      wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->UpdateActionKeyBindings(mActions);
   }
}

void WkDialogBuilder::DockWidget::PlatformSelectionChanged(const wkf::PlatformList& aSelected,
                                                           const wkf::PlatformList& aUnselected)
{
   for (const auto& wkfPlatform : aSelected)
   {
      auto* platform = dynamic_cast<warlock::Platform*>(wkfPlatform);
      if (platform->GetExternallyControlled())
      {
         mSimInterfacePtr->RequestXIO_ScriptInfo(platform->GetIndex());
      }
   }
   UpdatePlatformSelection();
}

void WkDialogBuilder::DockWidget::UpdatePlatformSelection()
{
   bool platformScopeValid = !wkfEnv.GetSelectedPlatforms().empty();
   for (auto& i : mButtons)
   {
      auto iter = mScriptData.find(i->property("id").toInt());
      if (iter != mScriptData.end())
      {
         if (iter->second.context == warlock::script::PLATFORM)
         {
            // If no platform is selected disable
            if (!platformScopeValid)
            {
               i->setEnabled(false);
            }
            // if a platform is selected and platform type has been specified
            else if (iter->second.platformType != "")
            {
               bool enable = true;
               // get all selected platforms and make sure they match the specified type,
               //  enable only if all platforms match the type.
               auto platformList = wkfEnv.GetSelectedPlatforms();
               for (auto& p : platformList)
               {
                  if (!p->IsOfType(iter->second.platformType.toStdString()))
                  {
                     enable = false;
                     break;
                  }
               }
               i->setEnabled(enable);
            }
            // if a platform is selected and no type has been specified
            else
            {
               i->setEnabled(true);
            }
         }
      }
   }
}

void WkDialogBuilder::DockWidget::ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue)
{
   // if aScriptId is in the set of scripts I have commanded
   if (mScriptsWaitingForReturnValues.count(aScriptId) > 0)
   {
      mScriptsWaitingForReturnValues.erase(aScriptId);

      auto type = aReturnValue.returnValue.GetType();
      if (type != ut::script::Data::Type::cUNDEFINED)
      {
         auto& dialog = ResultsDialog::MakeResultsDialog(this,
                                                         QString::fromStdString(aReturnValue.scriptName),
                                                         QString::fromStdString(aReturnValue.platformName));

         QStringList returnValue = warlock::script::UtScriptDataToString(aReturnValue.returnValue);
         for (const auto& r : returnValue)
         {
            dialog.AddReturnValue(r);
         }

         dialog.SetPosition(&mResultsDialogPosition, &mResultsDialogPositionValid);

         dialog.show();
      }
   }
}

void WkDialogBuilder::DockWidget::Clear()
{
   wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->RemoveActions(mActions);
   mActions.clear();
   for (auto& button : mButtons)
   {
      delete button;
   }
   mButtons.clear();
   mScriptData.clear();
   mScriptsWaitingForReturnValues.clear();
}

QVariant WkDialogBuilder::DockWidget::GetValue(const QString& aArgType, const QVariant& aData)
{
   if (aData.toString().isEmpty())
   {
      return QVariant();
   }

   QStringList arr = aData.toString().split(";");
   switch (warlock::script::DataTypeFromString(aArgType))
   {
   case warlock::script::cBOOL:
   case warlock::script::cDOUBLE:
   case warlock::script::cINTEGER:
   case warlock::script::cSTRING:
   case warlock::script::cWSF_PLATFORM:
   case warlock::script::cWSF_SENSOR:
   case warlock::script::cWSF_WEAPON:
   case warlock::script::cWSF_COMM:
      return aData;
   case warlock::script::cWSF_TRACK:
   {
      wkf::TrackId trackId(aData.toString());
      if (!trackId.IsNull())
      {
         return QVariant::fromValue(trackId);
      }
      return QVariant();
   }
   case warlock::script::cWSF_GEOPOINT:
   {
      UtLLAPos          llaPos;
      std::stringstream llaStr;

      llaStr << aData.toString().toStdString();
      llaStr >> llaPos;

      if (!llaStr.fail())
      {
         QVector<double> lla;
         lla.push_back(llaPos.GetLat());
         lla.push_back(llaPos.GetLon());
         lla.push_back(llaPos.GetAlt());
         return QVariant::fromValue(lla);
      }
      return QVariant();
   }
   case warlock::script::cARRAY_INTEGER:
   case warlock::script::cARRAY_DOUBLE:
   {
      QVector<QVariant> vec;
      for (const auto& a : arr)
      {
         if (!a.isEmpty())
         {
            vec.push_back(a.toDouble());
         }
      }
      return QVariant::fromValue(vec);
   }
   case warlock::script::cARRAY_BOOL:
   {
      QVector<QVariant> vec;
      for (const auto& a : arr)
      {
         if (QString::compare(a, "true") == 0)
         {
            vec.push_back(true);
         }
         else if (QString::compare(a, "false") == 0)
         {
            vec.push_back(false);
         }
         else if (!a.isEmpty())
         {
            vec.push_back(a.toInt());
         }
      }
      return QVariant::fromValue(vec);
   }
   case warlock::script::cARRAY_STRING:
   case warlock::script::cARRAY_WSF_PLATFORM:
   case warlock::script::cARRAY_WSF_SENSOR:
   case warlock::script::cARRAY_WSF_WEAPON:
   case warlock::script::cARRAY_WSF_COMM:
   case warlock::script::cARRAY_WSF_GEOPOINT:
   {
      QVector<QVariant> vec;
      for (const auto& a : arr)
      {
         if (!a.isEmpty())
         {
            vec.push_back(a.trimmed()); // trimmed() removes trailing and leading whitespace
         }
      }
      return QVariant::fromValue(vec);
   }
   case warlock::script::cARRAY_WSF_TRACK:
   {
      QVector<QVariant> vec;
      for (const auto& a : arr)
      {
         wkf::TrackId trackId(a.trimmed()); // trimmed() removes trailing and leading whitespace
         if (!trackId.IsNull())
         {
            vec.push_back(QVariant::fromValue(trackId));
         }
         else if (!a.isEmpty())
         {
            vec.push_back(QVariant());
         }
      }
      return QVariant::fromValue(vec);
   }
   default:
      return QVariant();
   }
}


bool WkDialogBuilder::DockWidget::BuildScriptInstance(const ScriptDefinition&    aDefinition,
                                                      warlock::script::Instance& aScriptInstance)
{
   bool scriptFound = false;

   // PlatformName is filled in later based on selection within Warlock
   aScriptInstance.platformName = "";

   // Get the script information based on the context of the script
   if (aDefinition.context == warlock::script::PLATFORM)
   {
      auto list = wkfEnv.GetSelectedPlatforms();

      if (!list.empty())
      {
         scriptFound = mSimInterfacePtr->GetPlatformScriptData((*list.begin())->GetName(),
                                                               aDefinition.scriptName.toStdString(),
                                                               aScriptInstance.script);
         aScriptInstance.script.clampPlatformSelection = aDefinition.clampPlatformSelection;
      }
   }
   else
   {
      scriptFound = mSimInterfacePtr->GetGlobalScriptData(aDefinition.scriptName.toStdString(), aScriptInstance.script);
   }

   if (scriptFound)
   {
      // If the script does not prompt the user for input, make sure the script data in the simulation
      //  has the same number of arguments as the script definition in the user created dialog.
      if (!aDefinition.promptUser && ut::cast_to_int(aScriptInstance.script.argTypes.size()) != aDefinition.args.size())
      {
         int simCount = static_cast<int>(aScriptInstance.script.argTypes.size());
         int guiCount = aDefinition.args.size();

         if (simCount > guiCount)
         {
            QMessageBox::warning(this,
                                 "Argument Count",
                                 QString("The number of arguments provided in the script definition is less than the "
                                         "number of arguments for the script in the simulation.\n") +
                                    "Default values have been used for the remaining arguments.\n" +
                                    "Argument Count in script definition : " + QString::number(guiCount) +
                                    "\nArgument Count in simulation script : " + QString::number(simCount));
         }
         else
         {
            QMessageBox::warning(this,
                                 "Argument Count",
                                 QString("The number of arguments provided in the script definition is greater than "
                                         "the number of arguments for the script in the simulation.\n") +
                                    "The additional arguments were ignored.\n" +
                                    "Argument Count in script definition : " + QString::number(guiCount) +
                                    "\nArgument Count in simulation script : " + QString::number(simCount));
         }
      }

      int count = 0;
      // Get the argument's type from the script data return by the simulation (not what the user inputs into the Dialog
      // Builder plugin)
      for (auto& i : aScriptInstance.script.argTypes)
      {
         warlock::script::Argument arg;
         arg.type = i;
         // If the user supplied a default argument, store that in the ScriptArgument
         if (count < aDefinition.args.size())
         {
            arg.data = GetValue(QString::fromStdString(arg.type), aDefinition.args.at(count).data);
            if (arg.type == "WsfPlatform" && arg.data.toString() == cUSE_POI && wkfEnv.GetPlatformOfInterest())
            {
               arg.data = QVariant(QString::fromStdString(wkfEnv.GetPlatformOfInterest()->GetName()));
            }
         }
         // Add the arguments to the ScriptInstance
         aScriptInstance.args.push_back(arg);
         ++count;
      }
      aScriptInstance.filters = aDefinition.filters;
   }
   return scriptFound;
}

void WkDialogBuilder::DockWidget::ExecuteScriptInstance(warlock::script::Instance& aInstance)
{
   if (aInstance.script.context == warlock::script::PLATFORM)
   {
      std::vector<std::string> platformSelectionList;
      if (aInstance.script.clampPlatformSelection)
      {
         platformSelectionList = mClampedPlatformList;
      }
      else
      {
         for (auto&& platform : wkfEnv.GetSelectedPlatforms())
         {
            platformSelectionList.push_back(platform->GetName());
         }
      }

      // If multiple platforms are selected, attempt to run the script on each of them
      for (const auto& platformName : platformSelectionList)
      {
         wkf::Scenario* scenario = vaEnv.GetStandardScenario();
         if (scenario)
         {
            wkf::Platform* plat = scenario->FindPlatformByName(platformName);
            if (plat)
            {
               if (!warlock::HasPermissionToControlPlatform(*plat))
               {
                  QMessageBox::warning(nullptr,
                                       "Script Not Executed",
                                       "User does not have permissions to modify platform " +
                                          QString::fromStdString(plat->GetName()) + ".");
               }
               else
               {
                  aInstance.platformName = plat->GetName();
                  mScriptsWaitingForReturnValues.insert(mSimInterfacePtr->ExecuteScript(aInstance));
               }
            }
         }
      }
   }
   else
   {
      mScriptsWaitingForReturnValues.insert(mSimInterfacePtr->ExecuteScript(aInstance));
   }
}

void WkDialogBuilder::DockWidget::PromptUserForArguments(const QString& aDisplayName, warlock::script::Instance& aInstance)
{
   QList<QWidget*> widgets;
   bool            ok = true;

   auto* prompt = new QWidget(this);
   prompt->setAttribute(Qt::WA_DeleteOnClose, true); // This is how the prompt along with all other "newed" QWidget in
                                                     // this function are cleaned up
   std::vector<std::string> clampedPlatformList;
   for (auto&& platform : wkfEnv.GetSelectedPlatforms())
   {
      clampedPlatformList.push_back(platform->GetName());
   }
   auto* vlayout = new QVBoxLayout();
   prompt->setLayout(vlayout);

   auto* glayout = new QGridLayout();
   vlayout->addLayout(glayout);
   vlayout->addStretch();

   for (int i = 0; i < static_cast<int>(aInstance.script.argNames.size()); ++i)
   {
      std::string argType = aInstance.script.argTypes[i];

      if (warlock::script::IsArrayType(argType))
      {
         auto* vlayout = new QVBoxLayout();
         auto* name    = new QLabel(QString::fromStdString(aInstance.script.argNames[i]));
         auto* type    = new QLabel("(" + QString::fromStdString(argType) + ")");

         vlayout->addWidget(name);
         vlayout->addWidget(type);
         vlayout->addStretch();

         glayout->addLayout(vlayout, i + 1, 0);
      }
      else
      {
         auto* label =
            new QLabel(QString::fromStdString(aInstance.script.argNames[i]) + "(" + QString::fromStdString(argType) + ")");
         glayout->addWidget(label, i + 1, 0);
      }

      if (argType == "WsfPlatform")
      {
         auto* platWidget = new wkf::PlatformWidget();
         widgets.push_back(platWidget);
         if (i < aInstance.args.size())
         {
            platWidget->SetPlatformName(aInstance.args.at(i).data.toString());
            auto filter = aInstance.filters.at(i);
            if (filter.isFiltered)
            {
               platWidget->SetFilter(filter.type, filter.value, filter.isInclusive);
            }
         }
         glayout->addWidget(platWidget, i + 1, 1);
      }
      else if (argType == "WsfTrack")
      {
         auto* trackWidget = new wkf::TrackWidget();
         widgets.push_back(trackWidget);
         if (i < aInstance.args.size())
         {
            auto trackId = aInstance.args.at(i).data.value<wkf::TrackId>();
            auto filter  = aInstance.filters.at(i);
            if (filter.isFiltered)
            {
               trackWidget->SetFilter(filter.type, filter.value, filter.isInclusive);
            }

            if (!trackId.GetPlatformName().empty())
            {
               trackWidget->SetTrackId(trackId);
            }
         }
         glayout->addWidget(trackWidget, i + 1, 1);
      }
      else if (argType == "WsfGeoPoint")
      {
         auto* locationWidget = new wkf::LocationWidget();
         widgets.push_back(locationWidget);
         if (i < aInstance.args.size())
         {
            QVector<double> location = aInstance.args.at(i).data.value<QVector<double>>();
            if (location.size() == 3)
            {
               locationWidget->SetLocation(location);
            }
         }
         glayout->addWidget(locationWidget, i + 1, 1);
      }
      else if (argType == "WsfSensor" || argType == "WsfWeapon" || argType == "WsfComm")
      {
         QComboBox*            comboBox = new QComboBox();
         std::set<std::string> namesIntersection; // intersection of part names
         std::set<std::string> names;             // names for one iteration
         std::set<std::string> temp;

         std::vector<std::string> selectedPlatforms;
         if (aInstance.script.clampPlatformSelection)
         {
            selectedPlatforms = clampedPlatformList;
         }
         else
         {
            for (auto&& platform : wkfEnv.GetSelectedPlatforms())
            {
               selectedPlatforms.push_back(platform->GetName());
            }
         }

         std::string defaultName = aInstance.args.at(i).data.toString().toStdString();

         for (const auto& platform : selectedPlatforms)
         {
            wkf::Scenario* scenario = vaEnv.GetStandardScenario();
            if (scenario)
            {
               if (namesIntersection.empty())
               {
                  if (argType == "WsfSensor")
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetSensorNames();
                  }
                  else if (argType == "WsfWeapon")
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetWeaponNames();
                  }
                  else // argType == WsfComm
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetCommNames();
                  }
               }
               else
               {
                  if (argType == "WsfSensor")
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetSensorNames();
                  }
                  else if (argType == "WsfWeapon")
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetWeaponNames();
                  }
                  else // argType == WsfComm
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetCommNames();
                  }

                  std::set_intersection(namesIntersection.begin(),
                                        namesIntersection.end(),
                                        names.begin(),
                                        names.end(),
                                        std::inserter(temp, temp.begin()));

                  std::swap(namesIntersection, temp);
                  temp.clear();
               }
            }

            // if intersection is null set after operation, then will always be null set
            if (namesIntersection.empty())
            {
               break;
            }
         }

         for (auto& partName : namesIntersection)
         {
            comboBox->addItem(QString::fromStdString(partName));

            if (partName == defaultName)
            {
               comboBox->setCurrentText(QString::fromStdString(partName));
            }
         }

         widgets.push_back(comboBox);
         glayout->addWidget(comboBox, i + 1, 1);
      }
      else if (argType == "Array<WsfSensor>" || argType == "Array<WsfWeapon>" || argType == "Array<WsfComm>")
      {
         std::set<std::string> names, namesIntersection, temp;
         auto                  defaultPartNames = aInstance.args.at(i).data.value<QVector<QVariant>>();
         auto*                 listWidget       = new QListWidget();
         auto*                 vlayout          = new QVBoxLayout();
         listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

         std::vector<std::string> selectedPlatforms;
         if (aInstance.script.clampPlatformSelection)
         {
            selectedPlatforms = clampedPlatformList;
         }
         else
         {
            for (auto&& platform : wkfEnv.GetSelectedPlatforms())
            {
               selectedPlatforms.push_back(platform->GetName());
            }
         }

         for (const auto& platform : selectedPlatforms)
         {
            wkf::Scenario* scenario = vaEnv.GetStandardScenario();
            if (scenario)
            {
               if (namesIntersection.empty())
               {
                  if (argType == "Array<WsfSensor>")
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetSensorNames();
                  }
                  else if (argType == "Array<WsfWeapon>")
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetWeaponNames();
                  }
                  else // argType == Array<WsfComm>
                  {
                     namesIntersection =
                        dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetCommNames();
                  }
               }
               else
               {
                  if (argType == "Array<WsfSensor>")
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetSensorNames();
                  }
                  else if (argType == "Array<WsfWeapon>")
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetWeaponNames();
                  }
                  else // argType == Array<WsfComm>
                  {
                     names = dynamic_cast<warlock::Platform*>(scenario->FindPlatformByName(platform))->GetCommNames();
                  }

                  std::set_intersection(namesIntersection.begin(),
                                        namesIntersection.end(),
                                        names.begin(),
                                        names.end(),
                                        std::inserter(temp, temp.begin()));

                  std::swap(namesIntersection, temp);
                  temp.clear();
               }
            }

            // if intersection is null set after operation, then will always be null set
            if (namesIntersection.empty())
            {
               break;
            }
         }

         for (const auto& name : namesIntersection)
         {
            auto* item = new QListWidgetItem(QString::fromStdString(name));
            listWidget->addItem(item);

            if (defaultPartNames.contains(QVariant::fromValue(QString::fromStdString(name))))
            {
               listWidget->setCurrentItem(item, QItemSelectionModel::SelectionFlag::SelectCurrent);
            }
         }

         QFont font;
         auto* hint = new QLabel(QString("Ctrl+click to select multiple"));
         font.setPointSize(7);
         hint->setAlignment(Qt::AlignmentFlag::AlignCenter);
         hint->setFont(font);

         vlayout->addWidget(listWidget);
         vlayout->addWidget(hint);
         vlayout->addStretch();
         glayout->addLayout(vlayout, i + 1, 1);

         widgets.push_back(listWidget);
      }
      else if (warlock::script::IsArrayType(argType))
      {
         auto* arrayWidget = new warlock::ArrayWidget(nullptr, warlock::script::BaseDataTypeFromString(argType.c_str()));
         widgets.push_back(arrayWidget);

         if (i < aInstance.args.size())
         {
            arrayWidget->Populate(aInstance.args.at(i).data.value<QVector<QVariant>>());
         }
         glayout->addWidget(arrayWidget, i + 1, 1);
      }
      else
      {
         auto* lineEdit = new QLineEdit();
         widgets.push_back(lineEdit);
         if (i < aInstance.args.size())
         {
            lineEdit->setText(aInstance.args.at(i).data.toString());
         }
         glayout->addWidget(lineEdit, i + 1, 1);
      }
   }

   if (!ok)
   {
      return;
   }

   auto executeButton = new QPushButton("Execute", this);
   auto hLayout       = new QHBoxLayout(this);
   hLayout->addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

   connect(executeButton,
           &QPushButton::clicked,
           this,
           [this, prompt, widgets, aInstance, clampedPlatformList]
           {
              warlock::script::Instance s     = aInstance;
              int                       count = 0;
              for (auto& i : widgets)
              {
                 QVariant variant;
                 auto*    lineEdit = dynamic_cast<QLineEdit*>(i);
                 if (lineEdit != nullptr)
                 {
                    variant = lineEdit->text();
                 }
                 else
                 {
                    auto* platform   = dynamic_cast<wkf::PlatformWidget*>(i);
                    auto* point      = dynamic_cast<wkf::LocationWidget*>(i);
                    auto* track      = dynamic_cast<wkf::TrackWidget*>(i);
                    auto* arr        = dynamic_cast<warlock::ArrayWidget*>(i);
                    auto* comboBox   = dynamic_cast<QComboBox*>(i);
                    auto* listWidget = dynamic_cast<QListWidget*>(i);
                    if (platform != nullptr)
                    {
                       variant = platform->GetPlatformName();
                    }
                    else if (point != nullptr)
                    {
                       if (point->LocationValid())
                       {
                          variant = QVariant::fromValue(point->GetLocation());
                       }
                       else
                       {
                          variant = QVariant();
                       }
                    }
                    else if (track != nullptr)
                    {
                       variant = QVariant::fromValue(track->GetTrackId());
                    }
                    else if (arr != nullptr)
                    {
                       variant = QVariant::fromValue(arr->GetArray());
                    }
                    else if (comboBox != nullptr)
                    {
                       variant = QVariant::fromValue(comboBox->currentText());
                    }
                    else if (listWidget != nullptr)
                    {
                       QVector<QVariant> arr;
                       auto              selectedItems = listWidget->selectedItems();

                       for (const auto* item : selectedItems)
                       {
                          QString text = item->text();
                          arr.push_back(item->text());
                       }

                       variant = QVariant::fromValue(arr);
                    }
                    else
                    {
                       // ERROR
                    }
                 }

                 // If the list of arguments already has the argument in it
                 if (s.args.size() > count)
                 {
                    s.args[count].data = variant;
                 }
                 else
                 {
                    // If there is no argument provide for the function.
                    //  This can occur when the user does not provide arguments for functions, then we will prompt for
                    //  all arguments, which means we will need to create a ScriptArgument for each response form the
                    //  user.
                    warlock::script::Argument arg;
                    arg.data = variant;
                    s.args.push_back(arg);
                 }
                 count++;
              }
              mClampedPlatformList = clampedPlatformList;
              ExecuteScriptInstance(s);
              mPromptDockWidget->RemoveUserPrompt();
           });

   hLayout->addWidget(executeButton);
   vlayout->addLayout(hLayout);
   vlayout->addSpacerItem(new QSpacerItem(1, 20, QSizePolicy::Fixed, QSizePolicy::Expanding));
   mPromptDockWidget->AddUserPrompt(prompt, aDisplayName, !warlock::script::IsVoidType(aInstance.script.returnType));
}

void WkDialogBuilder::DockWidget::HandleButtonClicked()
{
   int id = sender()->property("id").toInt();
   HandleScriptExecution(id);
}

void WkDialogBuilder::DockWidget::HandleScriptExecution(int aId)
{
   auto iter = mScriptData.find(aId);
   if (iter != mScriptData.end())
   {
      ScriptDefinition sDefinition = iter->second;
      if (sDefinition.scriptName.isEmpty() && sDefinition.link.isEmpty())
      {
         QMessageBox::warning(this,
                              "Invalid Display",
                              sDefinition.displayName + QString(" does not have a script or link associated with it."));
      }
      else
      {
         if (!sDefinition.scriptName.isEmpty())
         {
            bool                      okToRun = true;
            warlock::script::Instance sInstance;
            if (BuildScriptInstance(sDefinition, sInstance))
            {
               for (auto& i : sInstance.script.argTypes)
               {
                  if (!warlock::script::IsValidType(i))
                  {
                     QMessageBox::warning(this,
                                          "Invalid Argument Type(s)",
                                          sDefinition.scriptName +
                                             QString(" contains one or more unsupported argument types."));
                     okToRun = false;
                     break;
                  }

                  if (sInstance.script.context == warlock::script::GLOBAL &&
                      warlock::script::IsSupportedPlatformPartType(i))
                  {
                     QMessageBox::warning(
                        this,
                        "Invalid Argument Type(s)",
                        sDefinition.scriptName +
                           QString(
                              " contains one or more argument types that are not supported in the global context."));
                     okToRun = false;
                     break;
                  }
               }
               if (okToRun)
               {
                  if (sDefinition.promptUser)
                  {
                     // If we prompt the user for arguments, when the user is done setting argument,
                     //  the dialog shown is responsible for calling ExecuteScript.
                     PromptUserForArguments(sDefinition.displayName, sInstance);
                  }
                  else
                  {
                     ExecuteScriptInstance(sInstance);
                  }
               }
            }
            else
            {
               QString errorString;
               if (sDefinition.context == warlock::script::GLOBAL)
               {
                  errorString = "The script '" + sDefinition.scriptName + "' was not found in the global context.";
               }
               else
               {
                  errorString = "The script '" + sDefinition.scriptName + "' was not found on the selected platform.";
               }
               QMessageBox::warning(this, "Script Not Found", errorString);
            }
         }
         // Links can change the POI, so handle the links after script instance execution
         if (!sDefinition.link.isEmpty())
         {
            QString errorMsg;
            QString link = wkf::util::ParseLinks(sDefinition.link, false);
            if (!wkf::util::FollowLink(link, errorMsg))
            {
               QFileInfo fi(wkfEnv.GetDocumentationDir());
               if (fi.isDir())
               {
                  QString dir(fi.absoluteFilePath().append("/html/docs/wkf_plugin/wk_logs_chat.html"));
                  errorMsg +=
                     "Refer to the <a href=\"" + dir + "\">link documentation</a> for a list of accepted commands.";
               }
               QMessageBox::warning(this, "Link Error", errorMsg);
            }
         }
      }
   }
}
