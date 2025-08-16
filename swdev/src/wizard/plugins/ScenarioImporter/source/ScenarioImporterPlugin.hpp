// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SCENARIOIMPORTERPLUGIN_HPP
#define SCENARIOIMPORTERPLUGIN_HPP

#include <map>
#include <memory>
#include <vector>
#include <sstream>

#define _USE_MATH_DEFINES
#include "math.h"

#include "UtUnitTypes.hpp"
#include "UtRandom.hpp"
#include "UtMemory.hpp"

#include <QTextCodec>
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QStackedLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QTableWidget>
#include <QHeaderView>
#include <QRadioButton>
#include <QScrollArea>
#include <QInputDialog>
#include <QProcess>
#include <QString>

#include "WkfAction.hpp"
#include "Plugin.hpp"

#include "Types.hpp"
#include "Input.hpp"
#include "Output.hpp"
namespace ScenarioImporter
{
class Filter;

class ImporterMainWindow : public QMainWindow
{
      Q_OBJECT

   public:
      ImporterMainWindow(QWidget* parent)
         : QMainWindow(parent) {}
   protected:
      void closeEvent(QCloseEvent* event) override;

   signals:
      void close();
};

class Plugin : public wizard::Plugin
{
      Q_OBJECT
   public:
      Plugin(const QString& aPluginName,
             const size_t   aUniqueId);
      ~Plugin() override;

      void LoadPlugin();
      void UnloadPlugin();
      void LaunchScenarioImporterToolFromMenu();

      void UpdateGuiFilters();
      void UpdateGuiParserColumns();
      void displayGuiStage();
      void colorFileButtons();
      bool processParser();
      bool displayParsedData();
      bool populateParserTableWidget();

      const QStringList filterTypeList = { "Exclude lines numbered", "Include only lines numbered", "Exclude lines with text", "Include only lines with text" };
      const QStringList parserColumnDataTypeList = { "String", "Identifier", "SignedInteger", "UnsignedInteger", "FractionalNumber", "Lat", "Long" };
      const QStringList errorTypeList = { "Filter Errors", "Parser Errors", "Template Errors", "File Name Errors" };
      const QStringList lightThemeColorStrings = { "",                                         // Blank    = 0
                                                   "background-color: rgba(0,237,66,0.6)",     // Complete = 1  [Lime Green]
                                                   "background-color: rgba(237,67,55,0.6)",    // Error    = 2  [Red]
                                                   "background-color: rgba(255,215,0,0.6)",    // Warning  = 3  [Yellow]
                                                   "background-color: rgba(176,196,222,0.85)"  // Todo     = 4  [Light Blue]
                                                 };
      const QStringList darkThemeColorStrings = { "",                                          // Blank    = 0
                                                  "background-color: rgb(55,130,42)",          // Complete = 1  [Less Lime Green]
                                                  "background-color: rgb(237,67,55)",          // Error    = 2  [Red]
                                                  "background-color: rgb(189,196,117)",        // Warning  = 3  ["Golden"]
                                                  "background-color: rgb(15,47,99)"            // Todo     = 4  [Dark Blue]
                                                };
      struct ScenarioImporterGui
      {
         ImporterMainWindow* mainWindow;

         QPlainTextEdit* errorList;
         QTableWidget* errorTable;

         QProgressBar* progressBar;
         QPushButton* cancelButton;

         QPushButton* openFileButton;
         QLineEdit* openFileNameEdit;
         QPushButton* browseOutputDirectoryButton;
         QLineEdit* outputDirectoryNameEdit;
         QPushButton* saveOutputButton;
         QPushButton* loadImporterButton;
         QPushButton* saveImporterButton;

         QPushButton* stageButtons[5];
         QVBoxLayout* configVBoxLayouts[5];
         QHBoxLayout* configAutoHBoxLayouts[5];
         QWidget* configAutoProcessButtons[5];
         QCheckBox* autoProcessCheckBoxes[5];
         QPushButton* manualProcessButtons[5];

         QGroupBox* configureViewWidget;
         QStackedLayout* configureViewLayout;

         // PreProcess
         QProcess* preProcessProcess;
         QWidget* preProcessConfigWidget;

         QWidget* filterConfigWidget;

         QPushButton* selectPreProcessorButton;
         QLineEdit* preProcessConfigEdit;

         std::vector<std::pair<const QComboBox*, QLineEdit*>> filterConfigChildWidgets;
         QWidget* filterConfigAddFilterWidget;
         QPushButton* filterConfigAddFilterButton;

         QWidget* parserConfigWidget;
         QLineEdit* parserConfigNameEdit;
         QLineEdit* parserConfigDelimiterEdit;
         QComboBox* parserConfigTypeComboBox;
         // Delimiter handling for each column created:
         QCheckBox* parserConfigDiscardCheckBox;
         QCheckBox* parserConfigConsumeCheckBox;
         QCheckBox* parserConfigMultiLineRecordsCheckBox;

