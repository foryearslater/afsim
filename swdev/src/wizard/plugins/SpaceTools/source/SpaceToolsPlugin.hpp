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

#ifndef SPACETOOLSPLUGIN_HPP
#define SPACETOOLSPLUGIN_HPP

#include "Plugin.hpp"

//! A collection of tools designed to help create and edit space scenarios.
namespace SpaceTools
{

class Astrolabe;
class ConstellationMaker;
class SatelliteInserter;

//! The space tools plugin.
class Plugin : public wizard::Plugin
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::Action*>     GetActions() const override;
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   void BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr) override;

   void OnProjectOpened(wizard::Project* aProjectPtr);
   void OnProjectClosed(wizard::Project* aProjectPtr);

private:
   std::unique_ptr<SpaceTools::ConstellationMaker> mConstellationMakerPtr;
   std::unique_ptr<SpaceTools::Astrolabe>          mAstrolabePtr;
   std::unique_ptr<SpaceTools::SatelliteInserter>  mSatelliteInserterPtr;
   wkf::Action*                                    mConstellationMakerActionPtr;
   wkf::Action*                                    mSatelliteActionPtr;
};

} // namespace SpaceTools

#endif // SPACETOOLSPLUGIN_HPP
