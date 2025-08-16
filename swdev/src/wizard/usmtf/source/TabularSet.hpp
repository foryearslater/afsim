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
#ifndef TABULARSET_HPP
#define TABULARSET_HPP

#include "Set.hpp"

namespace usmtf
{
//! Useful conveniences for Sets that are Tabular in nature, see Onetrack.hpp for example
class USMTF_EXPORT TabularSet : public Set
{
public:
   //! Row Or Column
   using Series = std::vector<const Field*>;
   //! Multiple Rows Or Columns
   using NSeries = std::vector<Series>;
   TabularSet(const std::string& aSTF, const std::vector<Field>& aFields, const size_t aHeaderOffset = 1) noexcept;

protected:
   Series                   GetRow(const size_t aRowIndex) const;
   std::vector<std::string> GetRowContent(const size_t aRowIndex) const;
   Series                   GetColumn(const size_t aColumnIndex) const;
   std::vector<std::string> GetColumnContent(const size_t aColumnIndex) const;
   bool                     IsValidFieldCount() const noexcept;
   size_t                   GetRowCount() const noexcept;
   size_t                   GetColumnCount() const noexcept;
   const Field&             GetField(const size_t aRow, const size_t aColumn) const;
   const std::string&       GetFieldContent(const size_t aRow, const size_t aColumn) const;
   const size_t             mHeaderOffset;
};
} // namespace usmtf
#endif
