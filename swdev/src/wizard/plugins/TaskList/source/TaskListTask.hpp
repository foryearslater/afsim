// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2022 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TASK_LIST_TASK_HPP
#define TASK_LIST_TASK_HPP

// C++ STL Includes
#include <cstddef>

// Qt Includes
#include <QString>
#include <QTableWidget>

// Utility Includes
#include "UtPath.hpp"
#include "UtTextDocument.hpp"

// TaskList Includes
#include "TaskListPluginUtil.hpp"

namespace TaskList
{
//! Represents a task
//! @par What determines a Task?
//!   A task is a comment with a specific format.
//!   The following regex determine a task:
//!   @verbatim
//!     (//|/*|#)(#|*|/|\s)*<task-identifier>\W+<comment-body>(*/)?
//!   @endverbatim
//!   `<task-identifier>` is one of the task tags defined in preferences.
//!   `<comment-body>` is the rest of the comment.
class Task final
{
public:
   //! The path format
   static PathFormat mPathFormat;
   //! The working directory
   static UtPath mWorkingDirectory;

   //! Constructs a Task from the given UtTextDocumentRange and task tag
   //! @param aRange is the given UtTextDocumentRange
   //! @param aTag is the given task tag
   //! @return a Task containing the data from the given UtTextDocumentRange and task tag
   //! @retval "a valid Task" if the construction succeeds
   //! @retval "an invalid Task" if the construction fails
   static Task RangeToTask(const UtTextDocumentRange& aRange, const QString& aTag) noexcept;

   //! Constructs a Task from the given row of the given QTableWidget
   //! @param aTable is the given QTableWidget
   //! @param aRow is the given row
   //! @return a Task containing the data from the given row of the given QTableWidget
   //! @retval "a valid Task" if the construction succeeds
   //! @retval "an invalid Task" if the construction fails
   static Task RowToTask(QTableWidget* aTable, int aRow) noexcept;

   //! Determine whether this Task is valid
   //! @par details
   //!   A Task is valid when it has a tag, path, file, line, and column number.
   //!   It does not need a description to be valid.
   //! @return whether this Task is valid
   bool IsValid() const noexcept;

   //! @name Metadata
   //@{

   //! The task tag
   QString mTag;
   //! The task description (comment body)
   QString mDescription;
   //! The file path (directory + name)
   UtPath mPath;
   //! The file directory
   QString mDirectory;
   //! The file name
   QString mFile;
   //! The line number
   std::size_t mLine;
   //! The column number
   std::size_t mColumn;

   //@}

   //! Functor to simplify comparison of Tasks
   struct Compare
   {
      //! The part of the Task to compare
      ColumnIndex mIndex;
      //! The sort order controlling the comparison
      Qt::SortOrder mOrder;

      //! Overload of operator() for comparison
      //! @param aLhs is the left-hand side of the comparison
      //! @param aRhs is the right-hand side of the comparison
      //! @return the result of the comparison
      bool operator()(const Task& aLhs, const Task& aRhs) const;

      //! Implementation of the comparison
      //! @tparam is the type representing the part of the Task to compare
      //! @param aLhs is the left-hand side of the comparison
      //! @param aRhs is the right-hand side of the comparison
      //! @retval "a lesser-than comparison" if mOrder is Qt::AscendingOrder
      //! @retval "a greater-than comparison" if mOrder is Qt::DescendingOrder
      template<typename MEMBER>
      bool CompareImpl(const MEMBER& aLhs, const MEMBER& aRhs) const
      {
         switch (mOrder)
         {
         case Qt::AscendingOrder:
            return aLhs < aRhs;
         case Qt::DescendingOrder:
            return aLhs > aRhs;
         default:
            return false;
         }
      }
   };

private:
   //! Contains debugging information for the Task class
   static QString sDebugLog;

   //! @name Helper methods for RangeToTask
   //@{

   //! Determines whether the given text starts with a comment delimiter
   //! @param aText is the given text
   //! @return the index of the last match
   //! @pre The given text is ASCII
   //! @post aText no longer starts with the comment delimiter (the function removed it)
   static bool MatchCommentP(QString& aText) noexcept;

   //! Determines whether the given text starts with zero or more comment symbols, such as `#`, `*`, and `/`, and/or
   //! whitespace.
   //! @param aText is the text against which to match the regular expression
   //! @return the index of the last match
   //! @pre The given text is ASCII
   //! @post aText no longer starts with comment symbols or whitespace (the function removed it)
   static bool MatchCommentSymbolsAndWhitespaceP(QString& aText) noexcept;

   //! Determines whether the given text starts with the given identifier
   //! @param aText is the given text
   //! @param aIdentifier is the given identifier
   //! @pre The given text is ASCII
   //! @pre The given identifier is ASCII
   //! @return whether the given text starts with the given idenfier
   //! @post aText no longer starts with aIdentifier (the function removed it)
   static bool MatchIdentifierP(QString& aText, const QString& aIdentifier) noexcept;

   //! Determines whether the given text starts with zero or more non-word ASCII characters
   //! @param aText is the text against which to match the regular expression
   //! @return whether there was a match
   //! @pre The given text is ASCII
   //! @post aText now contains only the comment body and a possible ending block comment delimiter
   static bool MatchNonWordP(QString& aText) noexcept;

   //@}

   //! Contains debugging information for a Task
   mutable QString mDebugLog;
};

//! Determines whether two Tasks are equal
//! @retval true if aLhs is equal to aRhs
//! @retval false if aLhs is not equal to aRhs
bool operator==(const Task& aLhs, const Task& aRhs) noexcept;

//! Determines whether two Tasks are not equal
//! @retval true if aLhs is not equal to aRhs
//! @retval false if aLhs is equal to aRhs
bool operator!=(const Task& aLhs, const Task& aRhs) noexcept;

} // end namespace TaskList

#endif // TASK_LIST_TASK_HPP
