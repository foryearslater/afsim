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


#ifndef ZONEFACTORY_HPP
#define ZONEFACTORY_HPP

#include <memory>

#include "Message.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! currently supported zone types.
namespace ZoneType
{
const std::string cPOLYARC  = "POLYARC";
const std::string cPOLYGON  = "POLYGON";
const std::string cCORRIDOR = "CORRIDOR";
const std::string cTRACK    = "TRACK";
const std::string cORBIT    = "ORBIT";
const std::string cPOINT    = "POINT";
const std::string cCIRCLE   = "CIRCLE";
const std::string cRADARC   = "RADARC";
const std::string cLINE     = "LINE";
} // namespace ZoneType

//! Self Explanatory, instantiate a zone using the correct supported USMTF_Transformer
std::unique_ptr<Zone> CreateZone(const std::string& zoneType, const Segment& aSegment) noexcept;
} // namespace usmtf

#endif
