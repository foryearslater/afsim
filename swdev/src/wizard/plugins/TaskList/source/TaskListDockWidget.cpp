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
#include <QString>
#include <QStringList>
#include <QTime>

// Wizard Includes
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"

// Utility Includes
#include "UtQtMemory.hpp"

// WKF Includes
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

// WSF Includes
#include "WsfParseSourceInclude.hpp"
#include "WsfParser.hpp"

// TaskList Includes
#include "TaskListDockWidget.hpp"
#include "TaskListItem.hpp"

namespace TaskList
{
QString DockWidget::sDebugLog{};

DockWidget::DockWidget() noexcept
   : QDockWidget(wkfEnv.GetMainWindow(), Qt::WindowFlags{})
   , mPrefObject(wkfEnv.GetPreferenceObject<PrefObject>())
{
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }
   SetUpDockWidget();
   Connect();
}

void DockWidget::Populate(int aIndex) noexcept
{
   mComments.clear();
   // Clear the table by setting the row count to zero
   mUi.mTaskTable->setRowCount(0);
   // Then, rebuild the table
   auto enumIndex = static_cast<FilterIndex>(aIndex);
   switch (enumIndex)
   {
   case FilterIndex::AllScenarioFiles:
      return PopulateFromCurrentScenario();
   case FilterIndex::OpenScenarioFiles:
      return PopulateFromOpenDocuments();
   case FilterIndex::CurrentScenarioFile:
      return PopulateFromCurrentFile();
   default:
      return;
   }
}

void DockWidget::ChangeTableToolTip(int aIndex) noexcept
{
   QString tableToolTip{"List of tasks in "};
   switch (static_cast<FilterIndex>(aIndex))
   {
   case FilterIndex::AllScenarioFiles:
      tableToolTip += "the current scenario.";
      break;
   case FilterIndex::OpenScenarioFiles:
      tableToolTip += "all open files.";
      break;
   case FilterIndex::CurrentScenarioFile:
      tableToolTip += "the current file.";
      break;
   default:
      break;
   }
   mUi.mTaskTable->setToolTip(tableToolTip);
}

void DockWidget::OnFilterChanged(int aIndex) noexcept
{
   ChangeTableToolTip(aIndex);
   Populate(aIndex);
}

void DockWidget::OnCellDoubleClicked(int aRow, int /* aColumn */) noexcept
{
   const Task& t = mComments[aRow];
   if (!t.IsValid())
   {
      return;
   }

   UtPath      taskPath{t.mPath};
   std::string taskPathString{taskPath.GetNormalizedPath()};
   if (!taskPath.Exists())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  "
                  << QString::fromStdString(taskPathString) << "'s input file does not exist.";
      return;
   }
   wizard::Editor* editor{mWorkspace->GotoFile(taskPathString)};
   if (!editor)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  "
                  << QString::fromStdString(taskPathString) << "'s editor not found.";
      return;
   }

   if (!mPrefObject)
   {
      return;
   }
   int lineIndex{mPrefObject->GetColumnIndex("Line")};
   if (lineIndex == -1)
   {
      return;
   }
   QTableWidgetItem* lineItem{mUi.mTaskTable->item(aRow, static_cast<int>(ColumnIndex::Line))};
   if (!lineItem)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Failed to get the Line item.";
      return;
   }
   QString lineItemText{lineItem->text()};
   if (lineItemText.isEmpty())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Line item has no text.";
      return;
   }
   bool               ok;
   unsigned long long line{lineItemText.toULongLong(&ok)};
   if (!ok)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Conversion of line number from QString to unsigned long long failed.";
      return;
   }
   if (line == 0)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Line number equal to zero.";
      return;
   }
   // Change line back to zero-indexed for computational purposes
   --line;

   editor->GoToLine(line);
}

