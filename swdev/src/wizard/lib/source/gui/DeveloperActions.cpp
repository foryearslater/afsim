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

#include "DeveloperActions.hpp"

#include <QApplication>
#include <QInputDialog>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include "EditorPrefObject.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyChange.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "TextSourceCache.hpp"
#include "UtAlgorithm.hpp"
#include "UtQtMemory.hpp"
#include "VaUtils.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::DeveloperActions::DeveloperActions(QObject* aParent)
   : QObject(aParent)
{
   QMenu* dev = wkfEnv.GetMainWindow()->GetMenuByName("Developer");
   if (dev)
   {
      QList<QAction*> actions;
      actions << dev->addAction("Document Memory Usage", this, &DeveloperActions::MemoryUsage);
      actions << dev->addAction("Object Mode", this, &DeveloperActions::ObjectMode);
      actions << dev->addAction("Parse Tree Dump", this, &DeveloperActions::ParseTreeDump);
      actions << dev->addAction("Proxy Dump", this, &DeveloperActions::ProxyDump);
      actions << dev->addAction("Watch Proxy", this, &DeveloperActions::WatchProxy);

      mParserTimersMenu = ut::qt::make_qt_ptr<QMenu>("Parser/Proxy Timers", dev);
      mParserTimersMenu->setIcon(QIcon::fromTheme("time"));
      mGrammarTimer     = ut::qt::make_qt_ptr<QAction>("Grammar T: ");
      mParseTimer       = ut::qt::make_qt_ptr<QAction>("Parse T: ");
      mDeserializeTimer = ut::qt::make_qt_ptr<QAction>("Deserialize T: ");
      mMergeGUI_Timer   = ut::qt::make_qt_ptr<QAction>("Merge/GUI T: ");
      mParserTimersMenu->addAction(mGrammarTimer);
      mParserTimersMenu->addAction(mParseTimer);
      mParserTimersMenu->addAction(mDeserializeTimer);
      mParserTimersMenu->addAction(mMergeGUI_Timer);
      // Make the menu tearable
      mParserTimersMenu->setTearOffEnabled(true);
      dev->addMenu(mParserTimersMenu);

      // Lambda to populate the parser/proxy timer actions with the respective times
      auto populateParserTimers = [this]()
      {
         // Display the time to parse the WSF grammar
         // Note:  This becomes negligible after the user loads a scenario file for the first time.
         UtTimeValue grammar{ProjectWorkspace::Instance()->GetLastGrammarParseTime()};
         mGrammarTimer->setText("Grammar T: " + QString::fromStdString(vespa::VaUtils::ToString(grammar)));

         // Display the time to parse the WSF input files
         UtTimeValue parse{ProjectWorkspace::Instance()->GetLastInputParseTime()};
         mParseTimer->setText("Parse T: " + QString::fromStdString(vespa::VaUtils::ToString(parse)));

         // Display the time to deserialize the parse tree into the proxy
         UtTimeValue deserialize{ProjectWorkspace::Instance()->GetLastProxyDeserializeTime()};
         mDeserializeTimer->setText("Deserialize T: " + QString::fromStdString(vespa::VaUtils::ToString(deserialize)));

         // Display the time to update the GUIs after deserialization
         UtTimeValue mergeGUI{ProjectWorkspace::Instance()->GetLastProxyMergeGUI_Time()};
         mMergeGUI_Timer->setText("Merge/GUI T: " + QString::fromStdString(vespa::VaUtils::ToString(mergeGUI)));
      };

      // Lambda to clear the parser/proxy timer actions
      auto clearParserTimers = [this]()
      {
         mGrammarTimer->setText("Grammar T: ");
         mParseTimer->setText("Parse T: ");
         mDeserializeTimer->setText("Deserialize T: ");
         mMergeGUI_Timer->setText("Merge/GUI T: ");
      };

      // Lambda to disable/enable actions on project close/open
      auto enableActions = [this, actions](bool enabled)
      {
         for (QAction* a : actions)
         {
            a->setEnabled(enabled);
         }
      };

      // Disable the actions initially, as no project is open
      enableActions(false);

      // Enable the actions on project open
      connect(wizard::ProjectWorkspace::Instance(),
              &wizard::ProjectWorkspace::ProjectOpened,
              [enableActions]() { enableActions(true); });
      // Disable the actions on project close
      connect(wizard::ProjectWorkspace::Instance(),
              &wizard::ProjectWorkspace::ProjectClosed,
              [clearParserTimers, enableActions]()
              {
                 enableActions(false);
                 clearParserTimers();
              });
      // Populate the parser/proxy timer actions with the respective times once the full parse is complete
      connect(wizard::ProjectWorkspace::Instance(),
              &wizard::ProjectWorkspace::FullParseComplete,
              [populateParserTimers]() { populateParserTimers(); });
   }
}

