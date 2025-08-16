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

#ifndef TASK_LIST_DOCK_WIDGET_HPP
#define TASK_LIST_DOCK_WIDGET_HPP

// C++ STL Includes
#include <vector>

// Qt Includes
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>

// Utility Includes
#include "UtPath.hpp"
#include "UtTextDocument.hpp"

// WSF Includes/Forward Declarations
class WsfParseIndex;
class WsfParser;
class WsfParseSourceInclude;

// TaskList Includes
#include "TaskListPluginUtil.hpp"
#include "TaskListPrefObject.hpp"
#include "TaskListTask.hpp"
#include "ui_TaskListDockWidget.h"

namespace wizard
{
// Wizard Forward Declarations
class Editor;
class EditorManager;
class ParseResults;
class Project;
class ProjectWorkspace;
class TextSourceCache;

} // end namespace wizard

namespace TaskList
{
// TaskList Forward Declarations
class Item;

//! A dock widget containing a list of tasks to complete.
//! @par Purpose:
//!   This widget allows you to have one location containing reminders to complete tasks.
//!   It also provides navigation to the tasks by double-clicking the description (or any other cell in the same row).
//! @par How to use the Filter:
//!   Expand the combo box to select a particular filter.
//!   The filter will update the table accordingly.
//! @par How to sort tasks:
//!   Click on any column to sort the tasks by that column.
//!   The sorting cycles between ascending order, descending order, and no order.
//! @par How to configure the task tags:
//!   The preferences control which task tags the task table pick up.
//!   The default tags are:  ATTENTION, BUG, DEBUG, HACK, TASK, TODO, and UNDONE.
//!   To add a tag, type the name and press the + button.
//!   To remove a tag, select it in the list and press the - button.
//! @par How to configure the columns:
//!   You can configure the column order and visibility of the task table.
//!   The default visible columns are:  File, Line, Tag, and Description.
//!   To configure the order, you can drag the columns in the task table to different locations.
//!   Also, you can control the order in the preferences by selecting a column in the Visible Columns list and clicking
//!   the Up and Down arrows. To configure the visibility, you can control the visibility of the columns in preferences.
//!   To hide a column, select it in the Visible Columns and press the Left arrow.
//!   To show a column, select it in the Available Columns and press the Right arrow.
//! @note A more detailed help text can be found <a href="../../documentation/html/docs/wkf_plugin/wiz_task_list.html">here</a>
class DockWidget : public QDockWidget
{
public:
   //! Represents the index for the filter
   enum class FilterIndex
   {
      AllScenarioFiles,   //!< Pulls tasks from every file from the scenario
      OpenScenarioFiles,  //!< Pulls tasks from every open file
      CurrentScenarioFile //!< Pulls tasks from the current file
   };

   //! Constructs a DockWidget
   //! @post the DockWidget has "TaskList" as the name
   //! @post the parent widget is wkfEnv.GetMainWindow()
   //! @post the widget has no Qt::WindowFlags
   DockWidget() noexcept;
   //! Destructs a DockWidget
   ~DockWidget() override = default;

private slots:
   //! Slot executed when the task table should refresh its data
   //! @param aIndex is the filter index
   void Populate(int aIndex) noexcept;

   //! @name Slots pertaining to the filter
   //@{

   //! Slot executed when the filter changes
   //! @param aIndex is the index of the new filter
   void OnFilterChanged(int aIndex) noexcept;

   //@}

   //! @name Slots pertaining to the task table
   //@{

   //! Slot executed when the user double-clicks any cell in the task table
   //! @param aRow is the row index of the cell
   //! @param aColumn is the column index of the cell
   void OnCellDoubleClicked(int aRow, int aColumn) noexcept;

   //@}

   //! @name Slots pertaining to the task table column header
   //@{

   //! Slot executed when the user moves a column section header
   //! @param aLogicalIndex is the index of the section header in the model
   //! @param aOldVisualIndex is the old index of the section header in the view
   //! @param aNewVisualIndex is the new index of the section header in the view
   //! @post The TaskList::PrefObject reflects the new column order
   void OnColumnSectionMoved(int aLogicalIndex, int aOldVisualIndex, int aNewVisualIndex) noexcept;