void DockWidget::OnColumnSectionMoved(int aLogicalIndex, int aOldVisualIndex, int aNewVisualIndex) noexcept
{
   QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};
   int          columnCount{taskTableColumnHeader->count()};

   // Collect available and visible column data
   QStringList availableColumns{QString("     ").split(" ")};
   QStringList visibleColumns{availableColumns};
   for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
   {
      QTableWidgetItem* columnHeaderItem{mUi.mTaskTable->horizontalHeaderItem(columnIndex)};
      if (!columnHeaderItem)
      {
         continue;
      }
      QString columnHeaderItemText{columnHeaderItem->text()};
      int     visualIndex{taskTableColumnHeader->visualIndex(columnIndex)};
      bool    columnHidden{taskTableColumnHeader->isSectionHidden(columnIndex)};
      if (columnHidden)
      {
         availableColumns[visualIndex] = columnHeaderItemText;
      }
      else
      {
         visibleColumns[visualIndex] = columnHeaderItemText;
      }
   }

   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }

   // Assign the resulting column data to the PrefObject
   mPrefObject->SetColumnData(availableColumns, visibleColumns);
}

void DockWidget::OnColumnSortIndicatorChanged(int aLogicalIndex, Qt::SortOrder aOrder) noexcept
{
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }

   int           sortByColumnIndex{mPrefObject->GetSortByColumnIndex()};
   Qt::SortOrder sortByColumnOrder{mPrefObject->GetSortByColumnOrder()};
   // Allow the reset of the column sorting.
   // The reset happens on clicking the column header when the column header sorts in descending order.
   if (sortByColumnIndex == aLogicalIndex && sortByColumnOrder == Qt::DescendingOrder && aOrder == Qt::AscendingOrder)
   {
      UnsortTaskTable();
      // The table is unsorted
      sortByColumnIndex = -1;
   }
   else
   {
      sortByColumnIndex = aLogicalIndex;
   }
   // The sort column will be different whether the table is sorted.
   // The sort order will be the same regardless.
   mPrefObject->SetSortByColumnIndex(sortByColumnIndex);
   mPrefObject->SetSortByColumnOrder(aOrder);
   // The mComments need sorting, regardless if this is a column-sorting reset
   SortTasks(static_cast<ColumnIndex>(aLogicalIndex), aOrder);
}

void DockWidget::OnColumnResized(int aLogicalIndex, int aOldSize, int aNewSize) noexcept
{
   auto         expectedColumnCount = ut::cast_to_int(mColumnSizes.size());
   QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};
   int          actualColumnCount{taskTableColumnHeader->count()};
   if (actualColumnCount != expectedColumnCount)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Column header count does not match expected column count."
                     "\n  Expected:  " +
                        QString::number(expectedColumnCount) + "\n  Actual:  " + QString::number(actualColumnCount);
      return;
   }
   mColumnSizes[aLogicalIndex] = aNewSize;
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }
   mPrefObject->SetColumnSizes(mColumnSizes);
}

void DockWidget::OnFullParseComplete(wizard::Project* aProjectPtr) noexcept
{
   UnsortTaskTable();
   // By this point, the PrefObject has read its settings, so restore the preferred column sizes
   RestoreColumnSizes();
   Populate(mUi.mFilterBox->currentIndex());
   ResortTaskTable();
   // Connect this now to allow DockWidget::OnColumnResized to execute from user interaction
   if (!mColumnResizedConnection)
   {
      QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};
      mColumnResizedConnection =
         connect(taskTableColumnHeader, &QHeaderView::sectionResized, this, &DockWidget::OnColumnResized);
   }
}

void DockWidget::OnActiveEditorChanged(wizard::Editor* aEditorPtr) noexcept
{
   // Avoid repopulating the table in AllScenarioFiles mode
   int index{mUi.mFilterBox->currentIndex()};
   if (index != static_cast<int>(FilterIndex::AllScenarioFiles))
   {
      UnsortTaskTable();
      Populate(index);
      ResortTaskTable();
   }
}

void DockWidget::OnTaskTagsUpdated() noexcept
{
   Populate(mUi.mFilterBox->currentIndex());
}

void DockWidget::OnVisibleColumnsUpdated() noexcept
{
   UnsortTaskTable();
   // Rebuild the task table header before rebuilding the task table.
   // Make sure to save the column sizes before rebuilding the task table header.
   // Make sure to restore the column sizes after rebuilding the task table header.
   SaveColumnSizes();
   SetUpTaskTableHeader();
   RestoreColumnSizes();
   Populate(mUi.mFilterBox->currentIndex());
   ResortTaskTable();
}

