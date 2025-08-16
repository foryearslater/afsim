// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "ScenarioImporterPlugin.hpp"

#include <string>

#include <QMenu>
#include <QMessageBox>
#include <QTextStream>
#include <QToolBar>
#include <QString>

#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfAction.hpp"
#include "WkfGeneralPrefObject.hpp"

//#include "vld.h"

WKF_PLUGIN_DEFINE_SYMBOLS(ScenarioImporter::Plugin, "Scenario Importer", "Provides the capability to import scenarios from CSV files", "wizard")

// implementation of the custom QMainWindow class with a close signal
void ScenarioImporter::ImporterMainWindow::closeEvent(QCloseEvent* event)
{
   emit close();
}

ScenarioImporter::Plugin::Plugin(const QString& aPluginName,
                                 const size_t   aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mActionPtr(new wkf::Action("Scenario Importer...", wkfEnv.GetMainWindow()))
{
   QMenu* toolsMenu = wkfEnv.GetMainWindow()->GetMenuByName("Tools");
   // Add the action to the application action set
   connect(mActionPtr, &QAction::triggered, this, &ScenarioImporter::Plugin::LaunchScenarioImporterToolFromMenu);
   toolsMenu->addAction(mActionPtr);
   mActionPtr->setEnabled(true);
   //LoadPlugin();
}

ScenarioImporter::Plugin::~Plugin()
{
   QCoreApplication::quit();
   UnloadPlugin();
}

void ScenarioImporter::Plugin::LoadPlugin()
{
   ScenarioImporterGui& gui = this->gui;
   Data& data = this->data;
   Config& config = this->config;
   Errors& errors = this->errors;
   Stage& configStage = this->configStage;
   Stage& processingStage = this->processingStage;

   configStage = Stage::PreProcess;
   processingStage = Stage::PreProcess;

   data.rowCount = 47; // TODO quit cheating and set rowCount by reading data!
   gui.selectedParserColumnIndex = 0;
   config.parserColumns.emplace_back("1", ',', ScenarioImporter::ParserColumnDatatype::String, true, false);
   config.outputMultiFile = false;
   config.multiLineRecords = false;

   // Main window
   //auto theMainWindow = new QMainWindow(ViEnvironment::GetInstance().GetMainWindowPtr());
   auto theMainWindow = new ImporterMainWindow(wkfEnv.GetMainWindow());
   gui.mainWindow = theMainWindow;

   // Central widget
   auto windowWidget = new QWidget;
   auto windowLayout = new QVBoxLayout;
   windowWidget->setLayout(windowLayout);
   gui.mainWindow->setCentralWidget(windowWidget);

   // Horizontal splitter between errors list and remainder of gui
   auto errorsSplitter = new QSplitter;
   windowLayout->addWidget(errorsSplitter);

   // Vertical splitter between input- and output-view panes and top bar of controls (file, importer stages, and configuration widgets)
   auto controlSplitter = new QSplitter(errorsSplitter);
   errorsSplitter->setStretchFactor(0, 8);
   controlSplitter->setOrientation(Qt::Vertical);

   // Horizontal splitter in top bar of controls (file, importer stages, and configuration widgets)
   auto controlsWidget = new QSplitter(controlSplitter);
   controlSplitter->setStretchFactor(0, 2);

   // File groupbox for open file, pre-processed file, save file, and load & save importer buttons
   auto fileWidget = new QGroupBox(controlsWidget);
   auto fileLayout = new QVBoxLayout;
   fileWidget->setLayout(fileLayout);
   controlsWidget->setStretchFactor(0, 2);
   fileWidget->setTitle("File");
   QFontMetrics fontMetric(fileWidget->font());
   int fileButtonWidth = fontMetric.width("Select output directory") + 10;

   // Open file button and text line
   gui.openFileButton = new QPushButton("Open file");
   gui.openFileButton->setStyleSheet(getStyleSheet(Color::Todo));
   fileLayout->addWidget(gui.openFileButton);
   fileLayout->setAlignment(gui.openFileButton, Qt::AlignLeft);
   gui.openFileButton->setMinimumWidth(fileButtonWidth);
   gui.openFileNameEdit = new QLineEdit;
   fileLayout->addWidget(gui.openFileNameEdit);

   // Save file button and text line
   gui.browseOutputDirectoryButton = new QPushButton("Select output directory");
   fileLayout->addWidget(gui.browseOutputDirectoryButton);
   fileLayout->setAlignment(gui.browseOutputDirectoryButton, Qt::AlignLeft);
   gui.browseOutputDirectoryButton->setMinimumWidth(fileButtonWidth);
   gui.outputDirectoryNameEdit = new QLineEdit;
   fileLayout->addWidget(gui.outputDirectoryNameEdit);

   // Load & save importer buttons
   auto importerButtonLayout = new QHBoxLayout;
   fileLayout->addLayout(importerButtonLayout);
   gui.loadImporterButton = new QPushButton("Load importer");
   gui.loadImporterButton->setMinimumWidth(fileButtonWidth);
   importerButtonLayout->addWidget(gui.loadImporterButton);
   importerButtonLayout->setAlignment(gui.loadImporterButton, Qt::AlignLeft);
   gui.saveImporterButton = new QPushButton("Save importer");
   gui.saveImporterButton->setMinimumWidth(fileButtonWidth);
   importerButtonLayout->addWidget(gui.saveImporterButton);
   importerButtonLayout->setAlignment(gui.saveImporterButton, Qt::AlignLeft);
   importerButtonLayout->addStretch(1);

   gui.saveOutputButton = new QPushButton("Save output");
   fileLayout->addWidget(gui.saveOutputButton);
   fileLayout->setAlignment(gui.saveOutputButton, Qt::AlignLeft);
   gui.saveOutputButton->setMinimumWidth(fileButtonWidth);

   // Stages groupbox for pre-process, filter, parser, template, and preview stage buttons to transition between stages
   auto stagesWidget = new QGroupBox(controlsWidget);
   auto stagesLayout = new QGridLayout();
   stagesLayout->setColumnStretch(0, 1);
   stagesLayout->setColumnStretch(1, 1);
   stagesWidget->setLayout(stagesLayout);
   controlsWidget->setStretchFactor(1, 1);
   stagesWidget->setTitle("Process Stages                  Auto");

   gui.stageButtons[static_cast<int>(Stage::PreProcess)] = new QPushButton("Pre-process");
   gui.stageButtons[static_cast<int>(Stage::Filter)] = new QPushButton("Filter");
   gui.stageButtons[static_cast<int>(Stage::Parse)] = new QPushButton("Parser");
   gui.stageButtons[static_cast<int>(Stage::Template)] = new QPushButton("Template");
   gui.stageButtons[static_cast<int>(Stage::Preview)] = new QPushButton("Preview");

   for (size_t stage = 0; stage < sizeof(gui.stageButtons) / sizeof(gui.stageButtons[0]); stage++)
   {
      stagesLayout->addWidget(gui.stageButtons[stage], ut::cast_to_int(stage), 0, Qt::AlignLeft);

      gui.autoProcessCheckBoxes[stage] = new QCheckBox;
      gui.autoProcessCheckBoxes[stage]->setChecked(true);
      stagesLayout->addWidget(gui.autoProcessCheckBoxes[stage], ut::cast_to_int(stage), 1, Qt::AlignRight);
      gui.stageButtons[stage]->setMinimumWidth(fileButtonWidth);
   }

   // Configuration groupbox, which contains the stack of config widgets for each stage
   gui.configureViewWidget = new QGroupBox(controlsWidget);
   gui.configureViewLayout = new QStackedLayout;
   gui.configureViewWidget->setLayout(gui.configureViewLayout);
   controlsWidget->setStretchFactor(2, 10);
   gui.configureViewWidget->setTitle("Configure");

   // Preprocess config widget, and QProcess
   gui.preProcessProcess = new QProcess(theMainWindow);
   gui.preProcessConfigWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::PreProcess)] = new QVBoxLayout;
   gui.preProcessConfigWidget->setLayout(gui.configVBoxLayouts[static_cast<int>(Stage::PreProcess)]);
   gui.configureViewLayout->addWidget(gui.preProcessConfigWidget);

   auto selectPreProcessorWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::PreProcess)]->addWidget(selectPreProcessorWidget);
   auto selectPreProcessorLayout = new QGridLayout;
   selectPreProcessorWidget->setLayout(selectPreProcessorLayout);
   gui.selectPreProcessorButton = new QPushButton("Pre-Processor Program");
   gui.selectPreProcessorButton->setMaximumWidth(fileButtonWidth);
   selectPreProcessorLayout->addWidget(gui.selectPreProcessorButton, 0, 0, Qt::AlignLeft);
   gui.preProcessConfigEdit = new QLineEdit();
   gui.preProcessConfigEdit->setFixedWidth(5 * fileButtonWidth);
   selectPreProcessorLayout->addWidget(gui.preProcessConfigEdit, 0, 1, Qt::AlignLeft);
   selectPreProcessorLayout->setAlignment(Qt::AlignLeft);

   auto preProcessConfigButtonWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::PreProcess)]->addWidget(preProcessConfigButtonWidget);
   auto preProcessButtonLayout = new QHBoxLayout;
   preProcessButtonLayout->setAlignment(Qt::AlignLeft);
   preProcessConfigButtonWidget->setLayout(preProcessButtonLayout);

   // Filter config widget
   gui.filterConfigWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::Filter)] = new QVBoxLayout;
   gui.filterConfigWidget->setLayout(gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]);
   gui.configureViewLayout->addWidget(gui.filterConfigWidget);

   // Filter config add new filter line
   gui.filterConfigAddFilterWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->addWidget(gui.filterConfigAddFilterWidget);
   auto addFilterLayout = new QHBoxLayout;
   addFilterLayout->setAlignment(Qt::AlignLeft);
   gui.filterConfigAddFilterWidget->setLayout(addFilterLayout);
   gui.filterConfigAddFilterButton = new QPushButton("Add new filter");
   gui.filterConfigAddFilterButton->setMinimumWidth(fileButtonWidth);
   gui.filterConfigAddFilterButton->setMaximumWidth(fileButtonWidth);
   addFilterLayout->addWidget(gui.filterConfigAddFilterButton);

   // Parser config widget
   gui.parserConfigWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::Parse)] = new QVBoxLayout;
   gui.parserConfigWidget->setLayout(gui.configVBoxLayouts[static_cast<int>(Stage::Parse)]);
   gui.configureViewLayout->addWidget(gui.parserConfigWidget);

   auto nameWidget = new QWidget;
   auto nameHBoxLayout = new QHBoxLayout;
   nameWidget->setLayout(nameHBoxLayout);
   gui.configVBoxLayouts[static_cast<int>(Stage::Parse)]->addWidget(nameWidget);
   auto parserConfigNameLabel = new QLabel("Column name");
   parserConfigNameLabel->setMinimumWidth(fileButtonWidth / 1.5);
   nameHBoxLayout->addWidget(parserConfigNameLabel);
   gui.parserConfigNameEdit = new QLineEdit("1");
   gui.parserConfigNameEdit->setMaximumWidth(2 * fileButtonWidth);
   nameHBoxLayout->addWidget(gui.parserConfigNameEdit);
   nameHBoxLayout->setAlignment(Qt::AlignLeft);
   nameHBoxLayout->setAlignment(Qt::AlignTop);
   nameHBoxLayout->addStretch(1);

   auto delimiterWidget = new QWidget;
   auto delimiterHBoxLayout = new QHBoxLayout;
   delimiterWidget->setLayout(delimiterHBoxLayout);
   gui.configVBoxLayouts[static_cast<int>(Stage::Parse)]->addWidget(delimiterWidget);
   auto parserConfigDelimiterLabel = new QLabel("Delimiter");
   parserConfigDelimiterLabel->setMinimumWidth(fileButtonWidth / 1.5);
   delimiterHBoxLayout->addWidget(parserConfigDelimiterLabel);
   gui.parserConfigDelimiterEdit = new QLineEdit(",");
   gui.parserConfigDelimiterEdit->setMaximumWidth(2 * fileButtonWidth);
   delimiterHBoxLayout->addWidget(gui.parserConfigDelimiterEdit);
   delimiterHBoxLayout->setAlignment(Qt::AlignLeft);
   delimiterHBoxLayout->setAlignment(Qt::AlignTop);


   // Delimiter handling checkboxes
   gui.parserConfigDiscardCheckBox = new QCheckBox;
   delimiterHBoxLayout->addWidget(gui.parserConfigDiscardCheckBox);
   auto discardDelimiterLabel = new QLabel("Discard delimiter");
   delimiterHBoxLayout->addWidget(discardDelimiterLabel);
   gui.parserConfigDiscardCheckBox->setChecked(true);

   gui.parserConfigConsumeCheckBox = new QCheckBox;
   delimiterHBoxLayout->addWidget(gui.parserConfigConsumeCheckBox);
   auto consumeDelimiterLabel = new QLabel("Consume delimiter");
   delimiterHBoxLayout->addWidget(consumeDelimiterLabel);

   gui.parserConfigMultiLineRecordsCheckBox = new QCheckBox;
   delimiterHBoxLayout->addWidget(gui.parserConfigMultiLineRecordsCheckBox);
   auto parserConfigRegularLinesLabel = new QLabel("Multi Line Inputs");
   delimiterHBoxLayout->addWidget(parserConfigRegularLinesLabel);
   delimiterHBoxLayout->addStretch(1);

   auto typeWidget = new QWidget;
   auto typeHBoxLayout = new QHBoxLayout;
   typeWidget->setLayout(typeHBoxLayout);
   gui.configVBoxLayouts[static_cast<int>(Stage::Parse)]->addWidget(typeWidget);
   auto parserConfigTypeLabel = new QLabel("Data type");
   parserConfigTypeLabel->setMinimumWidth(fileButtonWidth / 1.5);
   typeHBoxLayout->addWidget(parserConfigTypeLabel);
   gui.parserConfigTypeComboBox = new QComboBox;
   gui.parserConfigTypeComboBox->addItems(parserColumnDataTypeList);
   gui.parserConfigTypeComboBox->setMinimumWidth(gui.parserConfigDelimiterEdit->width());
   gui.parserConfigTypeComboBox->setMaximumWidth(gui.parserConfigDelimiterEdit->width());
   typeHBoxLayout->addWidget(gui.parserConfigTypeComboBox);
   typeHBoxLayout->addStretch(1);

   // TODO handle space issues with the parser (remove prepending/internal/postpending spaces or turn spaces into underscores)

   // Template config widget
   gui.templateConfigWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::Template)] = new QVBoxLayout;
   gui.templateConfigWidget->setLayout(gui.configVBoxLayouts[static_cast<int>(Stage::Template)]);
   gui.configureViewLayout->addWidget(gui.templateConfigWidget);

   gui.templateConfigTextEdit = new QPlainTextEdit;
   gui.templateConfigTextEdit->setMaximumWidth(5 * fileButtonWidth);
   gui.configVBoxLayouts[static_cast<int>(Stage::Template)]->addWidget(gui.templateConfigTextEdit);
   // TODO allow user to specify delimiters

   // Preview config widget
   gui.previewConfigWidget = new QWidget;
   gui.configVBoxLayouts[static_cast<int>(Stage::Preview)] = new QVBoxLayout;
   gui.previewConfigWidget->setLayout(gui.configVBoxLayouts[static_cast<int>(Stage::Preview)]);
   gui.configureViewLayout->addWidget(gui.previewConfigWidget);

   auto previewConfigHBoxLayout = new QHBoxLayout;
   gui.configVBoxLayouts[static_cast<int>(Stage::Preview)]->addLayout(previewConfigHBoxLayout);

   auto previewConfigRadioVBoxLayout = new QVBoxLayout;
   previewConfigHBoxLayout->addLayout(previewConfigRadioVBoxLayout);

   gui.previewConfigSingleFileButton = new QRadioButton("Single file output");
   gui.previewConfigSingleFileButton->setFixedWidth(fileButtonWidth);
   previewConfigRadioVBoxLayout->addWidget(gui.previewConfigSingleFileButton);
   gui.previewConfigSingleFileButton->setChecked(true);
   gui.previewConfigMultiFileButton = new QRadioButton("Multi file output");
   gui.previewConfigMultiFileButton->setFixedWidth(fileButtonWidth);
   previewConfigRadioVBoxLayout->addWidget(gui.previewConfigMultiFileButton);

   auto previewConfigFileNameVBoxLayout = new QVBoxLayout;
   previewConfigHBoxLayout->addLayout(previewConfigFileNameVBoxLayout);
   previewConfigHBoxLayout->addStretch(1);
   auto previewConfigFileNameLabel = new QLabel("Output file name specification");
   previewConfigFileNameVBoxLayout->addWidget(previewConfigFileNameLabel);
   gui.previewConfigFileNameLineEdit = new QLineEdit;
   gui.previewConfigFileNameLineEdit->setFixedWidth(4 * fileButtonWidth);
   previewConfigFileNameVBoxLayout->addWidget(gui.previewConfigFileNameLineEdit);

   gui.previewConfigFileNamesTextEdit = new QPlainTextEdit;
   gui.configVBoxLayouts[static_cast<int>(Stage::Preview)]->addWidget(gui.previewConfigFileNamesTextEdit);
   gui.previewConfigFileNamesTextEdit->setReadOnly(true);

   // Auto process buttons and checks for each stage
   for (int stage = 0; stage < gui.configureViewLayout->count(); stage++)
   {
      gui.manualProcessButtons[stage] = new QPushButton;
      gui.manualProcessButtons[stage]->setText("Process");
      gui.manualProcessButtons[stage]->setMaximumWidth(fileButtonWidth);
      gui.manualProcessButtons[stage]->setStyleSheet(getStyleSheet(Color::Todo));
      gui.configVBoxLayouts[stage]->addWidget(gui.manualProcessButtons[stage]);
   }

   // Horizontal splitter between input and output view widgets
   auto viewPaneSplitter = new QSplitter(controlSplitter);
   controlSplitter->setStretchFactor(1, 32);

   // Input groupbox, which contains the stack of input widgets for each stage
   gui.inputViewWidget = new QTabWidget(viewPaneSplitter);
   gui.inputViewLayout = new QStackedLayout(gui.inputViewWidget);

   // Preprocess input widget
   gui.preProcessInputWidget = new QPlainTextEdit;
   gui.preProcessInputWidget->setReadOnly(true);
   gui.inputViewWidget->addTab(gui.preProcessInputWidget, "Pre Processor Input");

   // Filter input widget
   gui.filterInputWidget = new QPlainTextEdit;
   gui.filterInputWidget->setReadOnly(true);
   gui.inputViewWidget->addTab(gui.filterInputWidget, "Filter Input");

   // Parser input widget
   gui.parserInputWidget = new QPlainTextEdit;
   gui.parserInputWidget->setReadOnly(true);
   gui.inputViewWidget->addTab(gui.parserInputWidget, "Parser Input");

   // Template input widget
   gui.templateInputWidget = new QTableWidget;
   gui.templateInputWidget->verticalHeader()->setVisible(false);
   gui.templateInputHeader = gui.templateInputWidget->horizontalHeader();
   gui.inputViewWidget->addTab(gui.templateInputWidget, "Template Input");

   gui.templateInputWidget->setRowCount(static_cast<int>(data.records.size()));
   gui.templateInputWidget->setColumnCount(static_cast<int>(config.parserColumns.size()));
   gui.templateInputWidget->setHorizontalHeaderLabels(toHeaderLabelQStringList(config.parserColumns));
   gui.templateInputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   // Output groupbox, which contains the stack of output widgets for each stage
   gui.outputViewWidget = new QTabWidget(viewPaneSplitter);
   gui.outputViewLayout = new QStackedLayout(gui.outputViewWidget);

   // Preprocess output widget
   gui.preProcessOutputWidget = new QPlainTextEdit;
   gui.preProcessOutputWidget->setReadOnly(true);
   gui.outputViewWidget->addTab(gui.preProcessOutputWidget, "Pre Processor Output");

   // Filter output widget
   gui.filterOutputWidget = new QPlainTextEdit;
   gui.filterOutputWidget->setReadOnly(true);
   gui.outputViewWidget->addTab(gui.filterOutputWidget, "Filter Output");

   // Parser output widget
   gui.parserOutputWidget = new QTableWidget;
   gui.parserOutputWidget->verticalHeader()->setVisible(false);
   gui.parserOutputHeader = gui.parserOutputWidget->horizontalHeader();
   gui.parserOutputHeader->setContextMenuPolicy(Qt::CustomContextMenu);
   //gui.parserOutputHeader->setSectionsMovable(true); // TODO handle sections being moved and prevent the + column from being moved
   gui.outputViewWidget->addTab(gui.parserOutputWidget, "Parser Output");

   gui.parserOutputWidget->setRowCount((int)data.rowCount);
   gui.parserOutputWidget->setColumnCount(static_cast<int>(config.parserColumns.size()) + 1);
   auto stringList = toHeaderLabelQStringList(config.parserColumns);
   stringList << "+";
   gui.parserOutputWidget->setHorizontalHeaderLabels(stringList);
   gui.parserOutputWidget->setColumnWidth(1, 120);
   //gui.parserOutputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   // Template output widget
   gui.templateOutputWidget = new QPlainTextEdit;
   gui.templateOutputWidget->setReadOnly(true);
   gui.outputViewWidget->addTab(gui.templateOutputWidget, "Template Output");

   // Preview output widget
   gui.previewOutputTabWidget = new QTabWidget;
   gui.outputViewWidget->addTab(gui.previewOutputTabWidget, "Preview Output");

   // Error list widget
   auto errorsWidget = new QGroupBox(errorsSplitter);
   errorsSplitter->setStretchFactor(1, 6);
   auto errorsLayout = new QVBoxLayout;
   errorsWidget->setLayout(errorsLayout);
   errorsWidget->setTitle("Errors");

   gui.errorTable = new QTableWidget(0, 1);
   gui.errorTable->verticalHeader()->setVisible(false);
   gui.errorTable->setShowGrid(false);
   gui.errorTable->horizontalHeader()->setVisible(false);
   gui.errorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   gui.errorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
   errorsLayout->addWidget(gui.errorTable);

   // Progress bar & cancel button
   auto progressLayout = new QHBoxLayout;
   windowLayout->addLayout(progressLayout);

   // Progress bar has granularity of 200 per stage
   gui.progressBar = new QProgressBar;
   gui.progressBar->setRange(0, 1000);
   gui.progressBar->setTextVisible(true);
   gui.progressBar->setAlignment(Qt::AlignCenter);
   progressLayout->addWidget(gui.progressBar);

   gui.cancelButton = new QPushButton("Cancel");
   gui.cancelButton->setMinimumWidth(fileButtonWidth);
   progressLayout->addWidget(gui.cancelButton);

   /////////////////////
   // Signals & Slots //
   /////////////////////

   auto openFileButtonClicked = [&gui, &config, &data, &errors, &configStage, &processingStage, this]()
   {
      // First gather the relevant data from the gui
      // TODO configure this browse dialog nicely

      QString fileName = QFileDialog::getOpenFileName(gui.mainWindow, "Input file");

      if (!fileName.isEmpty())
      {
         gui.openFileNameEdit->setText(fileName);

         gui.openFileNameEdit->setToolTip(fileName);
         // TODO files should not be read on the GUI thread
         QFile file(fileName);
         if (file.open(QFile::ReadOnly))
         {
            data.source = file.readAll();
            // TODO did the file read all data successfully?
            file.close();

            gui.preProcessInputWidget->setPlainText(QString::fromLocal8Bit(data.source));
         }
         else
         {
            gui.preProcessInputWidget->setPlainText("Failed to read input file.");
         }
      }
      errors.fileNameErrors.clear();
      errors.filterErrors.clear();
      errors.parserErrors.clear();
      errors.preProcessorErrors.clear();
      errors.templateErrors.clear();

      colorFileButtons();
      processingStage = Stage::PreProcess;
      if (config.parsing)
      {
         cancelStage();
         parserColumnsChanged = true;
         return;
      }
      scenarioImporterProcess();
   };

   QObject::connect(gui.openFileButton, &QPushButton::clicked, openFileButtonClicked);

   auto browseOutputDirectoryButtonClicked = [&gui, this]()
   {
      // TODO configure this browse dialog nicely
      QString dirName = QFileDialog::getExistingDirectory(gui.mainWindow, "Output directory");

      if (!dirName.isEmpty())
      {
         if (dirName.at(dirName.length() - 1) != '/')
         {
            dirName += '/';
         }

         gui.outputDirectoryNameEdit->setText(dirName);
         gui.outputDirectoryNameEdit->setToolTip(dirName);
      }
      colorFileButtons();
   };

   QObject::connect(gui.browseOutputDirectoryButton, &QPushButton::clicked, browseOutputDirectoryButtonClicked);

   auto saveOutputButtonClicked = [&gui, &data, &errors, &config, this]()
   {
      // TODO process all stages which have not been processed yet (except potentially template stage)
      if (gui.previewOutputTabWidget->count() == 0 || gui.previewConfigFileNameLineEdit->text().isEmpty() || gui.outputDirectoryNameEdit->text().isEmpty())
      {
         QMessageBox messageBox;
         messageBox.setText("No output to save! Make sure you have selected an output directory, and that files exist in the preview stage");
         messageBox.setStandardButtons(QMessageBox::Ok);
         messageBox.setDefaultButton(QMessageBox::Ok);
         messageBox.exec();
         return;
      }

      // TODO identify issues with dirName
      const QString dirName = gui.outputDirectoryNameEdit->text();

      for (auto buffer : data.outputBufferMap)
      {
         // TODO files should not be written on the GUI thread
         QString fileName = dirName + buffer.first;
         QFile file(fileName);
         if (file.open(QFile::WriteOnly))
         {
            // TODO gather and report errors
            file.write(buffer.second);
         }
      }
      QMessageBox messageBox;
      QString successSavedText = "Saved files to location: ";
      successSavedText += QString(gui.outputDirectoryNameEdit->text()) + "\n\n";
      successSavedText += QString(gui.previewConfigFileNamesTextEdit->toPlainText());
      messageBox.setText(successSavedText);
      messageBox.setStandardButtons(QMessageBox::Ok);
      messageBox.setDefaultButton(QMessageBox::Ok);
      messageBox.exec();
      gui.saveOutputButton->setStyleSheet(getStyleSheet(Color::Complete));
      return;


   };

   QObject::connect(gui.saveOutputButton, &QPushButton::clicked, saveOutputButtonClicked);


   ///////////////////////////
   // Importer stages slots //
   ///////////////////////////

   auto preProcessStageButtonClicked = [&gui, &configStage, this]()
   {
      // TODO color buttons instead of just setting them as down
      configStage = Stage::PreProcess;
      displayGuiStage();
      scenarioImporterProcess();
   };

   QObject::connect(gui.stageButtons[static_cast<int>(Stage::PreProcess)], &QPushButton::clicked, preProcessStageButtonClicked);


   auto filterStageButtonClicked = [&gui, &configStage, this]()
   {
      configStage = Stage::Filter;
      displayGuiStage();
      scenarioImporterProcess();
   };

   QObject::connect(gui.stageButtons[static_cast<int>(Stage::Filter)], &QPushButton::clicked, filterStageButtonClicked);


   auto parserStageButtonClicked = [&gui, &configStage, this]()
   {
      configStage = Stage::Parse;
      displayGuiStage();
      scenarioImporterProcess();
   };

   QObject::connect(gui.stageButtons[static_cast<int>(Stage::Parse)], &QPushButton::clicked, parserStageButtonClicked);


   auto templateStageButtonClicked = [&gui, &configStage, &processingStage, this]()
   {
      if (parserColumnsChanged)
      {
         processingStage = Stage::Parse;
      }
      configStage = Stage::Template;
      displayGuiStage();
      scenarioImporterProcess();
   };

   QObject::connect(gui.stageButtons[static_cast<int>(Stage::Template)], &QPushButton::clicked, templateStageButtonClicked);


   auto previewStageButtonClicked = [&gui, &configStage, this]()
   {
      configStage = Stage::Preview;
      displayGuiStage();
      scenarioImporterProcess();
   };

   QObject::connect(gui.stageButtons[static_cast<int>(Stage::Preview)], &QPushButton::clicked, previewStageButtonClicked);


   ////////////////////
   // Config Updates //
   ////////////////////

   auto selectPreProcessorButtonClicked = [&gui, &config, &data, &errors, &processingStage, this]()
   {
      QString programName = QFileDialog::getOpenFileName(gui.mainWindow, "Select Pre-Processor Program");
      if (!programName.isEmpty())
      {
         gui.preProcessConfigEdit->setText(programName);
         config.preProcessorCommand = programName;
      }
      processingStage = Stage::PreProcess;
      if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::PreProcess)]->isChecked())
      {
         scenarioImporterProcess();
      }
   };

   QObject::connect(gui.selectPreProcessorButton, &QPushButton::clicked, selectPreProcessorButtonClicked);

   auto preProcessEditUpdated = [&gui, &config, &processingStage, this]()
   {
      config.preProcessorCommand = gui.preProcessConfigEdit->text();
   };

   QObject::connect(gui.preProcessConfigEdit, &QLineEdit::textChanged, preProcessEditUpdated);

   auto preProcessErrorOccured = [&gui, &config, &errors, &processingStage, this](QProcess::ProcessError error)
   {
      std::ifstream processFile(gui.preProcessConfigEdit->text().toStdString());
      if (!processFile.good())
      {
         errors.preProcessorErrors.emplace_back(false, std::string("Could Not Open Processing File"), ScenarioImporter::preProcessorErrorType::fileNameError, 0);
      }
      else
      {
         errors.preProcessorErrors.emplace_back(false, processErrorTypeEnumToString(gui.preProcessProcess->error()), ScenarioImporter::preProcessorErrorType::processError, 0);
      }
      processingStage = Stage::Filter;
      scenarioImporterProcess();
   };

   QObject::connect(gui.preProcessProcess, &QProcess::errorOccurred, preProcessErrorOccured);

   auto filterConfigAddNewFilterButtonClicked = [&gui, this]()
   {
      // Add a filter config widget
      auto filterWidget = new QWidget;
      gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->insertWidget(static_cast<int>(gui.filterConfigChildWidgets.size()), filterWidget);
      auto filterLayout = new QHBoxLayout;
      filterWidget->setLayout(filterLayout);
      auto filterType = new QComboBox;
      filterType->setMaximumWidth(gui.filterConfigAddFilterButton->width() * 1.5);
      filterLayout->setAlignment(filterType, Qt::AlignLeft);
      filterType->addItems(filterTypeList);
      filterLayout->addWidget(filterType);
      auto filterSpec = new QLineEdit;
      filterSpec->setMaximumWidth(gui.filterConfigAddFilterButton->width() * 1.5);
      filterLayout->setAlignment(filterSpec, Qt::AlignLeft);
      filterLayout->addWidget(filterSpec);
      auto filterDeleteButton = new QPushButton("-");
      filterDeleteButton->setMaximumWidth(gui.filterConfigAddFilterButton->width() / 3);
      filterLayout->setAlignment(filterSpec, Qt::AlignLeft);
      filterLayout->addWidget(filterDeleteButton);
      gui.filterConfigChildWidgets.emplace_back(filterType, filterSpec);
      filterLayout->addStretch(1);

      QObject::connect(filterSpec, &QLineEdit::editingFinished, this, &ScenarioImporter::Plugin::filterConfigUpdated);
      QObject::connect(filterType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ScenarioImporter::Plugin::filterConfigUpdatedInt);
      QObject::connect(filterDeleteButton, &QPushButton::clicked, this, &ScenarioImporter::Plugin::removeFilterButtonClicked);
   };

   QObject::connect(gui.filterConfigAddFilterButton, &QPushButton::clicked, filterConfigAddNewFilterButtonClicked);

   // Parser stage config
   auto parserOutputHeaderSectionClicked = [&gui, &config, &data, &errors, &configStage, &processingStage, this](int logicalIndex)
   {
      // Add a parser column
      if (logicalIndex == static_cast<int>(config.parserColumns.size()))
      {
         config.parserColumns.emplace_back(std::to_string(logicalIndex + 1), ',', ScenarioImporter::ParserColumnDatatype::String, true, false);
         UpdateGuiParserColumns();
         if (config.parsing)
         {
            cancelStage();
            parserColumnsChanged = true;
            return;
         }
         QCoreApplication::processEvents(QEventLoop::AllEvents);

         if (processingStage > Stage::Parse)
         {
            processingStage = Stage::Parse;
         }
         if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Parse)]->isChecked())
         {
            scenarioImporterProcess();
         }
      }

      // Update selected column index
      if (size_t(logicalIndex) < config.parserColumns.size())
      {
         gui.selectedParserColumnIndex = size_t(logicalIndex);
         ScenarioImporter::ParserColumn column = config.parserColumns.at(gui.selectedParserColumnIndex);

         // Update parser config displayed information
         gui.parserConfigNameEdit->setText(QString::fromStdString(column.attributeKey));
         gui.parserConfigDelimiterEdit->setText(QString(QChar(column.delimiter)));
         gui.parserConfigTypeComboBox->setCurrentIndex(int(column.datatype));
         gui.parserConfigDiscardCheckBox->setChecked(column.discardDelimiters);
         gui.parserConfigConsumeCheckBox->setChecked(column.consumeDelimiters);
         gui.parserConfigConsumeCheckBox->setDisabled(gui.parserConfigDiscardCheckBox->isChecked());
         gui.parserOutputWidget->selectColumn(logicalIndex);
         if (config.parsing)
         {
            cancelStage();
            parserColumnsChanged = true;
            return;
         }
         if (processingStage > Stage::Parse)
         {
            processingStage = Stage::Parse;
         }
         if (parserColumnsChanged)
         {
            scenarioImporterProcess();
         }
      }
   };

   QObject::connect(gui.parserOutputHeader, &QHeaderView::sectionPressed, parserOutputHeaderSectionClicked);

   // Parser stage config - slots defined in ScenarioImporter
   QObject::connect(gui.parserConfigNameEdit, &QLineEdit::editingFinished, this, &ScenarioImporter::Plugin::parserConfigUpdated);
   QObject::connect(gui.parserConfigNameEdit, &QLineEdit::returnPressed, this, &ScenarioImporter::Plugin::parserConfigUpdated);
   QObject::connect(gui.parserConfigDelimiterEdit, &QLineEdit::textEdited, this, &ScenarioImporter::Plugin::parserConfigUpdated);
   QObject::connect(gui.parserConfigDiscardCheckBox, &QCheckBox::clicked, this, &ScenarioImporter::Plugin::parserConfigUpdated);
   QObject::connect(gui.parserConfigConsumeCheckBox, &QCheckBox::clicked, this, &ScenarioImporter::Plugin::parserConfigUpdated);

   QObject::connect(gui.parserConfigMultiLineRecordsCheckBox, &QCheckBox::clicked, this, &ScenarioImporter::Plugin::parserConfigUpdated);
   QObject::connect(gui.parserConfigTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &ScenarioImporter::Plugin::parserConfigUpdatedInt);

   // Template stage config - slots defined in ScenarioImporter
   QObject::connect(gui.templateConfigTextEdit, &QPlainTextEdit::textChanged, this, &ScenarioImporter::Plugin::templateConfigUpdated);

   // Preview stage config - slots defined in ScenarioImporter
   QObject::connect(gui.previewConfigFileNameLineEdit, &QLineEdit::editingFinished, this, &ScenarioImporter::Plugin::previewConfigUpdated);
   QObject::connect(gui.previewConfigSingleFileButton, &QRadioButton::pressed, this, &ScenarioImporter::Plugin::previewConfigUpdatedBool);
   QObject::connect(gui.previewConfigMultiFileButton, &QRadioButton::pressed, this, &ScenarioImporter::Plugin::previewConfigUpdatedBool);

   for (QCheckBox* checkbox : gui.autoProcessCheckBoxes)
   {
      QObject::connect(checkbox, &QCheckBox::toggled, this, &ScenarioImporter::Plugin::configStageSetAuto);
      checkbox->setChecked(true);
   }
   for (QPushButton* manualProcessButton : gui.manualProcessButtons)
   {
      QObject::connect(manualProcessButton, &QPushButton::pressed, this, &ScenarioImporter::Plugin::configStageManualUpdated);
   }
   ///////////////////////////////
   // Delete Parser Column Slot //
   ///////////////////////////////

   // Offer option to delete parser column on right-click of existing column
   auto parserOutputHeaderContextMenuRequested = [&gui, &config, &processingStage, this](const QPoint & point)
   {
      int logicalIndex = gui.parserOutputHeader->logicalIndexAt(point);
      // At least one parser column must remain
      if (logicalIndex < static_cast<int>(config.parserColumns.size()) && config.parserColumns.size() > 1)
      {
         QMenu menu;
         QAction* deleteAction = menu.addAction("Delete parser column");
         QObject::connect(deleteAction, &QAction::triggered, this, [this, logicalIndex] { removeParserColumnOptionChosen(logicalIndex); });
         menu.exec(QCursor::pos());
      }
      if (logicalIndex == static_cast<int>(config.parserColumns.size()))
      {
         QMenu menu;
         QAction* setColumnsAction = menu.addAction("Set Number of Columns");
         QObject::connect(setColumnsAction, &QAction::triggered, this, [this, logicalIndex] { setParserColumnsOptionChosen(logicalIndex); });
         menu.exec(QCursor::pos());
      }
   };
   QObject::connect(gui.parserOutputHeader, &QHeaderView::customContextMenuRequested, parserOutputHeaderContextMenuRequested);


   ///////////////////////
   // Save Config Slots //
   ///////////////////////

   auto saveImporterButtonClicked = [&gui, &config, this]()
   {
      QString fileName = QFileDialog::getSaveFileName(gui.mainWindow, "Save importer configuration file");
      if (!fileName.isEmpty())
      {
         QFile file(fileName);
         if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
         {
            QTextStream out(&file);
            // Save the path to the selected Pre Processor command
            if (!config.preProcessorCommand.isEmpty())
            {
               out << "Processor command," << config.preProcessorCommand;
               out << endl;
            }

            // Do not Save processingStage and currentStage
            //out << "Stages, " << static_cast<int>(config.processingStage) << ','
            //<< static_cast<int>(config.currentStage) << endl;

            // Save data needed to reconstruct filters
            if (!config.filters.empty())
            {
               out << "Filters, " << config.filters.size() << ',';
               for (auto& filter : config.filters)
               {
                  out << QString::fromStdString(commaSeparatedStringFromFilter(filter.get())) << ',';
               }
               out << endl;
            }

            // Save data needed to reconstruct ParserColumn's
            if (!config.parserColumns.empty())
            {
               out << "Parser columns, " << config.parserColumns.size() << ',';
               for (auto& column : config.parserColumns)
               {
                  out << QString::fromStdString(commaSeparatedStringFromParserColumn(column)) << ',';
               }
               out << endl;
               out << "Delimiter settings, " << config.parserColumns.size() << ','; // two delimiter settings per column
               for (auto& column : config.parserColumns)
               {
                  (column.discardDelimiters) ? out << "true," : out << "false,";
                  (column.consumeDelimiters) ? out << "true," : out << "false,";
               }
               (config.multiLineRecords) ? out << "true," : out << "false,"; // plus one for multiLine
               out << endl;
            }

            if (config.templateString.size() > 0)
            {
               out << "Template,"
                   << QString::fromStdString(commaSeparatedStringFromTemplateByteArray(config.templateString))
                   << endl;
            }
            if (config.outputFileNameString.size() > 0)
            {
               out << "Output file, " << config.outputFileNameString << endl;
            }
            //if (config.saveConfigFileNameString.size() > 0)
            //{
            //out << "Save config file, " << config.saveConfigFileNameString << endl;
            //}

            out << "Multi file output, " << config.outputMultiFile << endl;

            file.close();
         }
         else
         {
            QMessageBox fileOpenMsg;
            fileOpenMsg.setText("Could not open file.");
            fileOpenMsg.exec();
         }
      }
   };

   QObject::connect(gui.saveImporterButton, &QPushButton::clicked, saveImporterButtonClicked);

   auto loadImporterButtonClicked = [&gui, &config, &data, &errors, &configStage, this]()
   {
      QString fileName = QFileDialog::getOpenFileName(gui.mainWindow, "Load importer configuration file");
      if (!fileName.isEmpty())
      {
         QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         {
            QMessageBox fileOpenMsg;
            fileOpenMsg.setText("Configuration file could not be opend.");
            fileOpenMsg.exec();
            return;
         }

         QTextStream inStream(&file);
         QString line;
         Config tempConfig;
         bool success = true;

         do
         {
            line = inStream.readLine();
            success &= processSavedConfigLine(tempConfig, line);
         }
         while (!line.isNull() && success);

         if (success)
         {
            // TODO Make sure this populates the gui correctly. ScenarioImporterProcess should handle data processing and input/output display.

            config.filters = std::move(tempConfig.filters);
            config.parserColumns = tempConfig.parserColumns;
            config.multiLineRecords = tempConfig.multiLineRecords;
            config.templateString = tempConfig.templateString;
            config.outputFileNameString = tempConfig.outputFileNameString;
            config.outputMultiFile = tempConfig.outputMultiFile;
            config.preProcessorCommand = tempConfig.preProcessorCommand;

            this->processingStage = Stage::PreProcess;

            // Update data and gui based on the processor arguments that were loaded into config
            gui.preProcessConfigEdit->setText(config.preProcessorCommand);

            // Update data and gui based on the filters that were loaded into config
            UpdateGuiFilters();

            // Update data and gui based on the parser columns that were loaded into config
            UpdateGuiParserColumns();

            // Update data and gui based on the template that was loaded into config
            gui.templateConfigTextEdit->setPlainText(config.templateString);

            gui.previewConfigFileNameLineEdit->setText(config.outputFileNameString);

            gui.previewConfigMultiFileButton->setChecked(config.outputMultiFile);
            gui.previewConfigSingleFileButton->setChecked(!config.outputMultiFile);

            gui.parserConfigMultiLineRecordsCheckBox->setChecked(config.multiLineRecords);
            for (QCheckBox* checkbox : gui.autoProcessCheckBoxes)
            {
               checkbox->setChecked(true);
            }
            // TODO something still seems off here. More testing.
            scenarioImporterProcess();
         }
         else
         {
            QMessageBox invalidInputMsg;
            invalidInputMsg.setText("Could not load configuration: Selected file contains invalid input.");
            invalidInputMsg.exec();
         }
         file.close();
         fileName.clear();
      }
   };

   QObject::connect(gui.loadImporterButton, &QPushButton::clicked, loadImporterButtonClicked);

   auto errorTableClicked = [&gui, &errors, &configStage, this](int row, int col)
   {
      int processorsEnd = static_cast<int>(errors.preProcessorErrors.size());
      int filtersEnd = processorsEnd + static_cast<int>(errors.filterErrors.size());
      int parsersEnd = filtersEnd + static_cast<int>(errors.parserErrors.size());
      int templatesEnd = parsersEnd + static_cast<int>(errors.templateErrors.size());

      if (row < processorsEnd)
      {
         configStage = Stage::PreProcess;
         displayGuiStage();
         if (errors.preProcessorErrors[row].type == ScenarioImporter::preProcessorErrorType::fileNameError)
         {
            gui.preProcessConfigEdit->selectAll();
            gui.preProcessConfigEdit->setFocus(Qt::OtherFocusReason);
         }
      }
      else if (row < filtersEnd)
      {
         row -= processorsEnd;
         configStage = Stage::Filter;
         displayGuiStage();
         gui.filterConfigChildWidgets[errors.filterErrors[row].filterIdx].second->selectAll();
         gui.filterConfigChildWidgets[errors.filterErrors[row].filterIdx].second->setFocus(Qt::OtherFocusReason);
      }
      else if (row < parsersEnd)
      {
         configStage = Stage::Parse;
         displayGuiStage();
         row -= filtersEnd;
         if (gui.parserOutputWidget->item(errors.lastParserErrorViewed.first, errors.lastParserErrorViewed.second) != nullptr)
         {
            gui.parserOutputWidget->item(errors.lastParserErrorViewed.first, errors.lastParserErrorViewed.second)->setSelected(false);
         }
         if (gui.parserOutputWidget->item(errors.parserErrors[row].row, errors.parserErrors[row].col))
         {
            gui.parserOutputWidget->item(errors.parserErrors[row].row, errors.parserErrors[row].col)->setSelected(true);
            gui.parserOutputWidget->setFocus(Qt::OtherFocusReason);
            gui.parserOutputWidget->scrollToItem(gui.parserOutputWidget->item(errors.parserErrors[row].row, errors.parserErrors[row].col),
                                                 QAbstractItemView::ScrollHint::PositionAtCenter);
            errors.lastParserErrorViewed = { errors.parserErrors[row].row, errors.parserErrors[row].col };
         }
      }
      else if (row < templatesEnd)
      {
         configStage = Stage::Template;
         displayGuiStage();
         row -= parsersEnd;
         QTextCursor cursor = gui.templateConfigTextEdit->textCursor();
         cursor.setPosition(errors.templateErrors[row].begin, QTextCursor::MoveAnchor);
         cursor.setPosition(errors.templateErrors[row].end, QTextCursor::KeepAnchor);
         gui.templateConfigTextEdit->setTextCursor(cursor);
         gui.templateConfigTextEdit->setFocus(Qt::OtherFocusReason);
      }
      else
      {
         configStage = Stage::Preview;
         displayGuiStage();
         row -= templatesEnd;
         gui.previewConfigFileNameLineEdit->setSelection(errors.fileNameErrors[row].begin, errors.fileNameErrors[row].end - errors.fileNameErrors[row].begin);
      }
   };

   QObject::connect(gui.errorTable, &QTableWidget::cellClicked, errorTableClicked);
   QObject::connect(gui.cancelButton, &QPushButton::clicked, this, &ScenarioImporter::Plugin::cancelStage);

   QObject::connect(gui.mainWindow, &ImporterMainWindow::close, this, &ScenarioImporter::Plugin::windowClosed);

   parserColumnsChanged = false;
   stageCanceled = false;
   config.parsing = false;
   configStage = Stage::PreProcess;
   displayGuiStage();

   // TODO remove this warning evading code
   equal(stringSliceFromCString("alpha"), stringSliceFromCString("beta"));
   // TODO remove this warning evading code
}

