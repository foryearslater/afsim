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

#ifndef PACKAGEDATA_HPP
#define PACKAGEDATA_HPP

#include "usmtf_export.h"

#include <string>
#include <tuple>
#include <vector>

#include "Field.hpp"
#include "FreeText.hpp"
#include "TabularSet.hpp"

namespace usmtf
{
/*!
THE 9PKGDAT SET PROVIDES INFORMATION TO THE MISSION COMMANDER ABOUT THE FORCES THAT ARE COMBINED TO
||||                 MAKE UP A PACKAGE.
||||
||||    C        9PKGDAT
||||             /PKGID                    /UNIT           /MSNNO            /PMSN                   /NO
||||             /                         /               /                 /                       /
||||             1 PACKAGE IDENTIFICATION  2 TASKED UNIT   3 MISSION NUMBER  4 PRIMARY MISSION TYPE  5 NUMBER OF AIRCRAFT
||||             M   [1-5 ANS]             M   [1-20 ABNS] M   [1-8 ABNS]    M   [2-6 AN]            M   [1-2 N]
||||
||||             /ACTYPE             /ACSIGN
||||             /                   /                     //
||||             6 TYPE OF AIRCRAFT  7 AIRCRAFT CALL SIGN
||||             M   [2/6 AN]        M   [1-12 ABNS]
*/
//! Detailed Document:
//! 9PKGDAT.UF.RTF
class USMTF_EXPORT PackageData : public TabularSet
{
public:
   using Unit  = std::tuple<FreeText, FreeText, FreeText, FreeText, int, FreeText, FreeText>;
   using Units = std::vector<Unit>;
   PackageData(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const Units& GetUnits() const noexcept;

private:
   Units mUnits;
};
} // namespace usmtf

#endif
