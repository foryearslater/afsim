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


#include "ImporterDialog.hpp"

#include <fstream>

#include <QDropEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QTextStream>

#include "AOrbit.hpp"
#include "APeriod.hpp"
#include "APoint.hpp"
#include "Acmid.hpp"
#include "Aco.hpp"
#include "AerialRefueling.hpp"
#include "AerialRefuelingInfo.hpp"
#include "AirAssetControl.hpp"
#include "AircraftMission.hpp"
#include "AircraftMissionData.hpp"
#include "AircraftMissionLocation.hpp"
#include "Ampn.hpp"
#include "Ato.hpp"
#include "Circle.hpp"
#include "Corridor.hpp"
#include "Efflevel.hpp"
#include "Exer.hpp"
#include "Geoline.hpp"
#include "InputBuilder.hpp"
#include "Onetrack.hpp"
#include "PackageCommander.hpp"
#include "PackageData.hpp"
#include "Polyarc.hpp"
#include "Polygon.hpp"
#include "Radarc.hpp"
#include "RefuelingTasking.hpp"
#include "TaskUnit.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtPath.hpp"
#include "UtTestDataService.hpp"
#include "WkfEnvironment.hpp"
#include "ui_ImporterDialog.h"


AcoImporter::Dialog::Dialog(QWidget* parent, Qt::WindowFlags f)
   : QDialog(parent, f)
{
   mUi.setupUi(this);

   mUi.sourcelistWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mUi.importPushButton->setDisabled(true);
   mUi.sourcelistWidget->setAcceptDrops(true);
   setAcceptDrops(true);

   connect(mUi.browsePushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::BrowsePushButtonClicked);
   connect(mUi.importPushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::ImportPushButtonClicked);
   connect(mUi.addInputFilePushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::AddPushButtonClicked);
   connect(mUi.addInputFolderPushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::GetSelectedFolder);
   connect(mUi.deletePushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::DeleteSelectedEntries);
   connect(mUi.closePushButton, &QPushButton::clicked, this, &AcoImporter::Dialog::CloseDialog);

   mOpenFileLocationForFileDialog   = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
   mOpenFolderLocationForFileDialog = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

   mOutputFolderChanged = false;
   mAddFolderChanged    = false;

   mUi.directoryLineEdit->setText(mOpenFileLocationForFileDialog);
   mUi.directoryLineEdit->setReadOnly(true);

   mUi.sourcelistWidget->setWhatsThis("This is a list of inputs that the import function will process to generate"
                                      " AFSIM Zones when the import button is pressed.");
   mUi.addInputFilePushButton->setWhatsThis(
      "The Add button allows you to specify the files that will be used as input.  Multiple files can be selected"
      " using the Shift and Ctrl keys.");
   mUi.deletePushButton->setWhatsThis("The Delete button allows you to delete selected files from the list of Source "
                                      "Files above the button.  Multiple files can be selected"
                                      " using the Shift and Ctrl keys.");

   mUi.directoryLineEdit->setWhatsThis(
      "The Directory field contains the folder where you want the AFSIM route files to be saved.  This field is filled "
      " in based on your folder selections when you click the Browse button.");
   mUi.browsePushButton->setWhatsThis("The Browse button lets you select the Directory/Folder where the generated "
                                      "AFSIM zone files will be saved. It opens "
                                      " a file chooser dialog to let you make your choice.");
   mUi.importPushButton->setWhatsThis("The Import Button initiates conversion of the specified ATO/ACO input files "
                                      "with the currently selected conversion parameters. A status  "
                                      " dialog is presented which gives the user feedback on whether the conversion is "
                                      "in progress, completed, or canceled by the user.");
   mUi.closePushButton->setWhatsThis("The Close Button will hide the converter dialog.  The list of selected input "
                                     "ATO/ACO files is cleared, but other selected "
                                     " options are maintained for the rest of the Wizard session.");
}

AcoImporter::Dialog::~Dialog() = default;