void ScenarioImporter::Plugin::UnloadPlugin()
{
}

void ScenarioImporter::Plugin::LaunchScenarioImporterToolFromMenu()
{
   // Re-load plugin when window is opened/re-opened so any changes made
   // between the last time it was open (or startup) are captured (e.g. theme changes)
   LoadPlugin();

   gui.mainWindow->showMaximized();
   gui.mainWindow->activateWindow();
}

void ScenarioImporter::Plugin::windowClosed()
{
   // Clear members of Config
   config.filters.clear();
   config.outputFileNameString.clear();
   config.parserColumns.clear();
   config.preProcessorCommand.clear();
   config.templateString.clear();
   // Clear any filter gui objects which were created, some
   // members don't get attached to the gui hierarchy
   UpdateGuiFilters();

   // Clears all Gui members by Qt parent child relations
   delete gui.mainWindow;

   // Clear members of data
   data.source.clear();
   data.preProcessedData.clear();
   data.filteredData.clear();
   data.exampleRecord.clear();
   data.records.clear();
   data.outputBuffer.clear();
   data.outputBufferMap.clear();

   // Clear members of errors
   errors.fileNameErrors.clear();
   errors.filterErrors.clear();
   errors.parserErrors.clear();
   errors.preProcessorErrors.clear();
   errors.templateErrors.clear();
}

