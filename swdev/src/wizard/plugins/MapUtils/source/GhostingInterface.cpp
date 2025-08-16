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

#include "GhostingInterface.hpp"

#include "Platform.hpp"
#include "PluginUtil.hpp"
#include "ScenarioTransformation.hpp"
#include "UtColor.hpp"
#include "VaEntity.hpp"
#include "VaPosition.hpp"
#include "VaUtils.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "annotation/WkfAttachmentRangeRing.hpp"
#include "zone/WkfAttachmentZone.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfAttachmentZoneSphere.hpp"
#include "zone/WkfZonePoint.hpp"

namespace MapUtils
{
namespace ghost
{

std::unique_ptr<Manager> Manager::mInstancePtr = nullptr;

Manager* Manager::Instance()
{
   assert(mInstancePtr != nullptr);
   return mInstancePtr.get();
}

void Manager::Create()
{
   assert(mInstancePtr == nullptr);
   // Need to do it this way, since default constructor is private
   mInstancePtr.reset(new Manager());
}

void Manager::ConstructGhostEntityAndAttachments(const vespa::VaEntity* aRealEntityPtr)
{
   // acquire the scenario and viewer for later use
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   wkf::Viewer*   viewerPtr   = vaEnv.GetStandardViewer();

   // name the ghost with a convenient name
   // warning:  changing the format of the ghost name can lead to FindGhostEntity not working correctly
   std::string ghostEntityStr = GhostEntityName(aRealEntityPtr->GetName());
   // check to see if the ghost entity already exists
   vespa::VaEntity* oldGhostEntityPtr = scenarioPtr->FindEntity(ghostEntityStr);
   if (oldGhostEntityPtr != nullptr)
   {
      scenarioPtr->DeleteEntity(oldGhostEntityPtr);
   }
   UtColor currentColor;
   UtColor modelColor;

   // construct the attachment zone list
   VaZoneList zoneList = GetRealZones(aRealEntityPtr);
   // construct the attachment range ring list
   RangeRingList rangeRingList = GetRealRangeRings(aRealEntityPtr);

   vespa::VaEntityPtr newGhostEntityPtr;
   // construct the ghost if the entity is a wizard::Platform
   //  Note:  There is no need to clone an actual wizard::Platform as there is no need for the proxy information.
   //         Therefore, delegate to the wkf::Platform cloning routine.
   // construct the ghost if the entity is a wkf::Platform
   if (aRealEntityPtr->IsA_TypeOf<wizard::Platform>() || aRealEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      // cast the entity to a wkf::Platform
      auto wkfPlatformPtr = static_cast<const wkf::Platform*>(aRealEntityPtr);
      newGhostEntityPtr = ConstructGhostWkfPlatform(viewerPtr, wkfPlatformPtr, zoneList, rangeRingList, ghostEntityStr);
   }
   // construct ghost if the entity is a wkf::Bullseye
   else if (aRealEntityPtr->IsA_TypeOf<wkf::Bullseye>())
   {
      // cast the entity to a wkf::Bullseye
      auto wkfBullseyePtr = static_cast<const wkf::Bullseye*>(aRealEntityPtr);
      newGhostEntityPtr = ConstructGhostWkfBullseye(viewerPtr, wkfBullseyePtr, zoneList, rangeRingList, ghostEntityStr);
   }
   // construct ghost if the entity is a wkf::PointOfInterest
   else if (aRealEntityPtr->IsA_TypeOf<wkf::PointOfInterest>())
   {
      // cast the entity to a wkf::PointOfInterest
      auto wkfPointOfInterestPtr = static_cast<const wkf::PointOfInterest*>(aRealEntityPtr);
      newGhostEntityPtr =
         ConstructGhostWkfPointOfInterest(viewerPtr, wkfPointOfInterestPtr, zoneList, rangeRingList, ghostEntityStr);
   }
   // construct ghost if the entity is a wkf::Entity
   else if (aRealEntityPtr->IsA_TypeOf<wkf::Entity>())
   {
      // cast the entity to a wkf::Entity
      auto wkfEntityPtr = static_cast<const wkf::Entity*>(aRealEntityPtr);
      newGhostEntityPtr = ConstructGhostWkfEntity(viewerPtr, wkfEntityPtr, zoneList, rangeRingList, ghostEntityStr);
   }
   // construct ghost if the entity is a vespa::VaEntity
   else // if (currentEntity->IsA_TypeOf<vespa::VaEntity>())
   {
      newGhostEntityPtr = ConstructGhostVaEntity(viewerPtr, aRealEntityPtr, zoneList, rangeRingList, ghostEntityStr);
   }
   // Force the entity and its attachments to display on the Map
   ShowGhostEntityAndAttachments(newGhostEntityPtr.get());
   // Transfer ownership of the entity and its attachments to the scenario
   TransferGhostToScenario(std::move(newGhostEntityPtr));
   mGhostEntityList.push_back(ghostEntityStr);
}

void Manager::ResetGhostEntityAndAttachments(const std::string& aRealEntity, const std::string& aGhostEntity)
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();

   vespa::VaEntity* ghostEntityPtr = scenarioPtr->FindEntity(aGhostEntity);
   vespa::VaEntity* realEntityPtr  = FindRealEntity(aGhostEntity);
   ScenarioTransformation::ApplyChangesToEntity6DOF(realEntityPtr, ghostEntityPtr);

   vespa::VaEntity::Attachments& ghostAttachments = ghostEntityPtr->GetAttachments();
   for (size_t index = 0; index < ghostAttachments.size(); ++index)
   {
      vespa::VaAttachment* realAttachmentPtr = realEntityPtr->FindAttachment(ghostAttachments[index]->GetName());
      // check to see if the attachment exists
      if (realAttachmentPtr != nullptr)
      {
         // attachment is a wkf::AttachmentRoute
         if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentRoute>())
         {
            ScenarioTransformation::ApplyRouteChange(ghostAttachments[index].get(), realAttachmentPtr);
         }
         // attachment is a wkf::AttachmentZoneCircle
         else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZoneCircle>())
         {
            ScenarioTransformation::ApplyCircleZoneChange(ghostAttachments[index].get(), realAttachmentPtr);
         }
         // attachment is a wkf::AttachmentZoneEllipse
         else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>())
         {
            ScenarioTransformation::ApplyEllipseZoneChange(ghostAttachments[index].get(), realAttachmentPtr);
         }
         // attachment is a wkf::AttachmentZonePolygon
         else if (realAttachmentPtr->IsA_TypeOf<wkf::AttachmentZonePolygon>())
         {
            ScenarioTransformation::ApplyPolygonZoneChange(ghostAttachments[index].get(), realAttachmentPtr);
         }
      }
   }
}

