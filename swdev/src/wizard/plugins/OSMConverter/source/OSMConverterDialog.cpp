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

#include "OSMConverterDialog.hpp"

#include <QTableWidget>
#include <QTextStream>

#include "WkfEnvironment.hpp"

OSMConverterDialog::OSMConverterDialog(QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
   , mDataContainer(new OSMConverterDataContainer())
   , mWayTagTable(new OSMConverterTable(this, Qt::WindowFlags(), OSMConverterTable::TableType::cWAY))
   , mNodeTagTable(new OSMConverterTable(this, Qt::WindowFlags(), OSMConverterTable::TableType::cNODE))
   , mUseDefaultDirectory(true)
{
   mUI.setupUi(this);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   // Handle way tag dialog button
   mWayTagTable->setWindowTitle("Way Tag Auxiliary Data Filter");
   connect(mUI.mWayTagButton, &QPushButton::clicked, [this]() { mWayTagTable->show(); });

   // Handle node tag dialog button
   mNodeTagTable->setWindowTitle("Node Tag Auxiliary Data Filter");
   connect(mUI.mNodeTagButton, &QPushButton::clicked, [this]() { mNodeTagTable->show(); });

   // Hande line edits for filenames
   connect(mUI.mTargetLineEdit, &OSMConverterLineEdit::DroppedFile, this, &OSMConverterDialog::HandlePathChanged);
   connect(mUI.mTargetLineEdit, &QLineEdit::editingFinished, this, &OSMConverterDialog::HandlePathChanged);

   // Handle export button
   connect(mUI.mExportButton, &QPushButton::clicked, this, &OSMConverterDialog::HandleExportButtonClicked);

   // Handle file path selection
   connect(mUI.mUploadButton,
           &QPushButton::clicked,
           [this]()
           {
              QString filePath = wkf::getOpenFileName(this, "Choose file", "", "*.osm");
              if (!filePath.isEmpty())
              {
                 mUI.mUploadLineEdit->setText(filePath);
              }
           });

   connect(mUI.mTargetButton,
           &QPushButton::clicked,
           [this]()
           {
              QString filePath = wkf::getSaveFileName(this, "Choose file", "");
              if (!filePath.isEmpty())
              {
                 mUI.mTargetLineEdit->setText(filePath);
              }
              HandlePathChanged();
           });

   // Autofill Output Handler
   connect(mUI.mUploadLineEdit,
           &QLineEdit::textChanged,
           [this]()
           {
              QString   inputFilePath = mUI.mUploadLineEdit->text();
              QFileInfo inputFileInfo(inputFilePath);
              // only autofill the output when the input is an osm file
              if (inputFilePath.right(4) == ".osm" && inputFileInfo.isFile())
              {
                 mUI.mUploadLineEdit->setStyleSheet(styleSheet());
                 QString   outputFilePath = mUI.mTargetLineEdit->text();
                 QFileInfo outputFileInfo(outputFilePath);
                 QDir      inputFileDir  = inputFileInfo.absoluteDir();
                 QDir      outputFileDir = outputFileInfo.absoluteDir();
                 if (mUseDefaultDirectory || outputFilePath.isEmpty() ||
                     (inputFileDir.canonicalPath() == outputFileDir.canonicalPath()))
                 {
                    mUI.mTargetLineEdit->setText(inputFileDir.absolutePath() + "/" + inputFileInfo.baseName() + ".txt");
                    mUseDefaultDirectory = true;
                 }
                 else // if (inputFileDir.canonicalPath() != outputFileDir.canonicalPath())
                 {
                    mUI.mTargetLineEdit->setText(outputFileDir.absolutePath() + "/" + inputFileInfo.baseName() + ".txt");
                 }
                 mUI.mNameLineEdit->setText(inputFileInfo.baseName().simplified().replace(" ", "_"));
              }
              else
              {
                 mUI.mUploadLineEdit->setStyleSheet("color: red;");
              }
           });
}

void OSMConverterDialog::closeEvent(QCloseEvent* event)
{
   QDialog::closeEvent(event);
   mWayTagTable->hide();
   mNodeTagTable->hide();
}

void OSMConverterDialog::HandlePathChanged()
{
   QString   inputFilePath  = mUI.mUploadLineEdit->text();
   QString   outputFilePath = mUI.mTargetLineEdit->text();
   QFileInfo inputFileInfo(inputFilePath);
   QFileInfo outputFileInfo(outputFilePath);
   mUseDefaultDirectory = false;
   if ((inputFileInfo.canonicalPath() == outputFileInfo.canonicalPath()) &&
       (inputFileInfo.baseName() == outputFileInfo.baseName()))
   {
      mUseDefaultDirectory = true;
   }
}

void OSMConverterDialog::HandleExportButtonClicked()
{
   QString importFile   = mUI.mUploadLineEdit->text();
   QString exportTarget = mUI.mTargetLineEdit->text();


   bool      writeToFile = true;
   QFileInfo checkFile(exportTarget);
   if (checkFile.exists() && checkFile.isFile())
   {
      QMessageBox::StandardButton response =
         QMessageBox::question(this,
                               "File Export Warning",
                               "The file already exists. Are you sure you want to overwrite it?",
                               QMessageBox::Yes | QMessageBox::No);
      if (response != QMessageBox::Yes)
      {
         writeToFile = false;
      }
   }

   if (writeToFile && (importFile.right(4) == ".osm") && !exportTarget.isEmpty())
   {
      QString parseErrorMsg = mDataContainer->ParseFile(importFile);
      if (parseErrorMsg.isEmpty())
      {
         QString                                 routeErrorMessage;
         OSMConverterDataContainer::ResultOutput results;
         QFileInfo                               importFileInfo(importFile);
         QString                                 text =
            "# File generated by Wizard OSM Converter Tool\n# Input file: " + importFileInfo.fileName() + "\n\n";
         text += mDataContainer->GetRouteNetwork(mNodeTagTable->GetTableData(),
                                                 mWayTagTable->GetTableData(),
                                                 mUI.mNameLineEdit->text(),
                                                 routeErrorMessage,
                                                 results);
         if (routeErrorMessage.isEmpty())
         {
            QFile exportFile(exportTarget);
            if (exportFile.open(QIODevice::WriteOnly))
            {
               QTextStream output(&exportFile);
               output << text;
               exportFile.close();

               // Create a results dialog after exporting
               QDialog* resultBox = new QDialog(this);
               resultBox->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
               resultBox->setWindowTitle("Export Results Summary");
               QLabel*       routesCreated  = new QLabel("Routes Created: " + QString::number(results.mRoutesCreated));
               QLabel*       wayLabel       = new QLabel("Way Tag Matches:");
               QLabel*       nodeLabel      = new QLabel("Node Tag Matches:");
               QTableWidget* nodeMatchTable = new QTableWidget(static_cast<int>(results.mNodeTagMatches.size()), 2);
               nodeMatchTable->setHorizontalHeaderLabels({"Tag Name", "Match Count"});
               QTableWidget* wayMatchTable = new QTableWidget(static_cast<int>(results.mWayTagMatches.size()), 2);
               wayMatchTable->setHorizontalHeaderLabels({"Tag Name", "Match Count"});
               unsigned int row = 0;
               for (auto& it : results.mNodeTagMatches)
               {
                  QTableWidgetItem* tagName       = new QTableWidgetItem(it.first);
                  QTableWidgetItem* tagMatchCount = new QTableWidgetItem(QString::number(it.second));
                  nodeMatchTable->setItem(row, 0, tagName);
                  nodeMatchTable->setItem(row++, 1, tagMatchCount);
               }
               nodeMatchTable->horizontalHeader()->setStretchLastSection(true);
               row = 0;
               for (auto& it : results.mWayTagMatches)
               {
                  QTableWidgetItem* tagName       = new QTableWidgetItem(it.first);
                  QTableWidgetItem* tagMatchCount = new QTableWidgetItem(QString::number(it.second));
                  wayMatchTable->setItem(row, 0, tagName);
                  wayMatchTable->setItem(row++, 1, tagMatchCount);
               }
               wayMatchTable->horizontalHeader()->setStretchLastSection(true);
               QGridLayout* verticalLayout = new QGridLayout;
               verticalLayout->addWidget(routesCreated);
               verticalLayout->addWidget(wayLabel, 1, 0);
               verticalLayout->addWidget(wayMatchTable, 2, 0);
               verticalLayout->addWidget(nodeLabel, 1, 1);
               verticalLayout->addWidget(nodeMatchTable, 2, 1);
               resultBox->setLayout(verticalLayout);
               resultBox->show();
            }
            else
            {
               QMessageBox::warning(this, "File Error", "Could not open file for export.");
            }
         }
         else
         {
            QMessageBox::warning(this, "Network Creation Error", routeErrorMessage);
         }
      }
      else
      {
         QMessageBox::warning(this, "Parsing Error", parseErrorMsg);
      }
   }
   else
   {
      QMessageBox::warning(this, "File Error", "Please check the validity of the upload and export file paths. As a note, the input file must have a .osm extension.");
   }
}
