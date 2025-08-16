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

#ifndef WKPLUGIN_SA_DISPLAY_HPP
#define WKPLUGIN_SA_DISPLAY_HPP

#include "SA_DisplaySimInterface.hpp"
#include "WkPlugin.hpp"
#include "sa_display/WkfSA_DisplayPluginBase.hpp"

namespace WkSA_Display
{
class Plugin : public warlock::PluginT<SimInterface, wkf::SA_Display::PluginBase>
{
public:
   Plugin(const QString& aPluginName, size_t aUniqueID);

protected:
   void GuiUpdate() override;
   void NewDisplay(wkf::Platform* aPlatform) override;
   void RemoveDisplay(size_t aIndex) override;

   bool HasSituationAwarenessProcessor(wkf::Platform* aPlatorm) override;
};
} // namespace WkSA_Display
#endif // WKPLUGIN_SA_DISPLAY_HPP
