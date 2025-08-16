// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "OutputDock.hpp"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QTextBlock>
#include <QTextCursor>
#include <QUrlQuery>

#include "ContextMenuActions.hpp"
#include "ProjectWorkspace.hpp"
#include "UiResources.hpp"
#include "UtQt.hpp"
#include "WkfEnvironment.hpp"
#include "WsfExe.hpp"
#include "WsfExecution.hpp"

SimulationManager::OutputDock::OutputDock(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : QDockWidget(parent, f)
   , mExecutionDoneConnection()
{
   setObjectName("OutputDock");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());
   setWindowTitle(widget()->windowTitle());

   mUi.outputText->setOpenLinks(false);
   mUi.outputText->setUndoRedoEnabled(false);
   mUi.statusLabel->setText("");
   mUi.killBn->setEnabled(false);
   mUi.killBn->setIcon(QIcon::fromTheme("delete"));
   mUi.outputText->setFont(wizard::UiResources::GetInstance().GetFont());

   mUi.outputText->setLineWrapMode(QTextEdit::NoWrap);
   mUi.outputText->setTextInteractionFlags(mUi.outputText->textInteractionFlags() | Qt::TextSelectableByKeyboard);
   mUi.logFileBn->setEnabled(true);
   mUi.logFileBn->setIcon(QIcon::fromTheme("folder"));

   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::StartingExecution,
           this,
           &OutputDock::StartingExecution);
   connect(mUi.outputText, &OutputWidget::anchorClicked, this, &OutputDock::AnchorClicked);
   connect(mUi.killBn, &QToolButton::clicked, this, &OutputDock::KillClick);
   connect(mUi.logFileBn, &QPushButton::clicked, this, &OutputDock::SelectOutputLogFile);
   connect(mUi.logFilePath, &QLabel::linkActivated, this, &OutputDock::OutputLogLinkClicked);

   mResultsMenuPtr = new QMenu(this);
   mUi.resultsButton->setMenu(mResultsMenuPtr);
}

void SimulationManager::OutputDock::AppendQueueToConsole()
{
   mUi.outputText->AppendQueueToConsole();
}

void SimulationManager::OutputDock::AnchorClicked(const QUrl& aUrl)
{
   QUrlQuery query(aUrl);
   if (query.hasQueryItem("file"))
   {
      QString             filePath = query.queryItemValue("file");
      wizard::TextSource* sourcePtr =
         wizard::ProjectWorkspace::Instance()->FindSource(filePath.toStdString(), false, true);
      if (sourcePtr)
      {
         size_t pos;
         if (query.hasQueryItem("line") && query.hasQueryItem("column"))
         {
            size_t lineNo = query.queryItemValue("line").toUInt();
            size_t colNo  = query.queryItemValue("column").toUInt();
            pos           = sourcePtr->GetLinePosition(lineNo - 1) + colNo - 1;
         }
         else
         {
            pos = sourcePtr->GetText().Size() - 1;
         }
         wizard::ProjectWorkspace::Instance()->GotoRange(UtTextDocumentRange(sourcePtr, pos, pos - 1));
      }
   }
}

void SimulationManager::OutputDock::StartingExecution(wizard::WsfExecution* aExecutionPtr)
{
   mResultsMenuPtr->clear();
   mUi.outputText->Reset();

   QTextDocument* doc = aExecutionPtr->mOutputText.data();
   doc->setMaximumBlockCount(1000);

   connect(aExecutionPtr, &wizard::WsfExecution::StandardOutputText, mUi.outputText, &OutputWidget::QueueForConsole);

   disconnect(mExecutionDoneConnection);
   mExecutionDoneConnection = connect(aExecutionPtr, &wizard::WsfExecution::Done, this, &OutputDock::ExecutionDone);
   mExecutionPtr            = aExecutionPtr;

   mExecutionPtr->SetOutputLogPath(mOutputLogPath);

   // (0,0) indicates we should show progress being made
   mUi.scenarioLabel->setText("Scenario: " + aExecutionPtr->mName);
   mUi.progressBar->setRange(0, 0);
   mUi.progressBar->setValue(0);
   mUi.progressBar->reset();
   mUi.killBn->setEnabled(true);
   mUi.statusLabel->setText("Running");
   mUi.logFileBn->setEnabled(false);
   mUi.resultsButton->hide();

   show();
   UtQtRaiseWidget(this);
}

void SimulationManager::OutputDock::ExecutionDone()
{
   mResultsMenuPtr->clear();
   if (mExecutionPtr)
   {
      mUi.progressBar->setRange(0, 1);
      if (mExecutionPtr->GetExitStatus() == QProcess::CrashExit)
      {
         mUi.progressBar->setValue(0);
         mUi.statusLabel->setText("Failed");
      }
      else
      {
         mUi.progressBar->setValue(1);
         mUi.statusLabel->setText("Done");
      }
      mUi.killBn->setEnabled(false);
      mUi.logFileBn->setEnabled(true);
      std::string logFile     = mExecutionPtr->GetExe()->GetPath().GetFileName(false) + ".log";
      UtPath      logFilePath = mExecutionPtr->GetWorkingDir() + logFile;
      if (logFilePath.Stat() == UtPath::cFILE)
      {
         ScanLogFile(logFilePath);
      }
      SetOutputLogLabel();

      mExecutionPtr = nullptr;
   }
}

void SimulationManager::OutputDock::ScanLogFile(const UtPath& aPath)
{
   QFile file(QString::fromStdString(aPath.GetSystemPath()));
   if (file.open(QFile::ReadOnly))
   {
      QRegExp     outputFileRegExp("^\\d+-\\d+-\\d+ \\d+:\\d+:\\d+ (.+) output file: (.+)$");
      QTextStream fileText(&file);
      while (!fileText.atEnd())
      {
         const QString& line = fileText.readLine();
         if (outputFileRegExp.exactMatch(line))
         {
            const QString& fileType = outputFileRegExp.cap(1);
            const QString& filePath = outputFileRegExp.cap(2);
            if (fileType == "DIS" || fileType == "Event" || fileType == "PD Map" || fileType == "AER")
            {
               const UtPath disReplayFile(mExecutionPtr->GetWorkingDir() + filePath.toStdString());
               if (disReplayFile.Stat() == UtPath::cFILE)
               {
                  mResultsMenuPtr->addAction(
                     new wizard::ContextMenuActions::OpenFileInProject(disReplayFile.GetSystemPath(), this, this));
                  mUi.resultsButton->show();
               }
            }
         }
      }
   }
}

void SimulationManager::OutputDock::KillClick()
{
   if (mExecutionPtr)
   {
      mExecutionPtr->Kill();
   }
}

void SimulationManager::OutputDock::SelectOutputLogFile()
{
   mOutputLogPath = QFileDialog::getSaveFileName(this, "Select scenario output log file");
   SetOutputLogLabel();
}

void SimulationManager::OutputDock::OutputLogLinkClicked(const QString& aPath)
{
   const QUrl pathUrl("file:///" + aPath);
   QDesktopServices::openUrl(pathUrl);
}

void SimulationManager::OutputDock::SetOutputLogLabel()
{
   if (mOutputLogPath.isEmpty())
   {
      mUi.logFilePath->setText("N/A");
   }
   else if (QFile(mOutputLogPath).exists())
   {
      mUi.logFilePath->setText(QString("<a href=\"%1\">%2</a>").arg(mOutputLogPath).arg(mOutputLogPath));
   }
   else
   {
      mUi.logFilePath->setText(mOutputLogPath);
   }
}
