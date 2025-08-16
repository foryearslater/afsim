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

#include "CommentSimEvents.hpp"

#include <QColor>

#include "UtTime.hpp"
#include "VaTimeManager.hpp"
#include "VaUtils.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "annotation/WkfAttachmentDecorator.hpp"
#include "comment/WkfCommentForwardingService.hpp"
#include "comment/WkfCommentPrefObject.hpp"

void WkComment::SimIntializedEvent::Process(WkComment::DataContainer& aComments)
{
   aComments.Clear();
}

WkComment::AddEvent::AddEvent(double aTime, const std::string& aPlatformName, const std::string& aMessage)
   : CommentSimEvent()
   , mCreationTime(aTime)
   , mPlatformName(aPlatformName)
   , mMessage(aMessage)
{
}

void WkComment::AddEvent::Process(WkComment::DataContainer& aComments)
{
   wkf::CommentForwardingService::CommentReceived(mCreationTime, mPlatformName.c_str(), mMessage.c_str());

   if (wkfEnv.GetPreferenceObject<wkf::CommentPrefObject>()->GetEnabled())
   {
      wkf::Viewer*   viewer   = vaEnv.GetStandardViewer();
      wkf::Scenario* scenario = vaEnv.GetStandardScenario();

      if (viewer != nullptr && scenario != nullptr)
      {
         wkf::Platform* entityPtr = scenario->FindPlatformByName(mPlatformName);

         if (entityPtr != nullptr)
         {
            // Add timestamp to message if appropriate
            std::string message = mMessage;
            if (wkfEnv.GetPreferenceObject<wkf::CommentPrefObject>()->GetShowTimestamp())
            {
               // Prepend the timestamp to the message
               UtTime timestamp;
               message =
                  timestamp.ToString(mCreationTime, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting()) +
                  " " + mMessage;
            }

            // If a decorator already exists for the entity, set the text and update the creation time in the map.
            vespa::VaAttachment* attachment = entityPtr->FindAttachment("decorator");
            if (attachment != nullptr)
            {
               auto* decorator = dynamic_cast<wkf::AttachmentDecorator*>(attachment);
               if (decorator != nullptr)
               {
                  decorator->SetText(QString::fromStdString(message));
                  aComments.Set(decorator->GetUniqueId(), mCreationTime, mMessage);
               }
            }
            // If a decorator does not exist for the entity, create one and add it to the map.
            else
            {
               QString team  = QString::fromStdString(entityPtr->GetSide());
               QColor  color = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamColor(team);

               auto comment =
                  vespa::make_attachment<wkf::AttachmentDecorator>(*entityPtr, viewer, QString::fromStdString(message));
               comment->SetColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());

               vespa::VaAttachment::LoadAttachment(*comment);
               aComments.Add(comment->GetUniqueId(), mCreationTime, mMessage);
            }
         }
      }
   }
}
