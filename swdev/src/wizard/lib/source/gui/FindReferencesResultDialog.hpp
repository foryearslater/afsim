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
#ifndef FINDREFERENCESRESULTDIALOG_HPP
#define FINDREFERENCESRESULTDIALOG_HPP

#include <vector>

#include <QDialog>

#include "../wizard_core/ui_FindReferencesResultDialog.h"
#include "TextLocationItemModel.hpp"
#include "UtMemory.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"

namespace wizard
{
class Project;

class VI_EXPORT FindReferencesResultDialog : public QDialog
{
public:
   explicit FindReferencesResultDialog(QWidget* aParentPtr);
   FindReferencesResultDialog(const FindReferencesResultDialog&) = delete;
   FindReferencesResultDialog(FindReferencesResultDialog&&)      = default;
   FindReferencesResultDialog& operator=(const FindReferencesResultDialog&) = delete;
   FindReferencesResultDialog& operator=(FindReferencesResultDialog&&) = default;
   ~FindReferencesResultDialog() override                              = default;
   void Show(Project* aProjectPtr, const std::vector<UtTextDocumentRange>& aLocationList, QString aLabel = QString());

   static void GoToLocationList(wizard::Project*                        aProjectPtr,
                                const std::vector<UtTextDocumentRange>& aLocationList,
                                QString                                 aLabel = QString());

protected:
   void ItemActivated(const QModelIndex& aIndex);

private:
   void                           JumpTo();
   Ui::FindReferencesResultDialog mUi;

   Project*                               mProjectPtr;
   std::unique_ptr<TextLocationItemModel> mModelPtr;
};
} // namespace wizard
#endif