   //! Slot executed when the user clicks the column section header to initiate sorting
   //! @param aLogicalIndex is the index of the section header in the model
   //! @param aOrder is the sort order
   void OnColumnSortIndicatorChanged(int aLogicalIndex, Qt::SortOrder aOrder) noexcept;

   //! Slot executed when the user resizes a column
   //! @param aLogicalIndex is the index of the section header in the model
   //! @param aOldSize is the old size of the column
   //! @param aNewSize is the new size of the column
   void OnColumnResized(int aLogicalIndex, int aOldSize, int aNewSize) noexcept;

   //@}

   //! @name Slots pertaining to Wizard core signals
   //@{

   //! Slot executed when Wizard finishes the WsfPProxyDeserialize
   void OnFullParseComplete(wizard::Project* aProjectPtr) noexcept;

   //! Slot executed when the active wizard::Editor changes
   void OnActiveEditorChanged(wizard::Editor* aEditorPtr) noexcept;

   //@}

   //! @name Slots pertaining to PrefObject signals
   //@{

   //! Slot executed when the user updates the task tag list in the PrefWidget
   void OnTaskTagsUpdated() noexcept;

   //! Slot executed when the user updates the visible column list in the PrefWidget
   void OnVisibleColumnsUpdated() noexcept;

   //! Slot executed when the user updates the path format in the PrefWidget
   void OnPathFormatUpdated() noexcept;

   //@}

private:
   //! Contains debugging information for the Task class
   static QString sDebugLog;

   //! @name Set-up routines
   //@{

   //! Set up the TaskList::DockWidget's QTableWidget's horizontal (column) header
   //! @note As this rebuilds the entire header, this resets the column order back to the default and makes all columns
   //! visible.  To restore the preferred column order and visibility, use RestoreColumnOrder.
   void SetUpTaskTableHeader() noexcept;

   //! Set the default column sizes
   void SetDefaultColumnSizes() noexcept;

   //! Set up the TaskList::DockWidget's QTableWidget
   void SetUpTaskTable() noexcept;

   //! Set up the TaskList::DockWidget
   void SetUpDockWidget() noexcept;

   //! Connects signals to slots
   void Connect() noexcept;

   //@}

   //! @name Sorting routines
   //@{

   //! Unsorts the task table
   //! @post The task table is unsorted (not sorted by any column)
   void UnsortTaskTable() noexcept;

   //! Resorts the task table by the preferred column
   //! @post The task table is sorted by the preferred column
   void ResortTaskTable() noexcept;

   //! Sorts the tasks by the given column and sort order
   void SortTasks(ColumnIndex aColumn, Qt::SortOrder aOrder) noexcept;

   //@}

   //! @name Connect routines
   //@{

   //! Connects filter signals to slots
   void ConnectFilter() noexcept;

   //! Connects task table signals to slots
   void ConnectTaskTable() noexcept;

   //! Connects task table column header signals to slots
   void ConnectTaskTableColumnHeader() noexcept;

   //! Connects Wizard core signals to slots
   void ConnectWizardCore() noexcept;

   //! Connects PrefObject signals to slots
   void ConnectPrefObject() noexcept;

   //@}

   //! Initializes necessary member variables
   //! @return whether initialization succeeded
   //! @note if any member variable fails to initialize, then initialization fails
   bool Initialize() noexcept;

   //! Add a comment to the list
   //! @param aRow is the row to insert the comment
   //! @param aTag is the task tag
   //! @param aDescription is the task description
   //! @param aDirectory is the input file directory
   //! @param aFile is the input file name
   //! @param aLine is the line number of the task
   //! @param aColumn is the column number of the task
   //! @post the table contains an additional task
   void AddComment(int            aRow,
                   const QString& aTag,
                   const QString& aDescription,
                   const QString& aDirectory,
                   const QString& aFile,
                   const QString& aLine,
                   const QString& aColumn) noexcept;

   //! Acquires an include object from a input file path and name
   //! @param aFile is the input file path and name
   //! @return the include object from which to extract the comments
   //! @post aText no longer starts with aIdentifier (the function removed it)
   WsfParseSourceInclude* AcquireIncludeFile(const UtPath& aFile) noexcept;

