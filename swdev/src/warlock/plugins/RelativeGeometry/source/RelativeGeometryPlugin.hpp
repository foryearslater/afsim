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
#ifndef RELATIVEPLUGIN_HPP
#define RELATIVEPLUGIN_HPP

#include "WkfPlugin.hpp"

namespace wkf
{
class Platform;
}

namespace RelativeGeometry
{
class Plugin : public wkf::Plugin
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   void GuiUpdate() override;

   std::unique_ptr<wkf::Updater> GetUpdater(const QString&     aPlatformName,
                                            const QString&     aDatum,
                                            const unsigned int aContext = 0) const override;
signals:
   void RedrawNeeded();

private:
   void Display(const QString& aFrom, const QString& aTo, const wkf::SpatialDomain& aDomain);
};
} // namespace RelativeGeometry
#endif