         // TODO add delimiter specs to templateConfigWidget
         QWidget* templateConfigWidget;
         QPlainTextEdit* templateConfigTextEdit;
         QCheckBox* templateConfigAutoProcessCheckBox;
         QPushButton* templateConfigManualProcessButton;

         QWidget* previewConfigWidget;
         QRadioButton* previewConfigSingleFileButton;
         QRadioButton* previewConfigMultiFileButton;
         QLineEdit* previewConfigFileNameLineEdit;
         QPlainTextEdit* previewConfigFileNamesTextEdit;

         QTabWidget* inputViewWidget;
         QStackedLayout* inputViewLayout;
         QPlainTextEdit* preProcessInputWidget;
         QPlainTextEdit* filterInputWidget;
         QPlainTextEdit* parserInputWidget;
         QTableWidget* templateInputWidget;
         QHeaderView* templateInputHeader;

         QTabWidget* outputViewWidget;
         QStackedLayout* outputViewLayout;
         QPlainTextEdit* preProcessOutputWidget;
         QPlainTextEdit* filterOutputWidget;
         QTableWidget* parserOutputWidget;
         QTableView* parserTableView;
         QHeaderView* parserOutputHeader;
         QPlainTextEdit* templateOutputWidget;
         QTabWidget* previewOutputTabWidget;

         size_t selectedParserColumnIndex;
      };

      enum struct Stage : u8
      {
         PreProcess = 0,
         Filter = 1,
         Parse = 2,
         Template = 3,
         Preview = 4,
         Default = 5
      };

      enum ProgressValue : int
      {
         Start = 0,
         PreProcessFinished = 200,
         FilterFinished = 400,
         ParserProcessFinished = 500,
         ParserFinished = 600,
         TemplateFinished = 800,
         PreviewProcessFinished = 900,
         PreviewFinished = 1000,
         Unknown
      };

      struct Config
      {
         QString preProcessorCommand;

         // Filter
         std::vector<std::unique_ptr<ScenarioImporter::Filter>> filters;

         // Parser
         std::vector<ScenarioImporter::ParserColumn> parserColumns;
         bool multiLineRecords;
         bool parsing;

         // Template
         QByteArray templateString;

         // Output
         QByteArray outputFileNameString;
         bool outputMultiFile;

         ~Config() = default;
      };

      struct Data
      {
         QByteArray source;

         // Output for the preprocessing stage
         QByteArray preProcessedData;
         StringSlice preProcessedSlice;

         // Output for the filter stage
         QByteArray filteredData;
         StringSlice filteredSlice; // TODO This could possibly be kept in separate lines
         int rowCount; // TODO get rid of this variable!

         // Output for the parsing stage
         Record exampleRecord; // use parserColumns for specifySyntax call
         std::vector<Record> records;

         // Output for the template stage
         QByteArray outputBuffer;

         // Output for the preview stage
         // A map of byte arrays containing the text to be output to to each file,
         // using filenames stored in byte arrays as keys.
         std::map<const QByteArray, QByteArray> outputBufferMap;

         ~Data() = default;
      };

      struct Errors
      {
         std::vector<ScenarioImporter::PreProcessorError> preProcessorErrors;
         std::vector<ScenarioImporter::FilterError> filterErrors;
         std::vector<ScenarioImporter::ParserError> parserErrors;
         std::vector<ScenarioImporter::TemplateError> templateErrors;
         std::vector<ScenarioImporter::TemplateError> fileNameErrors;
         std::pair<int, int> lastParserErrorViewed;
         std::pair<int, int> lastTemplateErrorViewed;

         ~Errors() = default;
      };

      enum struct ErrorType : int
      {
         preProcessorError = 0,
         filterError = 1,
         parserError = 2,
         templateError = 3,
         fileNameError = 4,
      };

      ScenarioImporterGui gui;
      Data data;
      Config config;
      Errors errors;
      Stage configStage;
      Stage processingStage;
      bool stageCanceled;
      bool parserColumnsChanged;

   public slots:
      void preProcessConfigUpdated();
      void preProcessConfigUpdatedInt(int i);
      void filterConfigUpdated();
      void filterConfigUpdatedInt(int i);
      void parserConfigUpdated();
      void parserConfigUpdatedInt(int i);
      void templateConfigUpdated();
      void previewConfigUpdated();
      void previewConfigUpdatedBool();
      void removeFilterButtonClicked();
      void removeParserColumnOptionChosen(int logicalindex);
      void setParserColumnsOptionChosen(int logicalindex);
      void configStageManualUpdated();
      void configStageSetAuto(bool update);
      void cancelStage();
      void windowClosed();


