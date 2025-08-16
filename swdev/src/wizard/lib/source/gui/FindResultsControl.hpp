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

#ifndef FINDRESULTSCONTROL_HPP
#define FINDRESULTSCONTROL_HPP

#include <QDockWidget>
#include <QModelIndex>

#include "FindTextRequest.hpp"
#include "UtTextDocument.hpp"
#include "ui_FindResultsControl.h"

class QLabel;

namespace wizard
{
class EditorManager;
class Project;
class TextLocationItemModel;

class FindResultsControl : public QDockWidget
{
   Q_OBJECT
public:
   FindResultsControl(QMainWindow* aParentPtr, EditorManager* aEditorPtr);
   ~FindResultsControl() override = default;

   void NewFind(FindTextRequest aRequest);

   //       virtual bool eventFilter(QObject* aObjectPtr,
   //                                QEvent*  aEventPtr);
protected slots:
   void ProjectOpened(Project* aProjectPtr);
   void ProjectClosed(Project* aProjectPtr);
   void ItemActivated(const QModelIndex& aIndex);

protected:
   // void ItemDoubleClicked();
   void AddResult(TextSource* aSourcePtr, int aCharOffset, int aCharOffsetEnd);

   int SearchFile(const FindTextRequest& aRequest, TextSource* aSourcePtr, const std::string& aLowerCaseString);

   void SearchDirectory(const QString& aDirectory, const FindTextRequest& aRequest, const std::string& aLowerCaseString);

   EditorManager*                             mEditorPtr;
   Ui::FindResultsControl                     mUi;
   TextLocationItemModel*                     mModelPtr;
   std::vector<UtTextDocumentAutoUpdateRange> mResults;
   Project*                                   mProjectPtr;
   QLabel*                                    mWaitBox;
};
} // namespace wizard

#endif
