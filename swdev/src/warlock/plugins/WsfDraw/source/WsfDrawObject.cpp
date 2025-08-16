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

#include "WsfDrawObject.hpp"

#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "VaObserver.hpp"
#include "VaViewerManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfDrawSimInterface.hpp"
#include "WsfPlatformObserver.hpp"
#include "wsf_draw/WkfOverlayWsfDraw.hpp"

namespace
{
bool IsImmersive(vespa::VaViewer* aViewer)
{
   UtVariant v = aViewer->GetData("type");
   if (!v.IsUnset())
   {
      return v.GetString() == "immersive";
   }
   return false;
}
} // namespace

WkWsfDraw::WsfDrawObject::WsfDrawObject(QObject* aParentPtr)
   : wkf::WsfDrawItemModel(aParentPtr, mViewerLayerMaps)
{
   mCallbacks.Add(vespa::VaObserver::EntityDeleted.Connect(&WsfDrawObject::EntityDeletedHandler, this));
   mCallbacks.Add(vespa::VaObserver::ViewerInitialized.Connect(&WsfDrawObject::ViewerInitializedHandler, this));
   mCallbacks.Add(vespa::VaObserver::ViewerDestroyed.Connect(&WsfDrawObject::ViewerDestroyedHandler, this));
   mCallbacks.Add(wkf::Observer::StandardScenarioRemoved.Connect(&WsfDrawObject::ScenarioRemovedHandler, this));

   // Add all viewers that were initialized prior to this constructor
   for (const auto& viewerIt : vaEnv.GetViewerManager()->GetViewers())
   {
      if (viewerIt == vaEnv.GetStandardViewer() || IsImmersive(viewerIt))
      {
         mViewerLayerMaps.emplace(viewerIt, LayerMap());
      }
   }
}

void WkWsfDraw::WsfDrawObject::ShowViewerLayer(vespa::VaViewer* aViewer, const QString& aName, bool aState)
{
   auto layerMap = mViewerLayerMaps.find(aViewer);
   if (layerMap != mViewerLayerMaps.end())
   {
      auto layer = layerMap->second.find(aName);
      if (layer != layerMap->second.end())
      {
         layer->second->SetVisible(aState);
      }
   }
}

void WkWsfDraw::WsfDrawObject::EntityDeletedHandler(vespa::VaEntity* aEntity)
{
   for (const auto& viewerIt : mViewerLayerMaps)
   {
      for (const auto& layerIt : viewerIt.second)
      {
         layerIt.second->DeactivateElementsWithReference(aEntity->GetName());
      }
   }
}

void WkWsfDraw::WsfDrawObject::ViewerDestroyedHandler(vespa::VaViewer* aViewer)
{
   auto mapIt = mViewerLayerMaps.find(aViewer);
   if (mapIt != mViewerLayerMaps.end())
   {
      for (const auto& layerIt : mapIt->second)
      {
         aViewer->RemoveOverlay(layerIt.second->GetName());
      }
   }
   mViewerLayerMaps.erase(aViewer);
}

void WkWsfDraw::WsfDrawObject::ViewerInitializedHandler(vespa::VaViewer* aViewer)
{
   if (IsImmersive(aViewer))
   {
      mViewerLayerMaps.emplace(aViewer, LayerMap());
      for (const auto& eventIt : mPastEvents)
      {
         DrawEventHandler(eventIt.second.mStartTime,
                          eventIt.second.mLayerName,
                          {eventIt.second.mEvent},
                          aViewer,
                          mViewerLayerMaps[aViewer]);
      }
   }
}

void WkWsfDraw::WsfDrawObject::ScenarioRemovedHandler(wkf::Scenario* aScenario)
{
   for (auto& viewerIt : mViewerLayerMaps)
   {
      for (auto& layerIt : viewerIt.second)
      {
         viewerIt.first->RemoveOverlay(layerIt.second->GetName());
      }
      viewerIt.second.clear();
   }
}

void WkWsfDraw::WsfDrawObject::ProcessDrawEvents(double                                 aSimTime,
                                                 const QString&                         aLayer,
                                                 const std::vector<WsfDraw::DrawEvent>& aEvents)
{
   if (vaEnv.GetStandardScenario() != nullptr)
   {
      for (const auto& eventIt : aEvents)
      {
         mPastEvents.emplace(aSimTime + eventIt.mDuration, EventInfo(aSimTime, aLayer, eventIt));
      }
      // Get the smallest endTime that is greater than or equal to the currentTime, and remove everything below this time
      auto mapIt = mPastEvents.lower_bound(aSimTime);
      if (mapIt != mPastEvents.begin())
      {
         mPastEvents.erase(mPastEvents.begin(), mapIt);
      }

      for (auto& viewerIt : mViewerLayerMaps)
      {
         DrawEventHandler(aSimTime, aLayer, aEvents, viewerIt.first, viewerIt.second);
      }
   }
}

