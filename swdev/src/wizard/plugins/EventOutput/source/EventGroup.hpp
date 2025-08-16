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

#ifndef CHECK_BOX_GROUP_HPP
#define CHECK_BOX_GROUP_HPP

#include <list>

#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QPointer>
#include <QVBoxLayout>
#include <QWidget>

#include "UtOptional.hpp"
#include "UtQtUiPointer.hpp"
#include "ui_EventItem.h"

namespace wizard
{
namespace EventOutput
{
namespace editor
{
namespace tokenizer
{

class Tokenizer;

}
} // namespace editor

namespace event
{

//! Represents the state of an event::Item
enum class Value
{
   cDEFAULT, //!< denotes the default state
   cFALSE,   //!< denotes the false state
   cTRUE,    //!< denotes the true state
};

//! An event item
//! @par details
//!   This class represents an item in the event::Group.
//!   it is a visual representation of an event CommandValuePair.
//!   it contains a label and three radio buttons to represent the command and value of the event.
//!   it provides a custom context menu to provide a documentation link.
class Item : public QWidget
{
   Q_OBJECT
public:
   //! Constructs a WidgetInterface
   //! @param aLabel is the name (command) of the property
   //! @param aFalseValue is the identifier for the false value
   //! @param aTrueValue is the identifier for the true value
   //! @param aDefaultValue is the identifier for the default value
   //! @param aParent is a pointer to the parent widget
   Item(const QString& aLabel,
        const QString& aFalseValue   = "false",
        const QString& aTrueValue    = "true",
        const QString& aDefaultValue = "default",
        QWidget*       aParent       = nullptr);
   //! Destructs a WidgetInterface
   ~Item() override = default;

   //! Set the name of the property (the "command")
   //! @param aName is the name of the property (the "command")
   void SetCommand(const QString& aName) { mUi.mCommand->setText(aName); }
   //! Get the name of the property (the "command")
   //! @return the name of the property (the "command")
   QString GetCommand() const { return mUi.mCommand->text(); }
   //! Set the false value
   void SetFalse() { mUi.mFalse->setChecked(true); }
   //! Get the false value
   //! @return whether or not the false value is set
   bool GetFalse() const { return mUi.mFalse->isChecked(); }
   //! Set the true value
   void SetTrue() { mUi.mTrue->setChecked(true); }
   //! Get the false value
   //! @return whether or not the false value is set
   bool GetTrue() const { return mUi.mTrue->isChecked(); }
   //! Set the overall widget state
   //! @param aState is the overall state of the widget
   void SetState(const Value aState);
   //! Get the overall widget state
   //! @return the overall widget state
   Value GetState() const { return mState; }
   //! Get the overall widget state as a string
   //! @return the overall widget state as a string
   QString GetStateString() const;

   //! @name methods for editing the property information
   //! @note the property for the event::WidgetInterface is event
   //@{

   //! Read the event information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void ReadEvent(const editor::tokenizer::Tokenizer* aRange);
   //! Write the event information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void WriteEvent(editor::tokenizer::Tokenizer* aRange);
   //! Restores the default state of the widget
   //! @note this does not edit any information in the back-end
   void RestoreDefault();

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

signals:
   //! Signal to notify connected objects that the widget changed its state
   //! @param aState is the overall state of the widget
   void StateChanged(Value aState);

private:
   //! @name The user interface for the event::Item
   Ui::EventItem mUi;

   //! The overall state of the widget
   Value mState{Value::cDEFAULT};

   //! Denotes whether or not the widget is enabled
   bool mEnabled{true};

   //! @name the state strings
   //@{

   //! The default state
   const QString cDEFAULT_VALUE;
   //! The false state
   const QString cFALSE_VALUE;
   //! The true state
   const QString cTRUE_VALUE;

