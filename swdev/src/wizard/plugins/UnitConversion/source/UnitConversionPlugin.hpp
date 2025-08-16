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
#ifndef UNITCONVERSIONPLUGIN_HPP
#define UNITCONVERSIONPLUGIN_HPP

#include "Plugin.hpp"

namespace wizard
{
class Editor;
}
class QMenu;

namespace UnitConversion
{

class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   void        OnEditorContextMenu(wizard::Editor* aEditor, QMenu& aMenu);
   static void ReplaceRange(UtTextDocumentRange aRange, const std::string& aText);
};
} // namespace UnitConversion
#endif