void ScenarioImporter::Plugin::displayGuiStage()
{
   for (int i = 0; i < static_cast<int>(sizeof(gui.stageButtons) / sizeof(gui.stageButtons[0])); i++)
   {
      gui.stageButtons[i]->setDown(static_cast<int>(configStage) == i);
   }
   if (static_cast<int>(configStage) == 4)
   {
      gui.inputViewWidget->setCurrentIndex(static_cast<int>(configStage) - 1);
   }
   else
   {
      gui.inputViewWidget->setCurrentIndex(static_cast<int>(configStage));
   }
   gui.outputViewWidget->setCurrentIndex(static_cast<int>(configStage));
   gui.configureViewLayout->setCurrentIndex(static_cast<int>(configStage));
}

void ScenarioImporter::Plugin::UpdateGuiFilters()
{
   // Delete all current filter widgets but leave the "add filter" widget
   for (size_t i = 0; i < gui.filterConfigChildWidgets.size(); ++i)
   {
      QWidget* filterWidget;
      QLayoutItem* filterLayoutItem = gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->itemAt(0);

      if (filterLayoutItem && filterLayoutItem->widget())
      {
         filterWidget = filterLayoutItem->widget();
         QLineEdit* lineEdit = filterWidget->findChild<QLineEdit*>();
         if (lineEdit)
         {
            std::string text = lineEdit->text().toStdString();
         }
         gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->removeWidget(filterWidget);
         filterWidget->setVisible(false);
         delete filterWidget;
      }
   }

   gui.filterConfigChildWidgets.clear();

   for (auto& i : config.filters)
   {
      ScenarioImporter::Filter* filter = i.get();
      // Add a filter config widget
      auto filterWidget = new QWidget;
      gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->insertWidget(static_cast<int>(gui.filterConfigChildWidgets.size()), filterWidget);
      auto filterLayout = new QHBoxLayout;
      filterWidget->setLayout(filterLayout);

      auto filterType = new QComboBox;
      filterType->setMaximumWidth(gui.filterConfigAddFilterButton->width() * 1.5);
      filterType->addItems(filterTypeList);
      int filterTypeIdx = filterType->findText(getFilterType(filter));
      if (filterTypeIdx == -1)
      {
         // TODO log error
         return;
      }
      filterType->setCurrentIndex(filterTypeIdx);
      filterLayout->addWidget(filterType);
      filterLayout->setAlignment(filterType, Qt::AlignLeft);

      auto filterSpec = new QLineEdit;
      filterSpec->setMaximumWidth(gui.filterConfigAddFilterButton->width() * 1.5);
      bool saving = false;
      filterSpec->setText(QString::fromStdString(filter->GetSpecString(saving)));
      filterLayout->addWidget(filterSpec);
      filterLayout->setAlignment(filterSpec, Qt::AlignLeft);

      auto filterDeleteButton = new QPushButton("-");
      filterDeleteButton->setMaximumWidth(gui.filterConfigAddFilterButton->width() / 3);
      filterLayout->addWidget(filterDeleteButton);
      filterLayout->setAlignment(filterDeleteButton, Qt::AlignLeft);
      filterLayout->addStretch(1);

      gui.filterConfigChildWidgets.emplace_back(filterType, filterSpec);

      QObject::connect(filterSpec, &QLineEdit::editingFinished, this, &ScenarioImporter::Plugin::filterConfigUpdated);
      QObject::connect(filterType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ScenarioImporter::Plugin::filterConfigUpdatedInt);
      QObject::connect(filterDeleteButton, &QPushButton::clicked, this, &ScenarioImporter::Plugin::removeFilterButtonClicked);
   }
}

