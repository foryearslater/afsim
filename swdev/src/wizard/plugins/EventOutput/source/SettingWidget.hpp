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

#ifndef SETTINGS_WIDGET_HPP
#define SETTINGS_WIDGET_HPP

#include <map>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPoint>
#include <QPointer>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "SettingValueWidget.hpp"
#include "Tokenizer.hpp"
#include "UtOptional.hpp"
#include "UtQtUiPointer.hpp"
#include "ui_SettingItem.h"
#include "ui_SettingWidget.h"

namespace wizard
{
namespace EventOutput
{

//! Namespace for objects, functions, and variables related to the csv_event_output and event_output settings
namespace setting
{

//! Data defining a setting
struct Data
{
   //! The name of the setting (the "command")
   QString mCommand;
   //! The type of the ValueWidget::WidgetInterface to use for the value
   ValueWidget::Type mValueType;
   //! The list of possible values for the setting
   QStringList mPossibleValues;
   //! The default value of the setting
   //! @note some ValueWidget::Types do not have a default
   const QString* mDefaultValue;
   //! The current value of the setting
   //! @note the optional has_value() when the user defines the setting
   ut::optional<QString> mValue;
};

//! Represents an item
//! @par details
//!   This class represents an item in the setting::Widget.
//!   it is a visual representation of a setting::Data.
//!   it contains a check box to denote whether or not to add the property to the block.
//!   it also contains a command label and a ValueWidget::WidgetInterface to represent the command and value of the
//!   setting. there are factory methods to create the proper ValueWidget::WidgetInterface inside this class as well.
//! @see ValueWidget::WidgetInterface and derived types for more information.
class Item : public QWidget
{
   Q_OBJECT
public:
   //! Constructs an Item
   //! @param aParent is a pointer to the parent widget
   Item(QWidget* aParent = nullptr);
   //! Constructs an Item
   //! @param aSetting is the data to build the Item
   //! @param aParent is a pointer to the parent widget
   Item(Data aSetting, QWidget* aParent = nullptr);
   //! Destructs an Item
   ~Item() override = default;

   //! Get the setting data
   //! @return the setting data
   const Data& GetSetting() const { return mSetting; }
   //! Get the value widget
   //! @return a pointer to the value widget (immutable)
   const ValueWidget::WidgetInterface* GetWidget() const { return mValueWidget; }

   //! @name methods for editing the property information
   //! @note the property for the setting::Item is setting
   //@{

   //! Read the setting information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the setting information
   void ReadSetting(const editor::tokenizer::Tokenizer* aRange);
   //! Write the setting information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the setting information
   void WriteSetting(editor::tokenizer::Tokenizer* aRange);
   //! Restores the default state of the Item
   //! @note this does not edit any information in the back-end
   void RestoreDefault();

   //@}

   //! Adds the Item to the setting::Widget table
   //! @param aTable is the table into which to place the Item
   void AddToTable(QTableWidget* aTable);

   //! Disables the widget
   void Disable();
   //! Determines whether or not the widget is disabled
   //! @return whether or not the widget is disabled
   bool IsDisabled() const { return !mEnabled; }
   //! Enables the widget
   void Enable();
   //! Determines whether or not the widget is enabled
   //! @return whether or not the widget is enabled
   bool IsEnabled() const { return mEnabled; }

signals:
   //! Signal to acquire the output file extension (usually .csv or .evt, but the extensions can be anything)
   //! @param aExtension is the "return value parameter" denoting the output file extension
   void AcquireExtension(QString& aExtension);
   //! Signal to acquire the type of input block (either csv_event_output or event_output)
   //! @param aType is the "return value parameter" denoting the type of input block
   void AcquireType(QString& aType);
   //! Signal to notify the widget that the user modified the Add to Block? check box
   void AddToBlockToggled();
   //! Signal to notify connected objects that the user modified the state of the Item
   void ModifiedByUser();

private slots:
   //! Slot executed when the user right-clicks the command label
   //! @param aPos is the point where the user right-clicked
   //! @post a context menu appears, containing a menu action that, when clicked, will open up the appropriate
   //! documentation in the web browser
   //! @note Chrome and Firefox support opening URLs with fragments (id est:  page.html#fragment) via system calls; Edge
   //! and Internet Explorer do not.
   void OnContextMenuRequest(const QPoint& aPos);

private:
   //! @name factory methods for creating value widgets
   //@{

   //! Factory method to create a particular value widget
   //! @return a pointer to the value widget
   //! @note the type of value widget is contained in mSetting
   ValueWidget::WidgetInterface* DetermineValueWidget();

