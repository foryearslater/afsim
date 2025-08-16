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

#ifndef TASKUNIT_HPP
#define TASKUNIT_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "FreeText.hpp"
#include "NamedLocation.hpp"
#include "Set.hpp"


namespace usmtf
{

/*!
THE TASKUNIT SET INDICATES THE UNIT THAT IS TASKED TO SUPPORT A MISSON.

USMTF Example:
   TASKUNIT/63 RS/ICAO:KXXQ/00254-00310/DET 1 FOL//

Standard Excerpt:
   TASKUNIT/                         /......................./......................................../...............//
             1 TASKED UNIT DESIGNATOR  2 TASKED UNIT LOCATION  3 LINK 16 UNIT ADDRESS BLOCK ASSIGNMENT  4 COMMENTS
               M   [1-30 ABNS]           O   [1/20 ABNS]         O   [11 NS]                              O   [1-68 ABNS]

Detailed Document:
TASKUNIT.UF.RTF
*/
class USMTF_EXPORT TaskUnit : public Set
{
public:
   TaskUnit(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   TaskUnit(const TaskUnit&) = delete;
   const FreeText&      GetUnitDesignator() const noexcept;
   const NamedLocation& GetLocation() const noexcept;
   const std::string&   GetComments() const noexcept;

private:
   FreeText      mUnitDesignator;
   NamedLocation mLocation;
   std::string   mComments;
};
} // namespace usmtf

#endif
