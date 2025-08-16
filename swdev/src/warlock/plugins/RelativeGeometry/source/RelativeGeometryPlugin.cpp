// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "RelativeGeometryPlugin.hpp"

#include <QAction>
#include <QMenu>

#include "RelativeGeometryDialog.hpp"
#include "RelativeGeometryUpdater.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(RelativeGeometry::Plugin,
                          "Relative Geometry",
                          "Displays relative geometry data for platforms.",
                          "warlock")

RelativeGeometry::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wkf::Plugin(aPluginName, aUniqueId)
{
}

void RelativeGeometry::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      wkf::Platform*     platPtr     = dynamic_cast<wkf::Platform*>(aEntityPtr);
      wkf::SpatialDomain firstDomain = wkf::SPATIAL_DOMAIN_UNKNOWN;
      if (platPtr)
      {
         firstDomain = platPtr->GetSpatialDomain();
      }
      QString name = aEntityPtr->GetName().c_str();

      bool           added = false;
      wkf::Platform* poi   = wkfEnv.GetPlatformOfInterest();
      if (poi)
      {
         QString poiName = poi->GetName().c_str();
         if (poiName != name)
         {
            QMenu* relativeGeo = new QMenu("Relative Geometry", aMenu);
            aMenu->addMenu(relativeGeo);
            QAction* fromAction = new QAction(QString("From %1 to %2").arg(poiName, name), relativeGeo);
            relativeGeo->addAction(fromAction);
            QAction* toAction = new QAction(QString("From %1 to %2").arg(name, poiName), relativeGeo);
            relativeGeo->addAction(toAction);
            wkf::SpatialDomain secondDomain = poi->GetSpatialDomain();
            connect(fromAction, &QAction::triggered, this, [=]() { Display(poiName, name, secondDomain); });
            connect(toAction, &QAction::triggered, this, [=]() { Display(name, poiName, firstDomain); });
            added = true;
         }
      }
      if (!added)
      {
         QMenu* relativeGeo = new QMenu("Relative Geometry", aMenu);
         aMenu->addMenu(relativeGeo);
         QAction* fromAction = new QAction(QString("From %1").arg(name), relativeGeo);
         relativeGeo->addAction(fromAction);
         QAction* toAction = new QAction(QString("To %1").arg(name), relativeGeo);
         relativeGeo->addAction(toAction);
         connect(fromAction, &QAction::triggered, this, [=]() { Display(name, "", firstDomain); });
         connect(toAction, &QAction::triggered, this, [=]() { Display("", name, firstDomain); });
      }
   }
}

void RelativeGeometry::Plugin::GuiUpdate()
{
   emit RedrawNeeded();
}

std::unique_ptr<wkf::Updater> RelativeGeometry::Plugin::GetUpdater(const QString&     aPlatformName,
                                                                   const QString&     aDatum,
                                                                   const unsigned int aContext) const
{
   QStringList sl = aPlatformName.split(' ');
   if (sl.size() == 3)
   {
      if (aDatum == "Range (linear)")
      {
         return wkf::make_updater<LinearRangeUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Radial")
      {
         return wkf::make_updater<RadialUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "In-Track")
      {
         return wkf::make_updater<InTrackUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Cross-Track")
      {
         return wkf::make_updater<CrossTrackUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Bearing")
      {
         return wkf::make_updater<BearingUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Elevation")
      {
         return wkf::make_updater<ElevationUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Down Range")
      {
         return wkf::make_updater<DownRangeUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Cross Range")
      {
         return wkf::make_updater<CrossRangeUpdater>(sl[0], sl[2]);
      }
      else if (aDatum == "Range Rate")
      {
         return wkf::make_updater<RangeRateUpdater>(sl[0], sl[2]);
      }
   }
   return nullptr;
}

void RelativeGeometry::Plugin::Display(const QString& aFrom, const QString& aTo, const wkf::SpatialDomain& aDomain)
{
   Dialog* dlg = new Dialog(wkfEnv.GetMainWindow(), aFrom, aTo, aDomain, UniqueId());
   wkfEnv.GetMainWindow()->addDockWidget(Qt::RightDockWidgetArea, dlg);
   dlg->show();
   connect(this, &Plugin::RedrawNeeded, dlg, &Dialog::GuiUpdate);
}
