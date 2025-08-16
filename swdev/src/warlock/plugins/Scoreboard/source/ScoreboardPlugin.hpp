// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SCOREBOARDPLUGIN_HPP
#define SCOREBOARDPLUGIN_HPP

#include "ScoreboardDialog.hpp"
#include "ScoreboardSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfPlugin.hpp"

namespace WkScoreboard
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
   QPointer<Dialog> mDialog;
   DataContainer    mScoreboardData;
};
} // namespace WkScoreboard
#endif