void AcoImporter::Dialog::Done(int r)
{
   mUi.sourcelistWidget->clear();
   QDialog::done(r);
}

void AcoImporter::Dialog::DeleteEntry()
{
   QList<QListWidgetItem*> items = mUi.sourcelistWidget->selectedItems();
   if (items.length() > 0)
   {
      QListWidgetItem* item = items.at(0);
      delete mUi.sourcelistWidget->takeItem(mUi.sourcelistWidget->row(item));
   }

   // If there are no file in the source list, disable the import button
   mUi.importPushButton->setDisabled(mUi.sourcelistWidget->count() == 0);
}

void AcoImporter::Dialog::DeleteSelectedEntries()
{
   QList<QListWidgetItem*> items = mUi.sourcelistWidget->selectedItems();
   if (items.length() > 0)
   {
      QListWidgetItem* item = items.at(0);
      delete mUi.sourcelistWidget->takeItem(mUi.sourcelistWidget->row(item));
   }

   // If there are no file in the source list, disable the import button
   mUi.importPushButton->setDisabled(mUi.sourcelistWidget->count() == 0);
}

void AcoImporter::Dialog::GetSelectedFolder()
{
   QFileDialog fileDialog;
   fileDialog.setDirectory(QStandardPaths::writableLocation(
      QStandardPaths::DocumentsLocation)); // seems to share default with other file dialogs if not explicitly set

   if (mOpenFolderLocationForFileDialog.length() == 0)
   {
      fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
   }
   else
   {
      QDir theFile(mOpenFolderLocationForFileDialog);
      if (mAddFolderChanged)
      {
         bool wentUpOneLevel = theFile.cdUp();
         if (wentUpOneLevel)
         {
            QString parentPath = theFile.absolutePath();
            fileDialog.setDirectory(parentPath);
         }
      }
   }
   fileDialog.setFileMode(QFileDialog::DirectoryOnly);
   fileDialog.setNameFilter("*.*");

   if (fileDialog.exec() == QFileDialog::Accepted)
   {
      QStringList inputFiles = fileDialog.selectedFiles();
      if (!inputFiles.empty())
      {
         bool fileDialogFolderChanged = false;

         QStringList currentList;
         for (int i = 0; i < mUi.sourcelistWidget->count(); i++)
         {
            currentList.push_back(mUi.sourcelistWidget->item(i)->text());
         }

         for (const QString& inputFile : inputFiles)
         {
            bool found = false;
            for (const QString& filename : currentList)
            {
               if (inputFile.compare(filename) == 0)
               {
                  found = true;
                  break;
               }
            }

            if (!found)
            {
               mUi.sourcelistWidget->addItem(inputFile);
               // change the location to open the file dialog to the last folder used to add files
               if (!fileDialogFolderChanged)
               {
                  QDir theFirstFile(inputFile);
                  mOpenFolderLocationForFileDialog = theFirstFile.absolutePath();
                  fileDialogFolderChanged          = true;
                  mAddFolderChanged                = true;
               }
            }
         }
      }
   }

   // If there are no file in the source list, disable the import button
   mUi.importPushButton->setDisabled(mUi.sourcelistWidget->count() == 0);
}

void AcoImporter::Dialog::CloseDialog()
{
   mUi.sourcelistWidget->clear();
   mUi.importPushButton->setDisabled(true); // after clearing out input file list, can't import till we add more files
   done(QDialog::Accepted);
}


