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

#ifndef EVENTS_GROUP_BOX_WIDGET_HPP
#define EVENTS_GROUP_BOX_WIDGET_HPP

#include <list>
#include <map>

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

class WsfParseRule;

#include "EventGroup.hpp"

namespace wizard
{
namespace EventOutput
{

//! Namespace for objects, functions, and variables related to the csv_event_output and event_output settings
namespace event
{

//! Represents the events list in the EventOutput::Dialog
//! @par details
//!   This widget is a list of available events in csv_event_output and event_output.
//!   it contains a list of event::Groups displayed in a list format.
//!   it also contains a list of available events retrieved from the WsfParser.
//! @see Group for more information.
class Widget : public QWidget
{
public:
   //! converts the name of a WsfParseRule to the name of an event group
   //! @param aRuleName is the name of the WsfParseRule
   //! @return the name of the event group
   static QString RuleToEventGroup(const QString& aRuleName);

   //! Constructs an Widget
   //! @param aParent is a pointer to the parent widget
   Widget(QWidget* aParent = nullptr);
   //! Destructs an Widget
   ~Widget() override = default;

   //! Get the available event names
   //! @return a list of the available event names
   const QStringList& GetAvailableCommands() const;
   //! Set the "modified by user" property
   //! @param aModifiedByUser denotes whether or not the user modified the Widget
   void SetModifiedByUser(bool aModifiedByUser) { mModifiedByUser = aModifiedByUser; }
   //! Get the "modified by user" property
   //! @return whether or not the user modified the Widget
   bool ModifiedByUser() const { return mModifiedByUser; }

   //! @name methods for editing the property information
   //! @note the property for the event::Group is event
   //@{

   //! Read the event information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void ReadEvents(const editor::tokenizer::Tokenizer* aRange);
   //! Write the event information
   //! @param aRange is a pointer to the tokenizer::Tokenizer containing the event information
   void WriteEvents(editor::tokenizer::Tokenizer* aRange);
   //! Restores the default state
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
   //! Constructs the root Group and populates it with the available events
   void PopulateRootGroup();
   //! Acquire the available events from the WsfParser and constructs the root Group
   //! @par details
   //!   This function finds all the valid event commands for (csv_)event_output.
   //!   it searches the event_output-event-type rule to find all of the event names.
   //!   it stores these for use later to populate the event::Widget.
   void AcquireEventsFromParser();

   //! recursive helper function for AcquireEventsFromParser
   //! @par details
   //!   This function recursively searches a given rule for valid commands.
   //!   it will do a recursive call when it reaches another rule.
   //!   when it hits a leaf rule, it will store the names of the alternates.
   //! @param aData is a reference to where to store the extracted information
   //! @param aRulePtr is a pointer to the WsfParseRule to search
   void AcquireEventsFromParserH(GroupData& aData, WsfParseRule* aRulePtr);

   //! The root EventGroup
   ut::qt::UiPointer<Group> mRoot;

   //! The available event names
   QStringList mAvailableCommands;
   //! The available event names by group
   GroupData mAvailableGroups;
   //! The "modified by user" property
   bool mModifiedByUser{false};
   //! Denotes whether or not the widget is enabled
   bool mEnabled{true};
};

} // namespace event
} // namespace EventOutput
} // namespace wizard

#endif // EVENTS_GROUP_BOX_WIDGET
