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

#include "CrdImporterDialog.hpp"

#include <climits>

#include <QDir>
#include <QDoubleValidator>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QShortcut>
#include <QStandardPaths>

#include "ImportWorker.hpp"
#include "StatusAndResultsDialog.hpp"
#include "UtPath.hpp"

#define wkfEnv wkf::Environment::GetInstance()

CrdImporter::Dialog::Dialog(QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
   , mStatusDialogPtr(new StatusAndResults::Dialog(this))
{
   mUI.setupUi(this);

   mUI.sourceListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection); // MultiSelection);
   mUI.geoToleranceLineEdit->setValidator(new QDoubleValidator(mUI.geoToleranceLineEdit));
   mUI.geoToleranceLineEdit->setText("0.000001");
   mUI.recurseTransitionsCheckBox->setChecked(true);
   mUI.simpleOrbitCheckBox->setChecked(true);
   mUI.importPushButton->setDisabled(true); // wait till a file is in the list at least
   mUI.sourceListWidget->setAcceptDrops(true);
   setAcceptDrops(true); // since we can't easily reimplement sourceListWidget's drop action since it's auto-generated

   connect(mUI.importPushButton, &QPushButton::clicked, this, &Dialog::ImportPushButtonClicked);
   connect(mUI.outputFolderPushButton, &QPushButton::clicked, this, &Dialog::BrowsePushButtonClicked);
   connect(mUI.addPushButton, &QPushButton::clicked, this, &Dialog::AddPushButtonClicked);
   connect(mUI.addFolderButton, &QPushButton::clicked, this, &Dialog::GetSelectedFolder);
   connect(mUI.deleteButton, &QPushButton::clicked, this, &Dialog::DeleteSelectedEntries);
   connect(mUI.closeButton, &QPushButton::clicked, this, &Dialog::CloseDialog);

   mOpenFileLocationForFileDialog   = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
   mOpenFolderLocationForFileDialog = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

   mOutputFolderChanged = false;
   mAddFolderChanged    = false;

   mUI.outputFolderLineEdit->setText(mOpenFileLocationForFileDialog);
   mUI.outputFolderLineEdit->setReadOnly(true);

   QRegExp regExp("[a-z,A-Z][a-z,A-Z,0-9,_]*");
   mUI.fileBaseNameLineEdit->setValidator(new QRegExpValidator(regExp));

   // some tooltips and Shift-F1 help
   QString messageString = "This is a list of inputs that the import function will process to generate"
                           " AFSIM route files when the import button is pressed.";
   mUI.sourceListWidget->setWhatsThis(messageString);
   messageString =
      "The Add button allows you to specify the files that will be used as input.  Multiple files can be selected"
      " using the Shift and Ctrl keys.";
   mUI.addPushButton->setWhatsThis(messageString);
   messageString = "The Delete button allows you to delete selected files from the list of Source Files above the "
                   "button.  Multiple files can be selected"
                   " using the Shift and Ctrl keys.";
   mUI.deleteButton->setWhatsThis(messageString);
   messageString =
      "The File Base Name field allows you to specify a string that will be prepended to the automatically generated "
      " output file names.  Its value is constrained to contain only alphanumeric characters and the underscore "
      "character.";
   mUI.fileBaseNameLineEdit->setWhatsThis(messageString);
   messageString =
      "The Directory field contains the folder where you want the AFSIM route files to be saved.  This field is filled "
      " in based on your folder selections when you click the Browse button.";
   mUI.outputFolderLineEdit->setWhatsThis(messageString);
   messageString = "The Browse button lets you select the Directory/Folder where the generated AFSIM route files will "
                   "be saved. It opens "
                   " a file chooser dialog to let you make your choice.";
   mUI.outputFolderPushButton->setWhatsThis(messageString);
   messageString = "The Lat/Lon Format control lets you choose whether you want to display the route points for AFSIM "
                   "as decimal degree values,  "
                   " or as Degrees, Minutes, Seconds (DMS) values.  ";
   mUI.latLonFormatComboBox->setWhatsThis(messageString);
   messageString =
      "The Geo Tolerance field specifies a quantity in decimal degrees.  If the route converter encounters adjacent "
      "points where the absolute value of the "
      "differences of the points' lat and lon values are both within this quantity,  "
      " the adjacent point will be considered a duplicate point of the first point, and will not be added to the route "
      "list.  It is recommended to leave this value"
      " at 0.000001 degrees, which is equivalent to about four inches at the equator for latitude and longitude.";
   mUI.geoToleranceLineEdit->setWhatsThis(messageString);
   messageString = "This button includes debugging information in the generated AFSIM route files as well as a file in "
                   "My Documents/crdlogfile.txt."
                   " This information can be useful in identifying problems with the conversion process, including "
                   "finding errors in the XML content "
                   " and locating transition and intent elements responsible for each AFSIM route point.  This option "
                   "may increase the time for conversion from "
                   " less than a couple seconds to over ten seconds.";
   mUI.debugCheckBox->setWhatsThis(messageString);
   messageString = "This option allows the user to specify reduced route fidelity.  JMPS allows the user to create "
                   "routes that have only top level "
                   " CRD Transition elements, or to specify multiple levels of detail. To capture the highest detail "
                   "of the route available in the CRD input file, "
                   " this box should be left checked.";
   mUI.recurseTransitionsCheckBox->setWhatsThis(messageString);
   messageString = "If this option is checked, the generated AFSIM routes will include the unselected coordinate "
                   "format in a comment for each point. "
                   " For example, if Decimal format is selected for Lat/Lon Format, and this box is checked, AFSIM "
                   "routes will show the DMS values for each lat/lon point "
                   " as a comment on the same line as the AFSIM position command for that point.";
   mUI.printAltFormatCheckbox->setWhatsThis(messageString);
   messageString = "The Import Button initiates conversion of the specified .crd input files with the currently "
                   "selected conversion parameters. A status  "
                   " dialog is presented which gives the user feedback on whether the conversion is in progress, "
                   "completed, or canceled by the user.";
   mUI.importPushButton->setWhatsThis(messageString);
   messageString = "The Close Button will hide the converter dialog.  The list of selected input .crd files is "
                   "cleared, but other selected "
                   " options are maintained for the rest of the Wizard session.";
   mUI.closeButton->setWhatsThis(messageString);

   // delete key shortcut to delete a selected item from the list of files.  The action will select the next list
   // element after the deleted item to allow for repeated hammering of the delete key to delete multiple files AFSIM
   // method for creating a user configurable preference shortcut
   // wkf::Action* deleteShortcut2 = new wkf::Action("Remove Platform", wkfEnv.GetMainWindow(),
   // QKeySequence(Qt::Key_Delete)); connect(deleteShortcut2, &QAction::triggered, this,
   // &Plugin::DeleteCurrentlySelectedPlatforms); mActions.push_back(deleteShortcut);
   // standard qt shortcut
   QShortcut* deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), mUI.sourceListWidget);
   // connect(deleteShortcut, SIGNAL(activated()), this, SLOT(DeleteEntry()));
   connect(deleteShortcut, SIGNAL(activated()), this, SLOT(DeleteSelectedEntries()));


   // mActions.push_back(deleteShortcut);
}

