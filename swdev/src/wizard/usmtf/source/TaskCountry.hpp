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

#ifndef TASKCOUNTRY_HPP
#define TASKCOUNTRY_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "Set.hpp"


namespace usmtf
{

/*!
The TSKCNTRY Set is used to represent a country in the world. At the moment, it does not verify this
according to table 59A (the reference table for acceptable 2 letter code)
of the usmtf standard because its used to just name a side in a scenario. This can be added though.

USMTF Example:
      TSKCNTRY/PS//

Standard Excerpt:
   TSKCNTRY/                    //
          1 COUNTRY OF THE WORLD

Detailed Document:
TSKCNTRY.UF.RTF
*/
class USMTF_EXPORT TaskCountry : public Set
{
public:
   TaskCountry(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   TaskCountry(const TaskCountry&) = delete;
   const std::string& GetCountry() const noexcept;

private:
   std::string mCountry;
};
} // namespace usmtf

#endif