Manager::EntityList::iterator Manager::DestructGhostEntityAndAttachments(const std::string aGhostEntity)
{
   wkf::Scenario*   scenarioPtr    = vaEnv.GetStandardScenario();
   vespa::VaEntity* ghostEntityPtr = scenarioPtr->FindEntity(aGhostEntity);
   if (ghostEntityPtr != nullptr)
   {
      scenarioPtr->DeleteEntity(ghostEntityPtr);
   }
   auto ghostIter = std::find(mGhostEntityList.begin(), mGhostEntityList.end(), aGhostEntity);
   if (ghostIter != mGhostEntityList.end())
   {
      vespa::VaEntity* realEntityPtr = scenarioPtr->FindEntity(RealEntityName(aGhostEntity));
      if (realEntityPtr == nullptr)
      {
         return mGhostEntityList.end();
      }
      return mGhostEntityList.erase(ghostIter);
   }
   return mGhostEntityList.end();
}

void Manager::ConstructGhostEntitiesAndAttachments(const PluginUtil::WhatEntities aEntitiesToGhost)
{
   EntityList destinationList;
   // acquire the real entities
   vespa::VaEntityList sourceList = PluginUtil::AcquireEntities(aEntitiesToGhost);

   // iterate through all of the entities in the scenario/selection
   for (const auto& currentEntity : sourceList)
   {
      ConstructGhostEntityAndAttachments(currentEntity);
   }
}

void Manager::ResetGhostEntitiesAndAttachments()
{
   for (const auto& currentGhostEntityName : mGhostEntityList)
   {
      ResetGhostEntityAndAttachments(RealEntityName(currentGhostEntityName), currentGhostEntityName);
   }
}

void Manager::DestructGhostEntitiesAndAttachments()
{
   EntityList::iterator ghostIter = mGhostEntityList.begin();
   while (ghostIter != mGhostEntityList.end())
   {
      ghostIter = DestructGhostEntityAndAttachments(*ghostIter);
   }
   mGhostEntityList.clear();
}

vespa::VaPosition Manager::GetGhostCentroid(const EntityList& aEntityList)
{
   VaPositionList positionList;
   for (const auto& entity : GetGhostEntities(aEntityList))
   {
      // add the entity position to the list
      positionList.push_back(entity->GetPosition());
   }
   return vespa::VaPosition::ComputeCentroid(positionList);
}

vespa::VaPosition Manager::GetGhostCentroid()
{
   return GetGhostCentroid(mGhostEntityList);
}

vespa::VaEntityList Manager::GetGhostEntities(const EntityList& aEntityList)
{
   wkf::Scenario*      scenarioPtr = vaEnv.GetStandardScenario();
   vespa::VaEntity*    entityPtr;
   vespa::VaEntityList entityList;
   for (const auto& entity : aEntityList)
   {
      // acquire the entity
      entityPtr = scenarioPtr->FindEntity(entity);
      if (entityPtr != nullptr)
      {
         // add the entity pointer to the list
         entityList.push_back(entityPtr);
      }
   }
   return entityList;
}

vespa::VaEntityList Manager::GetGhostEntities()
{
   return GetGhostEntities(mGhostEntityList);
}

UtColor Manager::GetGhostModelColor(const vespa::VaEntity* aEntityPtr)
{
   UtColor currentColor = cGHOST_ENTITY_COLOR;
   UtColor modelColor;
   if (aEntityPtr->IsA_TypeOf<wizard::Platform>() || aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      auto platform = static_cast<const wkf::Platform*>(aEntityPtr);
      wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(platform->GetSide()).Get(currentColor);
      modelColor.Set(currentColor[0], currentColor[1], currentColor[2], 0.5f);
   }
   return modelColor;
}

UtColor Manager::GetGhostRouteColor(const vespa::VaEntity* aEntityPtr)
{
   return GetGhostModelColor(aEntityPtr);
}

bool Manager::AddGhostToScenario(vespa::VaEntity* aGhostEntityPtr) const
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr != nullptr)
   {
      return scenarioPtr->AddEntity(aGhostEntityPtr);
   }
   return false;
}

bool Manager::TransferGhostToScenario(vespa::VaEntityPtr aGhostEntityPtr) const
{
   if (aGhostEntityPtr != nullptr)
   {
      wkf::Scenario* scenarioPtr{vaEnv.GetStandardScenario()};
      if (scenarioPtr != nullptr)
      {
         auto entityPtr = scenarioPtr->FindEntity(aGhostEntityPtr->GetName());
         if (entityPtr == aGhostEntityPtr.get())
         {
            aGhostEntityPtr.release();
            return true;
         }
      }
   }
   return false;
}

std::string Manager::GhostEntityName(const std::string& aRealEntity)
{
   return aRealEntity + "_ghost";
}

std::string Manager::RealEntityName(const std::string& aGhostEntity)
{
   return aGhostEntity.substr(0, aGhostEntity.find("_ghost"));
}

vespa::VaEntityPtr Manager::ConstructGhostVaEntity(wkf::Viewer*           aViewerPtr,
                                                   const vespa::VaEntity* aRealEntityPtr,
                                                   const VaZoneList&      aRealZoneList,
                                                   const RangeRingList&   aRealRangeRingList,
                                                   const std::string&     aGhostEntityName)
{
   auto ghostEntityPtr = ut::make_unique<vespa::VaEntity>(aGhostEntityName);
   // Add the ghost vespa::VaEntity to the scenario WITHOUT transferring ownership.
   if (ghostEntityPtr != nullptr && AddGhostToScenario(ghostEntityPtr))
   {
      // Initialize and load the vespa::VaEntity
      ghostEntityPtr->Initialize();
      ghostEntityPtr->Load();

      // set general entity properties
      ghostEntityPtr->SetScenario(aRealEntityPtr->GetScenario());
      ghostEntityPtr->SetDraggable(false);

      auto realModelPtr = GetRealModel(aRealEntityPtr);
      // construct the ghost attachment model
      ConstructGhostAttachmentModel(aViewerPtr, realModelPtr, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));

      auto realRoutePtr = GetRealRoute(aRealEntityPtr);
      // construct the ghost route
      ConstructGhostAttachmentRoute(aViewerPtr, realRoutePtr, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));

      // construct the ghost zone(s)
      for (const auto& zone : aRealZoneList)
      {
         ConstructGhostAttachmentZone(aViewerPtr, zone, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));
      }

      // construct the ghost range ring(s)
      for (const auto& rangeRing : aRealRangeRingList)
      {
         ConstructGhostAttachmentRangeRing(aViewerPtr, rangeRing, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));
      }

      // finish setting up ghost vespa::VaEntity
      ghostEntityPtr->SetPosition(aRealEntityPtr->GetPosition());
      ghostEntityPtr->SetHeading(aRealEntityPtr->GetHeading());
   }
   return ghostEntityPtr;
}