void DockWidget::OnPathFormatUpdated() noexcept
{
   UnsortTaskTable();
   Populate(mUi.mFilterBox->currentIndex());
   ResortTaskTable();
}

void DockWidget::SetUpTaskTableHeader() noexcept
{
   QTableWidget* taskTable{mUi.mTaskTable};
   // Clear the header by setting the column count to zero
   taskTable->setColumnCount(0);
   // Then, rebuild the header
   Qt::ItemFlags itemFlags{Qt::ItemIsSelectable | Qt::ItemIsEnabled};
   taskTable->setColumnCount(6);

   auto createHeaderItem = [itemFlags, taskTable](int aColumnIndex, const QString& aColumnName) {
      auto* columnHeaderItem = new QTableWidgetItem{aColumnName};
      columnHeaderItem->setFlags(itemFlags);
      taskTable->setHorizontalHeaderItem(aColumnIndex, columnHeaderItem);
   };
   createHeaderItem(0, "Tag");
   createHeaderItem(1, "Description");
   createHeaderItem(2, "Directory");
   createHeaderItem(3, "File");
   createHeaderItem(4, "Line");
   createHeaderItem(5, "Column");

   // Stretch the Description column to take up the remaining space
   taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
   mColumnOrderRestored = false;
}

void DockWidget::SetDefaultColumnSizes() noexcept
{
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }

   // By this point, the PrefObject has not read its settings, so it contains the default sizes
   mColumnSizes = mPrefObject->GetColumnSizes();
}

void DockWidget::SetUpTaskTable() noexcept
{
   mUi.mTaskTable->setItemPrototype(Item::cPROTOTYPE);

   SetUpTaskTableHeader();

   // Allow automatic sorting of the tasks by clicking the columns
   mUi.mTaskTable->setSortingEnabled(true);
   // Allow the user to move the columns
   mUi.mTaskTable->horizontalHeader()->setSectionsMovable(true);

   SetDefaultColumnSizes();
   ChangeTableToolTip(mUi.mFilterBox->currentIndex());
}

void DockWidget::SetUpDockWidget() noexcept
{
   mUi.setupUi(this);

   // Ported from wkf::DockWidget's constructor
   //{
   setObjectName("wizard::TaskList");
   setMinimumSize(64, 64);
   wkfEnv.GetMainWindow()->addDockWidget(Qt::BottomDockWidgetArea, this);
   hide();
   //}

   SetUpTaskTable();
}

void DockWidget::UnsortTaskTable() noexcept
{
   const QSignalBlocker cCOLUMN_BLOCKER{mUi.mTaskTable->horizontalHeader()};
   mUi.mTaskTable->sortItems(-1, Qt::AscendingOrder);
}

void DockWidget::ResortTaskTable() noexcept
{
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }
   int           columnIndex{mPrefObject->GetSortByColumnIndex()};
   Qt::SortOrder columnOrder{mPrefObject->GetSortByColumnOrder()};
   mUi.mTaskTable->sortItems(columnIndex, columnOrder);
}

void DockWidget::SortTasks(ColumnIndex aColumn, Qt::SortOrder aOrder) noexcept
{
   return std::sort(mComments.begin(), mComments.end(), Task::Compare{aColumn, aOrder});
}

void DockWidget::ConnectFilter() noexcept
{
   connect(mUi.mFilterBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DockWidget::OnFilterChanged);
}

void DockWidget::ConnectTaskTable() noexcept
{
   connect(mUi.mTaskTable, &QTableWidget::cellDoubleClicked, this, &DockWidget::OnCellDoubleClicked);
}

void DockWidget::ConnectTaskTableColumnHeader() noexcept
{
   QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};

   connect(taskTableColumnHeader, &QHeaderView::sectionMoved, this, &DockWidget::OnColumnSectionMoved);
   connect(taskTableColumnHeader, &QHeaderView::sortIndicatorChanged, this, &DockWidget::OnColumnSortIndicatorChanged);
   // Do not connect QHeaderView::sectionResized to DockWidget::OnColumnResized here.
   // This connection happens in DockWidget::OnFullParseComplete, as to avoid emissions of QHeaderView::sectionResized
   // during internal set-up of the QTableWidget.
}

