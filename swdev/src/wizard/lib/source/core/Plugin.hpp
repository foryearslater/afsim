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
#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <QObject>
class UtCalendar;
#include "Environment.hpp"
#include "ViExport.hpp"
#include "WkfPlugin.hpp"

namespace wizard
{
template<class PClass>
class PluginT : public PClass
{
   static_assert(std::is_base_of<wkf::Plugin, PClass>::value,
                 "Template argument to wizard::PluginT must be a subclass of wkf::Plugin");

   using ThisClass = PluginT<PClass>;

public:
   PluginT(const QString& aPluginName, const size_t aUniqueId)
      : PClass(aPluginName, aUniqueId)
   {
      QObject::connect(&wizEnv, &Environment::MarginClick, this, &PluginT::MarginClick);
      QObject::connect(&wizEnv, &Environment::MarginDrawRequest, this, &PluginT::MarginDraw);
      QObject::connect(&wizEnv, &Environment::HighlightLinesRequest, this, &PluginT::GetHighlightLines);
      QObject::connect(&wizEnv, &Environment::FocusedCopyRequest, this, &PluginT::FocusedCopy);
      QObject::connect(&wizEnv, &Environment::PlatformUpdated, this, &PluginT::PlatformUpdated);
      QObject::connect(&wizEnv, &Environment::PlatformDeleted, this, &PluginT::PlatformDeleted);
   }
   ~PluginT() override = default;

   virtual void MarginClick(Qt::MouseButton aButton, const QString& aSourcePath, size_t aLineNumber) {}
   virtual void MarginDraw(const QString& aSourcePath, QPainter& aPainter, const QMap<size_t, QPoint>& aVisibleLineMap)
   {
   }
   virtual void GetHighlightLines(const QString& aSourcePath, QList<size_t>& aList) {}
   virtual void FocusedCopy() {}
   virtual void PlatformUpdated(const QString& aPlatformName) {}
   virtual void PlatformDeleted(const QString& aPlatformName) {}

private:
};
using Plugin = PluginT<wkf::Plugin>;
} // namespace wizard

#endif // PLUGIN_HPP