   //! Creates a Constant value widget
   //! @return a pointer to the Constant value widget
   ValueWidget::Constant* CreateConstant();
   //! Creates a Boolean value widget
   //! @return a pointer to the Boolean value widget
   ValueWidget::Boolean* CreateBoolean();
   //! Creates a MultipleChoice value widget
   //! @return a pointer to the MultipleChoice value widget
   ValueWidget::MultipleChoice* CreateMultipleChoice();
   //! Creates a UserInput value widget
   //! @return a pointer to the UserInput value widget
   ValueWidget::UserInput* CreateUserInput();

   //@}

   //! The underlying setting data
   Data mSetting;
   //! A check box denoting whether or not to add the setting into the block
   ut::qt::UiPointer<QCheckBox> mAddToBlock;
   //! The setting name
   ut::qt::UiPointer<QLabel> mCommand;
   //! The setting value
   ut::qt::UiPointer<ValueWidget::WidgetInterface> mValueWidget;
   //! Determines whether or not the widget is enabled
   bool mEnabled{true};
};

//! Represents the settings list in the EventOutput::Dialog
//! @par details
//!   This widget is a table of settings available in csv_event_output and event_output.
//!   it contains a list of Items displayed in a table format.
//!   it also contains a list of available settings retrieved from the WsfParser.
//! @see Item for more information.
class Widget : public QWidget
{
   Q_OBJECT
public:
   //! Constructs a Widget
   //! @param aParent is a pointer to the parent widget
   Widget(QWidget* aParent = nullptr);
   //! Constructs a Widget
   //! @param aTags denotes the begin and end tag of the block
   //! @param aParent is a pointer to the parent widget
   Widget(editor::tokenizer::BlockTags aTags, QWidget* aParent = nullptr);
   //! Destructs a Widget
   ~Widget() override = default;

   //! Builds the setting table
   //! @param aTags denotes the begin and end tag of the block
   //! @note this should be executed only once (typically, right after construction of a setting::Widget)
   void BuildTable(editor::tokenizer::BlockTags aTags);

   //! Set the block tags
   //! @param aTags denotes the begin and end tags of the block
   //! @note the begin tag is also the WsfParser rule to look up the available settings
   void SetBlockTags(editor::tokenizer::BlockTags aTags) { mRule = std::move(aTags); }
   //! Get the block tags
   //! @return the block tags
   const editor::tokenizer::BlockTags& GetBlockTags() const { return mRule; }
   //! Get the names of the available settings
   //! @return the names of the available settings
   const QStringList& GetAvailableCommands() const;
   //! Get the data of the available settings
   //! @return the data of the available settings
   const QMap<QString, Data>& GetAvailableSettings() const;
   //! Set the "modified by user" property
   //! @param aModifiedByUser denotes whether or not the user modified the Widget
   void SetModifiedByUser(bool aModifiedByUser) { mModifiedByUser = aModifiedByUser; }
   //! Get the "modified by user" property
   //! @return whether or not the user modified the Widget
   bool ModifiedByUser() const { return mModifiedByUser; }

   //! @name methods for editing the property information
   //! @note the property for the setting::Widget is setting
   //@{

   //! Read the setting information for each Item
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the setting information
   void ReadSettings(const editor::tokenizer::Tokenizer* aRange);
   //! Write the setting information for each Item
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the setting information
   void WriteSettings(editor::tokenizer::Tokenizer* aRange);
   //! Restores the default state for each Item
   //! @note this does not edit any information in the back-end
   void RestoreDefaults();

   //@}

   //! Disables the widget
   void Disable();
   //! Determines whether or not the widget is disabled
   //! @return whether or not the widget is disabled
   bool IsDisabled() const { return !mEnabled; }
   //! Enables the widget
   void Enable();
   //! Determines whether or not the widget is enabled
   //! @return whether or not the widget is enabled
   bool IsEnabled() const { return mEnabled; }

private:
   //! Acquire the available settings from the WsfParser
   void AcquireSettingsFromGrammar();

   //! Creates an Item
   //! @param aSetting is the data from which to create the Item
   void CreateSettingsItem(const Data& aSetting);

   //! The user interface of the setting::Widget
   Ui::SettingWidget mUi;

   //! The block tags
   editor::tokenizer::BlockTags mRule;
   //! The names of the available settings
   QStringList mAvailableCommands;
   //! The data of the available settings
   QMap<QString, Data> mAvailableSettings;
   //! The Items in the setting table
   std::map<QString, QPointer<Item>> mAvailableItems;
   //! The "modified by user" property
   bool mModifiedByUser{false};
   //! Determines whether or not the widget is enabled
   bool mEnabled{true};
};

} // namespace setting
} // namespace EventOutput
} // namespace wizard

#endif // SETTINGS_WIDGET_HPP
