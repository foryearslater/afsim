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

#ifndef TASK_LIST_ITEM_HPP
#define TASK_LIST_ITEM_HPP

// Qt Includes
#include <QIcon>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>

// TaskList Includes
#include "TaskListPluginUtil.hpp"

namespace TaskList
{
//! Represents an item in the task table
class Item final : public QTableWidgetItem
{
public:
   //! The prototype
   //! @note The task table takes ownership of this Item
   static Item* cPROTOTYPE;

   //! @name Constructors from QTableWidgetItem
   //! @see https://doc.qt.io/qt-5/qtablewidgetitem.html
   //@{
   Item(int aType = QTableWidgetItem::Type);
   Item(const QString& aText, int aType = QTableWidgetItem::Type);
   Item(const QIcon& aIcon, const QString& aText, int aType = QTableWidgetItem::Type);
   //@}

   //! Construct an Item given the column index and the text
   //! @param aColumn is the column index
   //! @param aText is the text
   Item(ColumnIndex aColumn, const QString& aText);

   //! Specialized lesser-than operator for sorting Items in the task table
   //! @par details
   //!   If this Item represents the task description, then sort by ColumnIndex::Description, then by ColumnIndex::File,
   //!   and then by ColumnIndex::Line. If this Item represents the file in which the task resides, then sort by
   //!   ColumnIndex::File and then by ColumnIndex::Line. Otherwise, compare the Items normally.
   //! @param aRhs is the right-hand side of the lesser-than comparison
   bool operator<(const QTableWidgetItem& aRhs) const override;

private:
   //! The column in the table where the Item is
   ColumnIndex mColumnInTable;
};

} // end namespace TaskList

#endif // TASK_LIST_ITEM_HPP
