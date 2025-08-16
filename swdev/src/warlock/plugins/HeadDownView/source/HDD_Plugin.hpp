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

#ifndef HDD_PLUGIN_HPP
#define HDD_PLUGIN_HPP

#include <set>

#include "HDD_SimInterface.hpp"
#include "WkPlugin.hpp"
#include "hdd/WkfHDD_DataContainer.hpp"
#include "hdd/WkfHDD_Dockable.hpp"
#include "hdd/WkfHDD_PrefObject.hpp"
#include "hdd/WkfHDD_PrefWidget.hpp"
namespace HDD
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   /** Build the context menu entry for opening a HDD.
    * @param aMenu menu to add to.
    * @param aEntityPtr entity the menu option is being added for.
    */
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   /** Connect to a platform, adding and opening a HDD.
    * @param aPlatformName name of the platform to connect to.
    * @param aParent parenting window for the new HDD window.
    */
   void ConnectToPlatform(const QString& aPlatformName, QMainWindow* aParent);

   /** Return the list of preference widgets. */
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   /** Update the GUI. */
   void GuiUpdate() override;

   /** Release a platform, closing it's HDD window.
    * @param aPlatformName name of the platform to release.
    */
   void ReleasePlatform(const QString& aPlatformName);

   /** Request a resource instance for a platform.
    * @param aResourceName name of the resource to request.
    * @param aPlatformName name of the platform to request for.
    */
   bool RequestPlatformResourceInstance(const QString& aResourceName, const QString& aPlatformName) override;

   /** Update the HDDs using a data container.
    * @param aDataContainer container to populate data with.
    */
   void Update(const DataContainer& aDataContainer);

private:
   /** Handle a new platform connection. */
   void ConnectToPlatformActionHandler();

   DataContainer                         mDataContainer; ///< Container for all HDD data.
   std::map<QString, QPointer<Dockable>> mHdds;          ///< Mapping of platforms to HDD windows.
   const PrefObject*                     mPrefObjectPtr; ///< Pointer to the preference object for this plugin.
   PluginUiPointer<PrefWidget>           mPrefWidgetPtr; ///< Pointer to the preference widget for this plugin.
};
} // namespace HDD
#endif