wkf::EntityPtr Manager::ConstructGhostWkfEntity(wkf::Viewer*         aViewerPtr,
                                                const wkf::Entity*   aRealEntityPtr,
                                                const VaZoneList&    aRealZoneList,
                                                const RangeRingList& aRealRangeRingList,
                                                const std::string&   aGhostEntityName)
{
   auto ghostEntityPtr = ut::make_unique<wkf::Entity>(aGhostEntityName);
   // Add the ghost wkf::Entity to the scenario WITHOUT transferring ownership.
   if (ghostEntityPtr != nullptr && AddGhostToScenario(ghostEntityPtr))
   {
      // Initialize and load the wkf::Entity
      ghostEntityPtr->Initialize();
      ghostEntityPtr->Load();

      // set general entity properties
      ghostEntityPtr->SetScenario(aRealEntityPtr->GetScenario());
      ghostEntityPtr->SetDraggable(false);

      auto realModelPtr = GetRealModel(aRealEntityPtr);
      // construct the ghost attachment model
      ConstructGhostAttachmentModel(aViewerPtr, realModelPtr, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));

      // there is no need to create an attachment route for a wkf::Entity

      // construct the ghost zone(s)
      for (const auto& zone : aRealZoneList)
      {
         ConstructGhostAttachmentZone(aViewerPtr, zone, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));
      }

      // construct the ghost range ring(s)
      for (const auto& rangeRing : aRealRangeRingList)
      {
         ConstructGhostAttachmentRangeRing(aViewerPtr, rangeRing, static_cast<vespa::VaEntity*>(ghostEntityPtr.get()));
      }

      // finish setting up ghost wkf::Entity
      ghostEntityPtr->SetPosition(aRealEntityPtr->GetPosition());
      ghostEntityPtr->SetHeading(aRealEntityPtr->GetHeading());
   }
   return ghostEntityPtr;
}

wkf::PointOfInterestPtr Manager::ConstructGhostWkfPointOfInterest(wkf::Viewer*                aViewerPtr,
                                                                  const wkf::PointOfInterest* aRealEntityPtr,
                                                                  const VaZoneList&           aRealZoneList,
                                                                  const RangeRingList&        aRealRangeRingList,
                                                                  const std::string&          aGhostEntityName)
{
   auto ghostPointOfInterestPtr = ut::make_unique<wkf::PointOfInterest>();
   // Add the ghost wkf::PointOfInterest to the scenario WITHOUT transferring ownership.
   if (ghostPointOfInterestPtr != nullptr && AddGhostToScenario(ghostPointOfInterestPtr))
   {
      // Initialize and load the wkf::PointOfInterest
      ghostPointOfInterestPtr->Initialize();
      ghostPointOfInterestPtr->Load();

      // set general POI properties
      ghostPointOfInterestPtr->SetName(aGhostEntityName);
      ghostPointOfInterestPtr->SetScenario(aRealEntityPtr->GetScenario());
      ghostPointOfInterestPtr->SetDraggable(false);

      auto realModelPtr = GetRealModel(aRealEntityPtr);
      // construct the ghost attachment model
      ConstructGhostAttachmentModel(aViewerPtr, realModelPtr, static_cast<vespa::VaEntity*>(ghostPointOfInterestPtr.get()));

      // there is no need to create an attachment route for a wkf::PointOfInterest

      // construct the ghost zone(s)
      for (const auto& zone : aRealZoneList)
      {
         ConstructGhostAttachmentZone(aViewerPtr, zone, static_cast<vespa::VaEntity*>(ghostPointOfInterestPtr.get()));
      }

      // construct the ghost range ring(s)
      for (const auto& rangeRing : aRealRangeRingList)
      {
         ConstructGhostAttachmentRangeRing(aViewerPtr,
                                           rangeRing,
                                           static_cast<vespa::VaEntity*>(ghostPointOfInterestPtr.get()));
      }

      // finish setting up ghost wkf::PointOfInterest
      ghostPointOfInterestPtr->SetPosition(aRealEntityPtr->GetPosition());
      ghostPointOfInterestPtr->SetHeading(aRealEntityPtr->GetHeading());
   }
   return ghostPointOfInterestPtr;
}

wkf::BullseyePtr Manager::ConstructGhostWkfBullseye(wkf::Viewer*         aViewerPtr,
                                                    const wkf::Bullseye* aRealEntityPtr,
                                                    const VaZoneList&    aRealZoneList,
                                                    const RangeRingList& aRealRangeRingList,
                                                    const std::string&   aGhostEntityName)
{
   auto ghostBullseyePtr = ut::make_unique<wkf::Bullseye>();
   // Add the ghost wkf::Bullseye to the scenario WITHOUT transferring ownership.
   if (ghostBullseyePtr != nullptr && AddGhostToScenario(ghostBullseyePtr))
   {
      // Initialize and load the wkf::Bullseye
      ghostBullseyePtr->Initialize();
      ghostBullseyePtr->Load();

      // set general bullseye properties
      ghostBullseyePtr->SetName(aGhostEntityName);
      ghostBullseyePtr->SetScenario(aRealEntityPtr->GetScenario());
      ghostBullseyePtr->SetDraggable(false);

      auto realModelPtr = GetRealModel(aRealEntityPtr);
      // construct the ghost attachment model
      ConstructGhostAttachmentModel(aViewerPtr, realModelPtr, static_cast<vespa::VaEntity*>(ghostBullseyePtr.get()));

      // there is no need to create an attachment route for a wkf::Bullseye

      // construct the ghost zone(s)
      for (const auto& zone : aRealZoneList)
      {
         ConstructGhostAttachmentZone(aViewerPtr, zone, static_cast<vespa::VaEntity*>(ghostBullseyePtr.get()));
      }

      // construct the ghost range ring(s)
      for (const auto& rangeRing : aRealRangeRingList)
      {
         ConstructGhostAttachmentRangeRing(aViewerPtr, rangeRing, static_cast<vespa::VaEntity*>(ghostBullseyePtr.get()));
      }

      // finish setting up ghost wkf::Bullseye
      ghostBullseyePtr->SetPosition(aRealEntityPtr->GetPosition());
      ghostBullseyePtr->SetHeading(aRealEntityPtr->GetHeading());
   }
   return ghostBullseyePtr;
}

