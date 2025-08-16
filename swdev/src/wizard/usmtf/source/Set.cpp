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

#include "Set.hpp"

#include <sstream>

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtCast.hpp"

namespace usmtf
{
Set::Set(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : mSetTextFormat(aSTF)
   , mFields(aFields)
{
}

const std::string& Set::GetType() const noexcept
{
   return mSetTextFormat;
}

bool Set::IsEmpty() const noexcept
{
   return mFields.empty();
}

int Set::GetFieldCount() const noexcept
{
   return ut::cast_to_int(mFields.size());
}

const Field& Set::GetField(size_t aIndex) const
{
   if (aIndex >= mFields.size())
   {
      throw SetError("Cannot retrieve Field at " +
                        std::to_string(aIndex) + ". Doing so would result in buffer overflow. Make sure index is within the Message SetCount and Message is not empty.",
                     aIndex);
   }
   return mFields[aIndex];
}

const std::string& Set::GetFieldContent(size_t aIndex) const
{
   if (IsValid())
   {
      return GetField(aIndex).GetContent();
   }
   else
   {
      return NULL_FIELD;
   }
}

const std::string Set::GetReconstructedSet() const noexcept
{
   std::stringstream ss{};
   for (const auto& field : mFields)
   {
      std::string content = field.HasDescriptor() ? field.GetDescriptor() + ":" + field.GetContent() : field.GetContent();
      ss << content << "/";
   }
   ss << "/";
   return ss.str();
}
} // namespace usmtf
