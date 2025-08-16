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

#ifndef AIRASSETCONTROL_HPP
#define AIRASSETCONTROL_HPP

#include "usmtf_export.h"

#include <string>

#include "AircraftControlAgency.hpp"
#include "Field.hpp"
#include "FreeText.hpp"
#include "LatLon.hpp"
#include "Set.hpp"
#include "UTM_Location.hpp"
#include "VariantValidator.hpp"
#include "VerifiedLatLonDS.hpp"
#include "VerifiedLatLonTM.hpp"


namespace usmtf
{
/*!
 TTHE CONTROLA SET PROVIDES CONTROL INFORMATION FOR A MISSION.

USMTF Example:
      CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/NAME:ALFA//

Standard Excerpt:
||||    O   R    CONTROLA/                                  /               /                    /......................
||||                     1 TYPE OF AIRCRAFT CONTROL AGENCY  2 CALL SIGN     3 PRIMARY FREQUENCY  4 SECONDARY FREQUENCY
||||                     M   [3-4 A]                        M   [1-17 ABNS] M   [1/8 ANS]        O   [1/8 ANS]
||||
||||                     /................................/..................//
||||                     5 LINK 16 ABBREVIATED CALL SIGN  6 REPORT-IN POINT
||||                     O   [4 AN]                       O   [1/22 ABNS]

Detailed Document:
AMSNLOC.UF.RTF
*/
class USMTF_EXPORT AirAssetControl : public Set
{
public:
   AirAssetControl(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   AirAssetControl(const AirAssetControl&) = delete;
   const AircraftControlAgency& GetAgency() const noexcept;
   const FreeText&              GetCallsign() const noexcept;
   const FreeText&              GetPrimaryFrequency() const noexcept;
   const FreeText&              GetSecondaryFrequency() const noexcept;
   const FreeText&              GetLink16Callsign() const noexcept;
   const VariantValidator<LatLon, UTM_Location, VerifiedLatLonTM, VerifiedLatLonDS, FreeText>& GetReportInPoint() const noexcept;

private:
   AircraftControlAgency mAgency;
   FreeText              mCallSign;
   FreeText mPrimaryFrequency;   // Not validating as its not useful to our current used cases. If you need correct
                                 // Frequency, please model.
   FreeText mSecondaryFrequency; // Not validating as its not useful to our current used cases. If you need correct
                                 // Frequency, please model.
   FreeText mLink16CallSign;
   // FreeText must go at the end because everything passes this check. Unfortunately a string literal is a valid report
   // in point. User must supplement definition.
   VariantValidator<LatLon, UTM_Location, VerifiedLatLonTM, VerifiedLatLonDS, FreeText> mReportInPoint;
};
} // namespace usmtf

#endif