void ScenarioImporter::Plugin::UpdateGuiParserColumns()
{
   // Ensure that correct # of columns of parsed content are maintained

   gui.templateInputWidget->setColumnCount(static_cast<int>(config.parserColumns.size()));
   gui.templateInputWidget->setHorizontalHeaderLabels(toHeaderLabelQStringList(config.parserColumns));
   // Allocate another empty column for the "+" button
   gui.parserOutputWidget->setColumnCount(static_cast<int>(config.parserColumns.size()) + 1);
   auto stringList = toHeaderLabelQStringList(config.parserColumns);
   stringList << "+";
   gui.parserOutputWidget->setHorizontalHeaderLabels(stringList);

   for (int col = 0; col < gui.parserOutputWidget->columnCount(); col++)
   {
      gui.templateInputWidget->resizeColumnToContents(col);
      gui.parserOutputWidget->resizeColumnToContents(col);
   }
   gui.templateInputWidget->repaint();
   gui.parserOutputWidget->repaint();
   gui.parserOutputWidget->setColumnWidth(gui.parserOutputWidget->columnCount() - 1, 120);
}

// Slots for updating configuration filters

void ScenarioImporter::Plugin::cancelStage()
{
   stageCanceled = true;
}

void ScenarioImporter::Plugin::configStageManualUpdated()
{
   int stageIndex = 0;
   int i = 0;
   for (auto button : gui.manualProcessButtons)
   {
      if (button == QObject::sender())
      {
         stageIndex = i;
      }
      i++;
   }

   processingStage = Stage(stageIndex);
   scenarioImporterProcess();
}

