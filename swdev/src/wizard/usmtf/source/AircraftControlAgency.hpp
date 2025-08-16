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

#ifndef AIRCRAFTCONTROLAGENCY_HPP
#define AIRCRAFTCONTROLAGENCY_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an AircrafControlAgency.
//! Table TABLE 7  TYPE OF AIRCRAFT CONTROL AGENCY.
//! 7A.UF.html
/*!
DATA ITEM CODE EXPLANATION
(U)	AIR OPERATIONS CENTER	(U)	AOC
(U)	AIR SUPPORT OPERATIONS CENTER	(U)	ASOC
(U)	AIRBORNE COMMAND, CONTROL, COMMUNICATIONS (ABCCC)	(U)	ABC
(U)	AIRBORNE WARNING AND CONTROL SYSTEM	(U)	AWAC
(U)	ASSAULT SUPPORT COORDINATOR (AIRBORNE)	(U)	ASCA
(U)	COMBAT CONTROL TEAM	(U)	CCT
(U)	COMBINED/COALITION AIR OPERATIONS CENTER	(U)	CAOC
(U)	CONTROL REPORTING CENTER	(U)	CRC
(U)	CONTROL REPORTING ELEMENT	(U)	CRE
(U)	CONTROL REPORTING POST	(U)	CRP
(U)	DIRECT AIR SUPPORT CENTER	(U)	DASC
(U)	DIRECT AIR SUPPORT CENTER (AIRBORNE)	(U)	DASA
(U)	EARLY WARNING/CONTROL SITE (MARINE)	(U)	EWC
(U)	FORWARD AIR CONTROL POST	(U)	FACP
(U)	FORWARD AIR CONTROLLER	(U)	FAC
(U)	FORWARD AIR CONTROLLER (AIRBORNE)	(U)	FACA
(U)	GROUND CONTROL INTERCEPT	(U)	GCI
(U)	HELICOPTER SUPPORT TEAM	(U)	HST	(U)	THE TEAM WHICH PROVIDES SUPPORT TO HELICOPTER OPERATIONS.
(U)	INITIAL TERMINAL GUIDANCE TEAM	(U)	ITG
(U)	JOINT AIR OPERATIONS CENTER	(U)	JAOC
(U)	MARINE AIR TRAFFIC CONTROL DETACHMENT	(U)	MATC
(U)	MARINE AIR TRAFFIC CONTROL MOBILE TEAM	(U)	MMT
(U)	OTHER	(U)	OTR	(U)	EXPLAIN IN FREE-TEXT SET.
(U)	TACTICAL AIR COMMAND CENTER (MARINE)	(U)	TACC
(U)	TACTICAL AIR CONTROL PARTY	(U)	TACP
(U)	TACTICAL AIR COORDINATOR, AIRBORNE	(U)	TACA	(U)	AN OFFICER WHO COORDINATES, FROM AN AIRCRAFT, THE ACTION OF
COMBAT AIRCRAFT ENGAGED IN CLOSE SUPPORT OF GROUND OR SEA FORCES. (U)	TACTICAL AIR DIRECTION CENTER (MARINE)	(U)
TADC (U)	TACTICAL AIR OPERATIONS CENTER (MARINE)	(U)	TAOC
*/
//! USMTF Excerpt:
//! EXAMPLE: /CRC
class USMTF_EXPORT AircraftControlAgency : public RegexValidator
{
public:
   AircraftControlAgency();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetAgency() const noexcept;

protected:
   std::string mAgency;
};
} // namespace usmtf
#endif