void AcoImporter::Dialog::BrowsePushButtonClicked()
{
   QFileDialog fileDialog;

   QString currentOutputFolder = mUi.directoryLineEdit->text();

   if (currentOutputFolder.length() == 0)
   {
      fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
   }
   else
   {
      QDir theFile(currentOutputFolder);
      bool wentUpOneLevel = true;
      if (mOutputFolderChanged)
      {
         wentUpOneLevel = theFile.cdUp();
      }
      if (wentUpOneLevel || !mOutputFolderChanged)
      {
         QString parentPath = theFile.absolutePath();
         fileDialog.setDirectory(parentPath);
      }
   }

   fileDialog.setFileMode(QFileDialog::DirectoryOnly);
   fileDialog.setNameFilter("*.*");
   fileDialog.setWindowTitle("Please select an output directory");

   if (fileDialog.exec() == QFileDialog::Accepted)
   {
      QStringList inputFiles = fileDialog.selectedFiles();
      if (!inputFiles.empty())
      {
         const QString& outputDirectory = inputFiles.at(0);

         if (!outputDirectory.isEmpty())
         {
            mUi.directoryLineEdit->setText(outputDirectory);
            mOutputFolderChanged = true;
         }
      }
   }
}

void AcoImporter::Dialog::ImportPushButtonClicked()
{
   std::string              outputFolder = mUi.directoryLineEdit->text().toStdString();
   std::vector<std::string> sourceFiles;
   for (int i = 0; i < static_cast<int>(mUi.sourcelistWidget->count()); i++)
   {
      std::string currFile = mUi.sourcelistWidget->item(i)->text().toStdString();
      UtPath      path(currFile);

      if (path.Exists())
      {
         if (path.IsDirectory())
         {
            std::vector<std::string> files;
            std::vector<std::string> directories;
            path.ListDir(files, directories);

            if (!files.empty())
            {
               std::string suffix     = ".txt";
               size_t      suffixSize = suffix.size();
               for (auto& fileInDir : files)
               {
                  std::transform(fileInDir.begin(), fileInDir.end(), fileInDir.begin(), ::tolower);
                  size_t fileNameSize = fileInDir.size();

                  bool isAcoFile =
                     fileNameSize > suffixSize && fileInDir.compare(fileNameSize - suffixSize, suffixSize, suffix) == 0;

                  if (isAcoFile)
                  {
                     sourceFiles.emplace_back(currFile + "/" + fileInDir);
                  }
               }
            }
            else
            {
               sourceFiles.push_back(currFile);
            }
         }
         else // single file
         {
            sourceFiles.push_back(currFile);
         }
      }
   }

   for (const std::string& MESSAGE : sourceFiles)
   {
      QMessageBox msgBox;
      msgBox.setTextFormat(Qt::RichText);

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(MESSAGE);
      try
      {
         auto mMessage = parser.ReadMessage();
         auto mFact    = usmtf::MessageFactory::GetFactory();
         if (!(mFact->CastIfRegistered<usmtf::Aco>(*mMessage) || mFact->CastIfRegistered<usmtf::Ato>(*mMessage)))
         {
            QString out = "Error Importing " + QString::fromStdString(mType) +
                          "  Message type you are attempting to Import is not an " + QString::fromStdString(mType);
            msgBox.setText(out);
            msgBox.exec();
            CloseDialog();
            return;
         }

         if (!mMessage->IsValid())
         {
            std::string logFile;
            QFileInfo   imported(QString::fromStdString(MESSAGE));
            std::string baseName = imported.baseName().toStdString();
            logFile              = outputFolder + "/" + baseName + "_ImportErrors.txt";

            std::ofstream fs{logFile};
            if (fs)
            {
               mMessage->LogErrors(fs);
               fs.flush();
               QString link = QString::fromStdString("<a href=\"file:///" + logFile + "\">" + logFile + "</a>");
               msgBox.setText("Error Importing " + QString::fromStdString(mType) + " See log file for details: <br/>" +
                              link);
            }
            else
            {
               msgBox.setText("Failed to open log file for writing: <br/>" + QString::fromStdString(logFile));
            }
            msgBox.exec();
         }

         std::string output;
         QFileInfo   imported(QString::fromStdString(MESSAGE));
         std::string baseName     = imported.baseName().toStdString();
         output                   = outputFolder + "/" + baseName + "_Export.txt";
         QString             link = QString::fromStdString("<a href=\"file:///" + output + "\">" + output + "</a>");
         usmtf::InputBuilder sb{output};
         if (mType == "ACO")
         {
            ExportAco(*(dynamic_cast<const usmtf::Aco*>(mMessage.get())), sb);
            msgBox.setText("ACO Exported: <br/>" + link);
            msgBox.exec();
         }
         else
         {
            ExportAto(*(dynamic_cast<const usmtf::Ato*>(mMessage.get())), outputFolder, baseName + "_Export.txt");
            msgBox.setText("ATO Exported: <br/>" + link);
            msgBox.exec();
         }
      }
      catch (const usmtf::ImportError& err)
      {
         msgBox.setText(QString::fromStdString("Error Importing " + MESSAGE + "<br/>" + err.what()));
         msgBox.exec();
      }
   }
   CloseDialog();
}

