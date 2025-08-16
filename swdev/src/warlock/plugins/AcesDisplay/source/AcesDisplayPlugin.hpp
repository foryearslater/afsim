// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKPLUGIN_ACESDISPLAY_HPP
#define WKPLUGIN_ACESDISPLAY_HPP

#include "AcesDisplaySimInterface.hpp"
#include "WkPlugin.hpp"
#include "aces/WkfAcesDisplayPluginBase.hpp"

namespace WkAcesDisplay
{
class Plugin : public warlock::PluginT<SimInterface, wkf::AcesDisplay::PluginBase>
{
public:
   Plugin(const QString& aPluginName, size_t aUniqueID);

protected:
   void GuiUpdate() override;
   void NewDisplay(wkf::Platform* aPlatform) override;
   void RemoveDisplay(size_t aIndex) override;
};
} // namespace WkAcesDisplay
#endif // WKPLUGIN_ACESDISPLAY_HPP
