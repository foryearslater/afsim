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

#include "TabularSet.hpp"

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
TabularSet::TabularSet(const std::string& aSTF, const std::vector<Field>& aFields, const size_t aHeaderOffset) noexcept
   : Set(aSTF, aFields)
   , mHeaderOffset(aHeaderOffset == 0 ? 1 : aHeaderOffset)
{
}

TabularSet::Series TabularSet::GetRow(const size_t aRowIndex) const
{
   size_t                    columnCount = GetColumnCount();
   std::vector<Field const*> results;
   for (size_t column = 1; column <= columnCount; column++)
   {
      const Field& field = GetField(aRowIndex, column);
      results.push_back(&field);
   }
   return results;
}

std::vector<std::string> TabularSet::GetRowContent(const size_t aRowIndex) const
{
   size_t                   columnCount = GetColumnCount();
   std::vector<std::string> results;
   for (size_t column = 1; column <= columnCount; column++)
   {
      std::string content = GetFieldContent(aRowIndex, column);
      results.push_back(content);
   }
   return results;
}

TabularSet::Series TabularSet::GetColumn(const size_t aColumnIndex) const
{
   size_t                    rowCount = GetRowCount();
   std::vector<const Field*> results;
   for (size_t row = 1; row <= rowCount; row++)
   {
      const Field& field = GetField(row, aColumnIndex);
      results.push_back(&field);
   }
   return results;
}

std::vector<std::string> TabularSet::GetColumnContent(const size_t aColumnIndex) const
{
   size_t                   rowCount = GetRowCount();
   std::vector<std::string> results;
   for (size_t row = 1; row <= rowCount; row++)
   {
      std::string content = GetFieldContent(row, aColumnIndex);
      results.push_back(content);
   }
   return results;
}

bool TabularSet::IsValidFieldCount() const noexcept
{
   // -1 because of the identifier field
   return (GetFieldCount() - 1) % mHeaderOffset == 0;
}

size_t TabularSet::GetRowCount() const noexcept
{
   if (!IsValidFieldCount())
   {
      return 0;
   }
   // -1 because of the identifier field
   // -1 from total for entire header row
   return ((GetFieldCount() - 1) / mHeaderOffset) - 1;
}

size_t TabularSet::GetColumnCount() const noexcept
{
   return mHeaderOffset;
}

// Returns a field at a index position in the Set.
const Field& TabularSet::GetField(const size_t aRow, const size_t aColumn) const
{
   return Set::GetField((mHeaderOffset * aRow) + aColumn);
}
const std::string& TabularSet::GetFieldContent(const size_t aRow, const size_t aColumn) const
{
   return Set::GetFieldContent((mHeaderOffset * aRow) + aColumn);
}
} // namespace usmtf
