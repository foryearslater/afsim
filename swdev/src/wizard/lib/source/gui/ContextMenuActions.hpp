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

#ifndef CONTEXTMENUACTIONS_HPP
#define CONTEXTMENUACTIONS_HPP

#include <string>

#include <QAction>
#include <QPoint>
#include <QString>
#include <QWidget>

#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"

namespace wizard
{
namespace ContextMenuActions
{

class VI_EXPORT OpenFileInProject : public QAction
{
   Q_OBJECT
public:
   OpenFileInProject(const std::string& aFileFullPath, QWidget* aParentWidget = nullptr, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   std::string mFilePath;
   QWidget*    mParentWidgetPtr;
};

// -----------------------------------------------------------------------------

//! The QAction subclasses above can use this object to report messages (e.g.
//! errors) via a message box back up to the parent widget.
class VI_EXPORT MessageReporter : public QObject
{
   Q_OBJECT
public:
   MessageReporter(const QString& aMessageTitle, QWidget* aParentWidget, QObject* aParent = nullptr);
public slots:
   void ShowWarningMessage(const QString& aMessage);

private:
   QString  mMessageTitle;
   QWidget* mParentWidgetPtr;
};

// -----------------------------------------------------------------------------

class VI_EXPORT SetAsStartupFile : public QAction
{
   Q_OBJECT
public:
   SetAsStartupFile(const QString& aFilePath, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   QString mFilePath;
};

// -----------------------------------------------------------------------------

class VI_EXPORT AddToStartupFiles : public QAction
{
   Q_OBJECT
public:
   AddToStartupFiles(const QString& aFilePath, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   QString mFilePath;
};

// -----------------------------------------------------------------------------

class VI_EXPORT RemoveFromStartupFiles : public QAction
{
   Q_OBJECT
public:
   RemoveFromStartupFiles(const QString& aFilePath, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   QString mFilePath;
};

// -----------------------------------------------------------------------------

class VI_EXPORT ShowStartupFilesDialog : public QAction
{
   Q_OBJECT
public:
   explicit ShowStartupFilesDialog(QObject* aParent = nullptr);
private slots:
   void OnTriggered();
};

// -----------------------------------------------------------------------------

class VI_EXPORT OpenFileInEditor : public QAction
{
   Q_OBJECT
public:
   OpenFileInEditor(const QString& aFilePath, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   QString mFilePath;
};

} // namespace ContextMenuActions
} // namespace wizard

#endif // CONTEXTMENUACTIONS_HPP