wkf::PlatformPtr Manager::ConstructGhostWkfPlatform(wkf::Viewer*         aViewerPtr,
                                                    const wkf::Platform* aRealEntityPtr,
                                                    const VaZoneList&    aRealZoneList,
                                                    const RangeRingList& aRealRangeRingList,
                                                    const std::string&   aGhostEntityName)
{
   auto ghostPlatformPtr = ut::make_unique<wkf::Platform>(aGhostEntityName);
   // Add the ghost wkf::Platform to the scenario WITHOUT transferring ownership.
   if (ghostPlatformPtr != nullptr && AddGhostToScenario(ghostPlatformPtr))
   {
      // Initialize and load the wkf::Platform
      ghostPlatformPtr->Initialize();
      ghostPlatformPtr->Load();

      // set platform properties
      ghostPlatformPtr->SetIcon(aRealEntityPtr->GetIcon());
      ghostPlatformPtr->SetSide(aRealEntityPtr->GetSide());
      ghostPlatformPtr->SetSpatialDomain(aRealEntityPtr->GetSpatialDomain());
      ghostPlatformPtr->SetTypeList(aRealEntityPtr->GetTypeList());
      ghostPlatformPtr->SetDraggable(false);

      auto realModelPtr = GetRealModel(aRealEntityPtr);
      // construct the ghost attachment model
      ConstructGhostAttachmentModel(aViewerPtr, realModelPtr, static_cast<vespa::VaEntity*>(ghostPlatformPtr.get()));

      auto realRoutePtr = GetRealRoute(aRealEntityPtr);
      // construct the ghost route
      ConstructGhostAttachmentRoute(aViewerPtr, realRoutePtr, static_cast<vespa::VaEntity*>(ghostPlatformPtr.get()));

      // construct the ghost zone(s)
      for (const auto& zone : aRealZoneList)
      {
         ConstructGhostAttachmentZone(aViewerPtr, zone, static_cast<vespa::VaEntity*>(ghostPlatformPtr.get()));
      }

      // construct the ghost range ring(s)
      for (const auto& rangeRing : aRealRangeRingList)
      {
         ConstructGhostAttachmentRangeRing(aViewerPtr, rangeRing, static_cast<vespa::VaEntity*>(ghostPlatformPtr.get()));
      }

      // finish setting up ghost wkf::Platform
      ghostPlatformPtr->SetPosition(aRealEntityPtr->GetPosition());
      ghostPlatformPtr->SetHeading(aRealEntityPtr->GetHeading());
   }
   return ghostPlatformPtr;
}

void Manager::ConstructGhostAttachmentModel(wkf::Viewer*                    aViewerPtr,
                                            const vespa::VaAttachmentModel* aRealModelPtr,
                                            vespa::VaEntity*                aGhostEntityPtr)
{
   UtColor modelColor = GetGhostModelColor(aGhostEntityPtr);
   if (aGhostEntityPtr->IsA_TypeOf<wizard::Platform>() || aGhostEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      auto ghostPlatformPtr = static_cast<wkf::Platform*>(aGhostEntityPtr);
      ConstructGhostAttachmentModelWkfPlatform(aViewerPtr, aRealModelPtr, modelColor, ghostPlatformPtr);
   }
   else if (aGhostEntityPtr->IsA_TypeOf<wkf::Bullseye>())
   {
      auto ghostBullseyePtr = static_cast<wkf::Bullseye*>(aGhostEntityPtr);
      ConstructGhostAttachmentModelWkfBullseye(aViewerPtr, aRealModelPtr, modelColor, ghostBullseyePtr);
   }
   else if (aGhostEntityPtr->IsA_TypeOf<wkf::PointOfInterest>())
   {
      auto ghostPointOfInterestPtr = static_cast<wkf::PointOfInterest*>(aGhostEntityPtr);
      ConstructGhostAttachmentModelWkfPointOfInterest(aViewerPtr, aRealModelPtr, modelColor, ghostPointOfInterestPtr);
   }
   // else if (aGhostEntityPtr->IsA_TypeOf<wkf::Entity>())
   // else if (aGhostEntityPtr->->IsA_TypeOf<vespa::VaEntity>())
   // no need to attach models to wkf::Entities or vespa::VaEntities (global anchor entities are hidden, thus should not have models)
}

void Manager::ConstructGhostAttachmentModelWkfPointOfInterest(wkf::Viewer*                    aViewerPtr,
                                                              const vespa::VaAttachmentModel* aRealModelPtr,
                                                              const UtColor&                  aModelColor,
                                                              wkf::PointOfInterest*           aGhostPointOfInterestPtr)
{
   if (aRealModelPtr != nullptr)
   {
      // the model for a wkf::PointOfInterest is created in the constructor, so no need to create another one
      auto* ghostModelPtr = aGhostPointOfInterestPtr->FindFirstAttachmentOfType<vespa::VaAttachmentModel>();
      if (ghostModelPtr != nullptr)
      {
         // Initialize and load the AttachmentModel
         ghostModelPtr->Initialize();
         ghostModelPtr->Load();

         ghostModelPtr->SetName(aRealModelPtr->GetName());
         ghostModelPtr->SetProxy(aRealModelPtr->IsProxy());
         ghostModelPtr->SetColor(aModelColor);
      }
   }
}

void Manager::ConstructGhostAttachmentModelWkfBullseye(wkf::Viewer*                    aViewerPtr,
                                                       const vespa::VaAttachmentModel* aRealModelPtr,
                                                       const UtColor&                  aModelColor,
                                                       wkf::Bullseye*                  aGhostBullseyePtr)
{
   if (aRealModelPtr != nullptr)
   {
      // the model for a wkf::Bullseye is created in the constructor, so no need to create another one
      auto* ghostModelPtr = aGhostBullseyePtr->FindFirstAttachmentOfType<wkf::AttachmentModel>();
      if (ghostModelPtr != nullptr)
      {
         // Initialize and load the AttachmentModel
         ghostModelPtr->Initialize();
         ghostModelPtr->Load();

         ghostModelPtr->SetName(aRealModelPtr->GetName());
         ghostModelPtr->SetProxy(aRealModelPtr->IsProxy());
         ghostModelPtr->SetColor(aModelColor);
      }
   }
}

