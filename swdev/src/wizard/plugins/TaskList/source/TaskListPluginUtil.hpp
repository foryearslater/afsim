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

#ifndef TASK_LIST_PLUGIN_UTIL_HPP
#define TASK_LIST_PLUGIN_UTIL_HPP

namespace TaskList
{
//! Represents the path format
enum class PathFormat
{
   Absolute, //!< The full path (example:  "/a/b/c")
   Relative  //!< The path relative to another path (example:  "a/b/c")
};

//! Represents the logical index for the task table's column header
enum class ColumnIndex
{
   Tag,         //!< The task tag
   Description, //!< The task description
   Directory,   //!< The input file directory
   File,        //!< The input file name
   Line,        //!< The line number of the task
   Column       //!< The column number of the task
};

} // end namespace TaskList

#endif // TASK_LIST_PLUGIN_UTIL_HPP