void DockWidget::ConnectWizardCore() noexcept
{
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::FullParseComplete,
           this,
           &DockWidget::OnFullParseComplete);
   connect(wizSignals, &wizard::Signals::ActiveEditorChanged, this, &DockWidget::OnActiveEditorChanged);
}

void DockWidget::ConnectPrefObject() noexcept
{
   if (!mPrefObject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return;
   }
   connect(mPrefObject, &PrefObject::TaskTagsUpdated, this, &DockWidget::OnTaskTagsUpdated);
   connect(mPrefObject, &PrefObject::VisibleColumnsUpdated, this, &DockWidget::OnVisibleColumnsUpdated);
   connect(mPrefObject, &PrefObject::PathFormatUpdated, this, &DockWidget::OnPathFormatUpdated);
}

void DockWidget::Connect() noexcept
{
   ConnectFilter();
   ConnectPrefObject();
   ConnectTaskTable();
   ConnectTaskTableColumnHeader();
   ConnectWizardCore();
}

bool DockWidget::Initialize() noexcept
{
   mWorkspace = wizard::ProjectWorkspace::Instance();
   if (!mWorkspace)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  wizard::ProjectWorkspace is unavailable.";
      return false;
   }

   mEditorManager = mWorkspace->GetEditorManager();
   if (!mEditorManager)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  wizard::EditorManager is unavailable.";
      return false;
   }

   mSourceCache = mWorkspace->GetSourceCache();
   if (!mSourceCache)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  wizard::TextSourceCache is unavailable.";
      return false;
   }

   mProject = mWorkspace->GetProject();
   if (!mProject)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  wizard::Project is unavailable.";
      return false;
   }

   mStartupFiles = mProject->GetStartupFiles();
   if (mStartupFiles.empty())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  No start-up files available.";
      return false;
   }

   mParseResults = mProject->GetParseResults();
   if (!mParseResults)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  wizard::ParseResults is unavailable.";
      return false;
   }

   mParser = mParseResults->parser();
   if (!mParser)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  WsfParser is unavailable.";
      return false;
   }

   mParseIndex = mParser->GetParseIndex();
   if (!mParseIndex)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  WsfParseIndex is unavailable.";
      return false;
   }

   return true;
}

void DockWidget::AddComment(int            aRow,
                            const QString& aTag,
                            const QString& aDescription,
                            const QString& aDirectory,
                            const QString& aFile,
                            const QString& aLine,
                            const QString& aColumn) noexcept
{
   const Qt::ItemFlags cDEFAULT_ITEM_FLAGS{Qt::ItemIsSelectable | Qt::ItemIsEnabled};

   QTableWidget* taskTable{mUi.mTaskTable};
   auto createCellItem = [cDEFAULT_ITEM_FLAGS, taskTable](int aRowIndex, int aColumnIndex, const QString& aText) {
      // TODO:  make a factory function for QTableWidgetItem
      auto* cellItem = new Item(static_cast<ColumnIndex>(aColumnIndex), aText);
      cellItem->setFlags(cDEFAULT_ITEM_FLAGS);
      taskTable->setItem(aRowIndex, aColumnIndex, cellItem);
   };
   createCellItem(aRow, 0, aTag);
   createCellItem(aRow, 1, aDescription);
   createCellItem(aRow, 2, aDirectory);
   createCellItem(aRow, 3, aFile);
   createCellItem(aRow, 4, aLine);
   createCellItem(aRow, 5, aColumn);
}

WsfParseSourceInclude* DockWidget::AcquireIncludeFile(const UtPath& aFile) noexcept
{
   wizard::TextSource* source{mSourceCache->FindSource(aFile)};
   if (!source)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Could not find wizard::TextSource associated with "
                  << QString::fromStdString(aFile.GetNormalizedPath()) << '.';
      return nullptr;
   }

   // TODO:  Currently, the Open Files filter will only pick up on files included by the start-up file(s).
   //        In the future, we may want to extend this to any open file regardless.
   WsfParseSourceInclude* sourceInclude{mParser->GetInclude(source)};
   if (!sourceInclude)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Could not find WsfParseSourceInclude associated with "
                  << QString::fromStdString(aFile.GetNormalizedPath()) << '.';
      return nullptr;
   }

   return sourceInclude;
}