void Manager::ConstructGhostAttachmentModelWkfPlatform(wkf::Viewer*                    aViewerPtr,
                                                       const vespa::VaAttachmentModel* aRealModelPtr,
                                                       const UtColor&                  aModelColor,
                                                       wkf::Platform*                  aGhostPlatformPtr)
{
   if (aRealModelPtr != nullptr)
   {
      auto ghostModelPtr = vespa::make_attachment<wkf::AttachmentModel>(*aGhostPlatformPtr, aViewerPtr);
      if (ghostModelPtr != nullptr)
      {
         // Initialize and load the AttachmentModel
         ghostModelPtr->Initialize();
         ghostModelPtr->Load();

         ghostModelPtr->SetName(aRealModelPtr->GetName());
         ghostModelPtr->SetProxy(aRealModelPtr->IsProxy());
         ghostModelPtr->SetColor(aModelColor);
         ghostModelPtr->SetIcon(aGhostPlatformPtr->GetIcon());
      }
   }
}

void Manager::ConstructGhostAttachmentRoute(wkf::Viewer*                aViewerPtr,
                                            const wkf::AttachmentRoute* aRealRoutePtr,
                                            vespa::VaEntity*            aGhostEntityPtr)
{
   UtColor routeColor = GetGhostRouteColor(aGhostEntityPtr);
   // wizard::Platforms and wkf::Platforms are the entities attached to local routes
   if (aGhostEntityPtr->IsA_TypeOf<wizard::Platform>() || aGhostEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      auto ghostPlatformPtr = static_cast<wkf::Platform*>(aGhostEntityPtr);
      ConstructGhostAttachmentRouteWkfPlatform(aViewerPtr, aRealRoutePtr, routeColor, ghostPlatformPtr);
   }
   // wkf::Bullseye and wkf::PointOfInterest will never have attachment routes
   // vespa::VaEntities are the entities attached to global routes, so wkf::Entities will not have routes
   // todo:  change the code if the above note does not apply anymore for a future release
   else // if (aGhostEntityPtr->IsA_TypeOf<vespa::VaEntity>())
   {
      ConstructGhostAttachmentRouteVaEntity(aViewerPtr, aRealRoutePtr, routeColor, aGhostEntityPtr);
   }
}

void Manager::ConstructGhostAttachmentRouteVaEntity(wkf::Viewer*                aViewerPtr,
                                                    const wkf::AttachmentRoute* aRealRoutePtr,
                                                    const UtColor&              aRouteColor,
                                                    vespa::VaEntity*            aGhostEntityPtr)
{
   // this "catch-all" assumes the type is vespa::VaEntity
   // no need to cast to vespa::VaEntity as the parameter is that type
   if (aRealRoutePtr != nullptr)
   {
      auto ghostRoutePtr = vespa::make_attachment<wkf::AttachmentRoute>(*aGhostEntityPtr, aViewerPtr);
      if (ghostRoutePtr != nullptr)
      {
         ghostRoutePtr->ShowIndices(false);
         // Initialize and load the AttachmentModel
         ghostRoutePtr->Initialize();
         ghostRoutePtr->Load();

         ghostRoutePtr->SetName(aRealRoutePtr->GetName());
         ghostRoutePtr->SetTeamColor(aRouteColor);
         ghostRoutePtr->UpdateTeamColor();
         ghostRoutePtr->SetSelectionEnabled(false);
         ghostRoutePtr->SetContextMenuEnabled(false);

         std::vector<wkf::Waypoint*> waypoints = aRealRoutePtr->GetWaypoints();

         int index = 0;
         for (const auto& currentWaypoint : waypoints)
         {
            ghostRoutePtr->InsertWaypoint(currentWaypoint->GetLat(),
                                          currentWaypoint->GetLon(),
                                          currentWaypoint->GetAlt(),
                                          currentWaypoint->GetGoToIndex(),
                                          currentWaypoint->GetLabel(),
                                          index);
            ++index;
         }
      }
   }
}

void Manager::ConstructGhostAttachmentRouteWkfPlatform(wkf::Viewer*                aViewerPtr,
                                                       const wkf::AttachmentRoute* aRealRoutePtr,
                                                       const UtColor&              aRouteColor,
                                                       wkf::Platform*              aGhostPlatformPtr)
{
   if (aRealRoutePtr != nullptr)
   {
      auto ghostRoutePtr = vespa::make_attachment<wkf::AttachmentRoute>(*aGhostPlatformPtr, aViewerPtr);
      if (ghostRoutePtr != nullptr)
      {
         ghostRoutePtr->ShowIndices(false);
         // Initialize and load the AttachmentRoute
         ghostRoutePtr->Initialize();
         ghostRoutePtr->Load();

         ghostRoutePtr->SetName(aRealRoutePtr->GetName());
         ghostRoutePtr->SetTeamColor(aRouteColor);
         ghostRoutePtr->UpdateTeamColor();
         ghostRoutePtr->SetSelectionEnabled(false);
         ghostRoutePtr->SetContextMenuEnabled(false);

         std::vector<wkf::Waypoint*> waypoints = aRealRoutePtr->GetWaypoints();

         int index = 0;
         for (const auto& currentWaypoint : waypoints)
         {
            ghostRoutePtr->InsertWaypoint(currentWaypoint->GetLat(),
                                          currentWaypoint->GetLon(),
                                          currentWaypoint->GetAlt(),
                                          currentWaypoint->GetGoToIndex(),
                                          currentWaypoint->GetLabel(),
                                          index);
            ++index;
         }
      }
   }
}

