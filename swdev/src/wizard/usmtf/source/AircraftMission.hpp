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

#ifndef AIRCRAFTMISSON_HPP
#define AIRCRAFTMISSON_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "Set.hpp"


namespace usmtf
{

/*!
The MSNACFT Set is used to represent aircraft for a mission. At the moment, it does not verify the
aircraft type according to table 513A of the usmtf standard because its used to just
name a platform definition which the user supplements. This can be added though. Not all fields
are useful at this time, so some are ignored or unvalidated.

USMTF Example:
      MSNACFT/2/ACTYP:FA18/HAMMER 53/2HARM/2S2WG/HR53/30/00201/B:20153/00202/B:30153//

Standard Excerpt:
   MSNACFT/                     /                   /                     /
    1 NUMBER OF AIRCRAFT  2 TYPE OF AIRCRAFT  3 AIRCRAFT CALL SIGN  4 PRIMARY CONFIGURATION CODE
    M   [1-2 N]           M   [2/6 AN]        M   [1-12 ABNS]       M   [1-15 ABNS]

    /.............................../................................/................
    5 SECONDARY CONFIGURATION CODE  6 LINK 16 ABBREVIATED CALL SIGN  7 TACAN CHANNEL
    O   [1-15 ABNS]                 O   [4 AN]                       O   [1-3 N]


    /.................R................/............R...........//
    8 PRIMARY JTIDS UNIT (JU) ADDRESS  9 IFF/SIF MODE AND CODE

Detailed Document:
MSNACFT.UF.RTF
*/
class USMTF_EXPORT AircraftMission : public Set
{
public:
   AircraftMission(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   AircraftMission(const AircraftMission&) = delete;
   int                GetNumberOfAircraft() const noexcept;
   const std::string& GetAircraftType() const noexcept;
   const std::string& GetAircraftCallsign() const noexcept;

private:
   int         mNumberOfAircraft;
   std::string mAircraftType;
   std::string mAircraftCallsign;
};
} // namespace usmtf

#endif