void AcoImporter::Dialog::dropEvent(QDropEvent* aEvent)
{
   QString     str         = aEvent->mimeData()->text();
   std::string fileStrings = str.toStdString();
   QStringList inputFiles;
   bool        invalidDroppedFile = false;
   if (aEvent->mimeData()->hasUrls())
   {
      QList<QUrl> urls = aEvent->mimeData()->urls();
      QUrl        adjustedUrl;

      if (urls.length() == 0)
      {
         invalidDroppedFile = true;
      }
      else
      {
         for (const QUrl& url : urls)
         {
            QString rawUrl = url.toDisplayString();
            if (rawUrl.startsWith("file:///"))
            {
               adjustedUrl        = url.adjusted(QUrl::RemoveScheme);
               QString fileString = adjustedUrl.toDisplayString(); // has a forward slash on the front end
               fileString         = fileString.right(fileString.size() - 1);
               inputFiles.append(fileString);
            }
            else
            {
               invalidDroppedFile = true;
            }
         }
      }
   }
   else
   {
      invalidDroppedFile = true;
   }
   if (invalidDroppedFile)
   {
      QMessageBox msgBox;
      msgBox.setText("One or more dropped files is not an accepted url.  Ensure that the files are all dragged from "
                     "a local drive or a drive mounted on the local file system.");
      msgBox.exec();
   }
   QStringList filenames;
   for (int i = 0; i < mUi.sourcelistWidget->count(); i++)
   {
      filenames.push_back(mUi.sourcelistWidget->item(i)->text());
   }
   bool nonMessageFilesFound = false;
   // For each item the user selected, make sure it is not already in
   // the list of input files prior to adding it to the list.
   for (const QString& inputFile : inputFiles)
   {
      bool found = false;
      for (auto& filename : filenames)
      {
         if (filename.compare(inputFile) == 0)
         {
            found = true;
            break;
         }
      }

      if (!found)
      {
         if (inputFile.endsWith(".txt", Qt::CaseInsensitive))
         {
            mUi.sourcelistWidget->addItem(inputFile);
         }
         else
         {
            nonMessageFilesFound = true;
         }
      }
   }
   mUi.importPushButton->setDisabled(mUi.sourcelistWidget->count() == 0);
   if (nonMessageFilesFound)
   {
      QMessageBox msgBox;
      QString message = QString::fromStdString("One or more dragged files does not have a .txt file extension. Only "
                                               ".txt files can be converted. If you are sure"
                                               " the file is a " +
                                               mType + "file, change its extension and try again.");
      msgBox.setText(message);
      msgBox.exec();
   }
}

void AcoImporter::Dialog::dragEnterEvent(QDragEnterEvent* aEvent)
{
   QPoint touchPoint = aEvent->pos();
   touchPoint        = this->mapToGlobal(touchPoint);
   // widget position in window coordinates
   int    left = mUi.sourcelistWidget->x();
   int    top  = mUi.sourcelistWidget->y();
   QPoint topLeft(left, top);
   QPoint globalLocation = mUi.sourcelistWidget->parentWidget()->mapToGlobal(mUi.sourcelistWidget->pos());
   int    width          = mUi.sourcelistWidget->width();
   int    height         = mUi.sourcelistWidget->height();
   QRect  fileWidgetFrame(globalLocation.x(), globalLocation.y(), width, height);

   if (fileWidgetFrame.contains(touchPoint))
   {
      aEvent->acceptProposedAction();
   }
}

