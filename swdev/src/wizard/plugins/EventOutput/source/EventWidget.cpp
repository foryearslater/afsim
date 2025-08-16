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

#include "EventWidget.hpp"

#include "CommandValuePair.hpp"
#include "ParseResults.hpp"
#include "PluginUtil.hpp"
#include "Project.hpp"
#include "Tokenizer.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WsfExe.hpp"
#include "WsfParser.hpp"

namespace wizard
{
namespace EventOutput
{
namespace event
{

const QString cDISABLE{"disable"};
const QString cENABLE{"enable"};

QString Widget::RuleToEventGroup(const QString& aRuleName)
{
   QString temp{aRuleName};
   temp           = temp.remove("<event_output");
   temp           = temp.remove("event-type>");
   auto tempSplit = temp.split('-');
   tempSplit.removeFirst();
   tempSplit.removeLast();
   return tempSplit.join('_').toUpper();
}

Widget::Widget(QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mRoot(ut::qt::make_qt_ptr<Group>("all", this))
{
   // set up the UI
   QSignalBlocker block{mRoot};

   // the root TitleCheckBox is oblivious to the child event::Groups
   mRoot->SetModifyStateOfChildren(false);
   mRoot->SetModifyStateOfParent(false);

   // add the root Group to a layout
   auto layout = ut::qt::make_qt_ptr<QVBoxLayout>();
   layout->addWidget(mRoot);
   setLayout(layout);

   // construct the root Group
   AcquireEventsFromParser();
   PopulateRootGroup();
   mRoot->LayoutWidgets();

   // do the necessary connections
   connect(mRoot.data(), &Group::ModifiedByUser, [this]() { mModifiedByUser = true; });
}

const QStringList& Widget::GetAvailableCommands() const
{
   return mAvailableCommands;
}

void Widget::ReadEvents(const editor::tokenizer::Tokenizer* aRange)
{
   // retrieve the appropriate command-value pair
   const auto* eventToken = aRange->GetEvent(mRoot->GetEventGroup());
   // check for existence and validity of the command-value pair
   bool validToken{eventToken && eventToken->IsValid()};
   // set the state of the root TitleCheckBox
   if (validToken)
   {
      if (eventToken->GetValue() == "disable")
      {
         mRoot->SetState(Value::cFALSE);
      }
      else if (eventToken->GetValue() == "enable")
      {
         mRoot->SetState(Value::cTRUE);
      }
   }
   // read the rest of the events
   mRoot->ReadEvents(aRange);
}

void Widget::WriteEvents(editor::tokenizer::Tokenizer* aRange)
{
   auto rootState = mRoot->GetState();
   switch (rootState)
   {
   // one of the checked/unchecked states
   case Value::cDEFAULT:
   case Value::cFALSE:
   case Value::cTRUE:
   {
      // retrieve the appropriate command-value pair
      const auto* eventToken = aRange->GetEvent(mRoot->GetEventGroup());
      // check for existence and validity of the command-value pair
      bool validToken{eventToken && eventToken->IsValid()};
      // the event warrants writing to the block
      if (rootState == Value::cFALSE || rootState == Value::cTRUE)
      {
         // the command-value pair is already in the block, but needs updating
         if (validToken)
         {
            // copy the command-value pair
            auto oldEvent = *static_cast<const editor::token::EventCVP*>(eventToken);
            // and update its value
            if (rootState == Value::cTRUE)
            {
               oldEvent.SetValue("enable");
            }
            else if (rootState == Value::cFALSE)
            {
               oldEvent.SetValue("disable");
            }
            // then, update the range
            aRange->SetEvent(oldEvent.GetCommand(), oldEvent.GetValue());
         }
         // the event warrants insertion into the block
         else
         {
            // create a new command-value pair
            editor::token::EventCVP newEvent;
            newEvent.SetCommand(mRoot->GetEventGroup());
            if (rootState == Value::cTRUE)
            {
               newEvent.SetValue("enable");
            }
            else if (rootState == Value::cFALSE)
            {
               newEvent.SetValue("disable");
            }
            // insert the command-value pair into the block
            aRange->InsertEvent(newEvent.GetCommand(), newEvent.GetValue());
         }
      }
      // the setting warrants removal from the block
      else if (rootState == Value::cDEFAULT && validToken)
      {
         aRange->RemoveEvent(eventToken->GetCommand());
      }
   }
   default:
      // write the rest of the events
      mRoot->WriteEvents(aRange);
      break;
   }
}

void Widget::RestoreDefaults()
{
   mRoot->RestoreDefaults();
}

void Widget::Disable()
{
   if (mRoot)
   {
      mRoot->Disable();
   }
   mEnabled = false;
}

void Widget::Enable()
{
   if (mRoot)
   {
      mRoot->Enable();
   }
   mEnabled = true;
}

void Widget::PopulateRootGroup()
{
   mRoot->SetEventGroup(mAvailableGroups.mTitle);
   mRoot->AddCheckBoxes(mAvailableGroups.mItems, "disable", "enable");
   for (auto elem : mAvailableGroups.mGroups)
   {
      mRoot->AddGroupBox(Group::DataToGroup(elem));
   }
}

void Widget::AcquireEventsFromParser()
{
   mAvailableCommands.clear();
   mAvailableGroups.Clear();

   // acquire the WsfParseRule list from the WsfParser
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
                  auto wizEventOutputEventTypeRule = wizParseRoot->Find("event_output-event-type");
                  if (wizEventOutputEventTypeRule)
                  {
                     auto wizEventOutputEventTypeAlternate = wizEventOutputEventTypeRule->GetSubordinateRule();
                     AcquireEventsFromParserH(mAvailableGroups, wizEventOutputEventTypeAlternate);
                  }
               }
            }
         }
      }
   }

   // make "all" the name of the root group
   mAvailableGroups.mTitle = "all";
   // add it to the available commands
   mAvailableCommands.append(mAvailableGroups.mTitle);
}

void Widget::AcquireEventsFromParserH(GroupData& aData, WsfParseRule* aRulePtr)
{
   if (aRulePtr->Type() == WsfParseRule::cRULE_REFERENCE)
   {
      auto actualRule = aRulePtr->GetSubordinateRule();
      if (actualRule)
      {
         AcquireEventsFromParserH(aData, actualRule);
      }
   }
   else if (aRulePtr->Type() == WsfParseRule::cNAMED_RULE)
   {
      auto subRule = aRulePtr->GetSubordinateRule();
      if (subRule)
      {
         GroupData subData;
         subData.mTitle = RuleToEventGroup(QString::fromStdString(aRulePtr->GetRuleDescription()));
         AcquireEventsFromParserH(subData, subRule);
         aData.mGroups.append(subData);
      }
   }
   else if (aRulePtr->Type() == WsfParseRule::cALTERNATE)
   {
      auto ruleAlternates = aRulePtr->GetAlternates();
      if (!ruleAlternates.empty())
      {
         for (const auto& alt : ruleAlternates)
         {
            AcquireEventsFromParserH(aData, alt);
         }
      }
   }
   else if (aRulePtr->Type() == WsfParseRule::cLITERAL)
   {
      auto eventName = QString::fromStdString(static_cast<WsfParseLiteral*>(aRulePtr)->GetRuleDescription());
      mAvailableCommands.append(eventName);
      aData.mItems.append(eventName);
   }
}

} // namespace event
} // namespace EventOutput
} // namespace wizard
