// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef ENGAGEPLUGIN_HPP
#define ENGAGEPLUGIN_HPP

#include "Dialog.hpp"
#include "Plugin.hpp"
#include "UtQtUiPointer.hpp"
#include "WkfAction.hpp"

namespace Engage
{
class Dialog;

class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::Action*> GetActions() const override;

protected:
private:
   void Start();

   wkf::Action*              mStartAction;
   ut::qt::UiPointer<Dialog> mDialogPtr;
};
} // namespace Engage
#endif
