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

#ifndef ACO_HPP
#define ACO_HPP

#include "usmtf_export.h"

#include <memory>

#include "Comment.hpp"
#include "InputBuilder.hpp"
#include "Message.hpp"
#include "Zone.hpp"

namespace usmtf
{
//! USMTF Excerpt:
//! PURPOSE: THE ACO IS USED TO PROVIDE SPECIFIC DETAILED ORDERS FOR AIRSPACE MANAGEMENT AND CONTROL FROM A HIGHER
//! COMMAND TO SUBORDINATE UNITS. GENERAL INSTRUCTIONS: THE ACO DEFINES AND ESTABLISHES SPECIAL PURPOSE AIRSPACE FOR
//! MILITARY OPERATIONS AS DEEMED NECESSARY BY THE APPROPRIATE MILITARY AUTHORITY. IT NOTIFIES ALL AGENCIES OF THE
//! EFFECTIVE TIME OF ACTIVATION AND THE COMPOSITE STRUCTURE OF THE AIRSPACE TO BE USED. THE ACO MAY INCLUDE AIR ROUTES,
//! BASE DEFENSE ZONES, COORDINATING MEASURES/LINES, DROP ZONES, PICKUP POINTS, RESTRICTED AREAS, ETC. ZONES MAY BE
//! DEFINED AS A CIRCLE, CORRIDOR, POLYARC, RADARC, LINE, POINT, TRACK, ORBIT, OR POLYGON. IF A NEW AREA IS ESTABLISHED
//! OR AN EXISTING AREA DELETED, A NEW MESSAGE SHOULD BE SENT.

//! Programmer Commentary:
//! For our purposes the Air Control Order (ACO) Message describes zones used for AFSIM Scenario lay down. The details
//! of each zone are indicated by separate segments started by the presence of an ACMID set. For many more examples see
//! tests/message_examples/Aco.txt

//! Example Segment that creates a Corridor Zone named B2
//! ACMID/ACM:ATC/DESIG:B2/CORRIDOR/USE:CLSB//
//! CORRIDOR/5NM/LATM:3500N07900W/LATM:3530N07000W/LATM:3530N06500W//
//! EFFLEVEL/FLFL:FL180-FL220//

//! Detailed Document:
//! ACO.UF.RTF
class USMTF_EXPORT Aco : public Message
{
public:
   Aco(const std::string& aMTF, std::vector<std::unique_ptr<Set>>&& aSets) noexcept;
   //! Get the zones that are extracted from the parsed ACO
   std::vector<const Zone*> GetZones() const noexcept;
   //! Get other relevant sets that are not zones but are useful as comments for Scenario Designers
   const Comment* GetHeader() const noexcept;

private:
   //! Constructs zone from segments identified as candidates
   void BuildZoneList() noexcept;
   //! Builds comment header of relevant sets that are not zones themselves
   void                               BuildHeader() noexcept;
   std::vector<std::unique_ptr<Zone>> mZones;
   std::unique_ptr<Comment>           mComments;
};

//! Exports the Aco to scenario lay down file indicated by the target InputBuilder
USMTF_EXPORT void ExportAco(const Aco& aAco, InputBuilder& ib);
} // namespace usmtf

#endif
