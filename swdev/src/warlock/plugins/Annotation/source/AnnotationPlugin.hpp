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

#ifndef WkAnnotationPlugin_HPP
#define WkAnnotationPlugin_HPP

#include "AnnotationSimInterface.hpp"
#include "WkPlugin.hpp"
#include "annotation/WkfAnnotationPlugin.hpp"

namespace WkAnnotation
{
class Plugin : public warlock::PluginT<SimInterface, Annotation::Plugin>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
   void GuiUpdate() override;
};
} // namespace WkAnnotation
#endif
