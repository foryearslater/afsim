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

#include "TablePlotterDialog.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QVBoxLayout>

#include "AFSIM_Parser.hpp"
#include "CsvParser.hpp"
#include "PlotParser.hpp"
#include "UtInput.hpp"
#include "UtInputFile.hpp"

TablePlotterDialog::TablePlotterDialog(QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
   , mParser(nullptr)
{
   setAttribute(Qt::WA_DeleteOnClose);

   mUi.setupUi(this);

   mUi.minXLineEdit->setValidator(new QDoubleValidator());
   mUi.maxXLineEdit->setValidator(new QDoubleValidator());
   mUi.minYLineEdit->setValidator(new QDoubleValidator());
   mUi.maxYLineEdit->setValidator(new QDoubleValidator());

   connect(mUi.minXLineEdit,
           &QLineEdit::editingFinished,
           this,
           [this]()
           {
              mUi.plotWidget->GetPlot()->SetXAxisBounds(mUi.minXLineEdit->text().toDouble(),
                                                        mUi.maxXLineEdit->text().toDouble());
           });
   connect(mUi.maxXLineEdit,
           &QLineEdit::editingFinished,
           this,
           [this]()
           {
              mUi.plotWidget->GetPlot()->SetXAxisBounds(mUi.minXLineEdit->text().toDouble(),
                                                        mUi.maxXLineEdit->text().toDouble());
           });
   connect(mUi.minYLineEdit,
           &QLineEdit::editingFinished,
           this,
           [this]()
           {
              mUi.plotWidget->GetPlot()->SetYAxisBounds(mUi.minYLineEdit->text().toDouble(),
                                                        mUi.maxYLineEdit->text().toDouble());
           });
   connect(mUi.maxYLineEdit,
           &QLineEdit::editingFinished,
           this,
           [this]()
           {
              mUi.plotWidget->GetPlot()->SetYAxisBounds(mUi.minYLineEdit->text().toDouble(),
                                                        mUi.maxYLineEdit->text().toDouble());
           });

   connect(mUi.openPushButton, &QPushButton::pressed, this, &TablePlotterDialog::OpenFilePressed);
   connect(mUi.plotComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this,
           &TablePlotterDialog::PlotComboboxIndexChanged);

   setAcceptDrops(true);

   mUi.plotWidget->SetEnableDefaultContextMenu(true);
   mUi.plotWidget->EnableMouseZoom(true);
   mUi.plotComboBox->setVisible(false);

   UtQtGL2DPlot* plot = mUi.plotWidget->GetPlot();
   plot->SetDefaultPlotStyle(UtQtGL2DPlot::LINE);
   plot->SetUseMultipleColorsForSeries(true);
   plot->SetAxesBounds(0, 0, 10, 10);

   mUi.scrollAreaWidgetContents->setLayout(new QVBoxLayout());
   mShowAllCheckBox = new QCheckBox("Show All", mUi.scrollAreaWidgetContents);
   mShowAllCheckBox->setChecked(true);
   mUi.scrollAreaWidgetContents->layout()->addWidget(mShowAllCheckBox);
   connect(mShowAllCheckBox,
           &QCheckBox::clicked,
           this,
           [this](bool aInput)
           {
              for (auto i : mSeriesCheckBoxes)
              {
                 i->setChecked(aInput);
              }
              mShowAllCheckBox->setTristate(false);
              mShowAllCheckBox->setChecked(aInput);
           });
}

void TablePlotterDialog::dragEnterEvent(QDragEnterEvent* aEvent)
{
   aEvent->acceptProposedAction();
}

void TablePlotterDialog::dropEvent(QDropEvent* aEvent)
{
   if (aEvent->mimeData()->hasText())
   {
      // Strip the file:/// from the beginning. We want a path--not URL
      LoadFile(aEvent->mimeData()->text().remove("file:///"));
   }
   else
   {
      // Output message about filetype not supported.
   }
}

void TablePlotterDialog::SeriesCheckBoxChecked(bool aChecked)
{
   int index = sender()->property("index").toInt();
   if (index != 0)
   {
      mUi.plotWidget->GetPlot()->SetSeriesVisible(aChecked, index);
      mUi.plotWidget->Update();
      mShowAllCheckBox->setTristate(true);
      mShowAllCheckBox->setCheckState(Qt::PartiallyChecked);
   }
}

void TablePlotterDialog::Clear()
{
   mUi.plotComboBox->clear();
   mUi.plotWidget->GetPlot()->ClearData();
   mUi.plotWidget->Update();
}

void TablePlotterDialog::DisplayErrorMessages(const QStringList& aStrings)
{
   if (!aStrings.empty())
   {
      QString message;
      for (auto& i : aStrings)
      {
         message = message + i + "\n";
      }
      QMessageBox::warning(this, "Parse Results", message);
   }
}

void TablePlotterDialog::OpenFilePressed()
{
   QString filename = QFileDialog::getOpenFileName(this, "Open File", "", "*.* *.txt *.csv");
   if (filename != "")
   {
      LoadFile(filename);
   }
}

void TablePlotterDialog::PlotComboboxIndexChanged()
{
   if (mParser != nullptr)
   {
      mParser->PopulatePlot(mUi.plotComboBox->currentData().toInt(), mUi.plotWidget->GetPlot());
      mUi.plotWidget->GetPlot()->MakePlotPretty();

      PopulateSideBar();
   }
}

void TablePlotterDialog::PopulateComboBox()
{
   mUi.plotComboBox->clear();
   if (mParser != nullptr)
   {
      auto& idsNames = mParser->GetPlotIdsAndNames();
      for (auto& i : idsNames)
      {
         mUi.plotComboBox->addItem(i.second, i.first);
      }
      mUi.plotComboBox->setCurrentIndex(0);
   }
}

void TablePlotterDialog::PopulateSideBar()
{
   UtQtGL2DPlot* plot = mUi.plotWidget->GetPlot();
   double        xMin, xMax, yMin, yMax;
   plot->GetXAxisBounds(xMin, xMax);
   plot->GetYAxisBounds(yMin, yMax);
   mUi.minXLineEdit->setText(QString::number(xMin));
   mUi.maxXLineEdit->setText(QString::number(xMax));
   mUi.minYLineEdit->setText(QString::number(yMin));
   mUi.maxYLineEdit->setText(QString::number(yMax));

   for (auto i : mSeriesCheckBoxes)
   {
      delete i;
   }
   mSeriesCheckBoxes.clear();

   const UtQtGL2DPlot::SeriesMap& map = plot->GetData();
   for (auto& i : map)
   {
      QCheckBox* box = new QCheckBox(i.second->Name(), mUi.scrollAreaWidgetContents);
      box->setProperty("index", i.first);
      box->setChecked(true);
      connect(box, &QCheckBox::toggled, this, &TablePlotterDialog::SeriesCheckBoxChecked);
      mSeriesCheckBoxes.push_back(box);
      mUi.scrollAreaWidgetContents->layout()->addWidget(box);
   }
}

void TablePlotterDialog::LoadFile(const QString& aFilename)
{
   Clear();
   if (mParser != nullptr)
   {
      delete mParser;
   }

   // If CSV
   if (aFilename.contains(".csv"))
   {
      mParser = new CsvParser(aFilename);
   }
   else // Assume AFSIM file
   {
      mParser = new AFSIM_Parser(aFilename);
   }

   DisplayErrorMessages(mParser->GetErrors());

   PopulateComboBox();
   (mUi.plotComboBox->count() <= 1) ? (mUi.plotComboBox->setVisible(false)) : (mUi.plotComboBox->setVisible(true));
   if (mParser->GetErrors().isEmpty())
   {
      show();
   }
}

void TablePlotterDialog::LoadTextInput(const std::string& aInput)
{
   Clear();
   if (mParser != nullptr)
   {
      delete mParser;
   }
   mParser = new AFSIM_Parser(aInput);
   DisplayErrorMessages(mParser->GetErrors());

   PopulateComboBox();
   mUi.openPushButton->setVisible(false);
   if (mParser->GetErrors().isEmpty())
   {
      show();
   }
}