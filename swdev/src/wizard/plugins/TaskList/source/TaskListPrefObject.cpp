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

// Qt Includes
#include <QDebug>
#include <QMessageBox>
#include <QTime>

// TaskList Includes
#include "TaskListPrefObject.hpp"

namespace TaskList
{
const QStringList PrefData::cDEFAULT_TAGS{"ATTENTION", "BUG", "DEBUG", "HACK", "TASK", "TODO", "UNDONE"};

const QStringList PrefData::cDEFAULT_AVAILABLE_TAGS{"Directory", "Column"};

const QStringList PrefData::cDEFAULT_VISIBLE_TAGS{"File", "Line", "Tag", "Description"};

QString PrefObject::sDebugLog{};

PrefObject::PrefObject(QObject* aParent)
   : wkf::PrefObjectT<PrefData>(aParent, cNAME)
{
}

QStringList PrefObject::GetTaskTags() const noexcept
{
   return mCurrentPrefs.mTaskTags;
}

QString PrefObject::GetTaskTagName(int aIndex) const noexcept
{
   return mCurrentPrefs.mTaskTags[aIndex];
}

int PrefObject::GetTaskTagIndex(const QString& aName) const noexcept
{
   return mCurrentPrefs.mTaskTags.indexOf(aName);
}

QStringList PrefObject::GetAvailableColumns() const noexcept
{
   return mCurrentPrefs.mAvailableColumns;
}

QStringList PrefObject::GetVisibleColumns() const noexcept
{
   return mCurrentPrefs.mVisibleColumns;
}

QString PrefObject::GetColumnName(int aIndex) const noexcept
{
   return mCurrentPrefs.mVisibleColumns[aIndex];
}

int PrefObject::GetColumnIndex(const QString& aName) const noexcept
{
   return mCurrentPrefs.mVisibleColumns.indexOf(aName);
}

void PrefObject::SetColumnData(const QStringList& aAvailableColumns, const QStringList& aVisibleColumns) noexcept
{
   mCurrentPrefs.mAvailableColumns = aAvailableColumns;
   mCurrentPrefs.mVisibleColumns   = aVisibleColumns;
   emit ColumnDataUpdated();
}

PathFormat PrefObject::GetPathFormat() const noexcept
{
   return mCurrentPrefs.mPathFormat;
}

int PrefObject::GetSortByColumnIndex() const noexcept
{
   return mCurrentPrefs.mSortByColumnIndex;
}

Qt::SortOrder PrefObject::GetSortByColumnOrder() const noexcept
{
   return mCurrentPrefs.mSortByColumnOrder;
}

void PrefObject::SetSortByColumnIndex(int aIndex) noexcept
{
   mCurrentPrefs.mSortByColumnIndex = aIndex;
}

void PrefObject::SetSortByColumnOrder(Qt::SortOrder aOrder) noexcept
{
   mCurrentPrefs.mSortByColumnOrder = aOrder;
}

void PrefObject::SetColumnSizes(const std::array<int, 6>& aColumnSizes) noexcept
{
   mCurrentPrefs.mColumnSizes = aColumnSizes;
}

const std::array<int, 6>& PrefObject::GetColumnSizes() const noexcept
{
   return mCurrentPrefs.mColumnSizes;
}

bool PrefObject::AddTaskTag(const QString& aTag) noexcept
{
   // Avoid adding existing task tags
   if (mCurrentPrefs.mTaskTags.indexOf(aTag) == -1)
   {
      mCurrentPrefs.mTaskTags.append(aTag);
      return true;
   }
   return false;
}

bool PrefObject::RemoveTaskTag(const QString& aTag) noexcept
{
   int tagIndex{mCurrentPrefs.mTaskTags.indexOf(aTag)};
   // Avoid removing non-existing task tags
   if (tagIndex != -1)
   {
      mCurrentPrefs.mTaskTags.removeAt(tagIndex);
      return true;
   }
   return false;
}

void PrefObject::Apply()
{
   // Only emit when the user ACTUALLY changed the task tags
   if (mCurrentPrefs.mTaskTagsUpdated)
   {
      emit TaskTagsUpdated();
      mCurrentPrefs.mTaskTagsUpdated = false;
   }
   // Only emit when the user ACTUALLY changed the visible columns
   if (mCurrentPrefs.mVisibleColumnsUpdated)
   {
      emit VisibleColumnsUpdated();
      mCurrentPrefs.mVisibleColumnsUpdated = false;
   }
   // Only emit when the user ACTUALLY changed the path format
   if (mCurrentPrefs.mPathFormatUpdated)
   {
      emit PathFormatUpdated();
      mCurrentPrefs.mPathFormatUpdated = false;
   }
}

void PrefObject::SetPreferenceDataP(const PrefData& aPrefData)
{
   // Determine whether the user ACTUALLY changed the task tags
   mCurrentPrefs.mTaskTagsUpdated       = mCurrentPrefs.mTaskTags != aPrefData.mTaskTags;
   mCurrentPrefs.mVisibleColumnsUpdated = mCurrentPrefs.mVisibleColumns != aPrefData.mVisibleColumns;
   mCurrentPrefs.mPathFormatUpdated     = mCurrentPrefs.mPathFormat != aPrefData.mPathFormat;

   // Set the current preference data
   mCurrentPrefs.mTaskTags          = aPrefData.mTaskTags;
   mCurrentPrefs.mAvailableColumns  = aPrefData.mAvailableColumns;
   mCurrentPrefs.mVisibleColumns    = aPrefData.mVisibleColumns;
   mCurrentPrefs.mPathFormat        = aPrefData.mPathFormat;
   mCurrentPrefs.mSortByColumnIndex = aPrefData.mSortByColumnIndex;
   mCurrentPrefs.mSortByColumnOrder = aPrefData.mSortByColumnOrder;
   mCurrentPrefs.mColumnSizes       = aPrefData.mColumnSizes;
}

void PrefObject::ReadTaskTagsP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("taskTags/size"))
   {
      aData.mTaskTags.clear();
      // Read the number of task tags
      int tagCount{aSettings.beginReadArray("tags")};
      for (int tagIndex = 0; tagIndex < tagCount; ++tagIndex)
      {
         // Read each task tag
         aSettings.setArrayIndex(tagIndex);
         QString tag{aSettings.value("name").toString()};
         // Avoid adding empty or existing task tags
         if (!tag.isEmpty() && !aData.mTaskTags.contains(tag))
         {
            aData.mTaskTags << tag;
         }
      }
      aSettings.endArray();
      aData.mTaskTags.sort();
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read taskTags.";
   }
}

