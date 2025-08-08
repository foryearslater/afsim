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

#ifndef RANGERING_PLUGIN_HPP
#define RANGERING_PLUGIN_HPP

#include "WkfPlugin.hpp"

namespace Training
{
   // EXERCISE 1 TASK 1a
   // Derive a new class Plugin from wkf::Plugin
   class Plugin : public wkf::Plugin
   {
   public:
      Plugin(const QString& aPluginName,
             const size_t   aUniqueId);

      ~Plugin() override = default;

   private:
      //! Adds range rings to all newly selected platforms.
      //! Removes range rings from all unselected platforms.
      //! @param aSelected This is a list of newly selected platforms.
      //! @param aUnselected This is a list of newly unselected platforms.
      void SelectionChanged(const wkf::PlatformList& aSelected,
                            const wkf::PlatformList& aUnselected);
   };
}

#endif