void Manager::ConstructGhostAttachmentZone(wkf::Viewer*               aViewerPtr,
                                           const wkf::AttachmentZone* aRealZonePtr,
                                           vespa::VaEntity*           aGhostEntityPtr)
{
   if (aRealZonePtr->IsA_TypeOf<wkf::AttachmentZoneCircle>())
   {
      auto zoneCirclePtr = static_cast<const wkf::AttachmentZoneCircle*>(aRealZonePtr);
      ConstructGhostAttachmentZoneCircle(aViewerPtr, zoneCirclePtr, aGhostEntityPtr);
   }
   else if (aRealZonePtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>())
   {
      auto zoneEllipsePtr = static_cast<const wkf::AttachmentZoneEllipse*>(aRealZonePtr);
      ConstructGhostAttachmentZoneEllipse(aViewerPtr, zoneEllipsePtr, aGhostEntityPtr);
   }
   else if (aRealZonePtr->IsA_TypeOf<wkf::AttachmentZonePolygon>())
   {
      auto zonePolygonPtr = static_cast<const wkf::AttachmentZonePolygon*>(aRealZonePtr);
      ConstructGhostAttachmentZonePolygon(aViewerPtr, zonePolygonPtr, aGhostEntityPtr);
   }
   else if (aRealZonePtr->IsA_TypeOf<wkf::AttachmentZoneSphere>())
   {
      auto zoneSpherePtr = static_cast<const wkf::AttachmentZoneSphere*>(aRealZonePtr);
      ConstructGhostAttachmentZoneSphere(aViewerPtr, zoneSpherePtr, aGhostEntityPtr);
   }
   else // if (aRealZonePtr->IsA_TypeOf<wkf::AttachmentZone>())
   {
      auto ghostZonePtr = vespa::make_attachment<wkf::AttachmentZone>(*aGhostEntityPtr, aViewerPtr);
      if (ghostZonePtr != nullptr)
      {
         // Initialize and load the AttachmentZone
         ghostZonePtr->Initialize();
         ghostZonePtr->Load();

         // set the general zone properties
         ghostZonePtr->SetName(aRealZonePtr->GetName()); // IMPORTANT!
         ghostZonePtr->SetColor(cGHOST_ZONE_LINE_COLOR);
         ghostZonePtr->SetFillColor(cGHOST_ZONE_FILL_COLOR);
         ghostZonePtr->SetBlendFactor(cGHOST_ZONE_FILL_COLOR[3]);
         ghostZonePtr->SetLineWidth(aRealZonePtr->GetLineWidth());
         ghostZonePtr->SetLineStyle(aRealZonePtr->GetLineStyle());
         ghostZonePtr->SetEnableFill(aRealZonePtr->GetEnableFill());
         ghostZonePtr->SetHeight(aRealZonePtr->GetHeight());
         ghostZonePtr->SetBaseHeight(aRealZonePtr->GetBaseHeight());
         ghostZonePtr->SetAuxData(aRealZonePtr->GetAuxData());

         // Reload the AttachmentZone
         ghostZonePtr->Load();
      }
   }
}

void Manager::ConstructGhostAttachmentZoneCircle(wkf::Viewer*                     aViewerPtr,
                                                 const wkf::AttachmentZoneCircle* aRealZoneCirclePtr,
                                                 vespa::VaEntity*                 aGhostEntityPtr)
{
   if (aRealZoneCirclePtr != nullptr)
   {
      auto ghostZoneCirclePtr = vespa::make_attachment<wkf::AttachmentZoneCircle>(*aGhostEntityPtr, aViewerPtr);
      if (ghostZoneCirclePtr != nullptr)
      {
         // Initialize and load the AttachmentZone
         ghostZoneCirclePtr->Initialize();
         ghostZoneCirclePtr->Load();

         // set the general zone properties
         ghostZoneCirclePtr->SetName(aRealZoneCirclePtr->GetName()); // IMPORTANT!
         ghostZoneCirclePtr->SetColor(cGHOST_ZONE_LINE_COLOR);
         ghostZoneCirclePtr->SetFillColor(cGHOST_ZONE_FILL_COLOR);
         ghostZoneCirclePtr->SetBlendFactor(cGHOST_ZONE_FILL_COLOR[3]);
         ghostZoneCirclePtr->SetLineWidth(aRealZoneCirclePtr->GetLineWidth());
         ghostZoneCirclePtr->SetLineStyle(aRealZoneCirclePtr->GetLineStyle());
         ghostZoneCirclePtr->SetEnableFill(aRealZoneCirclePtr->GetEnableFill());
         ghostZoneCirclePtr->SetHeight(aRealZoneCirclePtr->GetHeight());
         ghostZoneCirclePtr->SetBaseHeight(aRealZoneCirclePtr->GetBaseHeight());
         ghostZoneCirclePtr->SetAuxData(aRealZoneCirclePtr->GetAuxData());
         // set the circular zone properties
         ghostZoneCirclePtr->SetRadius(aRealZoneCirclePtr->GetRadius());
         ghostZoneCirclePtr->SetMinRadius(aRealZoneCirclePtr->GetMinRadius());
         ghostZoneCirclePtr->SetOrientation(aRealZoneCirclePtr->GetHeading(),
                                            aRealZoneCirclePtr->GetPitch(),
                                            aRealZoneCirclePtr->GetRoll());
         ghostZoneCirclePtr->SetBegAngle(aRealZoneCirclePtr->GetBegAngle());
         ghostZoneCirclePtr->SetEndAngle(aRealZoneCirclePtr->GetEndAngle());
         ghostZoneCirclePtr->SetX(aRealZoneCirclePtr->GetX());
         ghostZoneCirclePtr->SetY(aRealZoneCirclePtr->GetY());

         // Reload the AttachmentZone
         ghostZoneCirclePtr->Load();
      }
   }
}

void Manager::ConstructGhostAttachmentZoneEllipse(wkf::Viewer*                      aViewerPtr,
                                                  const wkf::AttachmentZoneEllipse* aRealZoneEllipsePtr,
                                                  vespa::VaEntity*                  aGhostEntityPtr)
{
   if (aRealZoneEllipsePtr != nullptr)
   {
      auto ghostZoneEllipsePtr = vespa::make_attachment<wkf::AttachmentZoneEllipse>(*aGhostEntityPtr, aViewerPtr);
      if (ghostZoneEllipsePtr != nullptr)
      {
         // Initialize and load the AttachmentZone
         ghostZoneEllipsePtr->Initialize();
         ghostZoneEllipsePtr->Load();

         // set the general zone properties
         ghostZoneEllipsePtr->SetName(aRealZoneEllipsePtr->GetName()); // IMPORTANT!
         ghostZoneEllipsePtr->SetColor(cGHOST_ZONE_LINE_COLOR);
         ghostZoneEllipsePtr->SetFillColor(cGHOST_ZONE_FILL_COLOR);
         ghostZoneEllipsePtr->SetBlendFactor(cGHOST_ZONE_FILL_COLOR[3]);
         ghostZoneEllipsePtr->SetLineWidth(aRealZoneEllipsePtr->GetLineWidth());
         ghostZoneEllipsePtr->SetLineStyle(aRealZoneEllipsePtr->GetLineStyle());
         ghostZoneEllipsePtr->SetEnableFill(aRealZoneEllipsePtr->GetEnableFill());
         ghostZoneEllipsePtr->SetHeight(aRealZoneEllipsePtr->GetHeight());
         ghostZoneEllipsePtr->SetBaseHeight(aRealZoneEllipsePtr->GetBaseHeight());
         ghostZoneEllipsePtr->SetAuxData(aRealZoneEllipsePtr->GetAuxData());
         // set the elliptical zone properties
         ghostZoneEllipsePtr->SetLatAxisRadius(aRealZoneEllipsePtr->GetLatAxisRadius());
         ghostZoneEllipsePtr->SetLonAxisRadius(aRealZoneEllipsePtr->GetLonAxisRadius());
         ghostZoneEllipsePtr->SetOrientation(aRealZoneEllipsePtr->GetHeading(),
                                             aRealZoneEllipsePtr->GetPitch(),
                                             aRealZoneEllipsePtr->GetRoll());
         ghostZoneEllipsePtr->SetBegAngle(aRealZoneEllipsePtr->GetBegAngle());
         ghostZoneEllipsePtr->SetEndAngle(aRealZoneEllipsePtr->GetEndAngle());
         ghostZoneEllipsePtr->SetX(aRealZoneEllipsePtr->GetX());
         ghostZoneEllipsePtr->SetY(aRealZoneEllipsePtr->GetY());

         // Reload the AttachmentZone
         ghostZoneEllipsePtr->Load();
      }
   }
}

