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

#ifndef WSFEXECUTION_HPP
#define WSFEXECUTION_HPP

#include <QFile>
#include <QProcess>
#include <QTextDocument>

#include "UtPath.hpp"
#include "ViExport.hpp"

namespace wizard
{
class WsfExe;

//! Encapsulates the execution of a WSF application
class VI_EXPORT WsfExecution : public QProcess
{
   Q_OBJECT

public:
   QSharedPointer<QTextDocument> mOutputText;

   static WsfExecution* NewExecution(WsfExe*                    aExe,
                                     const UtPath&              aWorkingDir,
                                     const std::vector<UtPath>& aStartupFiles,
                                     const QString&             aCmdLineArgs,
                                     const QString&             aProjectName,
                                     int                        aXioRecvPort,
                                     bool                       aUseDebugging);
   WsfExecution(QString aName, WsfExe* aExePtr, const UtPath& aWorkingDir, const QStringList& aArguments);

   void               Start(const std::vector<UtPath>& aStartupFiles);
   void               Kill();
   WsfExe*            GetExe() const { return mExePtr; }
   QStringList        GetArguments() const { return mArguments; }
   ExitStatus         GetExitStatus() const { return mExitStatus; }
   static std::string DebugSettingsString(int aPort);
   void               SetOutputLogPath(const QString& aPath);
   UtPath             GetWorkingDir() const { return mWorkingDir; }

   struct ConnectionParameters
   {
      ConnectionParameters()
         : mUsingXIO(false)
         , mXIO_Port(0)
      {
      }
      bool        mUsingXIO;
      std::string mXIO_Address;
      std::string mXIO_Interface;
      int         mXIO_Port;
   };

   ConnectionParameters mConnectionParameters;
   QString              mName; // scenario name .. temporary placeholder

signals:
   void Done();
   void StandardOutputText(const QString& aText);

protected slots:
   void ReadyReadStandardOutput();
   void Finished();

protected:
   void LimitOutputSize(QTextCursor* aCursorPtr);
   int  Exec();

   QFile       mOutputLog;
   UtPath      mWorkingDir;
   WsfExe*     mExePtr;
   QStringList mArguments;
   QRegExp     mErrExp;
   QRegExp     mInFileLineColExp;
   QRegExp     mInFileEofExp;
   bool        mDone;
   bool        mStarted;
   ExitStatus  mExitStatus;
};
} // namespace wizard

#endif
