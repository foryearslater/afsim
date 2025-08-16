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

#ifndef OSMCONVERTERDIALOG_HPP
#define OSMCONVERTERDIALOG_HPP

#include <OSMConverterDataContainer.hpp>
#include <OSMConverterTable.hpp>
#include <QDialog>

#include "ui_OSMConverterDialog.h"

class OSMConverterDialog : public QDialog
{
   Q_OBJECT
public:
   OSMConverterDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~OSMConverterDialog() override = default;

protected:
   void closeEvent(QCloseEvent* event) override;

private:
   void HandleExportButtonClicked();
   void HandlePathChanged();

   Ui::OSMConverterDialog     mUI;
   OSMConverterDataContainer* mDataContainer;
   OSMConverterTable*         mWayTagTable;
   OSMConverterTable*         mNodeTagTable;
   bool                       mUseDefaultDirectory;
};

#endif