   //@}

private slots:
   //! Slot executed when the default value is modified
   //! @param aChecked is whether or not the default value is checked
   void OnDefault(bool aChecked);
   //! Slot executed when the false value is modified
   //! @param aChecked is whether or not the false value is checked
   void OnFalse(bool aChecked);
   //! Slot executed when the true value is modified
   //! @param aChecked is whether or not the true value is checked
   void OnTrue(bool aChecked);
};

//! Represents a condensed form of an event::Group created by extracting information from the WsfParser
struct GroupData
{
   //! the title
   QString mTitle;
   //! the items of the group
   QStringList mItems;
   //! the groups of the group
   QList<GroupData> mGroups;

   void Clear()
   {
      mTitle.clear();
      mItems.clear();
      mGroups.clear();
   }
};

//! A group of event::Items organized in a group box
//! @par details
//!   A check box group is a widget that has a top-level event::Item that controls a list of sub- event::Item.
//!   The top-level event::Item contains three radio buttons to toggle the state of all sub- event::Item.
//!   The sub- event::Item contain three radio buttons each.
//!   this widget organizes the event::Items within a group box.
//!   it also allows nested event::Groups.
//! @par the layout of the Group
//!   (in amazing ASCII art!)
//!
//! @verbatim
//!    ----------------------------
//!    | Item (title)             |
//!    ----------------------------
//!    | Item                     |
//!    | Item                     |
//!    | Item                     |
//!    | ...                      |
//!    |   --------------------   |
//!    |   | Item (title)     |   |
//!    |   --------------------   |
//!    |   | Item             |   |
//!    |   | Item             |   |
//!    |   | Item             |   |
//!    |   | ...              |   |
//!    |   --------------------   |
//!    | ...                      |
//!    ----------------------------
//! @endverbatim
class Group : public QWidget
{
   Q_OBJECT
public:
   //! creates a Group from GroupData
   //! @param aData is the GroupData from which to create the Group
   //! @param aParent is a pointer to the parent widget
   static Group* DataToGroup(const GroupData& aData, QWidget* aParent = nullptr);

   //! Constructs a Group
   //! @param aParent is a pointer to the parent widget
   Group(QWidget* aParent = nullptr);
   //! Constructs a Group
   //! @param aTitle is the event group name
   //! @param aParent is a pointer to the parent widget
   Group(const QString& aTitle, QWidget* aParent = nullptr);
   //! Constructs a Group
   //! @param aLabel is the event group name
   //! @param aChildLabels is a list of the event names that are under the group
   //! @param aFalseValue is the identifier for the false value
   //! @param aTrueValue is the identifier for the true value
   //! @param aParent is a pointer to the parent widget
   Group(const QString&     aLabel,
         const QStringList& aChildLabels,
         const QString&     aFalseValue = "false",
         const QString&     aTrueValue  = "true",
         QWidget*           aParent     = nullptr);
   //! Destructs a Group
   ~Group() override = default;

   //! @name functions to add EventCheckBoxes
   //@{

   //! Constructs an EventCheckBox and appends it to the list
   //! @param aLabel is the event name
   //! @param aFalseValue is the identifier for the false value
   //! @param aTrueValue is the identifier for the true value
   void AddCheckBox(const QString& aLabel, const QString& aFalseValue = "false", const QString& aTrueValue = "true");
   //! Constructs multiple EventCheckBoxes and appends them to the list
   //! @param aChildLabels is a list of events belonging to the group
   //! @param aFalseValue is the identifier for the false value
   //! @param aTrueValue is the identifier for the true value
   void AddCheckBoxes(const QStringList& aChildLabels,
                      const QString&     aFalseValue = "false",
                      const QString&     aTrueValue  = "true");

   //@}

   //! @name functions to add Groups
   //@{

