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

#ifndef TablePlotterPLUGIN_HPP
#define TablePlotterPLUGIN_HPP

#include "Plugin.hpp"

namespace TablePlotter
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginname, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   void Start();
   void ActionsForCommandContentsRequested(const std::string& aContent,
                                           const std::string& aPath,
                                           QMenu*             aMenuPtr,
                                           QWidget*           aParentWidgetPtr);
   void ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu);
   void PopulateUtCurveList();

   std::vector<std::string> mValidCurveStrings;
};
} // namespace TablePlotter

#endif