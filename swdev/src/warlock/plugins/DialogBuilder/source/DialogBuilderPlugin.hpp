// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DialogBuilderPlugin_HPP
#define DialogBuilderPlugin_HPP

#include "DialogBuilderDockWidget.hpp"
#include "DialogBuilderPrefWidget.hpp"
#include "DialogBuilderScriptInputsDockWidget.hpp"
#include "WkPlugin.hpp"

namespace WkDialogBuilder
{
class Plugin : public warlock::Plugin
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;
   void                    BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

private:
   void DialogChangedCB();

   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<PrefWidget>                  mPrefWidgetPtr;
   std::shared_ptr<warlock::ScriptSimInterface> mSimInterfacePtr;
   std::map<QString, DockWidget*>               mDockWidgets;
   ScriptInputsDockWidget*                      mScriptInputsDockWidget;
};
} // namespace WkDialogBuilder
#endif
