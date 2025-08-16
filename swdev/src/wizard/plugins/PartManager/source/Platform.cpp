// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Platform.hpp"

#include "AddPlatformPart.hpp"
#include "PlatformPart.hpp"
#include "WsfPM_MilPlatform.hpp"
#include "WsfPM_Root.hpp"

PartManager::Platform::Platform(const WsfPProxyNode& proxyNode, QWidget* parent /*= nullptr*/)
   : ParentWidget(proxyNode, parent)
{
   WsfPM_Root root(GetNode().GetProxy());

   // AddLineEditAttr("Side",                  GetNode() + "side");
   // AddLineEditAttr("Icon",                  GetNode() + "icon");
   // AddEnumComboBoxAttr("Spatial domain",    GetNode() + "spatialDomain");
   // AddPositionAttr("Position",              GetNode());
   // AddAngleAttr("Heading",                  GetNode() + "heading");
   // AddAngleAttr("Pitch",                    GetNode() + "pitch");
   // AddAngleAttr("Roll",                     GetNode() + "roll");
   AddCheckBoxAttr("Destructible", GetNode() + "destructible");
   AddMapComboBoxAttr("Infrared signature", GetNode() + "infraredSignature", root.infraredSignatures());
   AddMapComboBoxAttr("Acoustic signature", GetNode() + "acousticSignature", root.acousticSignatures());
   AddMapComboBoxAttr("Optical signature", GetNode() + "opticalSignature", root.opticalSignatures());
   AddMapComboBoxAttr("Radar signature", GetNode() + "radarSignature", root.radarSignatures());
   AddMapComboBoxAttr("Reflectivity", GetNode() + "reflectivity", root.reflectivitySignatures());
   AddMapComboBoxAttr("Inherent contrast", GetNode() + "inherentContrast", root.inherentContrasts());
}

void PartManager::Platform::HandleAddPartButtonPressed()
{
   AddPlatformPart* addPartDialog = new AddPlatformPart(GetNode(), this);
   addPartDialog->show();
}

bool PartManager::Platform::IsAddPartSupported() const
{
   return true;
}

void PartManager::Platform::InitAllChildren()
{
   WsfPM_MilPlatform platform(GetNode());

   InitChildren(platform.Sensors());
   InitChildren(platform.Comms());
   InitChildren(platform.Weapons());
   InitChildren(platform.Processors());

   InitChild(platform.Mover());
   InitChild(platform.Fuel());
}
