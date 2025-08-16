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

#include "ScenarioTransformationDialogInterface.hpp"

#include "Environment.hpp"
#include "GhostingInterface.hpp"
#include "MapUtilsPlugin.hpp"
#include "Platform.hpp"
#include "ScenarioTransformation.hpp"
#include "UtQtMemory.hpp"
#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "VtkInterface.hpp"
#include "WkfAttachmentModel.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPluginManager.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

namespace MapUtils
{
namespace ScenarioTransformation
{

DialogInterface::DialogInterface(QWidget* aParent, Qt::WindowFlags aFlags, const Message* aHelpText)
   : QDialog(aParent, aFlags)
   , mEntitiesToGhost(PluginUtil::WhatEntities::cNONE)
   , mHelpTextPtr(aHelpText)
{
   connect(Plugin::Instance(), &Plugin::ForceClose, this, &DialogInterface::OnForceClose);
}

void DialogInterface::OnForceClose(DialogInterface* aDialogPtr)
{
   if (aDialogPtr != nullptr)
   {
      aDialogPtr->reject();
   }
}

void DialogInterface::ApplyPressed()
{
   if (mClickedButton != WhatButton::cOKAY && mClickedButton != WhatButton::cAPPLY)
   {
      mClickedButton = WhatButton::cAPPLY;
   }

   vespa::VaEntityMap      realEntityMap;
   vespa::VaAttachmentList realAttachmentList;

   wkf::Scenario* scenarioPtr   = vaEnv.GetStandardScenario();
   auto           ghostEntities = ghostMan->GetGhostEntities();
   for (const auto& currentGhost : ghostEntities)
   {
      std::string      currentGhostEntityName = currentGhost->GetName();
      std::string      realEntityName         = ghost::Manager::RealEntityName(currentGhostEntityName);
      vespa::VaEntity* realEntityPtr          = scenarioPtr->FindEntity(realEntityName);
      if (realEntityPtr != nullptr)
      {
         // apply the ghost entity's 6DOF to the real entity
         ApplyChangesToEntity6DOF(currentGhost, realEntityPtr);

         // Add entity to the entity map
         realEntityMap.emplace(realEntityPtr->GetUniqueId(), realEntityPtr);
      }

      vespa::VaEntity::Attachments& ghostAttachments = currentGhost->GetAttachments();
      for (size_t index = 0; index < ghostAttachments.size(); ++index)
      {
         vespa::VaAttachment* realAttachmentPtr = realEntityPtr->FindAttachment(ghostAttachments[index]->GetName());
         // check to see if the attachment exists
         if (realAttachmentPtr != nullptr)
         {
            // attachment is a wkf::AttachmentRoute
            if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentRoute>())
            {
               ApplyRouteChange(realAttachmentPtr, ghostAttachments[index].get());
            }
            // attachment is a wkf::AttachmentZoneCircle
            else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZoneCircle>())
            {
               ApplyCircleZoneChange(realAttachmentPtr, ghostAttachments[index].get());
            }
            // attachment is a wkf::AttachmentZoneCircle
            else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>())
            {
               ApplyEllipseZoneChange(realAttachmentPtr, ghostAttachments[index].get());
            }
            // attachment is a wkf::AttachmentZoneCircle
            else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZonePolygon>())
            {
               ApplyPolygonZoneChange(realAttachmentPtr, ghostAttachments[index].get());
            }
            // add attachment to the attachment list
            realAttachmentList.push_back(realAttachmentPtr);
         }
      }
   }

   // Update the entities and routes using the wkf::VtkEnvironments standard wkf::Scenario
   // The wkf::Scenario will emit the wkfEnv.MoveSelectionAndAttachmentComplete, thus notifying the VtkInterface to modify the text
   scenarioPtr->MoveSelectionComplete(vaEnv.GetStandardViewer(), realEntityMap, realAttachmentList);
}

void DialogInterface::ResetPressed()
{
   if (mClickedButton != WhatButton::cRESET)
   {
      mClickedButton = WhatButton::cRESET;
   }
   if (!mDidAProblemOccur)
   {
      // reset the ghosts
      ghostMan->ResetGhostEntitiesAndAttachments();
      // run the reset routine for the specific ScenarioTransformation dialog
      ResetP();
   }
}

void DialogInterface::AcceptP()
{
   // if (mClickedButton == WhatButton::cOKAY)
   //{
   ApplyPressed();
   // destroy the ghost scenario/selection
   ghostMan->DestructGhostEntitiesAndAttachments();
   // remove the centroid from the scenario
   // delete mCentroidPtr;
   //}
   QDialog::accept();
}

void DialogInterface::RejectP()
{
   // if (mClickedButton == WhatButton::cRESET)
   //{
   // destroy the ghost scenario/selection
   ghostMan->DestructGhostEntitiesAndAttachments();
   // remove the centroid from the scenario
   // delete mCentroidPtr;
   //}
   QDialog::reject();
}

void DialogInterface::BeforeModeSwitch()
{
   ResetPressed();
}

void DialogInterface::AfterModeSwitch()
{
   QSize thisSizeHint{sizeHint()};
   setMinimumSize(thisSizeHint);
   setMaximumSize(thisSizeHint);
   // create the ghost scenario/selection
   ghostMan->ConstructGhostEntitiesAndAttachments(mEntitiesToGhost);
   // get the ghost centroid
   mCentroid = ghostMan->GetGhostCentroid();
   InitializeComponents();
   UI_Enable();
}

void DialogInterface::HandleMessage(const Message* aProblem)
{
   if (aProblem != nullptr)
   {
      // display the problem to the user
      aProblem->Display();
      // handle the post-message throwing behavior specific to MapUtils::ScenarioTransformation dialogs
      switch (aProblem->GetLevel())
      {
      case Message::Level::Critical: // critical-level exceptions:  crash the plug-in
         QDialog::reject();
         break;
      case Message::Level::Warning: // warning-level exceptions:  set the mDidAProblemOccur flag
         mDidAProblemOccur = true;
         UI_Disable();
         break;
      case Message::Level::Information: // information-level exceptions:  ignore
         break;
      default: // other/default exception:  crash the plug-in
         QDialog::reject();
         break;
      }
   }
}

} // namespace ScenarioTransformation
} // namespace MapUtils
