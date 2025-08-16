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
#ifndef HUD_PLUGIN_HPP
#define HUD_PLUGIN_HPP

#include "HUD_SimInterface.hpp"
#include "WkPlugin.hpp"
#include "hud/WkfHUD_DataContainer.hpp"
#include "hud/WkfHUD_DockWidget.hpp"
#include "hud/WkfHUD_Overlay.hpp"
#include "hud/WkfHUD_PrefObject.hpp"
#include "hud/WkfHUD_PrefWidget.hpp"

namespace HeadsUpDisplay
{
/** The HeadsUpDisplay::Plugin is a Warlock plug-in that provides the
 * ability to view a head-up display (HUD) overlay and third/first-person
 * view of a platform.
 */
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);

   /** Adds an action to the context menu, allowing a HUD window to be opened.
    * @param aMenu menu to add to.
    * @param aEntityPtr entity to add.
    */
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   /** Connect a platform to a new HUD window.
    * @param aName platform name.
    * @param aParent parenting widget for the new HUD window.
    */
   void ConnectToPlatform(const QString& aName, QWidget* aParent);

   /** This returns the Wkf actions that are supported. */
   QList<wkf::Action*> GetActions() const override { return mActions; }

   /** Return a list of preference widgets. */
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   /** Update the GUI (all HUDs). */
   void GuiUpdate() override;

private:
   /** Handler for connecting to a platform (opening a HUD window). */
   virtual void ConnectToPlatformActionHandler();

   /** Cycle the HUD mode. */
   void CycleHUDMode();

   /** Set the first person state.
    * @param aState true for first person, false for third person.
    */
   void FirstPerson(bool aState);

   /** Return a reference to the data container. */
   wkf::HUD_DataContainer& GetDataContainer() { return mDataContainer; }

   /** Release(delete) a HUD window.
    * @param aDockWidget window to release.
    */
   void ReleasePlatform(QPointer<wkf::HUD_DockWidget> aDockWidget);

   /** Reset(clear) the list of HUDs. */
   void Reset();

   /** Set the show hud state.
    * @param aState true to set the hud visible, false to hide it.
    */
   void ShowHUD(bool aState);

   /** Update the data for a HUD.
    * @param aDockWidget HUD to update
    */
   void UpdateDataForHud(QPointer<wkf::HUD_DockWidget> aDockWidget);

   /** Handle post-initialization for a HUD window.
    * @param aDockWidget HUD window to post-initialize.
    */
   void ViewerInitializedHandler(QPointer<wkf::HUD_DockWidget> aDockWidget);

   QList<wkf::Action*>                              mActions;       ///< Container for actions.
   wkf::HUD_DataContainer                           mDataContainer; ///< Container for all HUD data.
   std::map<QString, QPointer<wkf::HUD_DockWidget>> mHuds;          ///< Mapping of platforms to HUD windows.
   const wkf::HUD_PrefObject*                       mPrefObjectPtr; ///< Pointer to the preference object.
   PluginUiPointer<wkf::HUD_PrefWidget>             mPrefWidgetPtr; ///< Pointer to the preference widget.
};
} // namespace HeadsUpDisplay
#endif // HUD_PLUGIN_HPP