void WkWsfDraw::WsfDrawObject::DrawEventHandler(double                                 aSimTime,
                                                const QString&                         aLayer,
                                                const std::vector<WsfDraw::DrawEvent>& aEvents,
                                                vespa::VaViewer*                       aViewer,
                                                LayerMap&                              aLayerMap)
{
   bool    newLayer = false;
   QString layer(!aLayer.isEmpty() ? aLayer : "WSF_DRAW");

   if (aLayerMap.find(layer) == aLayerMap.end())
   {
      wkf::OverlayWsfDraw* overlay = new wkf::OverlayWsfDraw(layer.toStdString());
      aLayerMap[layer]             = overlay;
      if (aViewer)
      {
         newLayer = true;

         if (aViewer == vaEnv.GetStandardViewer())
         {
            connect(overlay,
                    &wkf::OverlayWsfDraw::ElementAdded,
                    [this, layer](int aIndex) { ElementAddedCB(layer, aIndex); });
            connect(overlay,
                    &wkf::OverlayWsfDraw::ElementActivated,
                    [this, layer](int aIndex, bool aState) { ElementActivatedCB(layer, aIndex, aState); });
         }
      }
   }

   wkf::OverlayWsfDraw* draw = aLayerMap[layer];

   bool validVertex = true;
   for (auto& e : aEvents)
   {
      UtColor color = e.mColor;
      float   red, green, blue, alpha;
      color.Get(red, green, blue, alpha);

      float endTime = aSimTime + e.mDuration;

      switch (e.mDrawType)
      {
      case WsfDraw::DrawType::cLINE:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1, v2;
         if (MakeVertex(e.mVerts[0], v1) && MakeVertex(e.mVerts[1], v2))
         {
            draw->AddLine(aSimTime,
                          endTime,
                          e.mID,
                          v1,
                          v2,
                          e.mLineSize,
                          e.mLineStyle,
                          static_cast<unsigned char>(red * 255),
                          static_cast<unsigned char>(green * 255),
                          static_cast<unsigned char>(blue * 255),
                          static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cPOINT:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddPoint(aSimTime,
                           endTime,
                           e.mID,
                           v1,
                           e.mPointSize,
                           static_cast<unsigned char>(red * 255),
                           static_cast<unsigned char>(green * 255),
                           static_cast<unsigned char>(blue * 255),
                           static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cICON:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddIcon(aSimTime,
                          endTime,
                          e.mID,
                          v1,
                          e.mHeading,
                          e.mIcon,
                          static_cast<unsigned char>(red * 255),
                          static_cast<unsigned char>(green * 255),
                          static_cast<unsigned char>(blue * 255),
                          static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cELLIPSE:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddEllipse(aSimTime,
                             endTime,
                             e.mID,
                             v1,
                             e.mHeading,
                             e.mAxisA,
                             e.mAxisB,
                             e.mLineSize,
                             e.mLineStyle,
                             e.mEllipseMode,
                             static_cast<unsigned char>(red * 255),
                             static_cast<unsigned char>(green * 255),
                             static_cast<unsigned char>(blue * 255),
                             static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cERASE:
      {
         draw->Erase(aSimTime, e.mID);
         break;
      }

      case WsfDraw::DrawType::cELLIPSOID:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddEllipsoid(aSimTime,
                               endTime,
                               e.mID,
                               v1,
                               e.mHeading,
                               e.mPitch,
                               e.mRoll,
                               e.mAxisA,
                               e.mAxisB,
                               e.mAxisC,
                               e.mLineSize,
                               e.mLineStyle,
                               e.mEllipseMode,
                               static_cast<unsigned char>(red * 255),
                               static_cast<unsigned char>(green * 255),
                               static_cast<unsigned char>(blue * 255),
                               static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cQUADRILATERAL:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         wkf::OverlayWsfDrawBase::VertexObject v2;
         wkf::OverlayWsfDrawBase::VertexObject v3;
         wkf::OverlayWsfDrawBase::VertexObject v4;

         if (MakeVertex(e.mVerts[0], v1) && MakeVertex(e.mVerts[1], v2) && MakeVertex(e.mVerts[2], v3) &&
             MakeVertex(e.mVerts[3], v4))
         {
            draw->AddQuadrilateral(aSimTime,
                                   endTime,
                                   e.mID,
                                   v1,
                                   v2,
                                   v3,
                                   v4,
                                   static_cast<unsigned char>(red * 255),
                                   static_cast<unsigned char>(green * 255),
                                   static_cast<unsigned char>(blue * 255),
                                   static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cTEXT:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddText(aSimTime,
                          endTime,
                          e.mID,
                          v1,
                          e.mTextSize,
                          e.mText,
                          static_cast<unsigned char>(red * 255),
                          static_cast<unsigned char>(green * 255),
                          static_cast<unsigned char>(blue * 255),
                          static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }

      case WsfDraw::DrawType::cTIMER:
      {
         wkf::OverlayWsfDrawBase::VertexObject v1;
         if (MakeVertex(e.mVerts[0], v1))
         {
            draw->AddTimer(aSimTime,
                           endTime,
                           e.mID,
                           v1,
                           e.mPointSize,
                           static_cast<unsigned char>(red * 255),
                           static_cast<unsigned char>(green * 255),
                           static_cast<unsigned char>(blue * 255),
                           static_cast<unsigned char>(alpha * 255));
         }
         else if (newLayer)
         {
            delete aLayerMap[layer];
            aLayerMap.erase(layer);
            validVertex = false;
         }
         break;
      }
      case WsfDraw::DrawType::cNONE:
         break;
      }
   }
   if (aViewer)
   {
      if (validVertex && newLayer)
      {
         aViewer->AddOverlay(aLayerMap[layer]);
         if (aViewer == vaEnv.GetStandardViewer())
         {
            emit LayerAdded(layer);
         }
         else
         {
            aLayerMap[layer]->SetVisible(false);
         }
      }
   }
   if (newLayer)
   {
      HandleNewLayer(layer);
   }
}

void WkWsfDraw::WsfDrawObject::Clear()
{
   HandleClearLayers();
   emit ResetLayers();
   for (auto& viewerIt : mViewerLayerMaps)
   {
      for (auto&& mapIt : viewerIt.second)
      {
         viewerIt.first->RemoveOverlay(mapIt.second->GetName());
      }
      viewerIt.second.clear();
   }
   // mViewerLayerMap isn't cleared, since views can remain visible on restart
   mPastEvents.clear();
}

QList<QString> WkWsfDraw::WsfDrawObject::GetLayerStrings(vespa::VaViewer* aViewer) const
{
   QList<QString> stringList;
   auto           mapIt = mViewerLayerMaps.find(aViewer);
   if (mapIt != mViewerLayerMaps.end())
   {
      for (const auto& it : mapIt->second)
      {
         stringList.push_back(it.first);
      }
   }
   return stringList;
}

bool WkWsfDraw::WsfDrawObject::GetLayerVisibility(vespa::VaViewer* aViewer, const QString& aName)
{
   auto mapIt = mViewerLayerMaps.find(aViewer);
   if (mapIt != mViewerLayerMaps.end())
   {
      auto layerIt = mapIt->second.find(aName);
      if (layerIt != mapIt->second.end())
      {
         return layerIt->second->IsVisible();
      }
   }
   return false;
}

bool WkWsfDraw::WsfDrawObject::MakeVertex(const WsfDraw::VertexObject&           aVertex,
                                          wkf::OverlayWsfDrawBase::VertexObject& aOverlayVertex)
{
   bool success             = true;
   aOverlayVertex.mPosition = aVertex.mPosition;

   switch (aVertex.mVertexType)
   {
   case WsfDraw::cUNSET_VERTEX:
   case WsfDraw::cABSOLUTE_WCS:
      break;

   // if it's a relative vertex, we need the position of the platform associated with the VertexObject
   case WsfDraw::cRELATIVE_ZERO:
   case WsfDraw::cRELATIVE_ECS:
   case WsfDraw::cRELATIVE_NED:
   {
      auto* scenarioPtr = vaEnv.GetStandardScenario();
      if (scenarioPtr)
      {
         aOverlayVertex.mReferenceEntityPtr = scenarioPtr->FindPlatformByIndex(aVertex.mPlatformIndex);
         if (aOverlayVertex.mReferenceEntityPtr == nullptr)
         {
            success = false;
         }
      }
      break;
   }
   case WsfDraw::cABSOLUTE_SCREEN:
   {
      auto* scenarioPtr = vaEnv.GetStandardScenario();
      if (scenarioPtr)
      {
         aOverlayVertex.mReferenceEntityPtr = scenarioPtr->FindPlatformByIndex(aVertex.mPlatformIndex);
         if ((aOverlayVertex.mReferenceEntityPtr == nullptr) && (aVertex.mPlatformIndex != 0))
         {
            success = false;
         }
      }
      break;
   }
   }

   switch (aVertex.mVertexType)
   {
   case WsfDraw::cUNSET_VERTEX:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cUNSET_VERTEX;
      break;
   }

   case WsfDraw::cABSOLUTE_WCS:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cABSOLUTE_WCS;
      break;
   }

   case WsfDraw::cRELATIVE_ZERO:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cRELATIVE_ZERO;
      break;
   }

   case WsfDraw::cRELATIVE_ECS:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cRELATIVE_ECS;
      break;
   }

   case WsfDraw::cRELATIVE_NED:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cRELATIVE_NED;
      break;
   }

   case WsfDraw::cABSOLUTE_SCREEN:
   {
      aOverlayVertex.mVertexType = wkf::OverlayWsfDrawBase::cABSOLUTE_SCREEN;
      break;
   }

   default:
      break;
   }
   return success;
}
