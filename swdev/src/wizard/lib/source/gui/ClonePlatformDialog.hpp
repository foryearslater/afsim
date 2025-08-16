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
#ifndef CLONEPLATFORMDIALOG_HPP
#define CLONEPLATFORMDIALOG_HPP
#include <QDialog>
#include <QTimer>

#include "UtMemory.hpp"
#include "ViExport.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPM_Root.hpp"
#include "ui_CloneDialog.h"

class QMimeData;
class QStandardItem;
class QStandardItemModel;
class UtLatPos;
class UtLonPos;

namespace wizard
{
class PasteContext;

class VI_EXPORT ClonePlatformDialog : public QDialog
{
   Q_OBJECT

public:
   static void PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute);

   explicit ClonePlatformDialog(QWidget* aParentWindow);
   ~ClonePlatformDialog() override = default;

   void SetReferenceLocation(const UtLatPos& aLat, const UtLonPos& aLon);
   void Show(const std::vector<WsfPM_Platform>& aPlatforms);
   void SetDestinationFilePath(const std::string& aFilePath) { mDestinationFilePath = aFilePath; }

private slots:

   void GenerateNames();
   void ItemChanged(QStandardItem*);
   void ValidateNames();
   void UpdateAcceptStatus();

private:
   using QDialog::show; // private
   WsfPProxy*  GetProxy();
   WsfPM_Root  GetRoot();
   void        accept() override;
   void        reject() override;
   std::string GetNewName(const std::string& aOldName);


   void Clone(const WsfPM_Platform& aPlatform, const std::string& aName);

   QStandardItemModel*         mModelPtr;
   QTimer                      mDelayedValidationTimer;
   Ui::CloneDialog             mUi;
   std::vector<WsfPM_Platform> mPlatforms;
   double                      mCentroidLat;
   double                      mCentroidLon;
   bool                        mNamesValid;
   std::string                 mDestinationFilePath;
   bool                        mApplyToTextImmediately;
};
} // namespace wizard

#endif
