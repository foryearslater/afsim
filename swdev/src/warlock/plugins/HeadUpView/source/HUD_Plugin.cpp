// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "HUD_Plugin.hpp"

#include <QApplication>
#include <QDesktopWidget>

#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfKeyBindingsObject.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPrefWidget.hpp"
#include "hud/WkfHUD_DataContainer.hpp"
#include "hud/WkfHUD_PrefObject.hpp"
#include "hud/WkfHUD_PrefWidget.hpp"

namespace
{
constexpr const char* cHUD_VIEW = "HudView";
}

WKF_PLUGIN_DEFINE_SYMBOLS(
   HeadsUpDisplay::Plugin,
   "Head Up View",
   "The Head Up View plugin provides an out-the-window (OTW) display with a generic Head Up Display (HUD).",
   "warlock",
   false) // Plugin not loaded by default

HeadsUpDisplay::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId, eSECONDARY_VIEWER)
   , mPrefWidgetPtr(ut::qt::make_qt_ptr<wkf::HUD_PrefWidget>())
{
   mInterfacePtr->SetEnabled(false);

   mPrefObjectPtr = mPrefWidgetPtr.data()->GetPreferenceObject();

   // Action to cycle the HUD mode if h key is pressed with the joystick plugin active
   wkf::Action* hudModeCycleAction = new wkf::Action("hud_mode_cycle", wkfEnv.GetMainWindow());
   connect(hudModeCycleAction, &wkf::Action::triggered, this, &Plugin::CycleHUDMode);
   mActions.push_back(hudModeCycleAction);
}

void HeadsUpDisplay::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   // Check if our entity is a platform
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      wkf::Platform* platPtr = dynamic_cast<wkf::Platform*>(aEntityPtr);
      if ((mInterfacePtr->HasSituationAwarenessProcessor(platPtr->GetIndex())) &&
          mHuds.find(QString::fromStdString(platPtr->GetName())) == mHuds.end())
      {
         auto airCombatMenu = aMenu->findChild<QMenu*>("Air Combat");
         if (!airCombatMenu)
         {
            airCombatMenu = ut::qt::make_qt_ptr<QMenu>(QString("Air Combat"), aMenu);
            aMenu->addMenu(airCombatMenu);
            airCombatMenu->setObjectName("Air Combat");
         }

         QString      name          = aEntityPtr->GetName().c_str();
         wkf::Action* connectAction = new wkf::Action(QIcon::fromTheme("headsup"), QString("Head Up View"), airCombatMenu);
         connectAction->setData(name);
         connect(connectAction, &QAction::triggered, this, &Plugin::ConnectToPlatformActionHandler);
      }
   }
}

void HeadsUpDisplay::Plugin::ConnectToPlatform(const QString& aName, QWidget* aParent)
{
   // The Sim interface is disabled until needed
   mInterfacePtr->SetEnabled(true);
   // Set the connected platform names for both sim interface and data container
   mInterfacePtr->AddConnectedPlatform(aName.toStdString());

   // If another HUD window has been opened for platform with aName, delete the old one
   if (mHuds.end() != mHuds.find(aName))
   {
      delete mHuds[aName];
      mHuds.erase(aName);
   }

   // Create the HUD widget
   QPointer<wkf::HUD_DockWidget> aDockWidget = new wkf::HUD_DockWidget(aName, wkfEnv.GetMainWindow()->centralWidget());

   // Docking and immediately floating the window will guarantee the dock widget has a title bar.
   aDockWidget->setFloating(false);
   aDockWidget->setFloating(true);

   // Set the dock widget's geometry to spawn at the same location as the main window.
   aDockWidget->setGeometry(QRect{wkfEnv.GetMainWindow()->geometry().x(), wkfEnv.GetMainWindow()->geometry().y(), 0, 0});

   // Set the width and height of the dock widget to that which is specified in the preferences object.
   unsigned int width  = mPrefObjectPtr->GetResolutionX() / QApplication::desktop()->devicePixelRatio();
   unsigned int height = mPrefObjectPtr->GetResolutionY() / QApplication::desktop()->devicePixelRatio();
   aDockWidget->SetDefaultWidth(width);
   aDockWidget->SetDefaultHeight(height);

   connect(aDockWidget.data(),
           &wkf::ImmersiveViewDockWidget::ImmersiveViewClosed,
           [this, aDockWidget]() { ReleasePlatform(aDockWidget); });
   aDockWidget->setWindowTitle(aName);

   // If this check is not here, HUD will stop loading after being opened ~4 times.
   if (aDockWidget->IsInitialized())
   {
      ViewerInitializedHandler(aDockWidget);
   }
   else
   {
      connect(aDockWidget.data(),
              &wkf::HUD_DockWidget::ImmersiveViewInitialized,
              [this, aDockWidget]() { ViewerInitializedHandler(aDockWidget); });
   }

   // Add our dock widget(HUD) to the list of HUDs
   mHuds.emplace(aName, aDockWidget);

   // Connect our HUDs to update their containers.
   for (auto& dockWid : mHuds)
   {
      QPointer<wkf::HUD_DockWidget> wid = dockWid.second.data();
      connect(&mDataContainer, &wkf::HUD_DataContainer::Update_DataContainer, [this, wid]() { UpdateDataForHud(wid); });
   }
}