void DockWidget::AcquireCommentsFromFile(const QString& aFile) noexcept
{
   UtPath                 filePath{aFile.toStdString()};
   WsfParseSourceInclude* sourceInclude{AcquireIncludeFile(filePath)};
   if (!sourceInclude)
   {
      // No warning log here, since AcquireIncludeFile will output the same message.
      return;
   }

   const auto& entries = sourceInclude->mEntries;
   if (entries.empty())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  No WsfParseSourceInclude::Entries for "
                  << aFile << '.';
      return;
   }

   if (!mPrefObject)
   {
      return;
   }

   Task::mPathFormat       = mPrefObject->GetPathFormat();
   Task::mWorkingDirectory = mParser->GetWorkingDirectory();
   for (const auto& elem : entries)
   {
      if (elem.mType == WsfParseSourceInclude::cLINE_COMMENT || elem.mType == WsfParseSourceInclude::cBLOCK_COMMENT)
      {
         UtTextDocumentRange range{sourceInclude->mSourcePtr, elem.mLocation};
         QStringList         taskTags{mPrefObject->GetTaskTags()};
         for (const auto& elem : taskTags)
         {
            auto t = Task::RangeToTask(range, elem);
            if (!t.IsValid())
            {
               continue;
            }
            mComments.emplace_back(t);
         }
      }
   }
}

void DockWidget::RestoreColumnOrder() noexcept
{
   const QSignalBlocker cCOLUMNS_BLOCKER{mUi.mTaskTable->horizontalHeader()};
   if (!mPrefObject)
   {
      return;
   }

   auto moveOrHideColumn = [this](int aLogicalIndex, const QString& aColumnName) {
      int newVisualIndex{mPrefObject->GetColumnIndex(aColumnName)};
      int oldVisualIndex{mUi.mTaskTable->horizontalHeader()->visualIndex(aLogicalIndex)};
      if (newVisualIndex == -1)
      {
         mUi.mTaskTable->horizontalHeader()->hideSection(aLogicalIndex);
      }
      else if (oldVisualIndex != newVisualIndex)
      {
         mUi.mTaskTable->horizontalHeader()->swapSections(oldVisualIndex, newVisualIndex);
      }
   };
   moveOrHideColumn(0, "Tag");
   moveOrHideColumn(1, "Description");
   moveOrHideColumn(2, "Directory");
   moveOrHideColumn(3, "File");
   moveOrHideColumn(4, "Line");
   moveOrHideColumn(5, "Column");
   // Make sure to stretch the Description column, if visible.
   // Otherwise, make sure to stretch the last visible column.
   if (mUi.mTaskTable->horizontalHeader()->isSectionHidden(static_cast<int>(ColumnIndex::Description)))
   {
      mUi.mTaskTable->horizontalHeader()->setStretchLastSection(true);
   }
   else
   {
      mUi.mTaskTable->horizontalHeader()->setStretchLastSection(false);
      mUi.mTaskTable->horizontalHeader()->setSectionResizeMode(static_cast<int>(ColumnIndex::Description),
                                                               QHeaderView::Stretch);
   }
   mColumnOrderRestored = true;
}

void DockWidget::SaveColumnSizes() noexcept
{
   if (mColumnSizesRestored)
   {
      auto         expectedColumnCount = ut::cast_to_int(mColumnSizes.size());
      QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};
      int          actualColumnCount{taskTableColumnHeader->count()};
      if (actualColumnCount != expectedColumnCount)
      {
         QDebug debugStream{&mDebugLog};
         debugStream << QTime::currentTime().toString()
                     << "WizTaskList"
                        "\n  Column header count does not match expected column count."
                        "\n  Expected:  " +
                           QString::number(expectedColumnCount) + "\n  Actual:  " + QString::number(actualColumnCount);
         return;
      }
      for (int columnIndex = 0; columnIndex < expectedColumnCount; ++columnIndex)
      {
         mColumnSizes[ut::cast_to_size_t(columnIndex)] = taskTableColumnHeader->sectionSize(columnIndex);
      }
      if (!mPrefObject)
      {
         QDebug debugStream{&mDebugLog};
         debugStream << QTime::currentTime().toString()
                     << "WizTaskList"
                        "\n  TaskList::PrefObject is unavailable.";
         return;
      }
      mPrefObject->SetColumnSizes(mColumnSizes);
      mColumnSizesRestored = false;
   }
}