void PrefObject::ReadAvailableColumnsP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("availableColumns/size"))
   {
      aData.mAvailableColumns.clear();
      // aData.mAvailableColumns.reserve(6);
      // Read the number of columns
      int columnCount{aSettings.beginReadArray("availableColumns")};
      for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
      {
         // Read each column
         aSettings.setArrayIndex(columnIndex);
         QString columnName{aSettings.value("name").toString()};
         // Avoid adding empty or existing columns
         if (!columnName.isEmpty() && !aData.mAvailableColumns.contains(columnName))
         {
            aData.mAvailableColumns << columnName;
         }
         // int columnVisualIndex{ aSettings.value("visualIndex").toInt() };
         // aData.mAvailableColumns[columnVisualIndex] = columnName;
      }
      aSettings.endArray();
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read availableColumns.";
   }
}

void PrefObject::ReadVisibleColumnsP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("visibleColumns/size"))
   {
      aData.mVisibleColumns.clear();
      // aData.mVisibleColumns.reserve(6);
      // Read the number of columns
      int columnCount{aSettings.beginReadArray("visibleColumns")};
      for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
      {
         // Read each column
         aSettings.setArrayIndex(columnIndex);
         QString columnName{aSettings.value("name").toString()};
         // Avoid adding empty or existing columns
         if (!columnName.isEmpty() && !aData.mVisibleColumns.contains(columnName))
         {
            aData.mVisibleColumns << columnName;
         }
         // int columnVisualIndex{ aSettings.value("visualIndex").toInt() };
         // aData.mVisibleColumns[columnVisualIndex] = columnName;
      }
      aSettings.endArray();
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read visibleColumns.";
   }
}

void PrefObject::ReadPathFormatP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("pathFormat"))
   {
      QVariant pathFormatValue{aSettings.value("pathFormat")};
      int      pathFormatInt{pathFormatValue.toInt()};
      aData.mPathFormat = static_cast<PathFormat>(pathFormatInt);
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read pathFormat.";
   }
}

void PrefObject::ReadSortByColumnP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("sortByColumnIndex"))
   {
      QVariant sortByColumnIndexValue{aSettings.value("sortByColumnIndex")};
      aData.mSortByColumnIndex = sortByColumnIndexValue.toInt();
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read sortByColumnIndex.";
   }
   if (aSettings.contains("sortByColumnOrder"))
   {
      QVariant sortByColumnOrderValue{aSettings.value("sortByColumnOrder")};
      int      sortByColumnOrderInt{sortByColumnOrderValue.toInt()};
      aData.mSortByColumnOrder = static_cast<Qt::SortOrder>(sortByColumnOrderInt);
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read sortByColumnOrder.";
   }
}