void Manager::ConstructGhostAttachmentZonePolygon(wkf::Viewer*                      aViewerPtr,
                                                  const wkf::AttachmentZonePolygon* aRealZonePolygonPtr,
                                                  vespa::VaEntity*                  aGhostEntityPtr)
{
   if (aRealZonePolygonPtr != nullptr)
   {
      auto ghostZonePolygonPtr = vespa::make_attachment<wkf::AttachmentZonePolygon>(*aGhostEntityPtr, aViewerPtr);
      if (ghostZonePolygonPtr != nullptr)
      {
         // Initialize and load the AttachmentZone
         ghostZonePolygonPtr->Initialize();
         ghostZonePolygonPtr->Load();

         // set the general zone properties
         ghostZonePolygonPtr->SetName(aRealZonePolygonPtr->GetName()); // IMPORTANT!
         ghostZonePolygonPtr->SetColor(cGHOST_ZONE_LINE_COLOR);
         ghostZonePolygonPtr->SetFillColor(cGHOST_ZONE_FILL_COLOR);
         ghostZonePolygonPtr->SetBlendFactor(cGHOST_ZONE_FILL_COLOR[3]);
         ghostZonePolygonPtr->SetLineWidth(aRealZonePolygonPtr->GetLineWidth());
         ghostZonePolygonPtr->SetLineStyle(aRealZonePolygonPtr->GetLineStyle());
         ghostZonePolygonPtr->SetEnableFill(aRealZonePolygonPtr->GetEnableFill());
         ghostZonePolygonPtr->SetHeight(aRealZonePolygonPtr->GetHeight());
         ghostZonePolygonPtr->SetBaseHeight(aRealZonePolygonPtr->GetBaseHeight());
         ghostZonePolygonPtr->SetAuxData(aRealZonePolygonPtr->GetAuxData());
         // set the polygonal zone properties
         ghostZonePolygonPtr->SetClosed(aRealZonePolygonPtr->GetClosed());
         ghostZonePolygonPtr->SetHideZonePoints(aRealZonePolygonPtr->GetHideZonePoints());
         ghostZonePolygonPtr->SetRelative(aRealZonePolygonPtr->IsRelative());
         // set the modifiers
         for (size_t index = 0; index < aRealZonePolygonPtr->GetModifierCount(); ++index)
         {
            ghostZonePolygonPtr->SetModifier(aRealZonePolygonPtr->GetModifierKey(index),
                                             aRealZonePolygonPtr->GetModifierValue(
                                                aRealZonePolygonPtr->GetModifierKey(index)));
         }
         // set the ghost zone points
         const auto& zonePoints = aRealZonePolygonPtr->GetZonePoints();
         for (const auto& point : zonePoints)
         {
            vespa::VaPosition pos           = point->GetPosition();
            wkf::ZonePoint*   ghostPointPtr = new wkf::ZonePoint(*ghostZonePolygonPtr, pos);
            ghostZonePolygonPtr->AddZonePoint(ghostPointPtr, aViewerPtr);
         }

         // Reload the AttachmentZone
         ghostZonePolygonPtr->Load();
      }
   }
}

void Manager::ConstructGhostAttachmentZoneSphere(wkf::Viewer*                     aViewerPtr,
                                                 const wkf::AttachmentZoneSphere* aRealZoneSpherePtr,
                                                 vespa::VaEntity*                 aGhostEntityPtr)
{
   if (aRealZoneSpherePtr != nullptr)
   {
      auto ghostZoneSpherePtr = vespa::make_attachment<wkf::AttachmentZoneSphere>(*aGhostEntityPtr, aViewerPtr);
      if (ghostZoneSpherePtr != nullptr)
      {
         // Initialize and load the AttachmentZone
         ghostZoneSpherePtr->Initialize();
         ghostZoneSpherePtr->Load();

         // set the general zone properties
         ghostZoneSpherePtr->SetName(aRealZoneSpherePtr->GetName()); // IMPORTANT!
         ghostZoneSpherePtr->SetColor(cGHOST_ZONE_LINE_COLOR);
         ghostZoneSpherePtr->SetFillColor(cGHOST_ZONE_FILL_COLOR);
         ghostZoneSpherePtr->SetBlendFactor(cGHOST_ZONE_FILL_COLOR[3]);
         ghostZoneSpherePtr->SetLineWidth(aRealZoneSpherePtr->GetLineWidth());
         ghostZoneSpherePtr->SetLineStyle(aRealZoneSpherePtr->GetLineStyle());
         ghostZoneSpherePtr->SetEnableFill(aRealZoneSpherePtr->GetEnableFill());
         ghostZoneSpherePtr->SetMaxAltitudeSet(aRealZoneSpherePtr->IsMaxAltitudeSet());
         ghostZoneSpherePtr->SetMinAltitudeSet(aRealZoneSpherePtr->IsMinAltitudeSet());
         ghostZoneSpherePtr->SetMaxAltitude(aRealZoneSpherePtr->GetMaximumAltitude());
         ghostZoneSpherePtr->SetMinAltitude(aRealZoneSpherePtr->GetMinimumAltitude());
         ghostZoneSpherePtr->SetAuxData(aRealZoneSpherePtr->GetAuxData());
         // set the spherical zone properties
         ghostZoneSpherePtr->SetRadius(aRealZoneSpherePtr->GetRadius());
         ghostZoneSpherePtr->SetOrientation(aRealZoneSpherePtr->GetHeading(),
                                            aRealZoneSpherePtr->GetPitch(),
                                            aRealZoneSpherePtr->GetRoll());
         ghostZoneSpherePtr->SetBegAngle(aRealZoneSpherePtr->GetBegAngle());
         ghostZoneSpherePtr->SetEndAngle(aRealZoneSpherePtr->GetEndAngle());
         ghostZoneSpherePtr->SetX(aRealZoneSpherePtr->GetX());
         ghostZoneSpherePtr->SetY(aRealZoneSpherePtr->GetY());

         // Reload the AttachmentZone
         ghostZoneSpherePtr->Load();
      }
   }
}

