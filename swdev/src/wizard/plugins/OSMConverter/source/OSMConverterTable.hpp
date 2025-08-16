// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef OSMCONVERTERTABLE_HPP
#define OSMCONVERTERTABLE_HPP

#include <vector>

#include <QDialog>

#include "OSMConverterDataContainer.hpp"
#include "ui_OSMConverterTable.h"

class OSMConverterTable : public QDialog
{
   Q_OBJECT
public:
   enum class TableType
   {
      cNODE,
      cWAY,
      cOTHER
   };

   OSMConverterTable(QWidget*        parent    = nullptr,
                     Qt::WindowFlags f         = Qt::WindowFlags(),
                     TableType       tableType = TableType::cOTHER);
   ~OSMConverterTable() override = default;

   const std::vector<OSMConverterDataContainer::FilterData>& GetTableData() const { return mTableData; }
   void SetTableData(const std::vector<OSMConverterDataContainer::FilterData>& aTableData);

protected:
   void showEvent(QShowEvent* event) override;

private:
   void InsertRow();
   void RemoveRow();
   void SaveTagData();
   void SetDefaultData();
   void PopulateTableData();

   // A vector is used to preserve any custom tag ordering the user may want:
   std::vector<OSMConverterDataContainer::FilterData> mTableData;

   TableType             mTableType;
   Ui::OSMConverterTable mUI;
};

#endif
