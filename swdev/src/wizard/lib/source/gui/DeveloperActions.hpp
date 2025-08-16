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

#ifndef DEVELOPERMENU_HPP
#define DEVELOPERMENU_HPP

class QAction;
#include <QLineEdit>
class QMenu;
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>

#include "WsfPProxyValue.hpp"

namespace wizard
{
class Project;
class ProxyChange;

class DeveloperActions : public QObject
{
public:
   DeveloperActions(QObject* aParent);
   ~DeveloperActions() override = default;

private:
   void ParseTreeDump();
   void ProxyDump();
   void MemoryUsage();
   void WatchProxy();
   void ObjectMode();
   void OpenProxyDumpWindow(Project* aProject);

   //! Menu containing the parser/proxy timer actions
   QMenu* mParserTimersMenu;
   //! Action displaying the time spent parsing the WsfGrammar
   QAction* mGrammarTimer;
   //! Action displaying the time spent parsing the input files
   QAction* mParseTimer;
   //! Action displaying the time spent deserializing the parse tree into the proxy
   QAction* mDeserializeTimer;
   //! Action displaying the time spent merging the old and new proxies/updating the GUIs
   QAction* mMergeGUI_Timer;
};

class ProxyDumpWindow : public QWidget
{
   Q_OBJECT
public:
   explicit ProxyDumpWindow(Project* aProjectPtr);

private:
   void UpdateDisplay(bool aForce);

   QTimer                   mUpdateTimer;
   UtWeakReference<Project> mScenario;
   WsfPProxyValue           mRoot;
   std::string              mCurrentText;
   QTextEdit*               mTextPtr;
   QLineEdit*               mPathInput;
};

class ProxyWatchWindow : public QTextBrowser
{
public:
   explicit ProxyWatchWindow(const WsfPProxyPath& aPath);

private:
   void             AnyUpdate(const ProxyChange& aChange);
   void             DirectUpdate(const ProxyChange& aChange);
   void             PrintWhat(const ProxyChange& aChange, std::ostream& os);
   void             Log(std::string aLog);
   UtCallbackHolder mCallbacks;
};
} // namespace wizard

#endif
