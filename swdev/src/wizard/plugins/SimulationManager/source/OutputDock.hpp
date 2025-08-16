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

#ifndef OUTPUTDOCK_HPP
#define OUTPUTDOCK_HPP

#include <QDockWidget>

#include "ui_OutputDock.h"

class QMenu;
class QTextDocument;
class UtPath;
namespace wizard
{
class WsfExecution;
class OutputPrefObject;
} // namespace wizard

namespace SimulationManager
{
class OutputDock : public QDockWidget
{
public:
   OutputDock(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~OutputDock() override = default;

   void AppendQueueToConsole();

private:
   void AnchorClicked(const QUrl& aUrl);
   void StartingExecution(wizard::WsfExecution* aExecutionPtr);
   void ExecutionDone();
   void ScanLogFile(const UtPath& aPath);
   void KillClick();
   void SelectOutputLogFile();
   void OutputLogLinkClicked(const QString& aPath);
   void SetOutputLogLabel();

   QString               mOutputLogPath;
   wizard::WsfExecution* mExecutionPtr;
   Ui::OutputDock        mUi;
   QMenu*                mResultsMenuPtr;

   QMetaObject::Connection mExecutionDoneConnection;
};
} // namespace SimulationManager

#endif