CrdImporter::Dialog::~Dialog()
{
   mWorkThread.quit();
   mWorkThread.wait();
}

void CrdImporter::Dialog::done(int r)
{
   mUI.sourceListWidget->clear();
   QDialog::done(r);
}

void CrdImporter::Dialog::AddPushButtonClicked()
{
   QString startingFolder = mUI.outputFolderLineEdit->text();
   if (mOpenFileLocationForFileDialog.length() == 0)
   {
      mOpenFileLocationForFileDialog = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
   }
   QStringList inputFiles =
      QFileDialog::getOpenFileNames(this,
                                    "Please select input .CRD file(s)",
                                    mOpenFileLocationForFileDialog,
                                    "Common Route Definition files (*.crd *.xml);;All Files (*.*)");

   if (!inputFiles.isEmpty())
   {
      QStringList filenames;
      for (int i = 0; i < mUI.sourceListWidget->count(); i++)
      {
         filenames.push_back(mUI.sourceListWidget->item(i)->text());
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
            mUI.sourceListWidget->addItem(inputFile);
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
      mUI.importPushButton->setDisabled(mUI.sourceListWidget->count() == 0);
   }
}

void CrdImporter::Dialog::GetSelectedFolder()
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
         for (int i = 0; i < mUI.sourceListWidget->count(); i++)
         {
            currentList.push_back(mUI.sourceListWidget->item(i)->text());
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
               mUI.sourceListWidget->addItem(inputFile);
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
   mUI.importPushButton->setDisabled(mUI.sourceListWidget->count() == 0);
}

void CrdImporter::Dialog::DeleteSelectedEntries()
{
   QList<QListWidgetItem*> items = mUI.sourceListWidget->selectedItems();
   for (QListWidgetItem* item : items)
   {
      delete mUI.sourceListWidget->takeItem(mUI.sourceListWidget->row(item));
   }

   // If there are no file in the source list, disable the import button
   mUI.importPushButton->setDisabled(mUI.sourceListWidget->count() == 0);
}

void CrdImporter::Dialog::CloseDialog()
{
   mUI.sourceListWidget->clear();
   mUI.importPushButton->setDisabled(true); // after clearing out input file list, can't import till we add more files
   done(QDialog::Accepted);
}

void CrdImporter::Dialog::BrowsePushButtonClicked()
{
   QFileDialog fileDialog;

   QString currentOutputFolder = mUI.outputFolderLineEdit->text();
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
            mUI.outputFolderLineEdit->setText(outputDirectory);
            mOutputFolderChanged = true;
            // QDir theFile(outputDirectory);
            // openOutputFolderLocationForFileDialog = theFile.absolutePath();
         }
      }
   }
}

