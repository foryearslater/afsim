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
#include "SatelliteTetherPlugin.hpp"

#include <QMenu>

#include "SatelliteTetherDockWidget.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "satellite_tether/WkfSatelliteTetherPrefObject.hpp"
#include "satellite_tether/WkfSatelliteTetherPrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(SatelliteTether::Plugin, "Satellite Tether", "Provides a tether-view catered for satellites", "warlock")

namespace
{
constexpr const char* cSATELLITE_TETHER_VIEW = "Satellite-TetherView";
}

SatelliteTether::Plugin::Plugin(const QString& aPluginName, const size_t& aUniqueId)
   : wkf::Plugin(aPluginName, aUniqueId)
   , mPrefWidget(new wkf::SatelliteTetherPrefWidget)
   , mInterfacePtr(new SatelliteTether::SimInterface(aPluginName))
{
   auto tetherAction = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("satellite"),
                                                        QString("Satellite-Tether To Selection"),
                                                        wkfEnv.GetMainWindow());
   connect(tetherAction, &QAction::triggered, this, &Plugin::Tether);
   mActions.push_back(tetherAction);
}

void SatelliteTether::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   auto platform = dynamic_cast<wkf::Platform*>(aEntityPtr);
   if (platform)
   {
      if (platform->GetSpatialDomain() == wkf::SPATIAL_DOMAIN_SPACE)
      {
         QString  name = aEntityPtr->GetName().c_str();
         QAction* tetherAction =
            ut::qt::make_qt_ptr<QAction>(QIcon::fromTheme("satellite"), QString("Satellite-Tether to %1").arg(name), aMenu);
         aMenu->addAction(tetherAction);
         connect(tetherAction, &QAction::triggered, this, [=]() { ConnectToPlatform(name); });
      }
   }
}

void SatelliteTether::Plugin::GuiUpdate()
{
   if (mInterfacePtr)
   {
      mInterfacePtr->ProcessEvents(mWidgetList, mPropagationManager);
   }
   for (auto&& w : mWidgetList)
   {
      w.second->DataUpdate();
   }
}

QList<wkf::PrefWidget*> SatelliteTether::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidget;
   return prefs;
}

void SatelliteTether::Plugin::Tether()
{
   wkf::Platform* platPtr = wkfEnv.GetPlatformOfInterest();
   if (platPtr)
   {
      if (platPtr->GetSpatialDomain() == wkf::SPATIAL_DOMAIN_SPACE)
      {
         ConnectToPlatform(platPtr->GetName().c_str());
      }
   }
}

void SatelliteTether::Plugin::ConnectToPlatform(const QString& aPlatformName)
{
   // Store pointer to this dockWidget,
   // since name is not unique due to multiple views can point to same entity, use a key value
   auto dw = ut::qt::make_qt_ptr<SatelliteTether::DockWidget>(cSATELLITE_TETHER_VIEW + aPlatformName,
                                                              aPlatformName,
                                                              mWidgetNamer,
                                                              mInterfacePtr,
                                                              mPropagationManager,
                                                              wkfEnv.GetMainWindow()->centralWidget());
   dw->setWindowTitle(cSATELLITE_TETHER_VIEW + QString(" ") + aPlatformName);
   mWidgetList[mWidgetNamer] = dw;
   connect(dw, &wkf::SatelliteTetherDockWidget::CleanUp, [this](int aId) { mWidgetList.erase(aId); });
   connect(mPrefWidget->GetPreferenceObject(),
           &wkf::SatelliteTetherPrefObject::ShowNamesChanged,
           dw,
           &wkf::SatelliteTetherDockWidget::ShowNames);
   connect(mPrefWidget->GetPreferenceObject(),
           &wkf::SatelliteTetherPrefObject::ShowFutureTrackChanged,
           dw,
           &wkf::SatelliteTetherDockWidget::ShowFutureTracks);
   connect(mPrefWidget->GetPreferenceObject(),
           &wkf::SatelliteTetherPrefObject::LightingChanged,
           dw,
           &wkf::SatelliteTetherDockWidget::EnableLighting);
   dw->ShowNames(mPrefWidget->GetPreferenceObject()->GetShowNames());
   dw->ShowFutureTracks(mPrefWidget->GetPreferenceObject()->GetShowFutureTrack());
   ++mWidgetNamer;
}