void PrefObject::ReadColumnSizesP(QSettings& aSettings, PrefData& aData) const noexcept
{
   if (aSettings.contains("columnSizes/size"))
   {
      // Read the number of columns
      int columnCount{aSettings.beginReadArray("columnSizes")};
      for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
      {
         // Read each column size
         aSettings.setArrayIndex(columnIndex);
         QVariant columnSize{aSettings.value("size")};
         int      columnSizeInt{columnSize.toInt()};
         aData.mColumnSizes[columnIndex] = columnSizeInt;
      }
      aSettings.endArray();
   }
   else
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to read columnSizes.";
   }
}

PrefData PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData data;
   ReadTaskTagsP(aSettings, data);
   ReadAvailableColumnsP(aSettings, data);
   ReadVisibleColumnsP(aSettings, data);
   ReadPathFormatP(aSettings, data);
   ReadSortByColumnP(aSettings, data);
   ReadColumnSizesP(aSettings, data);
   return data;
}

void PrefObject::SaveTaskTagsP(QSettings& aSettings) const noexcept
{
   QStringList taskTags{mCurrentPrefs.mTaskTags};
   int         tagCount{taskTags.size()};
   // Write the number of task tags
   aSettings.beginWriteArray("taskTags");
   for (int tagIndex = 0; tagIndex < tagCount; ++tagIndex)
   {
      QString elem{taskTags[tagIndex]};
      // Avoid saving empty task tags
      if (!elem.isEmpty())
      {
         // Write each task tag
         aSettings.setArrayIndex(tagIndex);
         aSettings.setValue("name", elem);
      }
   }
   aSettings.endArray();
}

void PrefObject::SaveAvailableColumnsP(QSettings& aSettings) const noexcept
{
   QStringList availableColumns{mCurrentPrefs.mAvailableColumns};
   int         columnCount{availableColumns.size()};
   // Write the number of columns
   aSettings.beginWriteArray("availableColumns");
   for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
   {
      QString elem{availableColumns[columnIndex]};
      // Avoid saving empty columns
      if (!elem.isEmpty())
      {
         // Write each column
         aSettings.setArrayIndex(columnIndex);
         aSettings.setValue("name", elem);
      }
   }
   aSettings.endArray();
}

void PrefObject::SaveVisibleColumnsP(QSettings& aSettings) const noexcept
{
   QStringList visibleColumns{mCurrentPrefs.mVisibleColumns};
   int         columnCount{visibleColumns.size()};
   // Write the number of columns
   aSettings.beginWriteArray("visibleColumns");
   for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
   {
      QString elem{visibleColumns[columnIndex]};
      // Avoid saving empty columns
      if (!elem.isEmpty())
      {
         // Write each column
         aSettings.setArrayIndex(columnIndex);
         aSettings.setValue("name", elem);
      }
   }
   aSettings.endArray();
}

void PrefObject::SavePathFormatP(QSettings& aSettings) const noexcept
{
   PathFormat pathFormat{mCurrentPrefs.mPathFormat};
   auto       pathFormatInt = static_cast<int>(pathFormat);
   aSettings.setValue("pathFormat", pathFormatInt);
}

void PrefObject::SaveSortByColumnP(QSettings& aSettings) const noexcept
{
   int sortByColumnIndex{mCurrentPrefs.mSortByColumnIndex};
   aSettings.setValue("sortByColumnIndex", sortByColumnIndex);
   Qt::SortOrder sortByColumnOrder{mCurrentPrefs.mSortByColumnOrder};
   aSettings.setValue("sortByColumnOrder", sortByColumnOrder);
}

void PrefObject::SaveColumnSizesP(QSettings& aSettings) const noexcept
{
   const std::array<int, 6>& columnSizes = mCurrentPrefs.mColumnSizes;
   // Write the number of columns
   aSettings.beginWriteArray("columnSizes");
   int columnIndex = 0;
   for (int columnSize : columnSizes)
   {
      // Write each column size
      aSettings.setArrayIndex(columnIndex++);
      aSettings.setValue("size", columnSize);
   }
   aSettings.endArray();
}

void PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   SaveTaskTagsP(aSettings);
   SaveAvailableColumnsP(aSettings);
   SaveVisibleColumnsP(aSettings);
   SavePathFormatP(aSettings);
   SaveSortByColumnP(aSettings);
   SaveColumnSizesP(aSettings);
}

} // end namespace TaskList