void AcoImporter::Dialog::mouseMoveEvent(QMouseEvent* aEvent)
{
   if (mUi.sourcelistWidget->geometry().contains(aEvent->pos()))
   {
   }
}

void AcoImporter::Dialog::AddPushButtonClicked()
{
   QString startingFolder = mUi.directoryLineEdit->text();
   if (mOpenFileLocationForFileDialog.length() == 0)
   {
      mOpenFileLocationForFileDialog = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
   }
   QStringList inputFiles =
      QFileDialog::getOpenFileNames(this,
                                    "Please select input file(s)",
                                    mOpenFileLocationForFileDialog,
                                    "Common Route Definition files (*.txt *.xml);;All Files (*.*)");

   if (!inputFiles.isEmpty())
   {
      QStringList filenames;
      for (int i = 0; i < mUi.sourcelistWidget->count(); i++)
      {
         filenames.push_back(mUi.sourcelistWidget->item(i)->text());
      }

      // For each item the user selected, make sure it is not already in
      //  the list of input files prior to adding it to the list.
      for (const QString& inputFile : inputFiles)
      {
         bool found                   = false;
         bool fileDialogFolderChanged = false;
         for (auto& filename : filenames)
         {
            if (filename.compare(inputFile) == 0)
            {
               found = true;
               break;
            }
         }

         if (!found)
         {
            mUi.sourcelistWidget->addItem(inputFile);
            // change the location to open the file dialog to the last folder used to add files
            if (!fileDialogFolderChanged)
            {
               QDir theFirstFile(inputFile);
               mOpenFileLocationForFileDialog = theFirstFile.absolutePath();
               fileDialogFolderChanged        = true;
            }
         }
      }

      // If there are no file in the source list, disable the import button
      mUi.importPushButton->setDisabled(mUi.sourcelistWidget->count() == 0);
   }
}

void AcoImporter::Dialog::RegisterAcoRequirements()
{
   mType = "ACO";
   setWindowTitle({"ACO Importer"});
   auto sFact = usmtf::SetFactory::GetFactory();
   auto mFact = usmtf::MessageFactory::GetFactory();
   sFact->RegisterEntity("ACMID", usmtf::SetFactory::Construct<usmtf::Acmid>);
   sFact->RegisterEntity("APOINT", usmtf::SetFactory::Construct<usmtf::APoint>);
   sFact->RegisterEntity("AMPN", usmtf::SetFactory::Construct<usmtf::Ampn>);
   sFact->RegisterEntity("AORBIT", usmtf::SetFactory::Construct<usmtf::AOrbit>);
   sFact->RegisterEntity("APERIOD", usmtf::SetFactory::Construct<usmtf::APeriod>);
   sFact->RegisterEntity("CIRCLE", usmtf::SetFactory::Construct<usmtf::Circle>);
   sFact->RegisterEntity("CORRIDOR", usmtf::SetFactory::Construct<usmtf::Corridor>);
   sFact->RegisterEntity("EFFLEVEL", usmtf::SetFactory::Construct<usmtf::Efflevel>);
   sFact->RegisterEntity("EXER", usmtf::SetFactory::Construct<usmtf::Exer>);
   sFact->RegisterEntity("POLYGON", usmtf::SetFactory::Construct<usmtf::Polygon>);
   sFact->RegisterEntity("POLYARC", usmtf::SetFactory::Construct<usmtf::Polyarc>);
   sFact->RegisterEntity("1TRACK", usmtf::SetFactory::Construct<usmtf::OneTrack>);
   sFact->RegisterEntity("RADARC", usmtf::SetFactory::Construct<usmtf::Radarc>);
   sFact->RegisterEntity("GEOLINE", usmtf::SetFactory::Construct<usmtf::Geoline>);

   mFact->RegisterEntity("ACO", usmtf::MessageFactory::Construct<usmtf::Aco>);
}