void ScenarioImporter::Plugin::configStageSetAuto(bool update)
{
   int stageIndex = 0;
   int i = 0;
   for (auto button : gui.autoProcessCheckBoxes)
   {
      if (button == QObject::sender())
      {
         stageIndex = i;
      }
      i++;
   }

   gui.manualProcessButtons[stageIndex]->setDisabled(update);
}

void ScenarioImporter::Plugin::preProcessConfigUpdated()
{
   if (processingStage > Stage::PreProcess)
   {
      processingStage = Stage::PreProcess;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::PreProcess)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::preProcessConfigUpdatedInt(int i)
{
   preProcessConfigUpdated();
}

void ScenarioImporter::Plugin::filterConfigUpdated()
{
   readFilterConfig(config.filters, errors.filterErrors, gui.filterConfigChildWidgets);
   if (processingStage > Stage::Filter)
   {
      processingStage = Stage::Filter;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Filter)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::filterConfigUpdatedInt(int i)
{
   filterConfigUpdated();
}

// Called for changes which may or may not trigger a processing call (focus events)
// and quickly return to allow the current scenarioImporterProcess loop to re-start
void ScenarioImporter::Plugin::parserConfigUpdated()
{
   QCoreApplication::processEvents();
   gui.parserConfigConsumeCheckBox->setDisabled(gui.parserConfigDiscardCheckBox->isChecked());
   bool delimitersAreUnchanged = true;
   if (gui.parserConfigDiscardCheckBox->isChecked() != config.parserColumns.at(gui.selectedParserColumnIndex).discardDelimiters ||
       gui.parserConfigConsumeCheckBox->isChecked() != config.parserColumns.at(gui.selectedParserColumnIndex).consumeDelimiters ||
       gui.parserConfigMultiLineRecordsCheckBox->isChecked() != config.multiLineRecords)
   {
      config.multiLineRecords = gui.parserConfigMultiLineRecordsCheckBox->isChecked();
      delimitersAreUnchanged = false;
   }

   // Unfortunate method to differentiate between changes that will require a re-processing (focusEvents)
   if (gui.parserConfigNameEdit->text() == QString::fromStdString(config.parserColumns[gui.selectedParserColumnIndex].attributeKey) &&
       gui.parserConfigDelimiterEdit->text() == config.parserColumns[gui.selectedParserColumnIndex].delimiter &&
       gui.parserConfigTypeComboBox->currentIndex() == static_cast<int>(config.parserColumns[gui.selectedParserColumnIndex].datatype) && delimitersAreUnchanged)
   {
      return;
   }
   // In name / delimiter / consumeDelimiter / multiLine changed cases, end early so that headerSectionClicked can display any column change before the importerProcess runs
   // It wont run at all if the headercolumn has not been changed, but will inevitably run once another trigger is met (like moving to the next stage)
   if (gui.parserConfigNameEdit->text() != QString::fromStdString(config.parserColumns[gui.selectedParserColumnIndex].attributeKey) ||
       gui.parserConfigDelimiterEdit->text() != config.parserColumns[gui.selectedParserColumnIndex].delimiter)
   {
      readParserConfig(config.parserColumns.at(gui.selectedParserColumnIndex), gui.selectedParserColumnIndex, gui.parserConfigNameEdit, gui.parserConfigDelimiterEdit, gui.parserConfigTypeComboBox,
                       gui.parserConfigDiscardCheckBox, gui.parserConfigConsumeCheckBox);
      UpdateGuiParserColumns();
      // Tell a currently running scenarioImporterProcess to repeat its call to processParser()
      parserColumnsChanged = true;
      return;
   }
   readParserConfig(config.parserColumns.at(gui.selectedParserColumnIndex), gui.selectedParserColumnIndex, gui.parserConfigNameEdit, gui.parserConfigDelimiterEdit, gui.parserConfigTypeComboBox,
                    gui.parserConfigDiscardCheckBox, gui.parserConfigConsumeCheckBox);
   UpdateGuiParserColumns();

   if (processingStage > Stage::Parse)
   {
      processingStage = Stage::Parse;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Parse)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::parserConfigUpdatedInt(int i)
{
   parserConfigUpdated();
}

void ScenarioImporter::Plugin::templateConfigUpdated()
{
   readTemplateConfig(config.templateString, gui.templateConfigTextEdit);
   if (processingStage > Stage::Template)
   {
      processingStage = Stage::Template;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Template)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::previewConfigUpdated()
{
   QCoreApplication::processEvents();
   if (config.outputFileNameString != gui.previewConfigFileNameLineEdit->text().toUtf8())
   {
      readPreviewConfig(config.outputFileNameString, gui.previewConfigFileNameLineEdit);
      if (processingStage > Stage::Preview)
      {
         processingStage = Stage::Preview;
      }

      if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Preview)]->isChecked())
      {
         scenarioImporterProcess();
      }
   }
}

void ScenarioImporter::Plugin::previewConfigUpdatedBool()
{
   if (sender() == gui.previewConfigMultiFileButton)
   {
      gui.previewConfigSingleFileButton->setChecked(false);
      config.outputMultiFile = true;
   }
   else
   {
      gui.previewConfigMultiFileButton->setChecked(false);
      config.outputMultiFile = false;
   }
   if (processingStage > Stage::Preview)
   {
      processingStage = Stage::Preview;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Preview)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::removeFilterButtonClicked()
{
   QPushButton* senderButton = qobject_cast<QPushButton*>(sender());
   QObject* parent = senderButton->parent();;
   QWidget* filterWidget = qobject_cast<QWidget*>(parent);

   if (filterWidget)
   {
      QComboBox* filterType = filterWidget->findChild<QComboBox*>();
      QLineEdit* filterSpec = filterWidget->findChild<QLineEdit*>();

      if (filterType && filterSpec)
      {
         for (auto iter = gui.filterConfigChildWidgets.begin(); iter != gui.filterConfigChildWidgets.end(); ++iter)
         {
            // TODO pointer comparison is not the best way to do this - talk to NH about gui.filterConfigChildWidgets
            if (iter->first == filterType && iter->second == filterSpec)
            {
               gui.filterConfigChildWidgets.erase(iter);
               break;
            }
         }
         gui.configVBoxLayouts[static_cast<int>(Stage::Filter)]->removeWidget(filterWidget);
         filterWidget->setVisible(false);
         delete filterWidget;
      }
   }
   filterConfigUpdated();
}

void ScenarioImporter::Plugin::removeParserColumnOptionChosen(int logicalIndex)
{
   // At least one parser column must remain
   if (config.parserColumns.size() > 1 &&  ut::cast_to_size_t(logicalIndex) < config.parserColumns.size())
   {
      config.parserColumns.erase(config.parserColumns.begin() + logicalIndex);
      // Adjust parser column names to ensure that integer names are reasonable (no duplicates & in order)
      for (size_t i = 0; i != config.parserColumns.size(); ++i)
      {
         std::string currentName = config.parserColumns[i].attributeKey;
         if (atoi(currentName.c_str()))
         {
            config.parserColumns[i].attributeKey = std::to_string(i + 1);
         }
      }
      // Update selected column index to last column to ensure that it's a valid index
      // and to ensure that parserUpdateConfig responds correctly.
      gui.selectedParserColumnIndex = 0;
      UpdateGuiParserColumns();
      if (config.parsing)
      {
         cancelStage();
         parserColumnsChanged = true;
         return;
      }

      if (processingStage > Stage::Parse)
      {
         processingStage = Stage::Parse;
      }

      if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Parse)]->isChecked())
      {
         scenarioImporterProcess();
      }
   }
}

void ScenarioImporter::Plugin::setParserColumnsOptionChosen(int logicalIndex)
{
   bool ok;
   int newColumns = QInputDialog::getInt(gui.parserOutputWidget, tr(""), tr("Number of Columns:"), 1, 1, 50, 1, &ok);
   if (ok)
   {
      for (int i = 0; i < newColumns; i++)
      {
         if (ut::cast_to_size_t(i) < config.parserColumns.size())
         {
            // NH Why would we reset the parser columns we already have?
            //config.parserColumns[i] = ParserColumn{ config.parserColumns[i].attributeKey, ',', config.parserColumns[i].datatype };
         }
         else
         {
            config.parserColumns.emplace_back(std::to_string(i + 1), ',', ScenarioImporter::ParserColumnDatatype::String, true, true);
         }
      }
      config.parserColumns.erase(config.parserColumns.begin() + newColumns, config.parserColumns.end());
   }
   else
   {
      return;
   }

   if (gui.selectedParserColumnIndex >= ut::cast_to_size_t(newColumns))
   {
      gui.selectedParserColumnIndex = newColumns - 1;
      ScenarioImporter::ParserColumn column = config.parserColumns.at(gui.selectedParserColumnIndex);

      // Update parser config displayed information
      gui.parserConfigNameEdit->setText(QString::fromStdString(column.attributeKey));
      gui.parserConfigDelimiterEdit->setText(QString(QChar(column.delimiter)));
      gui.parserConfigTypeComboBox->setCurrentIndex(int(column.datatype));
      gui.parserConfigDiscardCheckBox->setChecked(column.discardDelimiters);
      gui.parserConfigConsumeCheckBox->setChecked(column.consumeDelimiters);
      gui.parserConfigConsumeCheckBox->setDisabled(gui.parserConfigDiscardCheckBox->isChecked());
      gui.parserOutputWidget->selectColumn(static_cast<int>(gui.selectedParserColumnIndex));
   }
   UpdateGuiParserColumns();
   if (config.parsing)
   {
      cancelStage();
      parserColumnsChanged = true;
      return;
   }
   QCoreApplication::processEvents(QEventLoop::AllEvents);

   if (processingStage > Stage::Parse)
   {
      processingStage = Stage::Parse;
   }
   if (gui.autoProcessCheckBoxes[static_cast<int>(Stage::Parse)]->isChecked())
   {
      scenarioImporterProcess();
   }
}

void ScenarioImporter::Plugin::colorFileButtons()
{
   // We already made sure the file was good when it was selected
   if (!gui.openFileNameEdit->text().isEmpty())
   {
      gui.openFileButton->setStyleSheet(getStyleSheet(Color::Complete));
      gui.browseOutputDirectoryButton->setStyleSheet(getStyleSheet(Color::Todo));
   }
   if (!gui.outputDirectoryNameEdit->text().isEmpty())
   {
      gui.browseOutputDirectoryButton->setStyleSheet(getStyleSheet(Color::Complete));
      gui.saveOutputButton->setStyleSheet(getStyleSheet(Color::Todo));
   }
}

QStringList ScenarioImporter::Plugin::toHeaderLabelQStringList(const std::vector<ScenarioImporter::ParserColumn>& parserColumns)
{
   QStringList list;

   for (const ScenarioImporter::ParserColumn& column : parserColumns)
   {
      list << toQString(column);
   }

   return list;
}

bool ScenarioImporter::Plugin::populateParserTableWidget()
{
   auto rowCount = static_cast<int>(data.records.size());
   auto colCount = static_cast<int>(config.parserColumns.size());

   gui.parserOutputWidget->setColumnCount(colCount + 1);
   gui.templateInputWidget->setColumnCount(colCount);

   // First, update row count
   gui.parserOutputWidget->setRowCount(rowCount);
   gui.templateInputWidget->setRowCount(rowCount);

   // Fill the parser output tablewidget cells
   for (int row = 0; row < rowCount; ++row)
   {
      for (int column = 0; column < colCount; ++column)
      {
         if (rowCount > 100)
         {
            if (row % (rowCount / 100) == 0)
            {
               gui.progressBar->setValue(ScenarioImporter::Plugin::ProgressValue::ParserProcessFinished + (100 * row) / rowCount);
            }
         }
         QCoreApplication::processEvents(QEventLoop::AllEvents);
         if (stageCanceled || ut::cast_to_size_t(column) >= config.parserColumns.size())
         {
            gui.progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::ParserProcessFinished));
            return false;
         }
         // TODO handle errors
         auto text = toQString(ScenarioImporter::getAttribute(config.parserColumns.at(column).attributeKey, data.records[row]));

         QTableWidgetItem* item = new QTableWidgetItem(text);
         gui.parserOutputWidget->setItem(row, column, item);

         item = new QTableWidgetItem(text);
         gui.templateInputWidget->setItem(row, column, item);

      }
   }

   gui.progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::ParserFinished));
   parserColumnsChanged = false;
   return (rowCount != 0);
}

