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
#include "CommentPlugin.hpp"

#include "CommentDataContainer.hpp"
#include "UtTime.hpp"
#include "VaAttachment.hpp"
#include "VaTimeManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfVtkEnvironment.hpp"
#include "annotation/WkfAttachmentDecorator.hpp"
#include "comment/WkfCommentPrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkComment::Plugin,
   "Comment Bubbles",
   "The Comment plugin processes comment commands from AFSIM and displays comment bubbles on the map display.",
   "warlock")

WkComment::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new wkf::CommentPrefWidget)
   , mDockWidgetPtr(new wkf::CommentDockWidget(wkfEnv.GetMainWindow()))
   , mEnabled(false)
{
   connect(&wkfEnv,
           &wkf::Environment::Initialize,
           this,
           [this]()
           {
              mEnabled = true;
              mInterfacePtr->SetEnabled(mEnabled);
           });

   wkfEnv.GetMainWindow()->addDockWidget(Qt::BottomDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   // If they disabled comments in the preferences, remove all comments.
   connect(mPrefWidgetPtr->GetPreferenceObject(), &wkf::CommentPrefObject::EnabledChanged, this, &Plugin::EnabledChangedHandler);
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::CommentPrefObject::ShowTimestampChanged,
           this,
           &Plugin::ShowTimestampChangedHandler);
}

QList<wkf::PrefWidget*> WkComment::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}


void WkComment::Plugin::GuiUpdate()
{
   if (mEnabled)
   {
      Cleanup();                                    // Remove comments that have lived beyond the timeout
      mInterfacePtr->ProcessEvents(mDataContainer); // Process all the new SimEvents
   }
}

void WkComment::Plugin::Cleanup()
{
   std::vector<unsigned int> removeIdList;
   // Remove comments that have lived beyond the timeout
   for (const auto& c : mDataContainer.GetComments())
   {
      int    timeout      = mPrefWidgetPtr->GetPreferenceObject()->GetTimeoutSec();
      double creationTime = c.second.time;
      double currentTime  = vaEnv.GetTime()->GetCurrentTime();

      // If time have expired
      if (currentTime - creationTime > timeout)
      {
         removeIdList.push_back(c.first);
      }
   }
   RemoveComments(removeIdList);
}

void WkComment::Plugin::RemoveComments(const std::vector<unsigned int>& aCommentIds)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   for (const auto& id : aCommentIds)
   {
      if (scenario != nullptr)
      {
         // Remove the associated attachment if one exists
         auto attachment = scenario->FindAttachment(id);
         if (attachment != nullptr)
         {
            attachment->GetParent().RemoveAttachment(id);
         }
      }
      // Remove the comment regardless of whether a standard scenario exist or not
      mDataContainer.Remove(id);
   }
}

void WkComment::Plugin::EnabledChangedHandler(bool aEnabled)
{
   if (!aEnabled)
   {
      // remove all comment attachments
      for (const auto& c : mDataContainer.GetComments())
      {
         wkf::Scenario* scenario = vaEnv.GetStandardScenario();
         if (scenario != nullptr)
         {
            auto attachment = scenario->FindAttachment(c.first);
            if (attachment != nullptr)
            {
               attachment->GetParent().RemoveAttachment(c.first);
            }
         }
      }
      mDataContainer.Clear();
   }
}

void WkComment::Plugin::ShowTimestampChangedHandler(bool aShow)
{
   // Reset the comment text to reflect the change in the 'show timestamp' state.
   for (auto& c : mDataContainer.GetComments())
   {
      std::string message = c.second.message;
      if (aShow)
      {
         // Prepend the timestamp to the message
         UtTime timestamp;
         message = timestamp.ToString(c.second.time, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting()) +
                   " " + message;
      }

      wkf::Scenario* scenario = vaEnv.GetStandardScenario();
      if (scenario != nullptr)
      {
         auto attachment = scenario->FindAttachment(c.first);
         if (attachment != nullptr)
         {
            auto* comment = dynamic_cast<wkf::AttachmentDecorator*>(attachment);
            if (comment != nullptr)
            {
               comment->SetText(QString::fromStdString(message));
            }
         }
      }
   }
}
