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

#ifndef PACKAGECOMMANDER_HPP
#define PACKAGECOMMANDER_HPP

#include "usmtf_export.h"

#include <string>
#include <vector>

#include "Field.hpp"
#include "FreeText.hpp"
#include "Set.hpp"

namespace usmtf
{
/*!
 THE PKGCMD SET PROVIDES INFORMATION ABOUT THE MISSION COMMANDER TO THE FORCES IN THE PACKAGE.
||||
||||    C        PKGCMD/                         /               /                 /
||||                   1 PACKAGE IDENTIFICATION  2 TASKED UNIT   3 MISSION NUMBER  4 AIRCRAFT CALL SIGN
||||                   M   [1-5 ANS]             M   [1-20 ABNS] M   [1-8 ABNS]    M   [1-12 ABNS]
||||
||||                   /................................//
||||                   5 LINK 16 ABBREVIATED CALL SIGN
||||                   O   [4 AN]
*/
//! Detailed Document:
//! PKGCMD.UF.html
class USMTF_EXPORT PackageCommander : public Set
{
public:
   PackageCommander(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const FreeText& GetPackageId() const noexcept;
   const FreeText& GetTaskedUnit() const noexcept;
   const FreeText& GetMissionNumber() const noexcept;
   const FreeText& GetAircraftCallsign() const noexcept;
   const FreeText& GetLink16Callsign() const noexcept;

private:
   FreeText mPackageId;
   FreeText mTaskedUnit;
   FreeText mMissionNumber;
   FreeText mAircraftCallsign;
   FreeText mLink16Callsign;
};
} // namespace usmtf

#endif