bool ScenarioImporter::Plugin::generateOutputMap(
   StringSlice fileNameSlice,
   StringSlice templateSlice,
   Record& exampleRecord,
   const std::vector<Record>& records,
   std::vector<ScenarioImporter::TemplateError>& fileNameErrors,
   std::vector<ScenarioImporter::TemplateError>& templateErrors,
   std::map<const QByteArray, QByteArray>& outputBufferMap,
   QProgressBar* progressBar)
{
   ScenarioImporter::OutputTemplate fileNameTemplate;
   ScenarioImporter::OutputTemplate outputTemplate;


   if (fileNameTemplate.specifySyntax(fileNameSlice, exampleRecord, fileNameErrors) &&
       outputTemplate.specifySyntax(templateSlice, exampleRecord, templateErrors))
   {
      int num = 0;
      ut::Random random;
      random.SetSeed(1000 * clock() + clock() / 1000);

      for (const auto& record : records)
      {
         QByteArray fileNameBuffer;
         fileNameTemplate.writeRecord(record, fileNameBuffer, templateErrors, num, fileNameTemplate.tokens, random, true);

         auto bufferPair = outputBufferMap.find(fileNameBuffer);
         if (bufferPair == outputBufferMap.end())
         {
            outputBufferMap.insert(std::pair<const QByteArray, QByteArray>(fileNameBuffer, QByteArray()));
            bufferPair = outputBufferMap.find(fileNameBuffer);
         }

         QByteArray& outputBuffer = bufferPair->second;
         outputTemplate.writeRecord(record, outputBuffer, templateErrors, num, outputTemplate.tokens, random, true);
         if (records.empty())
         {
            progressBar->setValue(progressBar->value() + 1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
         }
      }
   }
   else
   {
      return false;
   }
   progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::PreviewProcessFinished));
   return true;
}

// Read functions read configuration data from the gui and establish the config for the associated stage

void ScenarioImporter::Plugin::readFilterConfig(
   std::vector<std::unique_ptr<ScenarioImporter::Filter>>& filters,
   std::vector<ScenarioImporter::FilterError>& filterErrors,
   const std::vector<std::pair<const QComboBox*, QLineEdit*>>& filterWidgets)
{
   filters.clear();
   filterErrors.clear();

   for (auto filter : filterWidgets)
   {
      auto typeComboBox = filter.first;
      auto specLineEdit = filter.second;

      if (typeComboBox && specLineEdit)
      {
         FilterType type = FilterType(typeComboBox->currentIndex());
         auto specString = specLineEdit->text().toStdString();

         switch (type)
         {
            case FilterType::ExclusiveLineNumberFilter:
               filters.emplace_back(ut::make_unique<LineNumberFilter>(true, specString));
               break;
            case FilterType::InclusiveLineNumberFilter:
               filters.emplace_back(ut::make_unique<LineNumberFilter>(false, specString));
               break;
            case FilterType::ExclusiveMatchingTextFilter:
               filters.emplace_back(ut::make_unique<MatchingTextFilter>(true, specString));
               break;
            case FilterType::InclusiveMatchingTextFilter:
               filters.emplace_back(ut::make_unique<MatchingTextFilter>(false, specString));
               break;
            default:
               filterErrors.emplace_back(false, std::string("Unrecognized filter type"), StringSlice{ "", "" }, typeComboBox->currentIndex());
         }
      }
   }
}

void ScenarioImporter::Plugin::readParserConfig(
   ScenarioImporter::ParserColumn& column,
   const size_t selectedIndex,
   const QLineEdit* nameWidget,
   const QLineEdit* delimiterWidget,
   const QComboBox* typeWidget,
   const QCheckBox* discardDelimitersBox,
   const QCheckBox* consumeDelimitersBox)
{
   // TODO handle duplicated names

   // Get strings from name and delimiter text fields
   QByteArray delim = delimiterWidget->text().toLocal8Bit();

   // Update members of selected parser column
   column.attributeKey = nameWidget->text().toLocal8Bit().data();
   // TODO this is not the long term delimiter solution we want.
   column.delimiter = delim.size() > 0 ? delim.at(0) : '\0';
   column.datatype = ScenarioImporter::ParserColumnDatatype(typeWidget->currentIndex());

   column.discardDelimiters = discardDelimitersBox->isChecked();
   column.consumeDelimiters = consumeDelimitersBox->isChecked();
}

// Process functions perform processing on data using the config established for the corresponding stages
// TODO nest all these process_____ functions inside scenarioImporterProcess if no good use of them outside of it is found
// This should probably not be called individually outside of scenarioImporterProcess
bool ScenarioImporter::Plugin::processPreProcessing(
   QProcess& preProcessProcess,
   const QByteArray& source,
   QByteArray& preProcessedData,
   StringSlice& preProcessedSlice,
   QString preProcessorCommand,
   std::vector<ScenarioImporter::PreProcessorError>& preProcessorErrors,
   QProgressBar* progressBar)
{
   preProcessorErrors.clear();
   QStringList commandPathWithArgs = preProcessorCommand.split("/");
   QStringList arguments = commandPathWithArgs[commandPathWithArgs.size() - 1].split(QRegExp("\\s+"));
   commandPathWithArgs.removeLast();
   QString command = commandPathWithArgs.join("/") + "/" + arguments[0];
   arguments.removeFirst();

   std::ifstream processFile(command.toStdString());
   if (preProcessorCommand.isEmpty())
   {
      preProcessedData = source;
      preProcessedSlice = StringSlice{ source.begin(), source.end() };
      progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::PreProcessFinished));
      progressBar->setFormat("Pre-Processing skipped (none selected) - Idle");
      return false;
   }
   else if (!processFile.good())
   {
      preProcessorErrors.emplace_back(true, std::string("Could not find pre-processor"), ScenarioImporter::preProcessorErrorType::fileNameError, 0);
      return false;
   }

   preProcessProcess.start(command, arguments);
   preProcessProcess.write(source);
   preProcessProcess.closeWriteChannel();

   // Do nothing until finished and then . . .
   while (!preProcessProcess.waitForFinished())
   {
      QCoreApplication::processEvents(QEventLoop::AllEvents);
   }
   preProcessedData = preProcessProcess.readAllStandardOutput();
   preProcessedSlice = StringSlice{ preProcessedData.data(), preProcessedData.data() + preProcessedData.size() };
   std::istringstream errorStream(preProcessProcess.readAllStandardError().toStdString());
   std::string line;
   while (getline(errorStream, line))
   {
      QStringList splitOnCommas = QString::fromStdString(line).split(",");
      bool isWarning = (splitOnCommas[0] == "WARNING");
      splitOnCommas.removeFirst();
      preProcessorErrors.emplace_back(isWarning, splitOnCommas.join(",").toStdString(), ScenarioImporter::preProcessorErrorType::processError, 0);
   }
   progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::PreProcessFinished));
   progressBar->setFormat("Pre-Processing completed - Idle");
   return true;

}

// This should probably not be called individually outside of scenarioImporterProcess
bool ScenarioImporter::Plugin::processFilters(
   const StringSlice inputSlice,
   const std::vector<std::unique_ptr<ScenarioImporter::Filter>>& filters,
   std::vector<ScenarioImporter::FilterError>& filterErrors,
   QByteArray& outputData,
   StringSlice& outputSlice)
{
   std::vector<StringSlice> splitData = ScenarioImporter::splitIntoLines(inputSlice);
   filterErrors.clear();
   for (size_t i = 0; i < filters.size(); i++)
   {
      filters[i]->performFilter(splitData, filterErrors, ut::cast_to_int(i));
   }

   outputData.clear();

   for (auto slice : splitData)
   {
      ScenarioImporter::write(slice, outputData);
   }

   outputSlice.begin = outputData.data();
   outputSlice.end = outputData.data() + outputData.size();
   if (!filters.empty())
   {
      return true;
   }
   return false;
}

bool ScenarioImporter::Plugin::processParser()
{
   QCoreApplication::processEvents(QEventLoop::AllEvents);
   data.exampleRecord.clear();
   data.records.clear();
   errors.parserErrors.clear();

   // TODO figure out a max record count based on the number of rows in the filtered data and reserve enough size for data.records

   ScenarioImporter::Parser parser;
   parser.cursor = data.filteredSlice.begin;
   parser.end = data.filteredSlice.end;
   parser.lineNumber = 1;
   parser.lineBegin = data.filteredSlice.begin;
   parser.takeNextDelimiter = false;
   errors.parserErrors.clear();

   gui.progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::FilterFinished));
   gui.progressBar->setFormat("Processing Parser");

   while (parser.cursor != parser.end)
   {
      if (parser.cursor != data.filteredSlice.begin)
      {
         QCoreApplication::processEvents(QEventLoop::AllEvents);
         if (stageCanceled)
         {
            gui.progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::FilterFinished));
            return false;
         }
         gui.progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::FilterFinished +
                                                    static_cast<int>((100 * (parser.cursor - data.filteredSlice.begin)) / (data.filteredSlice.end - data.filteredSlice.begin))));

      }
      //TODO handle blank lines - right now, they will generate an error
      data.records.emplace_back();
      Record& record = data.records.back();

      for (size_t i = 0; i < config.parserColumns.size(); i++)
      {
         // save the current location of the parser for error before reading
         // the next token (for error reporting)
         u32 lineNumber = parser.lineNumber;

         //ParserColumn column = columns[i];
         StringSlice token = parserNextToken(parser, config.parserColumns[i].delimiter, config.parserColumns[i].discardDelimiters, config.parserColumns[i].consumeDelimiters);

         // error - token is missing from the current line
         if (isEmpty(token))
         {
            errors.parserErrors.emplace_back(true, std::string("Empty token found"), static_cast<int>(data.records.size()) - 1, ut::cast_to_int(i));
            continue;
         }

         bool valid = ScenarioImporter::validateToken(token, config.parserColumns[i].datatype);

         if (!valid)
         {
            errors.parserErrors.emplace_back(false, std::string("Invalid token, doesn't match datatype or is poorly formatted "), static_cast<int>(data.records.size()) - 1, ut::cast_to_int(i));
         }

         record.insert(std::pair<std::string, StringSlice>(config.parserColumns[i].attributeKey, token));

         // Skip the rest of the record if we moved to a new line and multiLineRecords is turned off
         if (!config.multiLineRecords && lineNumber != parser.lineNumber)
         {
            break;
         }
      }
      // consume the remainder of the current line, unless the parser already
      // jumped to the next line
      if (parser.cursor != parser.lineBegin)
      {
         parserConsumeLine(parser);
      }
   }

   // Create an example record with empty tokens for specifySyntax to parse with
   for (auto& parserColumn : config.parserColumns)
   {
      StringSlice dummyToken = stringSliceFromCString("dummyToken");
      data.exampleRecord.insert(std::pair<std::string, StringSlice>(parserColumn.attributeKey, dummyToken));
   }
   return true;

}

// This should probably not be called individually outside of scenarioImporterProcess
bool ScenarioImporter::Plugin::processTemplate(const QByteArray& config,
                                               const Record& exampleRecord,
                                               const std::vector<Record>& records,
                                               std::vector<ScenarioImporter::TemplateError>& templateErrors,
                                               QByteArray& outputBuffer,
                                               QProgressBar* progressBar,
                                               QPushButton* cancelButton)
{
   StringSlice templateSlice = toStringSlice(config);
   progressBar->setValue(ScenarioImporter::Plugin::ProgressValue::ParserFinished);
   ScenarioImporter::OutputTemplate outputTemplate;
   templateErrors.clear();
   if (!records.empty() && outputTemplate.specifySyntax(templateSlice, exampleRecord, templateErrors))
   {
      // TODO Figure out a better data structure than QByteArray, specifically so we can avoid the copy to a QString. This will possibly involve coming up with a Qt view
      outputBuffer.clear();
      int num = 0; // for error tracking
      ut::Random random;
      random.SetSeed(1000 * clock() + clock() / 1000);

      for (const auto& record : records)
      {
         if (!outputTemplate.writeRecord(record, outputBuffer, templateErrors, num, outputTemplate.tokens, random, true))
         {
            // We don't want to see the repeat errors
            // and would rather just return to correct the first broken record immediately.
            return false;
         }
         if (records.size() > 100)
         {
            if (records.empty())
            {
               progressBar->setValue(progressBar->value() + 1);
               QCoreApplication::processEvents(QEventLoop::AllEvents);
               if (cancelButton->isDown() == true)
               {
                  cancelButton->setDown(false);
                  return false;
               }
            }
         }
      }
   }
   progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::TemplateFinished));

   return true;
}

void ScenarioImporter::Plugin::colorStageButton(QPushButton* stageButton,
                                                ScenarioImporter::Plugin::Stage configStage,
                                                ScenarioImporter::Plugin::Stage errorStage,
                                                int numErrors,
                                                bool stageCompleted,
                                                bool hasOnlyWarnings)
{
   if (configStage >= errorStage && stageCompleted)
   {
      if (numErrors == 0)
      {
         stageButton->setStyleSheet(getStyleSheet(Color::Complete));
         return;
      }
      hasOnlyWarnings ? stageButton->setStyleSheet(getStyleSheet(Color::Complete)) :
      stageButton->setStyleSheet(getStyleSheet(Color::Error));
   }
   else if (configStage >= errorStage)
   {
      numErrors == 0 ? stageButton->setStyleSheet(getStyleSheet(Color::Blank)) :
      stageButton->setStyleSheet(getStyleSheet(Color::Error));
   }
   else
   {
      stageButton->setStyleSheet(getStyleSheet(Color::Blank));
   }
}