   private:
      void scenarioImporterProcess();

      inline QString toQString(StringSlice slice)
      {
         return QString(QByteArray::fromRawData(slice.begin, (int)length(slice)));
      }

      inline QString toQString(ScenarioImporter::ParserColumn column)
      {
         return QString::fromStdString(column.attributeKey) + "\n\'" + column.delimiter + "\' " + parserColumnDataTypeList.at(int(column.datatype));
      }

      inline StringSlice toStringSlice(const QByteArray& byteArray)
      {
         return StringSlice{ byteArray.data(), byteArray.data() + byteArray.size() };
      }

      QStringList toHeaderLabelQStringList(const std::vector<ScenarioImporter::ParserColumn>& parserColumns);

      void readFilterConfig(std::vector<std::unique_ptr<ScenarioImporter::Filter>>& filters,
                            std::vector<ScenarioImporter::FilterError>& filterErrors,
                            const std::vector<std::pair<const QComboBox*, QLineEdit*>>& filterWidgets);

      void readParserConfig(ScenarioImporter::ParserColumn& column,
                            const size_t selectedIndex,
                            const QLineEdit* nameWidget,
                            const QLineEdit* delimiterWidget,
                            const QComboBox* typeWidget,
                            const QCheckBox* discardDelimitersBox,
                            const QCheckBox* consumeDelimitersBox);

      bool generateOutputMap(StringSlice fileNameSlice,
                             StringSlice templateSlice,
                             Record& exampleRecord,
                             const std::vector<Record>& records,
                             std::vector<ScenarioImporter::TemplateError>& fileNameErrors,
                             std::vector<ScenarioImporter::TemplateError>& templateErrors,
                             std::map<const QByteArray, QByteArray>& outputBufferMap,
                             QProgressBar* progressBar);


      inline void readTemplateConfig(QByteArray& config, const QPlainTextEdit* widget)
      {
         config = widget->toPlainText().toLocal8Bit();
      }

      inline void readPreviewConfig(QByteArray& outputFileNameString, const QLineEdit* widget)
      {
         // Store the specification of the output file name
         outputFileNameString = widget->text().toUtf8();
      }

      // This should probably not be called individually outside of scenarioImporterProcess
      bool processPreProcessing(
         QProcess& preProcessProcess,
         const QByteArray& source,
         QByteArray& preProcessedData,
         StringSlice& preProcessedSlice,
         QString preProcessorCommand,
         std::vector<ScenarioImporter::PreProcessorError>& preProcessorErrors,
         QProgressBar* progressBar);

      // This should probably not be called individually outside of scenarioImporterProcess
      bool processFilters(
         const StringSlice inputSlice,
         const std::vector<std::unique_ptr<ScenarioImporter::Filter>>& filters,
         std::vector<ScenarioImporter::FilterError>& filterErrors,
         QByteArray& outputData,
         StringSlice& outputSlice);

      // This should probably not be called individually outside of scenarioImporterProcess
      bool processTemplate(const QByteArray& config,
                           const Record& exampleRecord,
                           const std::vector<Record>& records,
                           std::vector<ScenarioImporter::TemplateError>& templateErrors,
                           QByteArray& outputBuffer,
                           QProgressBar* progressBar,
                           QPushButton* cancelButton);

      // This should probably not be called individually outside of scenarioImporterProcess
      void processPreview(Record& exampleRecord,
                          const std::vector<Record>& inputRecords,
                          std::map<const QByteArray,
                          QByteArray>& outputBufferMap,
                          const QByteArray& outputBuffer,
                          const bool multiFile,
                          const QByteArray& outputFileNameString,
                          const QByteArray& outputTemplateString,
                          std::vector<ScenarioImporter::TemplateError>& fileNameErrors,
                          std::vector<ScenarioImporter::TemplateError>& templateErrors,
                          QProgressBar* progressBar);

      void displayErrors(ScenarioImporter::Plugin::ScenarioImporterGui& gui,
                         ScenarioImporter::Plugin::Errors& errors,
                         ScenarioImporter::Plugin::ErrorType type,
                         ScenarioImporter::Plugin::Stage processingStage,
                         bool StageCanceled);

      // Display functions fill in the gui using data generated via the process functions
      inline void displayPreProcessedData(QPlainTextEdit* preProcessOutputWidget,
                                          QPlainTextEdit* filterInputWidget,
                                          const QByteArray& data)
      {
         QString dataString = QString::fromLocal8Bit(data);
         preProcessOutputWidget->setPlainText(dataString);
         preProcessOutputWidget->moveCursor(QTextCursor::Start);
         preProcessOutputWidget->ensureCursorVisible();
         filterInputWidget->setPlainText(dataString);
         filterInputWidget->moveCursor(QTextCursor::Start);
         filterInputWidget->ensureCursorVisible();
      }