   //! Adds a Group to the list
   //! @param aGBI is the Group
   void AddGroupBox(Group* aGBI);
   //! @param aLabel is the event group name
   //! @param aChildLabels is a list of the event names that are under the group
   //! @param aFalseValue is the identifier for the false value
   //! @param aTrueValue is the identifier for the true value
   void AddGroupBox(const QString&     aLabel,
                    const QStringList& aChildLabels,
                    const QString&     aFalseValue = "false",
                    const QString&     aTrueValue  = "true");
   //! Constructs a Group and adds the child event::Groups to the list
   void AddGroupBox(const QString& aLabel, const QList<Group*>& aChildList);

   //@}

   //! Set the event group name
   //! @param aName is the event group name
   void SetEventGroup(const QString& aName) { mTitle->SetCommand(aName); }
   //! Get the event group name
   //! @return the event group name
   QString GetEventGroup() const { return mTitle->GetCommand(); }
   //! Get the event names
   //! @return a list of the event names
   QStringList GetEvents() const;
   //! Set the overall Group state
   //! @param aState is the overall Group state
   void SetState(Value aState);
   //! Get the overall Group state
   //! @return the overall Group state
   Value GetState() const { return mTitle->GetState(); }
   //! Set the "modify state of children" property
   //! @param aModifyStateOfChildren denotes whether or not the parent boolean widget modifies the state of the child
   //! boolean widgets
   void SetModifyStateOfChildren(bool aModifyStateOfChildren) { mModifyStateOfChildren = aModifyStateOfChildren; }
   //! Set the "modify state of children" property
   //! @return whether or not the parent boolean widget modifies the state of the child boolean widgets
   bool ModifyStateOfChildren() const { return mModifyStateOfChildren; }
   //! Set the "modify state of parent" property
   //! @param aModifyStateOfParent denotes whether or not the child boolean widgets modify the state of the parent boolean widget
   void SetModifyStateOfParent(bool aModifyStateOfParent) { mModifyStateOfParent = aModifyStateOfParent; }
   //! Set the "modify state of parent" property
   //! @return whether or not the child boolean widgets modify the state of the parent boolean widget
   bool ModifyStateOfParent() const { return mModifyStateOfParent; }

   //! @name methods for editing the property information
   //! @note the property for the event::Group is event
   //@{

   //! Read the event information for each boolean widget and Group
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void ReadEvents(const editor::tokenizer::Tokenizer* aRange);
   //! Write the event information for each boolean widget and Group
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void WriteEvents(editor::tokenizer::Tokenizer* aRange);
   //! Restores the default state for each boolean widget and Group
   //! @note this does not edit any information in the back-end
   void RestoreDefaults();

   //@}

   //! Lays the check boxes out in the group box
   //! @note see the class description for details
   void LayoutWidgets();

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
   //! Signal to notify connected objects that the user modified the Group
   void ModifiedByUser();

private:
   //! Modifies the check state of all the child boolean widgets
   //! @param aState is the overall state of the Group
   void CheckStateAll(Value aState);

   //! The title Item (this is the event group name)
   ut::qt::UiPointer<Item> mTitle;
   //! A horizontal rule to separate event group name from event names
   ut::qt::UiPointer<QFrame> mSeparator;
   //! The layout for the Group
   //! @note this layout is not for the group box, but to lay out the Item above the group box
   ut::qt::UiPointer<QVBoxLayout> mLayout;
   //! The group box
   ut::qt::UiPointer<QGroupBox> mGroup;
   //! The list of child Items (these are the events under the group)
   std::list<ut::qt::UiPointer<Item>> mEventList;
   //! The list of child Groups
   std::list<ut::qt::UiPointer<Group>> mGroupList;
   //! The "modify state of children" property
   bool mModifyStateOfChildren{true};
   //! The "modify state of parent" property
   bool mModifyStateOfParent{true};
   //! Denotes whether or not the widget is enabled
   bool mEnabled{true};

private slots:
   void OnEventContextMenu(const QPoint& aPos, Item* aEvent) const;
};

} // namespace event
} // namespace EventOutput
} // namespace wizard

#endif // CHECK_BOX_GROUP_HPP