void AcoImporter::Dialog::UnregisterAcoRequirements()
{
   auto sFact = usmtf::SetFactory::GetFactory();
   auto mFact = usmtf::MessageFactory::GetFactory();
   sFact->UnregisterEntity("ACMID");
   sFact->UnregisterEntity("APOINT");
   sFact->UnregisterEntity("AMPN");
   sFact->UnregisterEntity("AORBIT");
   sFact->UnregisterEntity("APERIOD");
   sFact->UnregisterEntity("CIRCLE");
   sFact->UnregisterEntity("CORRIDOR");
   sFact->UnregisterEntity("EFFLEVEL");
   sFact->UnregisterEntity("EXER");
   sFact->UnregisterEntity("POLYGON");
   sFact->UnregisterEntity("POLYARC");
   sFact->UnregisterEntity("1TRACK");
   sFact->UnregisterEntity("RADARC");
   sFact->RegisterEntity("GEOLINE", usmtf::SetFactory::Construct<usmtf::Geoline>);

   mFact->UnregisterEntity("ACO");
}

void AcoImporter::Dialog::RegisterAtoRequirements()
{
   mType = "ATO";
   setWindowTitle({"ATO Importer"});
   auto sFact = usmtf::SetFactory::GetFactory();
   auto mFact = usmtf::MessageFactory::GetFactory();
   sFact->RegisterEntity("MSNACFT", usmtf::SetFactory::Construct<usmtf::AircraftMission>);
   sFact->RegisterEntity("ARINFO", usmtf::SetFactory::Construct<usmtf::AerialRefuelingInfo>);
   sFact->RegisterEntity("AMSNLOC", usmtf::SetFactory::Construct<usmtf::AircraftMissionLocation>);
   sFact->RegisterEntity("AMSNDAT", usmtf::SetFactory::Construct<usmtf::AircraftMissionData>);
   sFact->RegisterEntity("AMPN", usmtf::SetFactory::Construct<usmtf::Ampn>);
   sFact->RegisterEntity("9PKGDAT", usmtf::SetFactory::Construct<usmtf::PackageData>);
   sFact->RegisterEntity("CONTROLA", usmtf::SetFactory::Construct<usmtf::AirAssetControl>);
   sFact->RegisterEntity("TASKUNIT", usmtf::SetFactory::Construct<usmtf::TaskUnit>);
   sFact->RegisterEntity("PKGCMD", usmtf::SetFactory::Construct<usmtf::PackageCommander>);
   sFact->RegisterEntity("REFTSK", usmtf::SetFactory::Construct<usmtf::RefuelingTasking>);
   sFact->RegisterEntity("EXER", usmtf::SetFactory::Construct<usmtf::Exer>);
   sFact->RegisterEntity("5REFUEL", usmtf::SetFactory::Construct<usmtf::AerialRefueling>);

   mFact->RegisterEntity("ATO", usmtf::MessageFactory::Construct<usmtf::Ato>);
}

void AcoImporter::Dialog::UnregisterAtoRequirements()
{
   auto sFact = usmtf::SetFactory::GetFactory();
   auto mFact = usmtf::MessageFactory::GetFactory();
   sFact->UnregisterEntity("MSNACFT");
   sFact->UnregisterEntity("ARINFO");
   sFact->UnregisterEntity("AMSNLOC");
   sFact->UnregisterEntity("AMSNDAT");
   sFact->UnregisterEntity("AMPN");
   sFact->UnregisterEntity("9PKGDAT");
   sFact->UnregisterEntity("CONTROLA");
   sFact->UnregisterEntity("TASKUNIT");
   sFact->UnregisterEntity("PKGCMD");
   sFact->UnregisterEntity("REFTSK");
   sFact->UnregisterEntity("EXER");
   sFact->UnregisterEntity("5REFUEL");

   mFact->UnregisterEntity("ATO");
}
