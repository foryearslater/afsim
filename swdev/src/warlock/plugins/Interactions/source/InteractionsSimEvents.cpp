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

#include "InteractionsSimEvents.hpp"

#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "VaViewer.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "interaction/WkfAttachmentInteraction.hpp"

bool WkInteractions::InteractionEvent::Process(vespa::VaViewer& aViewer, const wkf::InteractionPrefObject* aPrefObjectPtr)
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      vespa::VaEntity* sourceEntityPtr = scenarioPtr->FindPlatformByIndex(mSourcePlatformIndex);
      vespa::VaEntity* targetEntityPtr = scenarioPtr->FindPlatformByIndex(mTargetPlatformIndex);

      if (sourceEntityPtr && targetEntityPtr)
      {
         auto* tgtIntPtr = targetEntityPtr->FindFirstAttachmentOfType<wkf::AttachmentInteraction>();
         auto* srcIntPtr = sourceEntityPtr->FindFirstAttachmentOfType<wkf::AttachmentInteraction>();

         if (!tgtIntPtr)
         {
            tgtIntPtr = vespa::make_attachment<wkf::AttachmentInteraction>(*targetEntityPtr,
                                                                           &aViewer,
                                                                           "WkfAttachmentInteraction",
                                                                           aPrefObjectPtr);
            vespa::VaAttachment::LoadAttachment(*tgtIntPtr);
         }

         if (!srcIntPtr)
         {
            srcIntPtr = vespa::make_attachment<wkf::AttachmentInteraction>(*sourceEntityPtr,
                                                                           &aViewer,
                                                                           "WkfAttachmentInteraction",
                                                                           aPrefObjectPtr);
            vespa::VaAttachment::LoadAttachment(*srcIntPtr);
         }

         if (tgtIntPtr)
         {
            tgtIntPtr->SetStackingAllowed(aPrefObjectPtr->GetStackingAllowed());
            mStart ? tgtIntPtr->AddInteraction(std::make_pair(mType, wkf::AttachmentInteraction::eINCOMING),
                                               sourceEntityPtr,
                                               mAuxText,
                                               mId) :
                     tgtIntPtr->RemoveInteraction(std::make_pair(mType, wkf::AttachmentInteraction::eINCOMING),
                                                  sourceEntityPtr,
                                                  mId);
         }

         if (srcIntPtr)
         {
            srcIntPtr->SetStackingAllowed(aPrefObjectPtr->GetStackingAllowed());
            mStart ? srcIntPtr->AddInteraction(std::make_pair(mType, wkf::AttachmentInteraction::eOUTGOING),
                                               targetEntityPtr,
                                               mAuxText,
                                               mId) :
                     srcIntPtr->RemoveInteraction(std::make_pair(mType, wkf::AttachmentInteraction::eOUTGOING),
                                                  targetEntityPtr,
                                                  mId);
         }

         return true;
      }
   }
   return false;
}
