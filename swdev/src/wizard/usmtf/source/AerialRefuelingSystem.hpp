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

#ifndef AERIALREFUELINGSYSTEM_HPP
#define AERIALREFUELINGSYSTEM_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of AerialRefuelingSystem.
//! USMTF Excerpt:
/*!
   TABLE 539A
   AERIAL REFUELING SYSTEM
   ORDER BY DINAME
   DATA ITEM
   CODE
   EXPLANATION
   (U)	BOOM	(U)	                                          BOM
   (U)	BOOM AND CENTERLINE DROGUE	(U)	                     CBD
   (U)	BOOM AND CENTERLINE DROGUE, AND WINGTIP DROGUE	(U)	CWT
   (U)	BOOM AND WINGTIP DROGUE	(U)	                        BWD
   (U)	CENTERLINE DROGUE	(U)	                              CLD
   (U)	CENTERLINE DROGUE AND WINGTIP DROGUE	(U)	         CDT
   (U)	WINGTIP DROGUE	(U)	                                 WTD
*/
class USMTF_EXPORT AerialRefuelingSystem : public RegexValidator
{
public:
   AerialRefuelingSystem();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetSystem() const noexcept;

protected:
   std::string mSystem;
};
} // namespace usmtf
#endif