void CrdImporter::Dialog::DeleteEntry()
{
   std::cout << "delete entry method called on delete push!" << std::endl;
   QList<QListWidgetItem*> items = mUI.sourceListWidget->selectedItems();
   if (items.length() > 0)
   {
      QListWidgetItem* item = items.at(0);
      delete mUI.sourceListWidget->takeItem(mUI.sourceListWidget->row(item));
   }

   // If there are no file in the source list, disable the import button
   mUI.importPushButton->setDisabled(mUI.sourceListWidget->count() == 0);
}

void CrdImporter::Dialog::HandleResults(const QString& aParm)
{
   mStatusDialogPtr->SetOutputTree(mCrdImporter.GetFilenamesMap()); // replace name with getter for this in CrdFileImporter

   QString statusString = "Conversion completed normally.";
   if (mCrdImporter.GetErrorCount() > 0)
   {
      statusString = "Conversion process completed with errors.";
   }
   else if (mCrdImporter.GetCancelFlag())
   {
      statusString = "Conversion process canceled by user.";
   }
   mStatusDialogPtr->SetStatus(statusString);
}

void CrdImporter::Dialog::ImportPushButtonClicked()
{
   std::string outputFolder = mUI.outputFolderLineEdit->text().toStdString();
   std::string baseName     = mUI.fileBaseNameLineEdit->text().toStdString();

   std::vector<std::string> sourceFiles;
   for (int i = 0; i < mUI.sourceListWidget->count(); i++)
   {
      // some of the files in the file list may be folders, get the crd files in those folders to pass to importer
      std::string currFile = mUI.sourceListWidget->item(i)->text().toStdString();
      UtPath      path(currFile);

      if (path.Exists())
      {
         if (path.IsDirectory())
         {
            // look for all crd files in the folder specified
            std::vector<std::string> files;
            std::vector<std::string> directories;
            path.ListDir(files, directories);

            // not recursing, don't need the directories
            if (!files.empty())
            {
               std::string suffix     = ".crd";
               auto        suffixSize = suffix.size();
               for (std::string fileInDir : files)
               {
                  transform(fileInDir.begin(), fileInDir.end(), fileInDir.begin(), ::tolower);
                  auto filenameSize = fileInDir.size();

                  bool isCrdFile =
                     filenameSize > suffixSize && fileInDir.compare(filenameSize - suffixSize, suffixSize, suffix) == 0;

                  if (isCrdFile)
                  {
                     sourceFiles.emplace_back(currFile + "/" + fileInDir);
                  }
               }
            }
         }
         else
         {
            sourceFiles.push_back(currFile);
         }
      }
   }

   mCrdImporter.ClearFilenamesMap();
   mStatusDialogPtr->SetOutputTree(mCrdImporter.GetFilenamesMap());
   mCrdImporter.SetDebug(mUI.debugCheckBox->isChecked());
   mCrdImporter.SetOutputFolder(outputFolder);
   mCrdImporter.SetUseSimpleOrbits(mUI.simpleOrbitCheckBox->isChecked());
   mCrdImporter.SetGeneratePlatformStubs(mUI.platformStubsCheckBox->isChecked());
   mCrdImporter.SetGeoEqualityTolerance(mUI.geoToleranceLineEdit->text().toDouble());
   if (mUI.latLonFormatComboBox->currentText().toStdString() == "Decimal")
   {
      mCrdImporter.SetPointFormat(CrdFileImporter::cDECIMAL_FORMAT);
   }
   else
   {
      mCrdImporter.SetPointFormat(CrdFileImporter::cDMS_FORMAT);
   }
   mCrdImporter.SetRecurseTransitions(mUI.recurseTransitionsCheckBox->isChecked());
   mCrdImporter.SetPrintAltCoordinateFormat(mUI.printAltFormatCheckbox->isChecked());
   mCrdImporter.SetOutputFilenameBase(baseName);
   mCrdImporter.SetSourceCrdInputFiles(sourceFiles);

   ImportWorker importWorker;
   importWorker.SetCrdImporter(mCrdImporter);
   mStatusDialogPtr->SetCrdImporter(mCrdImporter, this);
   mStatusDialogPtr->SetStatus("Converting input files...");

   mStatusDialogPtr->ConversionStarted();
   importWorker.moveToThread(&mWorkThread);
   connect(&mWorkThread, &QThread::finished, &importWorker, &QObject::deleteLater);
   connect(this, &Dialog::Operate, &importWorker, &ImportWorker::DoConversion);
   connect(&importWorker, &ImportWorker::ResultReady, this, &Dialog::HandleResults);


   mWorkThread.start();
   Operate("start");

   mStatusDialogPtr->exec();
}

