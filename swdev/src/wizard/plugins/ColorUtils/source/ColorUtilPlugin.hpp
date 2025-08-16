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
#ifndef COLORUTILPLUGIN_HPP
#define COLORUTILPLUGIN_HPP

#include <QColor>
#include <QWidget>

#include "ColorTips.hpp"
#include "Plugin.hpp"

// ColorUtils namespace.
namespace ColorUtils
{
//! ColorUtils plugin.
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   ~Plugin() override = default;

protected:
private:
   void EditorToolTipEvent(wizard::WsfEditor* aEditorPtr, QHelpEvent* aEvent);

   void EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);
   void UpdateColorTips();
   void ColorConstruct(size_t aStartingPos, const QColor& aInitialColor);
   void ReplaceRange(const UtTextDocumentRange& aRange, const std::string& aText);

   wizard::ColorTips* mColorTipsPtr;
   wizard::Editor*    mCurrentEditorPtr;
};
} // namespace ColorUtils

#endif