QList<wkf::PrefWidget*> HeadsUpDisplay::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void HeadsUpDisplay::Plugin::GuiUpdate()
{
   // Process all the new SimEvents
   mInterfacePtr->ProcessEvents(mDataContainer);

   // If the simulation was reset, reset our HUDs container.
   if (mDataContainer.GetSimulationInitialized())
   {
      Reset();
      mDataContainer.SetSimulationInitialized(false);
   }
   // For each hud, update it's data.  This may be redundant with
   // the connection in ConnectToPlatform.
   for (auto hud : mHuds)
   {
      if (GetInterface()->IsConnectionDestroyed(hud.second.data()->GetConnectedPlatformName().toStdString()))
      {
         hud.second.data()->GetHUD()->SetConnectionDestroyed(true);
      }
      else
      {
         UpdateDataForHud(hud.second.data());
      }
   }
}

void HeadsUpDisplay::Plugin::ConnectToPlatformActionHandler()
{
   QAction* sender       = dynamic_cast<QAction*>(QObject::sender());
   QString  platformName = sender->data().toString();

   /** Check if our sender is valid. */
   if (sender)
   {
      /** If the platform has a hud, release the old one. */
      if (mHuds.find(platformName) != mHuds.end())
      {
         if (mHuds.at(platformName))
         {
            ReleasePlatform(mHuds.at(platformName));
         }
      }

      // Connect the platform to the new window.
      ConnectToPlatform(platformName, wkfEnv.GetMainWindow());
   }
}

void HeadsUpDisplay::Plugin::CycleHUDMode()
{
   // Get the connected platform from the action data.
   QAction* sender       = dynamic_cast<QAction*>(QObject::sender());
   QString  platformName = sender->data().toString();

   // If we have a hud to change
   if (mHuds.find(platformName) != mHuds.end())
   {
      wkf::HUD*                        hud  = mHuds[platformName]->GetHUD();
      wkf::HUD_DataContainer::eHudMode mode = hud->GetHudMode();
      switch (mode)
      {
      case wkf::HUD_DataContainer::eHUD_NAV_MODE:
         mode = wkf::HUD_DataContainer::eHUD_WPN_MODE;
         break;
      case wkf::HUD_DataContainer::eHUD_WPN_MODE:
         mode = wkf::HUD_DataContainer::eHUD_NAV_MODE;
         break;
      default:
         mode = wkf::HUD_DataContainer::eHUD_NAV_MODE;
         break;
      }
      hud->SetHudMode(mode);

      wkf::Action* hudAction =
         wkfEnv.GetPreferenceObject<wkf::KeyBindingsObject>()->GetActionByUniqueName("Joystick.HUDModeVoidScript");
      if (hudAction != nullptr) // If the other plugin isn’t loaded, nothing happens
      {
         // Set the connected platform name before triggering the action.
         hudAction->setData(QVariant(mode));
         hudAction->trigger();
      }
   }
}

void HeadsUpDisplay::Plugin::FirstPerson(bool aState)
{
   QAction* sender = dynamic_cast<QAction*>(QObject::sender());

   // If the sender is valid, update the checkbox and set first person for the sender.
   if (sender)
   {
      sender->blockSignals(true);
      sender->setChecked(aState);
      sender->blockSignals(false);
      mHuds[sender->objectName()].data()->SetFirstPerson(aState);
   }
}

void HeadsUpDisplay::Plugin::ReleasePlatform(QPointer<wkf::HUD_DockWidget> aDockWidget)
{
   // Find the HUD in the HUD list, and remove it.
   mHuds.at(aDockWidget.data()->GetConnectedPlatformName())->hide();
   mHuds.erase(aDockWidget.data()->GetConnectedPlatformName());
   // Reset the connection in the sim interface.
   GetInterface()->ReleaseConnectedPlatform(aDockWidget.data()->GetConnectedPlatformName().toStdString());
}

void HeadsUpDisplay::Plugin::Reset()
{
   for (auto hud : mHuds)
   {
      delete hud.second;
   }
   mHuds.clear();
}

void HeadsUpDisplay::Plugin::ShowHUD(bool aState)
{
   QAction* sender = dynamic_cast<QAction*>(QObject::sender());

   // If the sender is valid, update the checkbox and show/hide the HUD for the sender.
   if (sender)
   {
      sender->blockSignals(true);
      sender->setChecked(aState);
      sender->blockSignals(false);
      mHuds[sender->objectName()].data()->ShowHUD(aState);
   }
}

void HeadsUpDisplay::Plugin::UpdateDataForHud(QPointer<wkf::HUD_DockWidget> aDockWidget)
{
   // If our HUD window is valid
   if (aDockWidget)
   {
      // If our HUD is visible, update the data. Otherwise, return.
      if (!aDockWidget.data()->HUD_Visible())
      {
         return;
      }
      else
      {
         // Get the HUD
         wkf::HUD* hud = aDockWidget.data()->GetHUD();

         // Get our platform data map
         std::map<std::string, wkf::HUD_DataContainer::PlatformData> aDC = mDataContainer.GetPlatformData();

         // If our HUD isn't attached to any platform, we can't get data.  Return.
         if (aDC.find(aDockWidget->GetConnectedPlatformName().toStdString()) == aDC.end())
         {
            return;
         }

         // Get a reference to the platform data for this HUD.
         const wkf::HUD_DataContainer::PlatformData& platformData =
            aDC.at(aDockWidget->GetConnectedPlatformName().toStdString());

         // Set the HUD data
         hud->mPlatData = platformData;
      }
   }
}

void HeadsUpDisplay::Plugin::ViewerInitializedHandler(QPointer<wkf::HUD_DockWidget> aDockWidget)
{
   // Set the appropriate first person and hud values to match the preferences,
   // then raise the window to the front.
   aDockWidget.data()->SetFirstPerson(true);
   aDockWidget.data()->ShowHUD(true);
   aDockWidget->raise();
}