void DockWidget::RestoreColumnSizes() noexcept
{
   if (!mColumnSizesRestored)
   {
      const QSignalBlocker cCOLUMNS_BLOCKER{mUi.mTaskTable->horizontalHeader()};
      if (!mPrefObject)
      {
         QDebug debugStream{&mDebugLog};
         debugStream << QTime::currentTime().toString()
                     << "WizTaskList"
                        "\n  TaskList::PrefObject is unavailable.";
         return;
      }
      mColumnSizes = mPrefObject->GetColumnSizes();
      QHeaderView* taskTableColumnHeader{mUi.mTaskTable->horizontalHeader()};
      auto         expectedColumnCount = ut::cast_to_int(mColumnSizes.size());
      int          actualColumnCount{taskTableColumnHeader->count()};
      if (actualColumnCount != expectedColumnCount)
      {
         QDebug debugStream{&mDebugLog};
         debugStream << QTime::currentTime().toString()
                     << "WizTaskList"
                        "\n  Column header count does not match expected column count."
                        "\n  Expected:  " +
                           QString::number(expectedColumnCount) + "\n  Actual:  " + QString::number(actualColumnCount);
         return;
      }
      for (int columnIndex = 0; columnIndex < expectedColumnCount; ++columnIndex)
      {
         taskTableColumnHeader->resizeSection(columnIndex, mColumnSizes[ut::cast_to_size_t(columnIndex)]);
      }
      mColumnSizesRestored = true;
   }
}

void DockWidget::RebuildTaskTable() noexcept
{
   int rowIndex{0};
   if (mColumnOrderRestored)
   {
      SaveColumnSizes();
      SetUpTaskTableHeader();
      RestoreColumnSizes();
   }
   for (const auto& elem : mComments)
   {
      // Change line and column to one-indexed for display purposes
      AddComment(rowIndex++,
                 elem.mTag,
                 elem.mDescription,
                 elem.mDirectory,
                 elem.mFile,
                 QString::number(elem.mLine + 1),
                 QString::number(elem.mColumn + 1));
   }
   RestoreColumnOrder();
}

void DockWidget::PopulateFromCurrentFile() noexcept
{
   if (!Initialize())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Initialization failed.";
      return;
   }

   wizard::Editor* editor{mEditorManager->GetCurrentEditor()};
   if (!editor)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  No current wizard::Editor.";
      return;
   }

   std::string file{editor->GetSource()->GetFilePath().GetNormalizedPath()};
   AcquireCommentsFromFile(QString::fromStdString(file));
   mUi.mTaskTable->setRowCount(ut::cast_to_int(mComments.size()));
   RebuildTaskTable();
}

void DockWidget::PopulateFromOpenDocuments() noexcept
{
   if (!Initialize())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Initialization failed.";
      return;
   }

   const auto& files = mEditorManager->GetEditorMap();
   for (const auto& elem : files)
   {
      AcquireCommentsFromFile(elem.first);
   }
   mUi.mTaskTable->setRowCount(ut::cast_to_int(mComments.size()));
   RebuildTaskTable();
}

void DockWidget::PopulateFromCurrentScenario() noexcept
{
   if (!Initialize())
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Initialization failed.";
      return;
   }

   const auto& includeCountMap = mParseIndex->GetIncludes();
   for (auto& elem : includeCountMap)
   {
      AcquireCommentsFromFile(QString::fromStdString(elem.first.Get()));
   }
   mUi.mTaskTable->setRowCount(ut::cast_to_int(mComments.size()));
   RebuildTaskTable();
}

} // end namespace TaskList
