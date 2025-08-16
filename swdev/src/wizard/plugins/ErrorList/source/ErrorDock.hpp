// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ERRORDOCK_HPP
#define ERRORDOCK_HPP

#include <QDockWidget>

#include "UtTextDocument.hpp"
#include "ui_ErrorListControl.h"

namespace wizard
{
class Project;
class TextLocationItemModel;
} // namespace wizard

namespace ErrorList
{
class ErrorDock : public QDockWidget
{
public:
   ErrorDock(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~ErrorDock() override = default;

private:
   void ProjectOpened(wizard::Project* aProjectPtr);
   void ProjectClosed(wizard::Project* aProjectPtr);
   void ItemActivated(const QModelIndex& aIndex);
   void Update();
   void showEvent(QShowEvent* aEventPtr) override;


   static bool CompareRange(const UtTextDocumentRange& aLhs, const UtTextDocumentRange& aRhs);
   static bool RangeEqual(const UtTextDocumentRange& aLhs, const UtTextDocumentRange& aRhs);

   Ui::ErrorListControlUi mUi;

   std::vector<UtTextDocumentRange> mErrorList;

   wizard::TextLocationItemModel* mModelPtr;

   wizard::Project* mProjectPtr;
};
} // namespace ErrorList

#endif
