// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#ifndef PLOT_IT_MAIN_WINDOW_HPP
#define PLOT_IT_MAIN_WINDOW_HPP

#include <QCheckBox>
#include <QDialog>
#include <QDropEvent>
#include <QList>

#include "PlotParser.hpp"
#include "UtQtGL2DPlot.hpp"
#include "ui_TablePlotterDialog.h"

class TablePlotterDialog : public QDialog
{
   Q_OBJECT
public:
   TablePlotterDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~TablePlotterDialog() override = default;

   void LoadFile(const QString& aFilename);
   void LoadTextInput(const std::string& aInput);

protected:
   void dragEnterEvent(QDragEnterEvent* aEvent) override;
   void dropEvent(QDropEvent* aEvent) override;

   void SeriesCheckBoxChecked(bool aChecked);

   void Clear();
   void DisplayErrorMessages(const QStringList& aStrings);
   void OpenFilePressed();
   void PlotComboboxIndexChanged();
   void PopulateComboBox();
   void PopulateSideBar();

   PlotParser* mParser;

   QCheckBox*        mShowAllCheckBox;
   QList<QCheckBox*> mSeriesCheckBoxes;

   Ui::TablePlotterDialog mUi;
};
#endif
