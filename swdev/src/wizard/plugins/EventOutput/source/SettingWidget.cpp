// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SettingWidget.hpp"

#include <QDesktopServices>
#include <QFileInfo>
#include <QMenu>
#include <QUrl>

#include "CommandValuePair.hpp"
#include "ParseResults.hpp"
#include "PluginUtil.hpp"
#include "Project.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPrefObject.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfExe.hpp"
#include "WsfParser.hpp"

namespace wizard
{
namespace EventOutput
{
namespace setting
{

Item::Item(QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mSetting(Data{})
{
}

Item::Item(Data aSetting, QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mSetting(std::move(aSetting))
   , mAddToBlock(ut::qt::make_qt_ptr<QCheckBox>(this))
   , mCommand(ut::qt::make_qt_ptr<QLabel>(mSetting.mCommand, this))
   , mValueWidget(DetermineValueWidget())
{
   // set up tooltips because they are important...
   mAddToBlock->setToolTip("Do not write the setting command out to the block.");
   mCommand->setToolTip("The name of the setting command (" + mCommand->text() + ')');
   // make sure that a context menu appears
   mCommand->setContextMenuPolicy(Qt::CustomContextMenu);
   // the value widget is disabled on default
   // note:  to enable it, check the "add to block"
   mValueWidget->Disable();

   // do the necessary connections
   connect(mCommand.data(), &QWidget::customContextMenuRequested, this, &Item::OnContextMenuRequest);
   connect(mAddToBlock.data(),
           &QCheckBox::stateChanged,
           [this](const int aState)
           {
              // toggle the ableness of the value widget based on the state of the "add to block"
              switch (aState)
              {
              case Qt::Checked:
                 mValueWidget->Enable();
                 mAddToBlock->setToolTip("Write the setting command out to the block.");
                 break;
              case Qt::Unchecked:
                 mAddToBlock->setToolTip("Do not write the setting command out to the block.");
                 mValueWidget->Disable();
                 break;
              default:
                 break;
              }
              emit ModifiedByUser();
           });
   connect(mValueWidget.data(), &ValueWidget::WidgetInterface::ModifiedByUser, this, &Item::ModifiedByUser);
}

void Item::ReadSetting(const editor::tokenizer::Tokenizer* aRange)
{
   // retrieve the appropriate command-value pair
   const auto* settingToken = aRange->GetSetting(mSetting.mCommand);
   // read the value if the command-value pair exists and is valid
   if (settingToken && settingToken->IsValid())
   {
      mSetting.mValue = ut::make_optional<QString>(settingToken->GetValue());
      // check the "add to block"
      mAddToBlock->setChecked(true);
      mValueWidget->ReadValue(settingToken);
   }
}

void Item::WriteSetting(editor::tokenizer::Tokenizer* aRange)
{
   // retrieve the appropriate command-value pair
   const auto* settingToken = aRange->GetSetting(mSetting.mCommand);
   // check for existence and validity of the command-value pair
   bool validToken{settingToken && settingToken->IsValid()};
   // the setting warrants writing to the block
   if (mAddToBlock->isChecked())
   {
      // the command-value pair is already in the block, but needs updating
      if (validToken)
      {
         // copy the command-value pair
         editor::token::SettingCVP oldSetting = *static_cast<const editor::token::SettingCVP*>(settingToken);
         // and update its value
         mValueWidget->WriteValue(&oldSetting);
         // then, update the range
         aRange->SetSetting(oldSetting.GetCommand(), oldSetting.GetValue());
      }
      // the setting warrants insertion into the block
      else
      {
         // create a new command-value pair
         editor::token::SettingCVP newSetting;
         newSetting.SetCommand(mSetting.mCommand);
         mValueWidget->WriteValue(&newSetting);
         // insert the command-value pair into the block
         aRange->InsertSetting(newSetting.GetCommand(), newSetting.GetValue());
      }
   }
   // the setting warrants removal from the block
   else if (validToken)
   {
      aRange->RemoveSetting(settingToken->GetCommand());
   }
}

void Item::RestoreDefault()
{
   mAddToBlock->setChecked(false);
   mValueWidget->RestoreDefault();
}

void Item::AddToTable(QTableWidget* aTable)
{
   int rowIndex{aTable->rowCount()};
   aTable->insertRow(rowIndex);
   // set the widget for each cell in the row
   aTable->setCellWidget(rowIndex, 0, mAddToBlock);
   aTable->setCellWidget(rowIndex, 1, mCommand);
   aTable->setCellWidget(rowIndex, 2, mValueWidget->GetWidget());
}

void Item::Disable()
{
   mAddToBlock->setDisabled(true);
   mCommand->setDisabled(true);
   mEnabled = false;
}

void Item::Enable()
{
   mAddToBlock->setEnabled(true);
   mCommand->setEnabled(true);
   mEnabled = true;
}

ValueWidget::WidgetInterface* Item::DetermineValueWidget()
{
   // necessary conditional chain for a factory method...
   if (mSetting.mValueType == ValueWidget::Type::cCONSTANT)
   {
      return CreateConstant();
   }
   else if (mSetting.mValueType == ValueWidget::Type::cBOOLEAN)
   {
      return CreateBoolean();
   }
   else if (mSetting.mValueType == ValueWidget::Type::cMULTIPLE_CHOICE)
   {
      return CreateMultipleChoice();
   }
   else if (mSetting.mValueType == ValueWidget::Type::cUSER_INPUT_TEXT)
   {
      return CreateUserInput();
   }
   else
   {
      return nullptr;
   }
}

ValueWidget::Constant* Item::CreateConstant()
{
   ValueWidget::Constant* constLab{ut::qt::make_qt_ptr<ValueWidget::Constant>(mSetting, this)};
   mSetting.mValue = ut::make_optional<QString>(mSetting.mPossibleValues[0].split(':')[1]);
   return constLab;
}

ValueWidget::Boolean* Item::CreateBoolean()
{
   return {ut::qt::make_qt_ptr<ValueWidget::Boolean>(mSetting, this)};
}

ValueWidget::MultipleChoice* Item::CreateMultipleChoice()
{
   bool userDefined{mSetting.mValue.has_value()};
   auto multiChoiceBox = ut::qt::make_qt_ptr<ValueWidget::MultipleChoice>(mSetting, this);

   //! Slot executed when the combo box switches the current displayed item
   //! @param aIndex is the index of the current item after the switch
   auto slotOnMultipleChoiceSwitch = [=](const int aIndex)
   {
      bool userDefined{mSetting.mValue.has_value()};
      bool editable{multiChoiceBox->GetWidget()->itemData(aIndex).toBool()};
      multiChoiceBox->GetWidget()->setEditable(editable);
      // make sure the editable field reflects the correct text
      if (editable)
      {
         QString itemName{'<' + mSetting.mCommand + '>'};
         QString placeholder{mSetting.mPossibleValues[mSetting.mPossibleValues.size() - 1].split("Edit:")[1]};
         // set the text to what the user provided (id est:  read from the block)
         if (userDefined && !mSetting.mPossibleValues.contains("NoEdit:" + mSetting.mValue.value()))
         {
            itemName = mSetting.mValue.value();
         }
         // set the text as <file>.<extension>, where the extension is acquired from the Widget
         else
         {
            QString extension;
            emit    AcquireExtension(extension);
            itemName += extension;
         }
         // set the placeholder text (viewable when there is no text in the line edit "delegate")
         multiChoiceBox->GetWidget()->lineEdit()->setPlaceholderText(placeholder);
         // set the actual text of the line edit "delegate"
         multiChoiceBox->GetWidget()->setEditText(itemName);
      }
      // the setting has a value!
      mSetting.mValue = ut::make_optional<QString>(multiChoiceBox->GetWidget()->currentText());
      // set up the tooltip because it is important
      multiChoiceBox->GetWidget()->setToolTip("The value of the setting command (" + mSetting.mValue.value() + ')');
   };

   // set the current text of the combo box
   int index;
   if (userDefined)
   {
      QSignalBlocker block(multiChoiceBox);
      multiChoiceBox->GetWidget()->setCurrentText(mSetting.mValue.value());
      index = multiChoiceBox->GetWidget()->currentIndex();
   }
   // default to the first combo box entry
   else
   {
      QSignalBlocker block(multiChoiceBox);
      multiChoiceBox->GetWidget()->setCurrentIndex(0);
      index = 0;
   }
   slotOnMultipleChoiceSwitch(index);

   // do the necessary connections
   connect(multiChoiceBox->GetWidget(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, slotOnMultipleChoiceSwitch);
   connect(multiChoiceBox->GetWidget(),
           &QComboBox::editTextChanged,
           [this](const QString& aEditText) { mSetting.mValue = ut::make_optional<QString>(aEditText); });

   return multiChoiceBox;
}

ValueWidget::UserInput* Item::CreateUserInput()
{
   ValueWidget::UserInput* lineEdit{ut::qt::make_qt_ptr<ValueWidget::UserInput>(mSetting, this)};
   mSetting.mValue = ut::make_optional<QString>();

   // do the necessary connections
   connect(lineEdit->GetWidget(),
           &QLineEdit::editingFinished,
           this,
           [=]()
           {
              mSetting.mValue = ut::make_optional<QString>(lineEdit->GetWidget()->text());
              // set up the tooltip because it is important
              lineEdit->GetWidget()->setToolTip(QString("The value of the setting command (") +
                                                mSetting.mValue.value() + ')');
           });

   return lineEdit;
}

void Item::OnContextMenuRequest(const QPoint& aPos)
{
   // create the context menu
   QMenu docMenu;
   auto  docAction = ut::qt::make_qt_ptr<wkf::Action>(mSetting.mCommand + " Command documentation...", &docMenu);
   connect(docAction,
           &QAction::triggered,
           [this]()
           {
              QString type;
              emit    AcquireType(type);
              auto    docUrl = QUrl::fromLocalFile(QFileInfo{wkfEnv.GetDocumentationDir()}.absolutePath() +
                                                "/documentation/html/docs/" + type + ".html");
              docUrl.setFragment(mSetting.mCommand);
              QDesktopServices::openUrl(docUrl);
           });
   // show the context menu
   docMenu.exec(mCommand->mapToGlobal(aPos));
}

Widget::Widget(QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
{
   // set up the UI
   mUi.setupUi(this);
}

Widget::Widget(editor::tokenizer::BlockTags aTags, QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
{
   // set up the UI
   mUi.setupUi(this);
   // and build the table
   BuildTable(std::move(aTags));
}

void Widget::BuildTable(editor::tokenizer::BlockTags aTags)
{
   // acquire the available settings from the WsfParser
   mRule = std::move(aTags);
   AcquireSettingsFromGrammar();
   // and create the Items
   for (const auto& setting : mAvailableSettings)
   {
      CreateSettingsItem(setting);
   }
   // the, do some adjustments to the table
   mUi.mSettingsTable->resizeColumnsToContents();
   mUi.mSettingsTable->setColumnWidth(1, mUi.mSettingsTable->columnWidth(1) + 50);
   mUi.mSettingsTable->setColumnWidth(2, mUi.mSettingsTable->columnWidth(2) + 50);
   // including hiding the row numbers
   mUi.mSettingsTable->verticalHeader()->setHidden(true);
}

const QStringList& Widget::GetAvailableCommands() const
{
   return mAvailableCommands;
}

const QMap<QString, Data>& Widget::GetAvailableSettings() const
{
   return mAvailableSettings;
}

void Widget::ReadSettings(const editor::tokenizer::Tokenizer* aRange)
{
   for (auto& elem : mAvailableItems)
   {
      elem.second->ReadSetting(aRange);
   }
}

void Widget::WriteSettings(editor::tokenizer::Tokenizer* aRange)
{
   for (auto& elem : mAvailableItems)
   {
      elem.second->WriteSetting(aRange);
   }
}

void Widget::RestoreDefaults()
{
   for (auto& elem : mAvailableItems)
   {
      elem.second->RestoreDefault();
   }
}

void Widget::AcquireSettingsFromGrammar()
{
   std::string rule{mRule.mBeginTag.toStdString()};

   // acquire the WsfParseStruct from the WsfParser
   auto wizProj = wizard::Project::Instance();
   if (wizProj)
   {
      auto wizParseResults = wizProj->GetParseResults();
      if (wizParseResults)
      {
         auto wizParser = wizParseResults->parser();
         if (wizParser)
         {
            auto wizParseDefinitions = wizParser->GetDefinitions();
            if (wizParseDefinitions)
            {
               auto wizParseRoot = wizParseDefinitions->GetRoot();
               if (wizParseRoot)
               {
                  auto wizEventOutputStruct = wizParseRoot->FindStruct(rule);
                  if (wizEventOutputStruct)
                  {
                     // get the list of settings (as WsfParseRules)
                     auto wizEventOutputAlternate = wizEventOutputStruct->GetSubordinateRule();
                     if (wizEventOutputAlternate)
                     {
                        auto wizEventOutputSettingList = wizEventOutputAlternate->GetAlternates();
                        if (!wizEventOutputSettingList.empty())
                        {
                           QString                    command;
                           QStringList                values;
                           Data                       sd;
                           QVector<ValueWidget::Type> types;
                           // this routine goes through each WsfParseRule and acquires the sequence
                           for (auto settingRule : wizEventOutputSettingList)
                           {
                              if (settingRule->Type() == WsfParseRule::cSEQUENCE)
                              {
                                 bool foundCommand{false};
                                 auto settingRuleSequence = settingRule->GetSequence();
                                 // each entry in the sequence is a valid value for the setting
                                 for (auto sequenceRule : settingRuleSequence)
                                 {
                                    if (!foundCommand && sequenceRule->Type() == WsfParseRule::cLITERAL)
                                    {
                                       foundCommand = true;
                                       std::string settingName{sequenceRule->GetRuleDescription()};
                                       // this is the first command found
                                       if (command.isEmpty())
                                       {
                                          command = QString::fromStdString(settingName);
                                       }
                                       // use the change in command name to finish creating the setting::Data
                                       else if (command.toStdString() != settingName)
                                       {
                                          sd.mCommand      = command;
                                          sd.mValueType    = ValueWidget::DetermineValueType(types);
                                          sd.mDefaultValue = ValueWidget::DetermineDefaultValue(*wizEventOutputStruct, sd);
                                          mAvailableCommands.append(command);
                                          mAvailableSettings.insert(command, sd);
                                          sd      = Data();
                                          command = QString::fromStdString(settingName);
                                          types.clear();
                                       }
                                    }
                                    // the value is a pre-defined literal value
                                    // Note:  this is for the "file" command
                                    else if (sequenceRule->Type() == WsfParseRule::cLITERAL)
                                    {
                                       std::string settingType{"NoEdit:" + sequenceRule->GetRuleDescription()};
                                       sd.mPossibleValues.append(QString::fromStdString(settingType));
                                       types.append(ValueWidget::Type::cCONSTANT);
                                    }
                                    // the value is a user-defined literal value (quoted string, or whatever...)
                                    // Note:  this is for the "file" command
                                    else if (sequenceRule->Type() == WsfParseRule::cQUOTED_STRING)
                                    {
                                       std::string settingType{"Edit:" + sequenceRule->GetRuleDescription()};
                                       sd.mPossibleValues.append(QString::fromStdString(settingType));
                                       types.append(ValueWidget::Type::cUSER_INPUT_TEXT);
                                    }
                                    // the value references a WsfPProxy variable
                                    else if (sequenceRule->Type() == WsfParseRule::cRULE_REFERENCE)
                                    {
                                       std::string settingType = sequenceRule->GetRuleDescription();
                                       // these are the commands with boolean values
                                       // Note:  the WsfPProxy recognizes the following boolean terminologies:
                                       //        false, true
                                       //        disabled, enabled
                                       //        no, yes
                                       //        this, however, only uses the false/true terminology.
                                       if (settingType == "<bool>" || settingType == "<Bool>")
                                       {
                                          sd.mPossibleValues.append("NoEdit:false");
                                          sd.mPossibleValues.append("NoEdit:true");
                                          types.append(ValueWidget::Type::cBOOLEAN);
                                       }
                                       else if (settingType == "<string>" || settingType == "<String>")
                                       {
                                          if (command == "time_format")
                                          {
                                             // provide some pre-defined values
                                             sd.mPossibleValues.append("NoEdit:s.8");
                                             sd.mPossibleValues.append("NoEdit:s.6");
                                             sd.mPossibleValues.append("NoEdit:m:s.6");
                                             sd.mPossibleValues.append("NoEdit:m:s.4");
                                             sd.mPossibleValues.append("NoEdit:h:m:s.4");
                                             sd.mPossibleValues.append("NoEdit:h:m:s.2");
                                             sd.mPossibleValues.append("NoEdit:h:m:s");
                                             sd.mPossibleValues.append("Edit:[h:[m:]]s[.#]");
                                             types.append(ValueWidget::Type::cMULTIPLE_CHOICE);
                                          }
                                          else if (command == "lat_lon_format")
                                          {
                                             // provide some pre-defined values
                                             sd.mPossibleValues.append("NoEdit:d.8");
                                             sd.mPossibleValues.append("NoEdit:d.6");
                                             sd.mPossibleValues.append("NoEdit:d:m.6");
                                             sd.mPossibleValues.append("NoEdit:d:m.4");
                                             sd.mPossibleValues.append("NoEdit:d:m:s.4");
                                             sd.mPossibleValues.append("NoEdit:d:m:s.2");
                                             sd.mPossibleValues.append("NoEdit:d:m:s");
                                             sd.mPossibleValues.append("Edit:d[:m[:s]][.#]");
                                             types.append(ValueWidget::Type::cMULTIPLE_CHOICE);
                                          }
                                          // assume a user input value...
                                          else
                                          {
                                             sd.mPossibleValues.push_back("Placeholder:");
                                             types.append(ValueWidget::Type::cUSER_INPUT_TEXT);
                                          }
                                       }
                                       // this is the rule for the events list to search...
                                       else if (settingType == "<event_output-event-type>")
                                       {
                                          types.append(ValueWidget::Type::cNONE);
                                       }
                                    }
                                 }
                              }
                           }
                           // add the last setting::Data into the list
                           auto iter = mAvailableSettings.constFind(command);
                           if (iter == mAvailableSettings.constEnd())
                           {
                              sd.mCommand   = command;
                              sd.mValueType = DetermineValueType(types);
                              mAvailableCommands.append(command);
                              mAvailableSettings.insert(command, sd);
                           }
                           // remove unwanted settings (these are covered in the events list)
                           mAvailableCommands.removeOne("disable");
                           mAvailableSettings.remove("disable");
                           mAvailableCommands.removeOne("enable");
                           mAvailableSettings.remove("enable");
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

void Widget::CreateSettingsItem(const Data& aSetting)
{
   auto si = ut::qt::make_qt_ptr<Item>(aSetting, this);
   // add the item to the table and the list
   si->AddToTable(mUi.mSettingsTable);
   mAvailableItems.insert({aSetting.mCommand, si});

   // do necessary connections
   connect(si, &Item::AddToBlockToggled, [this]() { mModifiedByUser = true; });
   connect(si, &Item::ModifiedByUser, [this]() { mModifiedByUser = true; });
   if (aSetting.mCommand == "file")
   {
      connect(si,
              &Item::AcquireExtension,
              [this](QString& aExtension)
              {
                 if (mRule.mBeginTag == "csv_event_output")
                 {
                    aExtension = ".csv";
                 }
                 else if (mRule.mBeginTag == "event_output")
                 {
                    aExtension = ".evt";
                 }
                 else
                 {
                    aExtension = "";
                 }
              });
   }
   connect(si, &Item::AcquireType, [this](QString& aType) { aType = mRule.mBeginTag; });
}

void Widget::Disable()
{
   for (auto& elem : mAvailableItems)
   {
      elem.second->Disable();
   }
   mEnabled = false;
}

void Widget::Enable()
{
   for (auto& elem : mAvailableItems)
   {
      elem.second->Enable();
   }
   mEnabled = true;
}

} // namespace setting
} // namespace EventOutput
} // namespace wizard
