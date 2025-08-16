// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
//==============================================================================

#ifndef USERDEFINED_HPP
#define USERDEFINED_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Field.hpp"
#include "Message.hpp"
#include "Set.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "UtMemory.hpp"

class UserDefinedSet : public usmtf::Set
{
public:
   UserDefinedSet(const std::string& aType, const std::vector<usmtf::Field>& aFields)
      : Set(aType, aFields)
   {
      try
      {
         // positional correctness
         const usmtf::Field& yesField = GetField(1);
         const usmtf::Field& noField  = GetField(2);
         if (!YesFieldValidator(yesField.GetContent()) || !NoFieldValidator(noField.GetContent()))
         {
            AddError("Error", "invalid");
         }
      }
      catch (const usmtf::SetError&)
      {
         // representation of positional failures, we expected for this Set
         // type to have fields at positions were indexing into.
         AddError("Error", "invalid");
      }
   }

   std::string WhatIsYourAnswer() const { return GetField(2).GetContent(); }

private:
   static bool YesFieldValidator(const std::string& content) { return content == "YES"; }
   static bool NoFieldValidator(const std::string& content) { return content == "NO"; }
};


class UserDefinedMessage : public usmtf::Message
{
public:
   UserDefinedMessage(const std::string aType, std::vector<std::unique_ptr<usmtf::Set>>&& aSets)
      : Message(aType, std::move(aSets))
   {
      try
      {
         // Think big, this can be as complex or as simple as needed.
         const usmtf::Set&     set  = GetSet(2);
         const UserDefinedSet* sptr = usmtf::SetFactory::GetFactory()->CastIfRegistered<UserDefinedSet>(set);
         if (sptr == nullptr)
         {
            // set in expected position. Wrong type
            AddError("Set requirements unsatisfied", "NA", "Read docs");
         }
         bool setValid = set.IsValid();
         if (!setValid)
         {
            AddError("Set was invalid", "NA", "See below");
            AddErrors(set.GetErrors()); // sets were in position, right type, sets own validation failed.
         }
      }
      catch (const usmtf::MessageError&)
      {
         // Set positions weren't satisfied
         AddError("Set not found", "NA", "find the set");
      }
   }

   bool HasAnsweredYes() const
   {
      // Think big, this can be as complex or as simple as needed.
      // should check validity before accessing. Don't want to validate on any accessor.
      const usmtf::Set&     set  = GetSet(2);
      const UserDefinedSet* sptr = usmtf::SetFactory::GetFactory()->CastIfRegistered<UserDefinedSet>(set);
      return sptr->WhatIsYourAnswer() != "NO";
   }
};

#endif