void wizard::DeveloperActions::ParseTreeDump()
{
   wizard::Project::Instance()->WaitForParsing();
   ParseResults* results = wizard::Project::Instance()->GetParseResults();
   if (results && results->parseTree())
   {
      bool verbose =
         QMessageBox::question(nullptr, "Input", "Verbose?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
      std::stringstream ss;
      results->parseTree()->Print(ss, 0, verbose);
      QPlainTextEdit* ctrl = new QPlainTextEdit;
      ctrl->resize(500, 500);
      ctrl->setPlainText(ss.str().c_str());
      ctrl->show();
   }
}

void wizard::DeveloperActions::ProxyDump()
{
   if (wizard::Project::Instance())
   {
      OpenProxyDumpWindow(wizard::Project::Instance());
   }
}

void wizard::DeveloperActions::MemoryUsage()
{
   TextSourceCache*                  txtCache         = ProjectWorkspace::Instance()->GetSourceCache();
   const TextSourceCache::SourceMap& sources          = txtCache->GetSources();
   size_t                            bytesInDocuments = 0;
   for (auto&& it : sources)
   {
      UtTextDocument* doc = it.second->GetSource();
      if (doc)
      {
         bytesInDocuments += doc->GetText().GetAllocationSize();
      }
   }
   QString report = QString("Documents: %1K").arg(bytesInDocuments / 1000.0);
   QMessageBox::information(wkfEnv.GetMainWindow(), "Memory Usage", report);
}

void wizard::DeveloperActions::WatchProxy()
{
   bool        ok = false;
   std::string pathStr =
      QInputDialog::getText(wkfEnv.GetMainWindow(), "Watch Proxy", "Path to watch", QLineEdit::Normal, "", &ok).toStdString();
   if (ok)
   {
      WsfPProxyNode root = WsfPProxyNode::FromProxy(wizard::Project::Instance()->GetParseResults()->Proxy());
      WsfPProxyNode node(root.GetProxy(), WsfPProxyPath::FromString(root.GetValue(), pathStr));
      if (!node)
      {
         QMessageBox::information(wkfEnv.GetMainWindow(), "Bad Path", "Couldn't find value at path");
      }
      else
      {
         ProxyWatchWindow* ww = new ProxyWatchWindow(node.GetPath());
         ww->setWindowTitle(node.GetPathString().c_str());
         ww->show();
      }
   }
}

void wizard::DeveloperActions::ObjectMode()
{
   wizard::Project::Instance()->WaitForParsing();
   wizSignals->emitEditModeSwitch(Vi::cEDIT_MODE_VISUAL);
}

void wizard::DeveloperActions::OpenProxyDumpWindow(wizard::Project* aProject)
{
   new ProxyDumpWindow(aProject);
}

wizard::ProxyDumpWindow::ProxyDumpWindow(wizard::Project* aProjectPtr)
{
   setLayout(new QVBoxLayout());
   mTextPtr         = new QTextEdit(this);
   QWidget* options = new QWidget();
   options->setLayout(new QHBoxLayout);

   QPushButton* refreshBn = new QPushButton("Refresh", options);
   connect(refreshBn, &QPushButton::clicked, [this]() { UpdateDisplay(true); });
   options->layout()->addWidget(refreshBn);
   mPathInput = new QLineEdit(options);
   mPathInput->setToolTip("watching proxy path");
   connect(mPathInput, &QLineEdit::editingFinished, [this]() { UpdateDisplay(true); });
   options->layout()->addWidget(mPathInput);
   layout()->addWidget(options);

   layout()->addWidget(mTextPtr);
   mCurrentText = "";
   mScenario    = aProjectPtr;
   mUpdateTimer.setInterval(1000);
   connect(&mUpdateTimer, &QTimer::timeout, [this]() { UpdateDisplay(false); });
   mUpdateTimer.start();
   show();
   UpdateDisplay(false);
}

void wizard::ProxyDumpWindow::UpdateDisplay(bool aForce)
{
   if (!mScenario.IsValid())
   {
      close();
   }
   else
   {
      wizard::Project* scenarioPtr = mScenario.Get();
      ParseResults*    results     = scenarioPtr->GetParseResults();
      if (results)
      {
         WsfPProxyValue newRoot = results->ProxyRoot();
         if (newRoot && (aForce || !(newRoot == mRoot)))
         {
            mRoot = newRoot;

            WsfPProxyPath  path           = WsfPProxyPath::FromString(mRoot, mPathInput->text().toStdString());
            WsfPProxyValue displayedValue = path.Lookup(mRoot);


            std::stringstream ss;
            const int         outputFlags = WsfProxy::cJO_HIDE_UNSET_VALUES | WsfProxy::cJO_SHOW_INHERITANCE;
            results->ProxyRegistry()->OutputJSON(displayedValue, mRoot, ss, outputFlags);
            int         curScroll = mTextPtr->verticalScrollBar()->value();
            std::string newText   = ss.str();

            std::vector<std::string> currentLines = UtStringUtil::Split(mCurrentText, '\n');
            std::vector<std::string> newLines     = UtStringUtil::Split(newText, '\n');

            auto diffs = UtSequenceDiffLines::DiffLarge(currentLines, newLines, std::equal_to<std::string>());
            if (diffs.size() == 1 && diffs.front().mSectionType == UtSequenceDiffSection::cSAME)
            {
               // no real changes
            }
            else
            {
               mTextPtr->setPlainText("");
               QTextCursor     cursor = mTextPtr->textCursor();
               QTextCharFormat newFmt = cursor.charFormat();
               QTextCharFormat oldFmt = cursor.charFormat();
               oldFmt.setBackground(qApp->palette().color(QPalette::Active, QPalette::Base));
               newFmt.setBackground(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_highlight"));
               for (const auto& diff : diffs)
               {
                  if (diff.mSectionType == UtSequenceDiffSection::cADDED)
                  {
                     std::string lines =
                        UtStringUtil::Join("\n",
                                           std::vector<std::string>(newLines.begin() + diff.mAfterRange.first,
                                                                    newLines.begin() + diff.mAfterRange.second));
                     lines += '\n';
                     cursor.insertText(lines.c_str(), newFmt);
                  }
                  else if (diff.mSectionType == UtSequenceDiffSection::cSAME)
                  {
                     std::string lines =
                        UtStringUtil::Join("\n",
                                           std::vector<std::string>(newLines.begin() + diff.mAfterRange.first,
                                                                    newLines.begin() + diff.mAfterRange.second));
                     lines += '\n';
                     cursor.insertText(lines.c_str(), oldFmt);
                  }
               }
               mCurrentText = newText;
               // setPlainText(newText);

               mTextPtr->verticalScrollBar()->setValue(curScroll);
            }
         }
      }
   }
}

wizard::ProxyWatchWindow::ProxyWatchWindow(const WsfPProxyPath& aPath)
{
   mCallbacks += WatchProxy(aPath).AnyUpdate.Connect(&ProxyWatchWindow::AnyUpdate, this);
   mCallbacks += WatchProxy(aPath).ChildUpdate.Connect(&ProxyWatchWindow::DirectUpdate, this);
}

void wizard::ProxyWatchWindow::AnyUpdate(const ProxyChange& aChange)
{
   std::stringstream ss;
   ss << "ANY UPDATE\n";
   PrintWhat(aChange, ss);
   Log(ss.str());
}

void wizard::ProxyWatchWindow::DirectUpdate(const ProxyChange& aChange)
{
   std::stringstream ss;
   ss << "CHILD UPDATE\n";
   PrintWhat(aChange, ss);
   Log(ss.str());
}

void wizard::ProxyWatchWindow::PrintWhat(const ProxyChange& aChange, std::ostream& os)
{
   os << "  PATH: " << aChange.changed().GetPathString() << '\n';
   os << "  REASON: ";
   switch (aChange.reason())
   {
   case ProxyChange::cADDED:
      os << "ADDED";
      break;
   case ProxyChange::cREMOVED:
      os << "REMOVED";
      break;
   case ProxyChange::cRENAMED:
      os << "RENAMED from " << aChange.oldName();
      break;
   case ProxyChange::cUPDATED:
      os << "UPDATED";
      break;
   case ProxyChange::cLIST_INSERTION:
      os << "LIST_INSERTION";
      break;
   case ProxyChange::cLIST_DELETION:
      os << "LIST_DELETION";
      break;
   default:
      assert(false);
      break;
   }
   os << '\n';
}

void wizard::ProxyWatchWindow::Log(std::string aLog)
{
   QTextCursor cur = textCursor();
   cur.setPosition(0);
   cur.insertText(aLog.c_str());
}
