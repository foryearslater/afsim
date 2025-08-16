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

#ifndef WSFDRAWOBJECT_HPP
#define WSFDRAWOBJECT_HPP

#include <map>
#include <set>
#include <unordered_map>

#include <QObject>

#include "WkfObserver.hpp"
#include "WsfDraw.hpp"
#include "wsf_draw/WkfOverlayWsfDraw.hpp"
#include "wsf_draw/WkfWsfDrawItemModel.hpp"

namespace WkWsfDraw
{
class WsfDrawObject : public wkf::WsfDrawItemModel
{
   Q_OBJECT

public:
   WsfDrawObject(QObject* aParentPtr);

   void ShowViewerLayer(vespa::VaViewer* aViewer, const QString& aName, bool aState);

   void ProcessDrawEvents(double aSimTime, const QString& aLayer, const std::vector<WsfDraw::DrawEvent>& aEvents);
   void DrawEventHandler(double                                 aSimTime,
                         const QString&                         aLayer,
                         const std::vector<WsfDraw::DrawEvent>& aEvents,
                         vespa::VaViewer*                       aViewer,
                         LayerMap&                              aLayerMap);

   void           Clear();
   QList<QString> GetLayerStrings(vespa::VaViewer* aViewer) const;
   bool           GetLayerVisibility(vespa::VaViewer* aViewer, const QString& aName);

signals:
   void LayerAdded(const QString& aName);
   void ResetLayers();

private:
   bool MakeVertex(const WsfDraw::VertexObject& aVertex, wkf::OverlayWsfDrawBase::VertexObject& aVert);

   void EntityDeletedHandler(vespa::VaEntity* aEntity);
   void ViewerDestroyedHandler(vespa::VaViewer* aViewer);
   void ViewerInitializedHandler(vespa::VaViewer* aViewer);
   void ScenarioRemovedHandler(wkf::Scenario* aScenario);

   // Handles draws outside of the standard viewer
   std::unordered_map<vespa::VaViewer*, LayerMap> mViewerLayerMaps;

   // Stores events whose duration has not finished (for future draws on viewers that aren't created on startup)
   struct EventInfo
   {
      EventInfo(double aStartTime, const QString& aLayerName, const WsfDraw::DrawEvent& aEvent)
         : mStartTime(aStartTime)
         , mLayerName(aLayerName)
         , mEvent(aEvent)
      {
      }
      double             mStartTime;
      QString            mLayerName;
      WsfDraw::DrawEvent mEvent;
   };
   std::multimap<double, EventInfo> mPastEvents;

   vespa::VaCallbackHolder mCallbacks;
};
} // namespace WkWsfDraw
#endif // !WSFDRAWOBJECT_HPP
