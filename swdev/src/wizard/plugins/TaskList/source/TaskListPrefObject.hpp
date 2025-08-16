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

#ifndef TASK_LIST_PREF_OBJECT_HPP
#define TASK_LIST_PREF_OBJECT_HPP

// C++ STL Includes
#include <array>

// Qt Includes
#include <QString>
#include <QStringList>

// WKF Includes
#include "WkfPrefObject.hpp"

// TaskList Includes
#include "TaskListPluginUtil.hpp"

namespace TaskList
{
//! Structure storing the preference data
struct PrefData
{
   //! The default task tags
   static const QStringList cDEFAULT_TAGS;
   //! The default list of available columns
   //! @note This list contains the available, hidden columns
   static const QStringList cDEFAULT_AVAILABLE_TAGS;
   //! The default list of visible columns
   //! @note This list contains the available, shown columns
   static const QStringList cDEFAULT_VISIBLE_TAGS;

   //! @name Default sizes for the task table columns
   //@{
   static constexpr int cTAG_COLUMN_SIZE{100};
   static constexpr int cDESCRIPTION_COLUMN_SIZE{200};
   static constexpr int cDIRECTORY_COLUMN_SIZE{200};
   static constexpr int cFILE_COLUMN_SIZE{100};
   static constexpr int cLINE_COLUMN_SIZE{75};
   static constexpr int cCOLUMN_COLUMN_SIZE{75};
   //@}

   //! @name Preferences that do show up in the PrefWidget
   //@{

   //! The available task tags
   QStringList mTaskTags{cDEFAULT_TAGS};

   //! The available, hidden columns
   QStringList mAvailableColumns{cDEFAULT_AVAILABLE_TAGS};
   //! The available, shown columns
   QStringList mVisibleColumns{cDEFAULT_VISIBLE_TAGS};

   //! The path format
   PathFormat mPathFormat;

   //@}

   //! @name Preferences that do not up in the PrefWidget
   //@{

   //! The index of the column by which the user sorted the task table
   //! @note -1 means the task table is unsorted
   int mSortByColumnIndex{-1};

   //! The sort order of the column by which the user sorted the task table
   //! @note -1 means the task table is unsorted
   Qt::SortOrder mSortByColumnOrder;

   //! The preferred column sizes
   //! @bug GCC 4.8.5
   //! Doing std::array<int, 1> myArray = { 1 }; should work, but GCC throws an error.
   //! "error: array must be initialized with a brace-enclosed initializer"
   //! Explicit construction of a std::array without brace-initialization is a valid workaround.
   //! GCC 6 fixes this issue.
   std::array<int, 6> mColumnSizes = std::array<int, 6>{cTAG_COLUMN_SIZE,
                                                        cDESCRIPTION_COLUMN_SIZE,
                                                        cDIRECTORY_COLUMN_SIZE,
                                                        cFILE_COLUMN_SIZE,
                                                        cLINE_COLUMN_SIZE,
                                                        cCOLUMN_COLUMN_SIZE};

   //@}

   //! Determine whether the user updated the task tags
   bool mTaskTagsUpdated{false};
   //! Determine whether the user updated the visible columns
   bool mVisibleColumnsUpdated{false};
   //! Determine whether the user updated the path format
   bool mPathFormatUpdated{false};
};

//! The object storing the preference data for the TaskList:Plugin
class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT
public:
   //! The name of the PrefObject
   static constexpr const char* cNAME = "TaskList";

   //! Constructs a PrefObject
   //! @param aParent is the parent QObject
   explicit PrefObject(QObject* aParent = nullptr);
   //! Destructs a PrefObject
   ~PrefObject() noexcept override = default;

   //! Get the current list of task tags
   //! @return the current list of task tags
   QStringList GetTaskTags() const noexcept;

   //! Get the name of the task tag given its index in the list
   //! @param aIndex is the index of the task tag
   //! @return the name of the task tag
   QString GetTaskTagName(int aIndex) const noexcept;

   //! Get the index of the task tag given its name in the list
   //! @param aName is the name of the task tag
   //! @return the index of the task tag
   int GetTaskTagIndex(const QString& aName) const noexcept;

   //! Get the current list of available, hidden columns
   //! @return the current list of available, hidden columns
   QStringList GetAvailableColumns() const noexcept;

   //! Get the current list of available, visible columns
   //! @return the current list of available, visible columns
   QStringList GetVisibleColumns() const noexcept;

   //! Get the name of the task table column given its index in the list
   //! @param aIndex is the index of the task tag
   //! @return the name of the task tag
   QString GetColumnName(int aIndex) const noexcept;

   //! Get the index of the task table column given its name in the list
   //! @param aName is the name of the task tag
   //! @return the index of the task tag
   int GetColumnIndex(const QString& aName) const noexcept;

   //! Sets the column data
   //! @param aAvailableColumns is the new list of available, hidden columns
   //! @param aVisibleColumns is the new list of available, visible columns
   void SetColumnData(const QStringList& aAvailableColumns, const QStringList& aVisibleColumns) noexcept;

   //! Get the path format
   //! @return the path format
   PathFormat GetPathFormat() const noexcept;

   //! Get the index of the column by which the user sorted the task table
   //! @return the index of the column by which the user sorted the task table
   int GetSortByColumnIndex() const noexcept;

   //! Get the sort order of the column by which the user sorted the task table
   //! @return the sort order of the column by which the user sorted the task table
   Qt::SortOrder GetSortByColumnOrder() const noexcept;

   //! Set the index of the column by which the user sorted the task table
   //! @param aIndex is the index of the column by which the user sorted the task table
   void SetSortByColumnIndex(int aIndex) noexcept;

