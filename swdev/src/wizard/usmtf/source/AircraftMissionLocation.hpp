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

#ifndef AIRCRAFTMISSONLOCATION_HPP
#define AIRCRAFTMISSONLOCATION_HPP

#include "usmtf_export.h"

#include <string>

#include "Altitude.hpp"
#include "EllipticalArea.hpp"
#include "Field.hpp"
#include "FreeText.hpp"
#include "LatLon.hpp"
#include "Radius.hpp"
#include "RelativeBearingRange.hpp"
#include "SectorGeometry.hpp"
#include "Set.hpp"
#include "UtMemory.hpp"
#include "VariantValidator.hpp"
#include "VerifiedLatLonS.hpp"

namespace usmtf
{
/*!
 THE AMSNLOC SET PROVIDES MISSION LOCATION INFORMATION FOR MISSIONS WHICH HAVE NO SPECIFIC TARGET
 LOCATION, E.G., ORBITS, ALERT.

USMTF Example:
      AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT ALFA//

Standard Excerpt:
   AMSNLOC/                              /                             /........................
    1 DAY-TIME AND MONTH OF START  2 DAY-TIME AND MONTH OF STOP  3 MISSION LOCATION NAME
    M   [10 AN]                    M   [10 AN]                   O   [1-20 ABNS]

    /.............................../......................./................/.......R.......//
    4 ALTITUDE IN HUNDREDS OF FEET  5 AIR MISSION PRIORITY  6 AREA GEOMETRY  7 LOCATION
    O   [1-3 N]                     O   [1-3 AN]            O   [2/22 ANS]   O   [1/20 ABNS]

Detailed Document:
AMSNLOC.UF.RTF
*/
class USMTF_EXPORT AircraftMissionLocation : public Set
{
public:
   using AreaGeometry = VariantValidator<Radius, EllipticalArea, SectorGeometry>;
   using Location     = VariantValidator<LatLon, VerifiedLatLonS, RelativeBearingRange, FreeText>;
   using Locations    = std::vector<std::unique_ptr<Location>>;
   AircraftMissionLocation(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   AircraftMissionLocation(const AircraftMissionLocation&) = delete;
   const std::string&  GetMissionLocation() const;
   const Altitude&     GetAltitude() const noexcept;
   const AreaGeometry& GetAreaGeometry() const noexcept;
   const Locations&    GetLocations() const noexcept;

private:
   std::string  mMissionLocationName;
   Altitude     mAltittude;
   AreaGeometry mAreaGeometry;
   Locations    mLocations;
};
} // namespace usmtf

#endif
