// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TABLEWIDGET_HPP
#define TABLEWIDGET_HPP

#include <QTableWidget>

namespace Engage
{
class Dialog;
class TableRow;

class TableWidget : public QTableWidget
{
public:
   TableWidget();
   explicit TableWidget(QWidget* aParentPtr);
   ~TableWidget() override = default;

   std::vector<TableRow*> GetRows() const { return mTableRows; }
   void                   AddRow(Dialog* aDialogPtr);
   Dialog*                GetDialog() { return mDialogPtr; }

private:
   std::vector<TableRow*> mTableRows;
   Dialog*                mDialogPtr;
};
} // namespace Engage

#endif