void ScenarioImporter::Plugin::mergeEmptyTokenErrors()
{
   int numEmptyTokenErrors = 0;
   int last_row = 0;
   int last_col = 0;
   for (int i = 0; i < static_cast<int>(errors.parserErrors.size()); i++)
   {
      if (errors.parserErrors[i].explanation == "Empty token found")
      {
         last_row = errors.parserErrors[i].row;
         last_col = errors.parserErrors[i].col;
         errors.parserErrors.erase(errors.parserErrors.begin() + i);
         numEmptyTokenErrors++;
         i--;
      }
   }
   if (numEmptyTokenErrors != 0)
   {
      std::string explanation = std::to_string(numEmptyTokenErrors) + " Empty tokens found";
      errors.parserErrors.emplace_back(true, explanation, last_row, last_col);
   }
}

void ScenarioImporter::Plugin::handleErrorsAndWarnings(ScenarioImporter::GenericError error,
                                                       QString& tempErrorString,
                                                       bool& hasOnlyWarnings)
{
   if (error.isWarning)
   {
      tempErrorString.append("Warning: ");
   }
   else
   {
      hasOnlyWarnings = false;
      tempErrorString.append("Error: ");
   }
}

bool ScenarioImporter::Plugin::containsOnlyWarnings(ScenarioImporter::Plugin::Errors errors,
                                                    ScenarioImporter::Plugin::Stage configStage)
{
   switch (configStage)
   {
      case (ScenarioImporter::Plugin::Stage::PreProcess):
         for (auto error : errors.preProcessorErrors)
         {
            if (!error.isWarning)
            {
               return false;
            }
         }
         break;
      case (ScenarioImporter::Plugin::Stage::Filter):
         for (auto error : errors.filterErrors)
         {
            if (!error.isWarning)
            {
               return false;
            }
         }
         break;
      case (ScenarioImporter::Plugin::Stage::Parse):
         for (auto error : errors.parserErrors)
         {
            if (!error.isWarning)
            {
               return false;
            }
         }
         break;
      case (ScenarioImporter::Plugin::Stage::Template):
         for (auto error : errors.templateErrors)
         {
            if (!error.isWarning)
            {
               return false;
            }
         }
         break;
      case (ScenarioImporter::Plugin::Stage::Preview):
         for (auto error : errors.fileNameErrors)
         {
            if (!error.isWarning)
            {
               return false;
            }
         }
         break;
      case (ScenarioImporter::Plugin::Stage::Default):
         break;
   }
   return true;
}

// TODO Would make sense and be cleaner as a class function
void ScenarioImporter::Plugin::displayErrors(
   ScenarioImporter::Plugin::ScenarioImporterGui& gui,
   ScenarioImporter::Plugin::Errors& errors,
   ScenarioImporter::Plugin::ErrorType type,
   ScenarioImporter::Plugin::Stage configStage,
   bool stageCompleted)
{
   QString tempErrorString;
   bool hasOnlyWarnings = true;
   ScenarioImporter::Plugin::mergeEmptyTokenErrors();
   int processorsEnd = static_cast<int>(errors.preProcessorErrors.size());
   int filtersEnd = processorsEnd + static_cast<int>(errors.filterErrors.size());
   int parsersEnd = filtersEnd + static_cast<int>(errors.parserErrors.size());
   int templatesEnd = parsersEnd + static_cast<int>(errors.templateErrors.size());
   int fileNamesEnd = templatesEnd + static_cast<int>(errors.fileNameErrors.size());
   gui.errorTable->setRowCount(fileNamesEnd);

   switch (type)
   {
      case ScenarioImporter::Plugin::ErrorType::preProcessorError:
         for (int row = 0; row < static_cast<int>(errors.preProcessorErrors.size()); row++)
         {
            handleErrorsAndWarnings(errors.preProcessorErrors[row], tempErrorString, hasOnlyWarnings);
            tempErrorString.append(QString::fromStdString(errors.preProcessorErrors[row].explanation));
            QTableWidgetItem* item = new QTableWidgetItem(tempErrorString);
            gui.errorTable->setItem(row, 0, item);
            tempErrorString.clear();
         }
         colorStageButton(gui.stageButtons[0], configStage, ScenarioImporter::Plugin::Stage::PreProcess, static_cast<int>(errors.preProcessorErrors.size()), stageCompleted, hasOnlyWarnings);


      case ScenarioImporter::Plugin::ErrorType::filterError:
         for (int row = 0; row < static_cast<int>(errors.filterErrors.size()); row++)
         {
            handleErrorsAndWarnings(errors.filterErrors[row], tempErrorString, hasOnlyWarnings);
            tempErrorString.append(QString::fromStdString(errors.filterErrors[row].explanation));
            QTableWidgetItem* item = new QTableWidgetItem(tempErrorString);
            gui.errorTable->setItem(processorsEnd + row, 0, item);
            tempErrorString.clear();
         }
         colorStageButton(gui.stageButtons[1], configStage, ScenarioImporter::Plugin::Stage::Filter, static_cast<int>(errors.filterErrors.size()), stageCompleted, hasOnlyWarnings);

      case ScenarioImporter::Plugin::ErrorType::parserError:
         for (int row = 0; row < static_cast<int>(errors.parserErrors.size()); row++)
         {
            handleErrorsAndWarnings(errors.parserErrors[row], tempErrorString, hasOnlyWarnings);
            tempErrorString.append(QString::fromStdString(errors.parserErrors[row].explanation));
            QTableWidgetItem* item = new QTableWidgetItem(tempErrorString);
            gui.errorTable->setItem(filtersEnd + row, 0, item);
            tempErrorString.clear();
         }
         colorStageButton(gui.stageButtons[2], configStage, ScenarioImporter::Plugin::Stage::Parse, static_cast<int>(errors.parserErrors.size()), stageCompleted, hasOnlyWarnings);

      case ScenarioImporter::Plugin::ErrorType::templateError:
         for (int row = 0; row < static_cast<int>(errors.templateErrors.size()); row++)
         {
            handleErrorsAndWarnings(errors.templateErrors[row], tempErrorString, hasOnlyWarnings);
            tempErrorString.append(QString::fromStdString(errors.templateErrors[row].explanation));
            QTableWidgetItem* item = new QTableWidgetItem(tempErrorString);
            gui.errorTable->setItem(parsersEnd + row, 0, item);
            tempErrorString.clear();
         }
         colorStageButton(gui.stageButtons[3], configStage, ScenarioImporter::Plugin::Stage::Template, static_cast<int>(errors.templateErrors.size()), stageCompleted, hasOnlyWarnings);

      case ScenarioImporter::Plugin::ErrorType::fileNameError:
         for (int row = 0; row < static_cast<int>(errors.fileNameErrors.size()); row++)
         {
            handleErrorsAndWarnings(errors.fileNameErrors[row], tempErrorString, hasOnlyWarnings);
            tempErrorString.append(QString::fromStdString(errors.fileNameErrors[row].explanation));
            QTableWidgetItem* item = new QTableWidgetItem(tempErrorString);
            gui.errorTable->setItem(templatesEnd + row, 0, item);
            tempErrorString.clear();
         }
         colorStageButton(gui.stageButtons[4], configStage, ScenarioImporter::Plugin::Stage::Preview, static_cast<int>(errors.fileNameErrors.size()), stageCompleted, hasOnlyWarnings);

         break;
      default:
         break;
   }
}

// This should probably not be called individually outside of scenarioImporterProcess
void ScenarioImporter::Plugin::processPreview(Record& exampleRecord,
                                              const std::vector<Record>& inputRecords,
                                              std::map<const QByteArray,
                                              QByteArray>& outputBufferMap,
                                              const QByteArray& outputBuffer,
                                              const bool multiFile,
                                              const QByteArray& outputFileNameString,
                                              const QByteArray& outputTemplateString,
                                              std::vector<ScenarioImporter::TemplateError>& fileNameErrors,
                                              std::vector<ScenarioImporter::TemplateError>& templateErrors,
                                              QProgressBar* progressBar)
{
   templateErrors.clear();
   progressBar->setValue(ScenarioImporter::Plugin::ProgressValue::TemplateFinished);
   // Clear the output buffer map
   outputBufferMap.clear();

   // Multi file output indicates the file name is specified in the same way as the output template is specified
   if (multiFile && !inputRecords.empty())
   {
      StringSlice fileNameSlice = toStringSlice(outputFileNameString);
      StringSlice templateSlice = toStringSlice(outputTemplateString);

      // TODO generateOutputMap needs to handle the case of no records
      generateOutputMap(fileNameSlice, templateSlice, exampleRecord, inputRecords, fileNameErrors, templateErrors, outputBufferMap, progressBar);
   }
   else
   {
      // TODO ensure the outputBuffer has been generated

      // Here we try to take advantage of the output template data already being generated in the previous stage and not being changed in this stage.
      outputBufferMap.insert(std::pair<const QByteArray, QByteArray>(outputFileNameString, outputBuffer));
   }
}

// Primary process function for Scenario Importer. This should pretty much always be the function called to process Data after Config has been updated.
// Processes 'data' starting from 'processingStage' up to 'configStage', according to 'config', logging errors in 'errors'.
void ScenarioImporter::Plugin::scenarioImporterProcess()
{
   colorFileButtons();
   bool stageCompleted = true;
   switch (processingStage)
   {
      case Stage::PreProcess:
         gui.progressBar->setFormat("Performing Pre-Processing");
         gui.progressBar->setValue(static_cast<int>(ProgressValue::Start));

         displayGuiStage();
         stageCompleted = processPreProcessing(*gui.preProcessProcess, data.source, data.preProcessedData, data.preProcessedSlice, config.preProcessorCommand, errors.preProcessorErrors, gui.progressBar);
         displayPreProcessedData(gui.preProcessOutputWidget, gui.filterInputWidget, data.preProcessedData);

         processingStage = Stage::Filter;
         displayErrors(gui, errors, ErrorType::preProcessorError, configStage, stageCompleted);

      case Stage::Filter:
         if (configStage < Stage::Filter || !containsOnlyWarnings(errors, Stage::PreProcess))
         {
            break;
         }
         gui.progressBar->setFormat("Processing Filters");
         gui.progressBar->setValue(ProgressValue::PreProcessFinished);

         displayGuiStage();
         stageCompleted = processFilters(data.preProcessedSlice, config.filters, errors.filterErrors, data.filteredData, data.filteredSlice);
         displayFilteredData(gui.filterOutputWidget, gui.parserInputWidget, gui.progressBar, data.filteredData);

         processingStage = Stage::Parse;
         gui.progressBar->setFormat("Filters completed - Idle");
         displayErrors(gui, errors, ErrorType::filterError, configStage, stageCompleted);

      case Stage::Parse:
         if (configStage < Stage::Parse || !containsOnlyWarnings(errors, Stage::Filter))
         {
            break;
         }
         config.parsing = true;
         gui.progressBar->setFormat("Processing Parsing");
         displayGuiStage();

         do
         {
            stageCanceled = false;
            stageCompleted = processParser();
            if (stageCompleted)
            {
               stageCompleted = displayParsedData();
            }
         }
         while (parserColumnsChanged);
         UpdateGuiParserColumns();

         stageCompleted ? gui.progressBar->setFormat("Parsing completed - Idle") : gui.progressBar->setFormat("Filters completed - Idle");

         processingStage = Stage::Template;
         displayErrors(gui, errors, ErrorType::parserError, configStage, stageCompleted);
         config.parsing = false;

      case Stage::Template:
         if (configStage < Stage::Template || !containsOnlyWarnings(errors, Stage::Parse))
         {
            break;
         }
         gui.progressBar->setFormat("Processing Template");

         displayGuiStage();
         if (ScenarioImporter::Plugin::processTemplate(config.templateString, data.exampleRecord, data.records, errors.templateErrors, data.outputBuffer, gui.progressBar, gui.cancelButton))
         {
            stageCompleted = displayTemplate(gui.templateOutputWidget, data.outputBuffer);
         }
         else
         {
            displayErrors(gui, errors, ErrorType::templateError, configStage, stageCompleted = false);
         }

         gui.progressBar->setFormat("Template completed - Idle");
         processingStage = Stage::Preview;
         displayErrors(gui, errors, ErrorType::templateError, configStage, stageCompleted);

      case Stage::Preview:
         if (configStage < Stage::Preview || !containsOnlyWarnings(errors, Stage::Template))
         {
            break;
         }
         gui.progressBar->setFormat("Generating preview");

         displayGuiStage();
         processPreview(data.exampleRecord, data.records, data.outputBufferMap, data.outputBuffer, config.outputMultiFile, config.outputFileNameString, config.templateString,
                        errors.fileNameErrors, errors.templateErrors, gui.progressBar);
         stageCompleted = displayPreview(gui.previewOutputTabWidget, gui.previewConfigFileNamesTextEdit, gui.progressBar, data.outputBufferMap, config.outputMultiFile);

         gui.progressBar->setFormat("Preview generated - Idle");
         displayErrors(gui, errors, ErrorType::fileNameError, configStage, stageCompleted);

      default:
         processingStage = Stage::Default;
         displayGuiStage();
   }
}

