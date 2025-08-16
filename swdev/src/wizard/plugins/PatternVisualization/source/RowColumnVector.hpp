// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ROWCOLUMNVECTOR_HPP
#define ROWCOLUMNVECTOR_HPP

#include <cassert>
#include <vector>

namespace PatternVisualizer
{
// This class defines a data structure similar to a two-dimensional array.
// The dimensions of the two-dimensional array are fixed and are defined
// at construction time. In other words, the array cannot grow, but it's
// dimensions do not have to be fixed at compile time.
//
// Internally, the data in this fixed two-dimensional array is row major order.
//
// Important pitfall: The terminology for this class is 'row' and 'columns'.
// NOT 'x' and 'y'. Functions in this class take arguments in "rows and
// columns" order (i.e., y then x).
template<typename T>
class RowColumnVector
{
   using DataType = std::vector<T>;

public:
   using size_type             = uint32_t;
   using value_type            = T;
   static const size_type npos = static_cast<size_type>(-1);

   RowColumnVector(size_type aNumRows, size_type aNumColumns)
      : mNumRows(aNumRows)
      , mNumColumns(aNumColumns)
      , mData(mNumColumns * mNumRows)
   {
      assert(aNumRows != 0);
      assert(aNumColumns != 0);

      // Make sure that numRows * numColumns doesn't overflow size_type.
      assert(!(aNumColumns > std::numeric_limits<size_type>::max() / aNumRows));
   }

   template<typename Iter>
   RowColumnVector(size_type aNumRows, size_type aNumColumns, Iter aFirst, Iter aLast)
      : mNumRows(aNumRows)
      , mNumColumns(aNumColumns)
      , mData(aFirst, aLast)
   {
      assert(aNumRows != 0);
      assert(aNumColumns != 0);
      assert(mData.size() == aNumRows * aNumColumns);
   }

   size_type GetNumRows() const { return mNumRows; }
   size_type GetNumColumns() const { return mNumColumns; }
   size_type GetNumElements() const { return mNumRows * mNumColumns; }

   T&       operator()(size_type aRow, size_type aColumn) { return mData[GetIndex(aRow, aColumn)]; }
   const T& operator()(size_type aRow, size_type aColumn) const { return mData[GetIndex(aRow, aColumn)]; }

   struct Iterator
   {
      using ContainerType     = RowColumnVector<T>;
      using value_type        = T;
      using difference_type   = int32_t;
      using reference         = T&;
      using pointer           = T*;
      using iterator_category = std::forward_iterator_tag;

      friend ContainerType;

      Iterator()                       = default;
      Iterator(const Iterator& aOther) = default;
      Iterator& operator=(const Iterator& aOther) = default;

      Iterator& operator++()
      {
         Advance();
         return *this;
      }

      Iterator& operator++(int)
      {
         Advance();
         return *this;
      }

      const value_type& operator*() const { return *(mContainerPtr->mData.begin() + mCurrent); }

      bool operator!=(const Iterator& aRhs) const { return !(*this == aRhs); }
      bool operator==(const Iterator& aRhs) const
      {
         return (mContainerPtr == mContainerPtr) && (mCurrent == aRhs.mCurrent);
      }

   private:
      explicit Iterator(const RowColumnVector<T>* aContainer, size_type aCurrent, difference_type aStep)
         : mContainerPtr(aContainer)
         , mCurrent(aCurrent)
         , mStep(aStep)
      {
      }

      void Advance()
      {
         uint32_t numElements = mContainerPtr->GetNumElements();

         mCurrent += mStep;
         if (mCurrent >= numElements)
         {
            mCurrent = numElements;
         }
      }

      const ContainerType* mContainerPtr;
      size_type            mCurrent;
      difference_type      mStep;
   };

   struct Location
   {
      Location() = default;
      Location(size_type aRow, size_type aColumn)
         : mRow(aRow)
         , mColumn(aColumn)
      {
      }

      size_type mRow;
      size_type mColumn;
   };

   Location GetLocation(Iterator aIterator) const
   {
      uint32_t index  = GetIndex(aIterator);
      uint32_t row    = index / mNumColumns;
      uint32_t column = index % mNumColumns;
      return Location(row, column);
   }

   // Supports range-based for. Enumerates over the entire data structure in row major order.
   Iterator begin() const { return Iterator(this, 0, 1); }
   Iterator end() const { return Iterator(this, GetNumElements(), 1); }

   // Enumerates down a single row.
   Iterator RowBegin(size_type aRow) const { return Iterator(this, GetIndex(aRow, 0), 1); }
   Iterator RowEnd(size_type aRow) const { return RowBegin(aRow + 1); }

   // Enumerates down a single column.
   Iterator ColumnBegin(size_type aColumn) const
   {
      return Iterator{this, GetIndex(0, aColumn), static_cast<typename Iterator::difference_type>(mNumColumns)};
   }

   Iterator ColumnEnd(size_type aColumn) const { return end(); }

   // Enumerates down a single column, but in reverse.
   Iterator ColumnReverseBegin(size_type aColumn) const
   {
      return Iterator{this, GetIndex(mNumRows - 1, aColumn), -static_cast<typename Iterator::difference_type>(mNumColumns)};
   }

   Iterator ColumnReverseEnd(size_type aColumn) const { return end(); }

private:
   size_type GetIndex(Iterator aIterator) const { return aIterator.mCurrent; }

   size_type GetIndex(size_type aRow, size_type aColumn) const { return aRow * mNumColumns + aColumn; }

   size_type mNumRows;
   size_type mNumColumns;
   DataType  mData;
};
} // namespace PatternVisualizer

#endif