void CrdImporter::Dialog::dropEvent(QDropEvent* event)
{
   QString str = event->mimeData()->text();
   std::cout << " dropped mime data is " << str.toStdString() << std::endl;
   std::string fileStrings = str.toStdString();
   QStringList inputFiles;
   bool        invalidDroppedFile = false;
   if (event->mimeData()->hasUrls())
   {
      QList<QUrl> urls = event->mimeData()->urls();
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
   for (int i = 0; i < mUI.sourceListWidget->count(); i++)
   {
      filenames.push_back(mUI.sourceListWidget->item(i)->text());
   }
   bool nonCrdFilesFound = false;
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
         if (inputFile.endsWith(".crd", Qt::CaseInsensitive))
         {
            mUI.sourceListWidget->addItem(inputFile);
         }
         else
         {
            nonCrdFilesFound = true;
         }
      }
   }
   mUI.importPushButton->setDisabled(mUI.sourceListWidget->count() == 0);
   if (nonCrdFilesFound)
   {
      QMessageBox msgBox;
      msgBox.setText("One or more dragged files does not have a .crd file extension. Only .crd files can be converted. "
                     "If you are sure"
                     " the file is a CRD file, change its extension and try again.");
      msgBox.exec();
   }
}

void CrdImporter::Dialog::dragEnterEvent(QDragEnterEvent* event)
{
   QPoint touchPoint = event->pos();
   touchPoint        = this->mapToGlobal(touchPoint);
   // widget position in window coordinates
   int    left = mUI.sourceListWidget->x();
   int    top  = mUI.sourceListWidget->y();
   QPoint topLeft(left, top);
   QPoint globalLocation = mUI.sourceListWidget->parentWidget()->mapToGlobal(mUI.sourceListWidget->pos());
   int    width          = mUI.sourceListWidget->width();
   int    height         = mUI.sourceListWidget->height();
   QRect  fileWidgetFrame(globalLocation.x(), globalLocation.y(), width, height);

   if (fileWidgetFrame.contains(touchPoint))
   {
      //(mUI.sourceListWidget->geometry().contains(event->pos())) {
      event->acceptProposedAction();
   }
}

void CrdImporter::Dialog::mouseMoveEvent(QMouseEvent* event)
{
   if (mUI.sourceListWidget->geometry().contains(event->pos()))
   {
      // event->acceptProposedAction();  // at this point, a mouse event, not a drag event, so we don't have acceptProposedAction
   }
}
