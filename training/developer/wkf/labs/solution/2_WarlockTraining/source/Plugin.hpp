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

#ifndef WARLOCK_TRAINING_PLUGIN_HPP
#define WARLOCK_TRAINING_PLUGIN_HPP

#include "WkPlugin.hpp"

#include <QPointer>

#include "DockWidget.hpp"
#include "PrefWidget.hpp"
#include "SimInterface.hpp"

namespace WarlockTraining
{
   //! This represents the specific plugin we are creating.
   //! Inheriting from warlock::PluginT<T> tells the class what type of
   //! interface it has with the simulation.
   class Plugin : public warlock::PluginT<WarlockTraining::SimInterface>
   {
   public:
      Plugin(const QString& aPluginName,
             const size_t aUniqueId);

      ~Plugin() override = default;

      //! Called periodically to update the GUI.
      void GuiUpdate() override;

      //! Returns a list of the preferences widgets that this plugin provides.
      //! @note Without this function, the PrefWidget for this plugin will not appear in the Preferences menu.
      QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

      //! Returns a list of the actions that this plugin provides.
      //! @note Without this function, the actions for this plugin will do nothing.
      QList<wkf::Action*> GetActions() const override;

   private:
      //! The container of Data of this plugin
      DataContainer mDataContainer;
      //! The preferences widget to display.
      PluginUiPointer<PrefWidget> mPrefWidget;
      //! The DockWidget that displays information
      PluginUiPointer<DockWidget> mDockWidget;
      //! The actions that this plugin uses.
      QList<wkf::Action*> mActions;
   };
}

#endif