void Manager::ConstructGhostAttachmentRangeRing(wkf::Viewer*                    aViewerPtr,
                                                const wkf::AttachmentRangeRing* aRealRangeRingPtr,
                                                vespa::VaEntity*                aGhostEntityPtr)
{
   if (aRealRangeRingPtr != nullptr)
   {
      auto ghostRangeRingPtr = vespa::make_attachment<wkf::AttachmentRangeRing>(*aGhostEntityPtr, aViewerPtr, "");
      if (ghostRangeRingPtr != nullptr)
      {
         // Initialize and load the AttachmentRangeRing
         ghostRangeRingPtr->Initialize();
         ghostRangeRingPtr->Load();

         // set the general attachment properties
         ghostRangeRingPtr->SetName(aRealRangeRingPtr->GetName());
         // set the range ring properties
         ghostRangeRingPtr->SetProperties(aRealRangeRingPtr->GetProperties());
         ghostRangeRingPtr->SetRingColor(cGHOST_RANGE_RING_LINE_COLOR);
         ghostRangeRingPtr->SetRadialColor(cGHOST_RANGE_RING_RADIAL_COLOR);
      }
   }
}

void Manager::ShowGhostEntityAndAttachments(vespa::VaEntity* aGhostEntityPtr) const
{
   if (aGhostEntityPtr != nullptr)
   {
      aGhostEntityPtr->SetVisibilityContribution(true, 0, vaEnv.GetStandardViewer());
      for (auto& elem : aGhostEntityPtr->GetAttachments())
      {
         elem->SetStateVisibility(true);
      }
   }
}

const vespa::VaAttachmentModel* Manager::GetRealModel(const vespa::VaEntity* aRealEntityPtr)
{
   auto         attachmentVector = aRealEntityPtr->FindAttachmentsOfType<vespa::VaAttachmentModel>();
   wkf::Viewer* viewerPtr        = vaEnv.GetStandardViewer();
   for (const auto& it : attachmentVector)
   {
      // GetType() is an extra safeguard for the conditional to pick out ONLY attachment models
      // attachment models can ONLY be local and thus cannot have different names
      bool isModel = it->IsA_TypeOf<vespa::VaAttachmentModel>() ||
                     QString::fromStdString(it->GetName()).contains("mapDisplayModel");
      if (isModel && viewerPtr != nullptr && it->IsMyViewer(viewerPtr))
      {
         return static_cast<const vespa::VaAttachmentModel*>(it);
      }
   }
   return nullptr;
}

const wkf::AttachmentRoute* Manager::GetRealRoute(const vespa::VaEntity* aRealEntityPtr)
{
   auto         attachmentVector = aRealEntityPtr->FindAttachmentsOfType<wkf::AttachmentRoute>();
   wkf::Viewer* viewerPtr        = vaEnv.GetStandardViewer();
   for (const auto& it : attachmentVector)
   {
      // GetType is for global routes (global routes can have different names, but not different types)
      // GetName is for local routes (local routes cannot have different names)
      bool isRoute = it->IsA_TypeOf<wkf::AttachmentRoute>() || QString::fromStdString(it->GetName()).contains("route");
      if (isRoute && viewerPtr != nullptr && it->IsMyViewer(viewerPtr))
      {
         return static_cast<const wkf::AttachmentRoute*>(it);
      }
   }
   return nullptr;
}

Manager::VaZoneList Manager::GetRealZones(const vespa::VaEntity* aRealEntityPtr)
{
   auto&        realAttachmentList = aRealEntityPtr->GetAttachments();
   wkf::Viewer* viewerPtr          = vaEnv.GetStandardViewer();
   // construct the attachment zone list
   //(vespa::VaEntity::FindAttachmentsOfType cannot do what needs to be done)
   VaZoneList zoneList;
   for (const auto& attachment : realAttachmentList)
   {
      bool isCircleZone = attachment->IsA_TypeOf<wkf::AttachmentZoneCircle>() ||
                          QString::fromStdString(attachment->GetName()).contains("circle_zone");
      bool isEllipseZone = attachment->IsA_TypeOf<wkf::AttachmentZoneEllipse>() ||
                           QString::fromStdString(attachment->GetName()).contains("ellipse_zone");
      bool isSphereZone = attachment->IsA_TypeOf<wkf::AttachmentZoneSphere>() ||
                          QString::fromStdString(attachment->GetName()).contains("sphere_zone");
      bool isPolygonZone = attachment->IsA_TypeOf<wkf::AttachmentZonePolygon>() ||
                           QString::fromStdString(attachment->GetName()).contains("polygon_zone");

      // is valid zone type and in the current viewer
      if ((isCircleZone || isEllipseZone || isSphereZone || isPolygonZone) && viewerPtr != nullptr &&
          attachment->IsMyViewer(viewerPtr))
      {
         zoneList.push_back(static_cast<wkf::AttachmentZone*>(attachment.get()));
      }
   }

   return zoneList;
}

Manager::RangeRingList Manager::GetRealRangeRings(const vespa::VaEntity* aRealEntityPtr)
{
   auto&        realAttachmentList = aRealEntityPtr->GetAttachments();
   wkf::Viewer* viewerPtr          = vaEnv.GetStandardViewer();
   // construct the attachment range ring list
   //(vespa::VaEntity::FindAttachmentsOfType cannot do what needs to be done)
   RangeRingList rangeRingList;
   for (const auto& attachment : realAttachmentList)
   {
      bool isRangeRing = attachment->IsA_TypeOf<wkf::AttachmentRangeRing>() ||
                         QString::fromStdString(attachment->GetName()).contains("rangeRing");

      // is range ring and in the current viewer
      if (isRangeRing && viewerPtr != nullptr && attachment->IsMyViewer(viewerPtr))
      {
         rangeRingList.push_back(static_cast<wkf::AttachmentRangeRing*>(attachment.get()));
      }
   }

   return rangeRingList;
}

vespa::VaEntity* Manager::FindGhostEntity(const std::string& aRealEntity)
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   return scenarioPtr->FindEntity(GhostEntityName(aRealEntity));
}

vespa::VaEntity* Manager::FindRealEntity(const std::string& aGhostEntity)
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   return scenarioPtr->FindEntity(RealEntityName(aGhostEntity));
}

} // namespace ghost
} // namespace MapUtils
