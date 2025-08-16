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
#include "FindReferencesResultDialog.hpp"

#include "Project.hpp"
#include "TextLocationItemModel.hpp"
#include "UtQt.hpp"
#include "UtQtTextDelegate.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::FindReferencesResultDialog::FindReferencesResultDialog(QWidget* aParentPtr)
   : QDialog(aParentPtr)
   , mModelPtr(nullptr)
{
   mUi.setupUi(this);
   QObject::connect(mUi.jumpToBn, &QPushButton::clicked, this, &FindReferencesResultDialog::JumpTo);
   QObject::connect(mUi.referenceList, &QTreeView::activated, this, &FindReferencesResultDialog::ItemActivated);
}

void wizard::FindReferencesResultDialog::Show(Project*                                aProjectPtr,
                                              const std::vector<UtTextDocumentRange>& aLocationList,
                                              QString                                 aLabel)
{
   mProjectPtr          = aProjectPtr;
   mModelPtr            = ut::make_unique<TextLocationItemModel>(aLocationList);
   mModelPtr->mShowText = true;
   mUi.referenceList->setModel(mModelPtr.get());
   mUi.referenceList->setItemDelegateForColumn(1, new UtQtTextDelegate(mUi.referenceList));
   if (!aLabel.isEmpty())
   {
      mUi.listLabel->setText(aLabel);
   }
   else
   {
      mUi.listLabel->setText(tr("References:"));
   }
   show();
   mUi.referenceList->setFocus();
   mUi.referenceList->setCurrentIndex(mModelPtr->index(0, 0));
}

void wizard::FindReferencesResultDialog::JumpTo()
{
   int row = mUi.referenceList->currentIndex().row();
   if (row >= 0 && row < (int)mModelPtr->GetLocations().size())
   {
      UtTextDocumentRange range = mModelPtr->GetLocations()[row];
      mProjectPtr->GotoRange(range);
      hide();
   }
}
void wizard::FindReferencesResultDialog::ItemActivated(const QModelIndex& aIndex)
{
   JumpTo();
}

void wizard::FindReferencesResultDialog::GoToLocationList(Project*                                aProjectPtr,
                                                          const std::vector<UtTextDocumentRange>& aLocationList,
                                                          QString                                 aLabel /*=QString()*/)
{
   if (aLocationList.empty())
      return;
   if (aLocationList.size() == 1)
   {
      aProjectPtr->GotoRange(aLocationList[0]);
      return;
   }
   wizard::FindReferencesResultDialog* dlg = new wizard::FindReferencesResultDialog(wkfEnv.GetMainWindow());
   dlg->Show(aProjectPtr, aLocationList, aLabel);
}