      inline void displayFilteredData(QPlainTextEdit* filterOutputWidget,
                                      QPlainTextEdit* parserInputWidget,
                                      QProgressBar* progressBar,
                                      const QByteArray& data)
      {
         filterOutputWidget->clear();
         parserInputWidget->clear();
         auto beginIter = data.begin();

         // TODO if threading, granularity is replaced with whatever data the processing thread notifies as available.
         int granularity = 1;
         data.end() - data.begin() == 0 ? granularity = 1 : granularity = data.end() - data.begin();
         for (auto endIter = data.begin(); endIter <= data.end(); endIter += granularity)
         {
            filterOutputWidget->insertPlainText(QString::fromLocal8Bit(beginIter, static_cast<int>(endIter - beginIter)));
            parserInputWidget->insertPlainText(QString::fromLocal8Bit(beginIter, static_cast<int>(endIter - beginIter)));

            filterOutputWidget->moveCursor(QTextCursor::End);
            parserInputWidget->moveCursor(QTextCursor::End);

            if (endIter != data.begin())
            {
               progressBar->setValue(static_cast<int>(progressBar->value() + static_cast<int>((100 * (endIter - data.begin())) / (data.end() - data.begin()))));
            }
            beginIter = endIter;
         }
         if (beginIter != data.end())
         {
            filterOutputWidget->insertPlainText(QString::fromLocal8Bit(beginIter, static_cast<int>(data.end() - beginIter)));
            parserInputWidget->insertPlainText(QString::fromLocal8Bit(beginIter, static_cast<int>(data.end() - beginIter)));
         }
         progressBar->setValue(static_cast<int>(ScenarioImporter::Plugin::ProgressValue::FilterFinished));
         filterOutputWidget->moveCursor(QTextCursor::Start);
         filterOutputWidget->ensureCursorVisible();
         parserInputWidget->moveCursor(QTextCursor::Start);
         parserInputWidget->ensureCursorVisible();
      }

      void mergeEmptyTokenErrors();

      void colorStageButton(QPushButton* stageButton,
                            ScenarioImporter::Plugin::Stage configStage,
                            ScenarioImporter::Plugin::Stage errorStage,
                            int numErrors,
                            bool stageCompleted,
                            bool hasOnlyWarnings);

      bool containsOnlyWarnings(ScenarioImporter::Plugin::Errors errors,
                                ScenarioImporter::Plugin::Stage configStage);

      void handleErrorsAndWarnings(ScenarioImporter::GenericError error,
                                   QString& tempErrorString,
                                   bool& hasOnlyWarnings);

      inline bool displayTemplate(QPlainTextEdit* widget,
                                  const QByteArray& buffer)
      {
         widget->setPlainText(QString::fromLocal8Bit(buffer));
         widget->moveCursor(QTextCursor::Start);
         widget->ensureCursorVisible();
         return (buffer.size() != 0);
      }

      bool displayPreview(QTabWidget* outputTabWidget,
                          QPlainTextEdit* fileNamesWidget,
                          QProgressBar* progressBar,
                          const std::map<const QByteArray,
                          QByteArray>& bufferMap,
                          bool multiFileOutput);

      // Primary processing/input/display functions

      enum struct ConfigLineType : int
      {
         Stages = 0,
         Filters = 1,
         ParserColumns = 2,
         DelimiterSettings = 3,
         Template = 4,
         OutputFile = 5,
         SaveConfigFile = 6,
         SaveType = 7,
         PreProcessorCommand = 8,
         Unknown
      };

      // Should match the arrays lightThemeColorStrings, darkThemeColorStrings
      enum struct Color : int
      {
         Blank = 0,
         Complete = 1,
         Error = 2,
         Warning = 3,
         Todo = 4,
      };

      std::string processErrorTypeEnumToString(QProcess::ProcessError error);
      ConfigLineType configLineTypeStringToEnum(const std::string& str);
      bool validateConfigLineTypeEnumInt(int i);
      bool validateStageEnumInt(int i);

      std::string commaSeparatedStringFromFilter(ScenarioImporter::Filter* filter);
      std::string commaSeparatedStringFromParserColumn(const ScenarioImporter::ParserColumn& column);
      std::string commaSeparatedStringFromTemplateByteArray(const QByteArray& templateByteArray);
      bool processSavedConfigLine(ScenarioImporter::Plugin::Config& config,
                                  const QString& line);
      QString getFilterType(ScenarioImporter::Filter*);

      QString getStyleSheet(Color aColor) const;

      wkf::Action*      mActionPtr;
};
}
#endif
