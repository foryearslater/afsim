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

#include "AnnotationPlugin.hpp"

#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkAnnotation::Plugin,
                          "Annotation",
                          "The Annotation plugin is responsible for showing visual annotations on the map display.",
                          "warlock")

WkAnnotation::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface, Annotation::Plugin>(aPluginName, aUniqueId)
{
   connect(&wkfEnv, &wkf::Environment::DeleteShortcutTriggered, this, &Annotation::Plugin::DeleteCurrentlySelectedAnnotations);
}

void WkAnnotation::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDisplayInterface);
}
