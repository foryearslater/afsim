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

#ifndef USMTF_TRANSFORMER_HPP
#define USMTF_TRANSFORMER_HPP

#include "usmtf_export.h"

#include "Exportable.hpp"
#include "InputBlock.hpp"
#include "MacroEnabled.hpp"
#include "Message.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Transformer.hpp"
#include "UtStringUtil.hpp"
#include "Validatable.hpp"

namespace usmtf
{
//! Exists to Facilitate the contract for Polymorphic Script Block Exports From Set Segments
class USMTF_Transformer : public Validatable, public Exportable, public MacroEnabled
{
public:
   USMTF_Transformer() = default;
   explicit USMTF_Transformer(const Segment& aSegment)
      : mSegment(aSegment)
   {
   }
   virtual ~USMTF_Transformer() = default;
   template<typename SetType>
   const SetType* ExtractSet(const std::string& aStf,
                             const std::string& aLogicalConceptName,
                             bool               aSkipValidation = false,
                             bool               aIsOptional     = false)
   {
      static_assert(std::is_base_of<Set, SetType>::value, "Template argument must be subclass of Set");
      try
      {
         Segment::SetLocation loc = mSegment.GetSet(aStf);
         const SetType*       set = SetFactory::GetFactory()->CastIfRegistered<SetType>(loc.first);

         if (aSkipValidation)
         {
            return set;
         }
         if (!set)
         {
            AddError(aStf + " Set Type must be registered to create a " + aLogicalConceptName,
                     aStf + " Set",
                     "To create a " + aLogicalConceptName + " you must register a Set that handles " + aStf);
         }
         else if (!set->IsValid())
         {
            AddError(aStf + ": " + loc.first.GetReconstructedSet() + " is invalid. See below.",
                     aStf + " Set",
                     "To create a " + aLogicalConceptName + " " + aStf + " must be a valid set.");
            AddErrors(set->GetErrors());
         }
         return set;
      }
      catch (const SetError& err)
      {
         if (!aIsOptional)
         {
            AddError(aLogicalConceptName + " requirements are not satisfied", aLogicalConceptName, std::string(err.what()));
         }
         return nullptr;
      }
   }
   //! This is in transformer because thats where this is needed in the lib. Script Blocks cant have spaces in names.
   static const std::string ReplaceSpaces(const std::string& aTarget, char aDesiredChar) noexcept;

protected:
   const Segment mSegment;
};
} // namespace usmtf
#endif