bool ScenarioImporter::Plugin::displayParsedData()
{
   gui.progressBar->setValue(ScenarioImporter::Plugin::ProgressValue::ParserProcessFinished);
   gui.progressBar->setFormat("Displaying Parsed Data");

   // clear parsed data
   gui.parserOutputWidget->model()->removeRows(0, gui.parserOutputWidget->rowCount());
   gui.templateInputWidget->model()->removeRows(0, gui.templateInputWidget->rowCount());

   // Update parser output tablewidget contents
   return populateParserTableWidget();
}

bool ScenarioImporter::Plugin::displayPreview(
   QTabWidget* outputTabWidget,
   QPlainTextEdit* fileNamesWidget,
   QProgressBar* progressBar,
   const std::map<const QByteArray,
   QByteArray>& bufferMap,
   bool multiFileOutput)
{
   // Clear Preview output widget

   for (int tab = outputTabWidget->count() - 1; tab >= 0; tab--)
   {
      outputTabWidget->removeTab(tab);
      QCoreApplication::processEvents(QEventLoop::AllEvents);
   }


   QString outputFileNameListString;

   // Set Preview output widget
   size_t pairs = bufferMap.size();
   int i = 0;
   for (auto outputPair : bufferMap)
   {
      if (multiFileOutput)
      {
         auto tabText = new QPlainTextEdit;
         outputTabWidget->addTab(tabText, QString::fromLocal8Bit(outputPair.first));
         tabText->setPlainText(QString::fromLocal8Bit(outputPair.second));
         outputFileNameListString += outputPair.first + "\n";
      }
      else
      {
         if (i == 0)
         {
            auto tabText = new QPlainTextEdit;
            outputTabWidget->addTab(tabText, QString::fromLocal8Bit(outputPair.first));
            outputFileNameListString += outputPair.first + "\n";
         }
         QWidget* textWidget = outputTabWidget->currentWidget();
         QPlainTextEdit* outputTextEdit = (QPlainTextEdit*)textWidget;
         outputTextEdit->appendPlainText(QString::fromLocal8Bit(outputPair.second));
      }
      progressBar->setValue(progressBar->value() + static_cast<int>(100 / pairs));
      QCoreApplication::processEvents(QEventLoop::AllEvents);
      i++;
   }
   progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::PreviewFinished));

   fileNamesWidget->setPlainText(outputFileNameListString);
   return (!bufferMap.empty());
}

std::string ScenarioImporter::Plugin::processErrorTypeEnumToString(QProcess::ProcessError error)
{
   if (error == QProcess::FailedToStart)      { return "Processing Failed To Start"; }
   else if (error == QProcess::Crashed)       { return "Processing Crashed"; }
   else if (error == QProcess::Timedout)      { return "Processing Timed Out"; }
   else if (error == QProcess::WriteError)    { return "Processing Write Error"; }
   else if (error == QProcess::ReadError)     { return "Processing Read Error"; }
   else                                      { return "Unknown Processing Error"; }
}

ScenarioImporter::Plugin::ConfigLineType ScenarioImporter::Plugin::configLineTypeStringToEnum(const std::string& str)
{
   if (str == "Stages")                      { return ConfigLineType::Stages; }
   else if (str == "Filters")                { return ConfigLineType::Filters; }
   else if (str == "Parser columns")         { return ConfigLineType::ParserColumns; }
   else if (str == "Delimiter settings")     { return ConfigLineType::DelimiterSettings; }
   else if (str == "Template")               { return ConfigLineType::Template; }
   else if (str == "Output file")            { return ConfigLineType::OutputFile; }
   else if (str == "Save config fie")        { return ConfigLineType::SaveConfigFile; }
   else if (str == "Multi file output")      { return ConfigLineType::SaveType; }
   else if (str == "Processor command")      { return ConfigLineType::PreProcessorCommand; }
   else { return ConfigLineType::Unknown; }
}

// Returns true if int can be cast to ConfigLineType enum
bool ScenarioImporter::Plugin::validateConfigLineTypeEnumInt(int i)
{
   return i >= 0 && i <= 5;
}

// Returns true if int can be cast to ScenarioImporter::Plugin::Stage enum
bool ScenarioImporter::Plugin::validateStageEnumInt(int i)
{
   return i >= 0 && i <= 5;
}

std::string ScenarioImporter::Plugin::commaSeparatedStringFromFilter(ScenarioImporter::Filter* filter)
{
   std::string ret = "";
   if (dynamic_cast<ScenarioImporter::MatchingTextFilter*>(filter))
   {
      ret += "MatchingTextFilter,";
   }
   else if (dynamic_cast<ScenarioImporter::LineNumberFilter*>(filter))
   {
      ret += "LineNumberFilter,";
   }
   else
   {
      return ret;
   }

   ret += (filter->GetExclusionary() ? "true," : "false,");
   bool saving = true;
   ret += filter->GetSpecString(saving);
   return ret;
}

std::string ScenarioImporter::Plugin::commaSeparatedStringFromParserColumn(const ScenarioImporter::ParserColumn& column)
{
   std::string ret = column.attributeKey + ",";
   if (column.delimiter == ',')
   {
      ret += "comma,";
   }
   else
   {
      ret += column.delimiter + ",";
   }
   ret += std::to_string(static_cast<int>(column.datatype));
   return ret;
}

std::string ScenarioImporter::Plugin::commaSeparatedStringFromTemplateByteArray(const QByteArray& templateByteArray)
{
   QString templateCopy(templateByteArray);
   templateCopy.replace(QString(","), QString(" comma "));
   templateCopy.replace(QString("\n"), QString(","));
   return templateCopy.toStdString();
}

bool ScenarioImporter::Plugin::processSavedConfigLine(Config& config, const QString& line)
{
   if (line.isEmpty())
   {
      return true;
   }

   QStringList tokens = line.split(',');
   std::string lineTypeStr = tokens.at(0).toStdString();
   ConfigLineType lineType = configLineTypeStringToEnum(lineTypeStr);
   if (lineType == ConfigLineType::Unknown)
   {
      return false;
   }

   switch (lineType)
   {
      // TODO NH We don't want to save/load scenario importer stages when saving/loading config. When loading, the configStage should remain whatever it was prior to loading config,
      //  and the processingStage should be set either to the earliest stage with config loaded, or just the pre-processing stage (the first stage).
      case ConfigLineType::Stages:
      {
         // should have a type and two stages
         //if (tokens.size() != 3)
         //{
         //   return false;
         //}
         //bool ok = true;
         //bool stageOk = true;

         //int processingStageInt = tokens.at(1).toInt(&stageOk);
         //ok &= stageOk;
         //int currentStageInt = tokens.at(2).toInt(&stageOk);
         //ok &= stageOk;

         //if (ok && validateStageEnumInt(processingStageInt) && validateStageEnumInt(currentStageInt))
         //{
         //   processingStage = static_cast<ScenarioImporter::Plugin::Stage>(processingStageInt);
         //   configStage = static_cast<ScenarioImporter::Plugin::Stage>(currentStageInt);

         return true;
         //}
         //return false;
      }

      case ConfigLineType::Filters:
      {
         // should have at least a type and a filter count
         if (tokens.size() < 2)
         {
            return false;
         }
         bool ok = true;
         int filterCount = tokens.at(1).toInt(&ok);
         int filterSize = 3; // filterType, exclusionaryString, specStrin

         if (!ok || (tokens.size() < 2 + filterCount * filterSize))
         {
            return false;
         }

         int idx = 2;
         for (int i = 0; i < filterCount; ++i)
         {
            std::string filterType = tokens.at(idx++).toStdString();
            std::string exclusionaryString = tokens.at(idx++).toStdString();
            std::string specString = tokens.at(idx++).toStdString();

            QStringList splitOnCommas = QString::fromStdString(specString).split("comma");
            specString = splitOnCommas.join(",").toStdString();

            bool exclusionary;
            if (exclusionaryString == "true")
            {
               exclusionary = true;
            }
            else if (exclusionaryString == "false")
            {
               exclusionary = false;
            }
            else
            {
               return false;
            }

            if (filterType == "MatchingTextFilter")
            {
               config.filters.emplace_back(ut::make_unique<ScenarioImporter::MatchingTextFilter>(exclusionary, specString));
            }
            else if (filterType == "LineNumberFilter")
            {
               config.filters.emplace_back(ut::make_unique<ScenarioImporter::LineNumberFilter>(exclusionary, specString));
            }
            else
            {
               return false;
            }
         }
         return true;
      }

      case ConfigLineType::ParserColumns:
      {
         // should have at least a type and a column count
         if (tokens.size() < 2)
         {
            return false;
         }
         bool ok = true;
         int columnCount = tokens.at(1).toInt(&ok);
         int columnSize = 3; // attributeStr, delimiterStr, datatypeInt
         if (!ok || (tokens.size() < 2 + columnCount * columnSize))
         {
            return false;
         }

         int idx = 2;
         for (int i = 0; i < columnCount; ++i)
         {
            std::string attributeKey = tokens.at(idx++).toStdString();
            std::string delimiterStr = tokens.at(idx++).toStdString();
            if (delimiterStr.empty())
            {
               return false;
            }
            char delimiter = (delimiterStr == "comma") ? ',' : delimiterStr.at(0);
            int datatypeInt = tokens.at(idx++).toInt(&ok);
            if (!ok)
            {
               return false;
            }
            //TODO add fn to validate ParserColumnDatatype int
            auto datatype = static_cast<ScenarioImporter::ParserColumnDatatype>(datatypeInt);

            config.parserColumns.emplace_back(attributeKey, delimiter, datatype, true, false);
         }
         return true;
      }
      case ConfigLineType::DelimiterSettings:
      {
         // Should have type and column number, 2 for each column, plus one for multiLine and one for the final ,
         if (ut::cast_to_size_t(tokens.size()) != config.parserColumns.size() * 2 + 4)
         {
            return false;
         }
         bool ok = true;
         int columnCount = tokens.at(1).toInt(&ok);
         int idx = 2;
         for (int i = 0; i < columnCount; ++i)
         {
            (tokens.at(idx++).toStdString() == "true") ? config.parserColumns[i].discardDelimiters = true : config.parserColumns[i].discardDelimiters = false;
            (tokens.at(idx++).toStdString() == "true") ? config.parserColumns[i].consumeDelimiters = true : config.parserColumns[i].consumeDelimiters = false;
         }
         (tokens.at(idx).toStdString() == "true") ? config.multiLineRecords = true : config.multiLineRecords = false;
         return true;
      }
      case ConfigLineType::Template:
      {
         if (tokens.size() < 2)
         {
            return false;
         }
         QByteArray templateString;
         for (int i = 1; i < tokens.size(); ++i)
         {
            QStringList splitOnCommas = tokens.at(i).split(" comma ");
            auto commaLiteralToken = splitOnCommas.join(",");
            templateString.append(commaLiteralToken);
            templateString.append(QString("\n"));
         }
         // strip final (extra) newline
         templateString.remove(templateString.size() - 1, 1);

         config.templateString = templateString;
         return true;
      }

      case ConfigLineType::OutputFile:
      {
         if (tokens.size() != 2)
         {
            return false;
         }
         config.outputFileNameString = tokens.at(1).toLocal8Bit();
         return true;
      }

      // TODO NH We don't need this
      case ConfigLineType::SaveConfigFile:
      {
         //if (tokens.size() != 2)
         //{
         //return false;
         //}
         //config.saveConfigFileNameString = tokens.at(1).toLocal8Bit();
         return true;
      }

      case ConfigLineType::SaveType:
      {
         if (tokens.size() != 2)
         {
            return false;
         }
         config.outputMultiFile = tokens.at(1).toInt() == 1;
         return true;
      }

      case ConfigLineType::PreProcessorCommand:
      {
         if (tokens.size() < 2)
         {
            return false;
         }
         config.preProcessorCommand = tokens.at(1);
         return true;
      }

      default:
         return false;
   }
   return false;
}

QString ScenarioImporter::Plugin::getFilterType(ScenarioImporter::Filter* filter)
{
   if (dynamic_cast<ScenarioImporter::LineNumberFilter*>(filter))
   {
      if (filter->GetExclusionary())
      {
         return QString("Exclude lines numbered");
      }
      else
      {
         return QString("Include only lines numbered");
      }
   }
   else
   {
      if (filter->GetExclusionary())
      {
         return QString("Exclude lines with text");
      }
      else
      {
         return QString("Include only lines with text");
      }
   }
}

QString ScenarioImporter::Plugin::getStyleSheet(Color aColor) const
{
   int colorIdx = static_cast<int>(aColor);
   if (QIcon::themeName() == "Dark")
   {
      return darkThemeColorStrings.at(colorIdx);
   }
   // Use "Light" theme options by default
   return lightThemeColorStrings.at(colorIdx);
}
