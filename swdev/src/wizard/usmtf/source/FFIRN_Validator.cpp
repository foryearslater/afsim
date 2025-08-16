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

#include "FFIRN_Validator.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace usmtf
{
bool FFIRN_Validator::ValidateByDescriptor(const FFIRN_DescriptorMap& aDescriptorMap)
{
   // descriptor is always "" if not there. So lookup will check for that.
   auto fnLocation = aDescriptorMap.find(mField.GetDescriptor());

   // optimize by going directly to validator mapped to descriptor.
   if (fnLocation != aDescriptorMap.end())
   {
      return fnLocation->second();
   }
   else
   {
      return std::any_of(aDescriptorMap.begin(),
                         aDescriptorMap.end(),
                         [this](const std::pair<std::string, FFIRN_Func>& pair)
                         {
                            bool result = pair.second();
                            // This clears errors that may have been found before a validator passed.
                            if (result)
                            {
                               ClearErrors();
                            }
                            return result;
                         });
   }
}

void FFIRN_Validator::ClearErrorsIfValid(bool aHasAtLeastOnePassed) noexcept
{
   if (aHasAtLeastOnePassed)
   {
      ClearErrors();
   }
}

void FFIRN_Validator::SetField(const Field& aField) noexcept
{
   mField = aField;
}

bool FFIRN_Validator::IsInSelection(const std::string& aValue, const std::vector<std::string>& choices) noexcept
{
   return std::find(choices.begin(), choices.end(), aValue) != choices.end();
}

bool FFIRN_Validator::IsStringLength(const std::string& aValue, size_t aLength) noexcept
{
   return aValue.size() == aLength;
}

size_t FFIRN_Validator::CheckForAnyDelimeter(const std::string& aValue, const std::vector<std::string>& aDelimeters) noexcept
{
   for (const std::string& delimeter : aDelimeters)
   {
      size_t pos = aValue.find(delimeter);
      if (pos != std::string::npos)
      {
         return pos;
      }
   }
   return std::string::npos;
}

size_t FFIRN_Validator::CheckForAnyDelimeter(const std::string&              aValue,
                                             const std::vector<std::string>& aDelimeters,
                                             std::string&                    aFoundDelimeter) noexcept
{
   for (const std::string& delimeter : aDelimeters)
   {
      size_t pos = aValue.find(delimeter);
      if (pos != std::string::npos)
      {
         aFoundDelimeter = aValue.substr(pos, delimeter.size());
         return pos;
      }
   }
   return std::string::npos;
}

const std::string& FFIRN_Validator::GetRawContent() const noexcept
{
   return mField.GetContent();
}

std::vector<std::string> FFIRN_Validator::SplitOnDelimeter(const std::string& value, const std::string& delimeter)
{
   std::vector<std::string> splitValues{};
   std::size_t              current, previous = 0;
   current = value.find_first_of(delimeter);
   while (current != std::string::npos)
   {
      splitValues.push_back(value.substr(previous, current - previous));
      previous = current + 1;
      current  = value.find_first_of(delimeter, previous);
   }
   splitValues.push_back(value.substr(previous, current - previous));
   return splitValues;
}

const std::string& FFIRN_Validator::GetFieldContent() const noexcept
{
   return mField.GetContent();
}
} // namespace usmtf
