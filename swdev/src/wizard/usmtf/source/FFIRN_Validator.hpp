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

#ifndef FFIRNVALIDATOR_HPP
#define FFIRNVALIDATOR_HPP

#include "usmtf_export.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "Field.hpp"
#include "Validatable.hpp"

namespace usmtf
{
const static std::string NULL_FIELD = "-";

//! Descriptors serve as the optional tag a human might put on a field
//! in a USMTF Message. In this system we use them to optimize validation
//! algorithms if the user uses them in their set.
//! For reference please see section E of 4.3.6 Fields in the MIL-STD-6040
//! and individual set documents specify the optional descriptors for their
//! fields.
namespace ffirn_descriptors
{
const std::string cLATM = "LATM";
const std::string cLATS = "LATS";
const std::string cRARA = "RARA";
const std::string cFLFL = "FLFL";
const std::string cRAFL = "RAFL";
const std::string cBRFL = "BRFL";
const std::string cBRRA = "BRRA";
const std::string cSNWP = "SNWP";
} // namespace ffirn_descriptors

//! The idea behind FFIRNValidators is simple. In the USMTF Spec there lives FFIRN/FUDN numbers
//! that are identifiers to a set of rules that validate field content. Programmatically were
//! representing this as classes that the content of the field will be ran through. Interested
//! parties will extract the data they are interested in.
class USMTF_EXPORT FFIRN_Validator : public Validatable
{
public:
   using FFIRN_Func = std::function<bool(void)>;
   //! The Descriptor Map is an optimization technique so that if Descriptors are
   //! present only that set of validations needs to be ran.
   using FFIRN_DescriptorMap = std::unordered_map<std::string, FFIRN_Func>;
   FFIRN_Validator()         = default;

   //! I am not a huge fan of violating command query separation. But, in this case it makes sense. The functions  that
   //! come into ParseAndValidate have to parse to get value of the component, sometimes there are many components in a
   //! single field. Unfortunately parsing is an intertwined attribute of validation. You cant parse according to some
   //! rule set if the field is invalid, making it impossible to separate the two notions easily. I have opted to
   //! instead name the functions for what they are actually doing. Parsing components and validating them as well.
   //! Optionally members can be set within the function as well.
   virtual bool ParseAndValidate(const usmtf::Field& aField) = 0;

   //! Static for reusable outside a need of an instance.
   //! Confirms a string is within a vector of strings. I expect using Sets might be faster.
   static bool IsInSelection(const std::string& aValue, const std::vector<std::string>& choices) noexcept;
   static bool IsStringLength(const std::string& aValue, size_t aLength) noexcept;
   //! Returns the positions of a specified delimiter if present, npos if not.
   static size_t CheckForAnyDelimeter(const std::string& aValue, const std::vector<std::string>& aDelimeters) noexcept;
   static size_t CheckForAnyDelimeter(const std::string&              aValue,
                                      const std::vector<std::string>& aDelimeters,
                                      std::string&                    aFoundDelimeter) noexcept;

   template<class T>
   static bool IsInRange(T aValue, T aLowerBound, T aUpperBound) noexcept
   {
      return aValue >= aLowerBound && aValue <= aUpperBound;
   }

   //! Return the content of the field as it was in the USMTF set
   const std::string& GetRawContent() const noexcept;

   //! Values separated by delimeter
   std::vector<std::string> SplitOnDelimeter(const std::string& value, const std::string& delimeter);
   const std::string&       GetFieldContent() const noexcept;

protected:
   //! Speed up validation if descriptor is present. Runs all functions if it isn't.
   bool ValidateByDescriptor(const FFIRN_DescriptorMap& aDescriptorMap);

   //! Clear Errors If the validator is Valid. Confusingly this IS a possible state.
   //! Given Validation can pass one of many rules, the errors added by prior validations
   //! that passed will need to be cleared to make IsValid() accurate, as it just returns whether or not
   //! the error container has errors. So instead of forcing users to clear it in each of their validation function
   //! an potentially forgetting, we slim that potential to a single call in their ParseAndValidate
   //! override.
   void ClearErrorsIfValid(bool aHasAtLeastOnePassed) noexcept;
   //! Field Setter for internal use only.
   void  SetField(const Field& aField) noexcept;
   Field mField;
};
} // namespace usmtf
#endif // !
