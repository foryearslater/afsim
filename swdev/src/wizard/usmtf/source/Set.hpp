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
#ifndef SET_HPP
#define SET_HPP

#include "usmtf_export.h"

#include <string>
#include <vector>

#include "Field.hpp"
#include "Validatable.hpp"

namespace usmtf
{

/*!
Set Identifiers or Set Text Format
 ----------------
 Every Set starts with a set identifier, a keyword that identifies the set, its reason for existence and the format it
supports.

 Set
 ----
 A set is a predetermined field or sequence of fields, preceded by a set identifier, containing all the required
information concerning an aspect of the subject of a formatted message. All sets begin with a set identifier, have one
or more fields, and terminate with the end-of-set marker (//).

 i.e.
 ACMID/ACM:ATC/NAME:BBR-22/ORBIT/USE:CLSB//

 There are three types of set formats:
 - linear
 - columnar
 - Free Text

 For more detailed account of a set please see section 4.3.7 and 5.2.1 of the MIL-STD-6040 standard. We have a copy
under 1_Mtfrules.pdf

 Sets are collection of related data, the sentence of the USMTF if you will.

 Every Set besides columnar sets can be considered linear. Even columnar fields are laid out in the same way.

*/
class USMTF_EXPORT Set : public Validatable
{
public:
   Set(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   virtual ~Set() = default;

   //! Return the Set Identifiers/Set Text Format of the set instance.
   const std::string& GetType() const noexcept;

   //! Returns true if the Set has no fields, false otherwise
   //! Not possible in the standard, might be useful still.
   bool IsEmpty() const noexcept;
   int  GetFieldCount() const noexcept;

   //! Returns a field at a index position in the Set.
   const Field& GetField(size_t aIndex) const;

   //! Returns the Contents of a field at specified index.
   const std::string& GetFieldContent(size_t aIndex) const;

   //! Return a string that is reconstructed version of the Set
   //! as it was in the file.
   const std::string GetReconstructedSet() const noexcept;

protected:
   std::string        mSetTextFormat;
   std::vector<Field> mFields;
};
} // namespace usmtf
#endif