   //! Acquires all the tasks from the given input file
   //! @param aFile is the input file path and name
   void AcquireCommentsFromFile(const QString& aFile) noexcept;

   //! Restores the preferred column order and visibility
   //! @note To avoid unnecessary modifications to the column header, repeated calls to this function do nothing.
   void RestoreColumnOrder() noexcept;

   //! Saves the column sizes
   //! @note Use this before rebuilding the task table header
   void SaveColumnSizes() noexcept;

   //! Restore the column sizes
   //! @note Use this after rebuilding the task table header
   void RestoreColumnSizes() noexcept;

   //! Rebuilds the task table
   //! @par details
   //!   This function rebuilds the column header.
   //!   Then, it rebuilds the table.
   //!   Finally, it restores the preferred column order and visibility.
   //! @post the task table is current
   void RebuildTaskTable() noexcept;

   //! Populates the task table with tasks from the current file
   void PopulateFromCurrentFile() noexcept;

   //! Populates the task table with tasks from all open files
   void PopulateFromOpenDocuments() noexcept;

   //! Populates the task table with tasks from all of the input files in the current scenario
   void PopulateFromCurrentScenario() noexcept;

   //! Change the tooltip on the Task List table based on the filter.
   //! @param aIndex is the filter index
   void ChangeTableToolTip(int aIndex) noexcept;

   //! The TaskList::DockWidget UI
   Ui::TaskListDockWidget mUi;
   //! The TaskLIst::PrefObject
   PrefObject* mPrefObject;

   //! @name Necessary variables to acquire the tasks
   //@{
   wizard::ProjectWorkspace* mWorkspace;
   wizard::EditorManager*    mEditorManager;
   wizard::TextSourceCache*  mSourceCache;
   wizard::Project*          mProject;
   std::vector<UtPath>       mStartupFiles;
   wizard::ParseResults*     mParseResults;
   WsfParser*                mParser;
   WsfParseIndex*            mParseIndex;
   //@}

   //! Represents the connection between QHeaderView::sectionResized and DockWidget::OnColumnResized
   //! @par details
   //!   Invoking DockWidget::OnColumnResized programmatically will override the PrefData::mColumnSizes.
   //!   Delaying this connection allows the DockWidget to complete set-up without overriding the
   //!   PrefData::mColumnSizes. Connecting in DockWidget::OnFullParseComplete seems to be enough of a delay to avoid
   //!   triggering DockWidget::OnColumnResized programmatically.
   //! @note QTableView::updateGeometries will emit QHeaderView::sectionResized.
   //! Emitting QHeaderView::sectionResized will invoke DockWidget::OnColumnResized programmatically if connected in
   //! DockWidget::ConnectTaskTableHeader.
   QMetaObject::Connection mColumnResizedConnection;

   //! The comments from the input files
   //! @note The contents depends on the filter
   std::vector<Task> mComments;

   //! Prevents unnecessary modifications to the column order
   //! @par details
   //!   Each rebuild of the column header (via SetUpTaskTableHeader) will reset the column order and visibility to the
   //!   default. The default is to have all columns visible and in the order:  Tag, Description, Directory, File, Line,
   //!   Column. Calling SetUpTaskTableHeader will set this to false, indicating that the programmer should call
   //!   RestoreColumnOrder. RestoreColumnOrder will restore the preferred column order and visibility, setting this to
   //!   true.
   bool mColumnOrderRestored{true};

   //! The preferred column sizes
   std::array<int, 6> mColumnSizes;

   //! Prevents unnecessary modifications to the column order
   //! @par details
   //!   Each rebuild of the column header (via SetUpTaskTableHeader) will reset the column sizes.
   //!   Calling SaveColumnSizes will store the preferred column sizes, setting this to false.
   //!   Calling RestoreColumnSizes will restore the preferred column sizes, setting this to true.
   bool mColumnSizesRestored{false};

   //! Contains debugging information for a Task
   mutable QString mDebugLog;
};

} // end namespace TaskList

#endif // TASK_LIST_DOCK_WIDGET_HPP
