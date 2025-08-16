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

// TaskList Includes
#include "TaskListItem.hpp"

#include "TaskListTask.hpp"

namespace TaskList
{
Item* Item::cPROTOTYPE{new Item};

Item::Item(int aType /* = QTableWidgetItem::Type */)
   : QTableWidgetItem(aType)
{
}

Item::Item(const QString& aText, int aType /* = QTableWidgetItem::Type */)
   : QTableWidgetItem(aText, aType)
{
}

Item::Item(const QIcon& aIcon, const QString& aText, int aType /* = QTableWidgetItem::Type */)
   : QTableWidgetItem(aIcon, aText, aType)
{
}

Item::Item(ColumnIndex aColumn, const QString& aText)
   : QTableWidgetItem(aText)
   , mColumnInTable(aColumn)
{
}

bool Item::operator<(const QTableWidgetItem& aRhs) const
{
   QTableWidget* taskTable{tableWidget()};
   auto          lhsTask = Task::RowToTask(taskTable, row());
   auto          rhsTask = Task::RowToTask(taskTable, aRhs.row());
   return Task::Compare{mColumnInTable, Qt::AscendingOrder}(lhsTask, rhsTask);
}

} // end namespace TaskList