   //! Set the sort order of the column by which the user sorted the task table
   //! @param aOrder is the sort order of the column by which the user sorted the task table
   void SetSortByColumnOrder(Qt::SortOrder aOrder) noexcept;

   //! Get the column sizes to restore after a rebuild of the task table header
   //! @return the column sizes
   const std::array<int, 6>& GetColumnSizes() const noexcept;

   //! Set the column sizes to restore after a rebuild of the task table header
   //! @param aColumnSizes the column sizes
   void SetColumnSizes(const std::array<int, 6>& aColumnSizes) noexcept;

   //! Add the given task tag into the list
   //! @par details
   //!   It checks whether the task tag exists in the list.
   //!   If not, it will add the task tag into the list.
   //!   Otherwise, it does nothing.
   //! @param aTag is the given task tag
   //! @retval true if aTag does not exist in the list and was added
   //! @retval false if aTag does exists in the list
   bool AddTaskTag(const QString& aTag) noexcept;

   //! Remove the given task tag from the list
   //! @par details
   //!   It checks whether the task tag exists in the list.
   //!   If so, it will remove the task tag from the list.
   //!   Otherwise, it does nothing.
   //! @param aTag is the given task tag
   //! @retval true if aTag does exist in the list and was removed
   //! @retval false if aTag does not exist in the list
   bool RemoveTaskTag(const QString& aTag) noexcept;

signals:
   //! @name Signals emitted via user interaction with the TaskList::PrefWidget
   //@{

   //! Signal emitted every time the user updates the task tag list
   void TaskTagsUpdated();

   //! Signal emitted every time the user updates the visible columns list
   void VisibleColumnsUpdated();

   //! Signal emitted every time the user updates the path format
   void PathFormatUpdated();

   //@}

   //! @name Signals emitted via user interaction with the TaskList::DockWidget's table
   //@{

   //! Signal emitted every time the user moves a column header in the TaskList::DockWidget's table
   //! @param aOldIndex is the old index of the column header in the view
   //! @param aNewIndex is the new index of the column header in the view
   void VisibleColumnMoved(int aOldIndex, int aNewIndex);

   //! Signal emitted every time the user moves a column header in the TaskList::DockWidget's table
   void ColumnDataUpdated();

   //@}

private:
   //! Contains debugging information for the Task class
   static QString sDebugLog;

   //! Apply the current settings
   //! @par details
   //!   If the user actually changed the task tags, then emit the TaskTagsUpdated signal and clear the mUserUpdated
   //!   flag. Otherwise, do nothing.
   //! @note This emits the TaskTagsUpdated signal if necessary.
   void Apply() override;

   //! Reset the task tag list
   //! @par details
   //!   Clears the current task tag list.
   //!   Then, appends the default task tags to the listtPathsTo.
   void SetPreferenceDataP(const PrefData& aPrefData) override;

   //! Helper methods to read preference data
   //@{

   //! Helper method to read the task tags
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadTaskTagsP(QSettings& aSettings, PrefData& aData) const noexcept;

   //! Helper method to read the available, hidden columns
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadAvailableColumnsP(QSettings& aSettings, PrefData& aData) const noexcept;

   //! Helper method to read the available, visible columns
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadVisibleColumnsP(QSettings& aSettings, PrefData& aData) const noexcept;

   //! Helper method to read the path format
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadPathFormatP(QSettings& aSettings, PrefData& aData) const noexcept;

   //! Helper method to read the column by which to sort and the sort order
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadSortByColumnP(QSettings& aSettings, PrefData& aData) const noexcept;

   //! Helper method to read the column sizes
   //! @param aSettings is the preferences from which to read
   //! @param aData is the preference data to populate
   void ReadColumnSizesP(QSettings& aSettings, PrefData& aData) const noexcept;

   //@}

   //! Read the preferences from the given settings
   //! @par details
   //!   This reads the number of task tags recorded in the QSettings.
   //!   Then, it reads in each task tag and stores it in the PrefData.
   //! @param aSettings is the given settings
   //! @return the PrefData read from the QSettings
   PrefData ReadSettings(QSettings& aSettings) const override;

   //! Helper methods to save preference data
   //@{

   //! Helper method to save the task tags
   //! @param aSettings is the preferences to which to save
   void SaveTaskTagsP(QSettings& aSettings) const noexcept;

   //! Helper method to save the available, hidden columns
   //! @param aSettings is the preferences to which to save
   void SaveAvailableColumnsP(QSettings& aSettings) const noexcept;

   //! Helper method to save the available, visible columns
   //! @param aSettings is the preferences to which to save
   void SaveVisibleColumnsP(QSettings& aSettings) const noexcept;

   //! Helper method to save the path format
   //! @param aSettings is the preferences to which to save
   void SavePathFormatP(QSettings& aSettings) const noexcept;

   //! Helper method to save the column by which to sort and the sort order
   //! @param aSettings is the preferences to which to save
   void SaveSortByColumnP(QSettings& aSettings) const noexcept;

   //! Helper method to save the column sizes
   //! @param aSettings is the preferences to which to save
   void SaveColumnSizesP(QSettings& aSettings) const noexcept;

   //@}

   //! Save the preferences to the given settings
   //! @par details
   //!   This writes the number of task tags recorded in the PrefObject and stores it in the QSettings.
   //!   Then, it writes in each task tag and stores it in the QSettings.
   //! @param aSettings is the given settings
   void SaveSettingsP(QSettings& aSettings) const override;

   //! Contains debugging information for a Task
   mutable QString mDebugLog;
};

} // end namespace TaskList

//! Declare the PrefData as a QMetaType
Q_DECLARE_METATYPE(TaskList::PrefData)

#endif // TASK_LIST_PREF_OBJECT_HPP
