// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKCOMMVISPLUGIN_HPP
#define WKCOMMVISPLUGIN_HPP

#include "CommVisSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfPlugin.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"
#include "comm_vis/WkfCommVisDialog.hpp"

namespace WkCommVis
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   QList<wkf::Tip> GetTips() const override;

protected:
   void GuiUpdate() override;

private:
   ut::qt::UiPointer<wkf::CommVisDialog> mDialog;
   wkf::DataContainer                    mCommData;
};
} // namespace WkCommVis
#endif
