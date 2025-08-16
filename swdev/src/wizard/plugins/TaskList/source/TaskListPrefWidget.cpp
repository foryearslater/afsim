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
#include <QSignalBlocker>

// Utility Includes
#include "UtQtText.hpp"

// TaskList Includes
#include "TaskListPrefWidget.hpp"

namespace TaskList
{
PrefWidget::PrefWidget(QWidget* aParent)
   : PrefWidgetT<PrefObject>(aParent)
{
   mUi.setupUi(this);
   Connect();
}

void PrefWidget::Connect()
{
   connect(mUi.mAddButton, &QPushButton::clicked, this, &PrefWidget::OnAddClicked);
   connect(mUi.mRemoveButton, &QPushButton::clicked, this, &PrefWidget::OnRemoveClicked);
   connect(mUi.mTaskTagLineEdit, &QLineEdit::textEdited, this, &PrefWidget::OnTaskTagEdited);
   connect(mUi.mDown, &QPushButton::clicked, this, &PrefWidget::OnDownClicked);
   connect(mUi.mLeft, &QPushButton::clicked, this, &PrefWidget::OnLeftClicked);
   connect(mUi.mRight, &QPushButton::clicked, this, &PrefWidget::OnRightClicked);
   connect(mUi.mUp, &QPushButton::clicked, this, &PrefWidget::OnUpClicked);
   connect(mPrefObjectPtr.get(), &PrefObject::ColumnDataUpdated, this, &PrefWidget::OnColumnDataUpdated);
}

void PrefWidget::OnAddClicked()
{
   QString tagText{mUi.mTaskTagLineEdit->text()};
   // Avoid adding empty or existing task tags
   if (!tagText.isEmpty() && !mTaskTagList.contains(tagText))
   {
      mUi.mTaskTagList->addItem(tagText);
      mUi.mTaskTagList->sortItems();
      // Keep the data in-sync with the GUI
      mTaskTagList.append(tagText);
      mTaskTagList.sort();
   }
}

void PrefWidget::OnRemoveClicked()
{
   int              tagIndex{mUi.mTaskTagList->currentRow()};
   QListWidgetItem* tagItem{mUi.mTaskTagList->item(tagIndex)};
   // Guard against no selected item in the list widget
   if (tagItem)
   {
      QString tagText{tagItem->text()};
      // Avoid deleting non-existing task tags
      if (tagItem && mTaskTagList.contains(tagText))
      {
         delete mUi.mTaskTagList->takeItem(tagIndex);
         // Keep the data in-sync with the GUI
         mTaskTagList.removeOne(tagText);
      }
   }
}

void PrefWidget::OnTaskTagEdited(const QString& aTag)
{
   if (mTaskTagList.contains(aTag, Qt::CaseInsensitive))
   {
      // The task tag already exists in the list.
      // Change the line edit's text color to red, indicating a duplicate task tag.
      UtQtText::SetValid(false, *mUi.mTaskTagLineEdit);
      mUi.mAddButton->setDisabled(true);
   }
   else
   {
      // The task tag does not exist in the list.
      // Change the line edit's text color to white, indicating a unique task tag.
      UtQtText::SetValid(true, *mUi.mTaskTagLineEdit);
      mUi.mAddButton->setEnabled(true);
   }
}

void PrefWidget::OnDownClicked()
{
   int              columnIndex{mUi.mVisibleColumns->currentRow()};
   QListWidgetItem* currentColumn{mUi.mVisibleColumns->item(columnIndex)};
   if (currentColumn)
   {
      int columnCount{mUi.mVisibleColumns->count()};
      if (columnIndex + 1 < columnCount)
      {
         currentColumn = mUi.mVisibleColumns->takeItem(columnIndex);
         mUi.mVisibleColumns->insertItem(columnIndex + 1, currentColumn);
         mUi.mVisibleColumns->setCurrentRow(columnIndex + 1);
         // Keep the data in-sync with the GUI
         mVisibleColumnsList.move(columnIndex, columnIndex + 1);
      }
   }
}

void PrefWidget::OnLeftClicked()
{
   int                     columnIndex{mUi.mVisibleColumns->currentRow()};
   QList<QListWidgetItem*> selectedColumns{mUi.mVisibleColumns->selectedItems()};
   if (selectedColumns.empty())
   {
      return;
   }
   QListWidgetItem* selectedColumn{selectedColumns.front()};
   QListWidgetItem* currentColumn{mUi.mVisibleColumns->item(columnIndex)};
   if (selectedColumn && currentColumn && selectedColumn == currentColumn)
   {
      QString columnText{currentColumn->text()};
      if (!columnText.isEmpty() && !mAvailableColumnsList.contains(columnText))
      {
         currentColumn = mUi.mVisibleColumns->takeItem(columnIndex);
         mUi.mAvailableColumns->addItem(currentColumn);
         // Make sure only the item added to the available columns remains selected
         int availableColumnCount{mUi.mAvailableColumns->count()};
         mUi.mAvailableColumns->setCurrentRow(availableColumnCount - 1, QItemSelectionModel::ClearAndSelect);
         mUi.mVisibleColumns->setCurrentRow(columnIndex, QItemSelectionModel::Clear);
         // Keep the data in-sync with the GUI
         mVisibleColumnsList.removeAt(columnIndex);
         mAvailableColumnsList.append(columnText);
         // mAvailableColumnsList.sort();
      }
   }
}

void PrefWidget::OnRightClicked()
{
   int                     columnIndex{mUi.mAvailableColumns->currentRow()};
   QList<QListWidgetItem*> selectedColumns{mUi.mAvailableColumns->selectedItems()};
   if (selectedColumns.empty())
   {
      return;
   }
   QListWidgetItem* selectedColumn{selectedColumns.front()};
   QListWidgetItem* currentColumn{mUi.mAvailableColumns->item(columnIndex)};
   if (selectedColumn && currentColumn && selectedColumn == currentColumn)
   {
      QString columnText{currentColumn->text()};
      if (!columnText.isEmpty() && !mVisibleColumnsList.contains(columnText))
      {
         currentColumn = mUi.mAvailableColumns->takeItem(columnIndex);
         mUi.mVisibleColumns->addItem(currentColumn);
         // Make sure only the item added to the visible columns remains selected
         int visibleColumnCount{mUi.mVisibleColumns->count()};
         mUi.mVisibleColumns->setCurrentRow(visibleColumnCount - 1, QItemSelectionModel::ClearAndSelect);
         mUi.mAvailableColumns->setCurrentRow(columnIndex, QItemSelectionModel::Clear);
         // Keep the data in-sync with the GUI
         mAvailableColumnsList.removeAt(columnIndex);
         mVisibleColumnsList.append(columnText);
      }
   }
}

void PrefWidget::OnUpClicked()
{
   int              columnIndex{mUi.mVisibleColumns->currentRow()};
   QListWidgetItem* currentColumn{mUi.mVisibleColumns->item(columnIndex)};
   if (currentColumn && (columnIndex - 1 >= 0))
   {
      currentColumn = mUi.mVisibleColumns->takeItem(columnIndex);
      mUi.mVisibleColumns->insertItem(columnIndex - 1, currentColumn);
      mUi.mVisibleColumns->setCurrentRow(columnIndex - 1);
      // Keep the data in-sync with the GUI
      mVisibleColumnsList.move(columnIndex - 1, columnIndex);
   }
}

void PrefWidget::OnColumnDataUpdated()
{
   QStringList availableColumns{mPrefObjectPtr->GetAvailableColumns()};
   availableColumns.removeAll("");
   mUi.mAvailableColumns->clear();
   mUi.mAvailableColumns->addItems(availableColumns);
   QStringList visibleColumns{mPrefObjectPtr->GetVisibleColumns()};
   visibleColumns.removeAll("");
   mUi.mVisibleColumns->clear();
   mUi.mVisibleColumns->addItems(visibleColumns);
   // Keep the data in-sync with the GUI
   mAvailableColumnsList = availableColumns;
   mVisibleColumnsList   = visibleColumns;
}

void PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.mTaskTagList->clear();
   mUi.mTaskTagList->addItems(aPrefData.mTaskTags);
   mUi.mAvailableColumns->clear();
   mUi.mAvailableColumns->addItems(aPrefData.mAvailableColumns);
   mUi.mVisibleColumns->clear();
   mUi.mVisibleColumns->addItems(aPrefData.mVisibleColumns);
   switch (aPrefData.mPathFormat)
   {
   case PathFormat::Absolute:
      mUi.mAbsolutePathFormat->setChecked(true);
      break;
   case PathFormat::Relative:
      mUi.mRelativePathFormat->setChecked(true);
      break;
   default:
      break;
   }
   // Keep the data in-sync with the GUI
   mTaskTagList          = aPrefData.mTaskTags;
   mAvailableColumnsList = aPrefData.mAvailableColumns;
   mVisibleColumnsList   = aPrefData.mVisibleColumns;
}

void PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mTaskTags.clear();
   aPrefData.mTaskTags << mTaskTagList;
   WriteColumnDataP(aPrefData);
   aPrefData.mAvailableColumns.clear();
   aPrefData.mAvailableColumns = mAvailableColumnsList;
   aPrefData.mVisibleColumns.clear();
   aPrefData.mVisibleColumns = mVisibleColumnsList;
   if (mUi.mAbsolutePathFormat->isChecked())
   {
      aPrefData.mPathFormat = PathFormat::Absolute;
   }
   else if (mUi.mRelativePathFormat->isChecked())
   {
      aPrefData.mPathFormat = PathFormat::Relative;
   }
}

void PrefWidget::WriteColumnDataP(PrefData& aPrefData)
{
   // Organize the columns in visual order
   QStringList columnsInVisualOrder{"Tag", "Description", "Directory", "File", "Line", "Column"};
   for (int newVisualIndex = 0; newVisualIndex < mVisibleColumnsList.size(); ++newVisualIndex)
   {
      QString columnName{mVisibleColumnsList[newVisualIndex]};
      int     oldVisualIndex{columnsInVisualOrder.indexOf(columnName)};
      columnsInVisualOrder.move(oldVisualIndex, newVisualIndex);
   }

   // Populate available and visible column lists.
   // These need to be the same size of exactly 6 elements.
   QStringList availableColumns{QString("     ").split(" ")};
   QStringList visibleColumns;
   for (auto& elem : columnsInVisualOrder)
   {
      if (mAvailableColumnsList.contains(elem))
      {
         availableColumns += elem;
         visibleColumns += "";
      }
      else
      {
         availableColumns += "";
         visibleColumns += elem;
      }
   }

   // Set the PrefObject's column data.
   // Make sure to block the signal that notifies the PrefWidget of a column data change.
   const QSignalBlocker cPREF_OBJECT_BLOCK{mPrefObjectPtr.get()};
   mPrefObjectPtr->SetColumnData(availableColumns, visibleColumns);
}

} // end namespace TaskList
