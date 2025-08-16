// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QMainWindow>

#include "OutputItem.hpp"
#include "OutputRateItem.hpp"
#include "ui_Engage.h"

class QCloseEvent;


namespace Engage
{
class OutputItem;
class OutputRateItem;

class Dialog : public QDialog
{
public:
   Dialog(QWidget* aParent = nullptr, Qt::WindowFlags flags = nullptr);
   ~Dialog() override = default;


   QString GetLauncherPath() const { return mLauncherPath; }
   QString GetTrackerPath() const { return mTrackerPath; }
   QString GetTargetPath() const { return mTargetPath; }
   QString GetWeaponPath() const { return mWeaponPath; }

   std::vector<const QString*> GetLauncherList() const { return mLauncherList; }
   std::vector<const QString*> GetTrackerList() const { return mTrackerList; }
   std::vector<const QString*> GetTargetList() const { return mTargetList; }
   std::vector<const QString*> GetWeaponList() const { return mWeaponList; }
   std::vector<const QString*> GetRadarSigList() const { return mRadarSignatureList; }
   std::vector<const QString*> GetIconList() const { return mIconList; }
   std::vector<const QString*> GetSideList() const { return mSideList; }

private:
   void PopulateLists(QString aDirPath, std::vector<const QString*>* aCurList);

   void CreateFileCB();
   void AddRunOutputCB();
   void AddOutputRateCB();
   void RemoveOutputRateCB();
   void AddEventCB();
   void RemoveEventCB();
   void TargetBehaviorChangedCB(int aIndex);
   void SiteBehaviorChangedCB(int aIndex);
   void AddTargetFlightPathCB();
   void RemoveTargetFlightPathCB();
   void AddTargetFlightRouteCB();
   void RemoveTargetFlightRouteCB();
   void AddSiteCB();
   void RemoveSiteCB();
   void BrowseRootCB();
   void RootPathEditedCB(const QString& aStr);
   void SelectPkDirectoryCB();
   void closeEvent(QCloseEvent* event) override;
   void DeleteRunOutputCB();
   void EditRunOutputCB();
   void DeleteOutputRateCB();
   void EditOutputRateCB();

   QString GetRelativePath(QString aRootDir, QString aOtherDir);

   Ui::Dialog mUi;

   std::vector<const QString*> mLauncherList;
   std::vector<const QString*> mTrackerList;
   std::vector<const QString*> mTargetList;
   std::vector<const QString*> mWeaponList;
   std::vector<const QString*> mRadarSignatureList;
   std::vector<const QString*> mIconList;
   std::vector<const QString*> mSideList;
   std::vector<OutputItem>     mOutputItems;
   std::vector<OutputRateItem> mOutputRateItems;
   std::vector<const QString*> mIncludeFiles;

   QString mLauncherPath;
   QString mTargetPath;
   QString mTrackerPath;
   QString mWeaponPath;
   QString mRootPath;
};
} // namespace Engage

#endif
