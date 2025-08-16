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

#ifndef VARIANTVALIDATOR_HPP
#define VARIANTVALIDATOR_HPP

#include "usmtf_export.h"

#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>

#include "FFIRN_Validator.hpp"

// Variadic aware check to assert all Types are FFIRN_Validators
namespace
{
// code from
// https://stackoverflow.com/questions/13562823/parameter-pack-aware-stdis-base-of
template<bool... b>
struct static_all_of;

// implementation: recurse, if the first argument is true
template<bool... tail>
struct static_all_of<true, tail...> : static_all_of<tail...>
{
};

// end recursion if first argument is false -
template<bool... tail>
struct static_all_of<false, tail...> : std::false_type
{
};

// - or if no more arguments
template<>
struct static_all_of<> : std::true_type
{
};
} // namespace

namespace usmtf
{
//! VariantValidator is for composing together multiple Validators that dont have common components.
//! If the field Validator you are look to model shares common components with its usmtf specified options
//! its best to make a "standard" FFIRN_Validators like LatLon.cpp. This is reserved for when those specified
//! options share no common members and composition is needed. The route would be to model every individual rule set
//! separately and then use this class to compose them together and use as if you would any other validator. Please
//! see test_variant_validator.cpp for usage examples.
//! Note. This class enforces compile time check that ...Types are FFIRN_Validators.
template<typename... Types>
class VariantValidator : public FFIRN_Validator
{
   static_assert(static_all_of<std::is_base_of<FFIRN_Validator, Types>::value...>::value,
                 "Invalid Type, Must be an FFIRN_Validator");

public:
   VariantValidator() = default;
   // By design once a passed validator is found, there is no way to copy the ptr that is to an internal
   // data structure
   VariantValidator(const VariantValidator& aOther)  = delete;
   VariantValidator(const VariantValidator&& aOther) = delete;
   VariantValidator operator=(const VariantValidator& aOther) = delete;
   VariantValidator operator=(const VariantValidator&& aOther) = delete;

   //! API contract for a FFIRN_Validator, delegates to a recursive
   //! template to achieve the composite validation of its members.
   //! only one passing validator is stored as the variant, and its
   //! simply the first the passes ParseAndValidate.
   bool ParseAndValidate(const usmtf::Field& aField) override
   {
      bool res = ParseAndValidateP<0, sizeof...(Types)>{}(*this, aField);
      ClearErrorsIfValid(res);
      return res;
   }

   //! Returns casted * if the validator that passed is the same as T, nullptr
   //! otherwise. This can be used to effectively test the type of the passed variant,
   //! the validators validity state, get your needed validator or all 3.
   //! more examples in test_variant_validator.cpp.
   template<typename T>
   T* IsMatch() const
   {
      return dynamic_cast<T*>(mValidator);
   }

   //! Returns false if no match was made or Parse and Validate has yet to be called.
   bool HasMatch() const { return mValidator != nullptr; }

private:
   template<int N, int TypeCount>
   struct ParseAndValidateP
   {
      bool operator()(VariantValidator<Types...>& aValidator, const Field& aField)
      {
         auto& ref = std::get<N>(aValidator.mTypeInstances);
         if (ref.ParseAndValidate(aField))
         {
            aValidator.mValidator = &ref;
            return true;
         }
         else
         {
            aValidator.AddErrors(ref.GetErrors());
         }
         return ParseAndValidateP<N + 1, TypeCount>{}(aValidator, aField);
      }
   };

   template<int N>
   struct ParseAndValidateP<N, N>
   {
      bool operator()(VariantValidator<Types...>& aValidator, const Field& aField) { return false; }
   };

   FFIRN_Validator*     mValidator = nullptr;
   std::tuple<Types...> mTypeInstances;
};
} // namespace usmtf

#endif
