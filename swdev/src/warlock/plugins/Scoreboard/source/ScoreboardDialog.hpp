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

#ifndef SCOREBOARDDIALOG_HPP
#define SCOREBOARDDIALOG_HPP

#include <map>
#include <set>

#include <QDialog>
#include <QMenu>
#include <QString>

#include "ScoreboardDataContainer.hpp"
#include "ui_ScoreboardDialog.h"

struct FireStats
{
   int totalFired;
   int totalHit;
   int totalKill;
   int totalMiss;
};

namespace WkScoreboard
{
enum class FilterOptions
{
   PlatformName,
   PlatformType,
   Team,
   WeaponType
};

static const std::map<FilterOptions, QString> mFilterQStringMap = {{FilterOptions::PlatformName, "Platform Name"},
                                                                   {FilterOptions::PlatformType, "Platform Type"},
                                                                   {FilterOptions::Team, "Team Name"},
                                                                   {FilterOptions::WeaponType, "Weapon Type"}};

class Dialog : public QDialog
{
   Q_OBJECT

public:
   Dialog(DataContainer& aScoreboardData, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

   void Update();

private:
   void AddToMap(std::string aKey, WeaponEventType aWeaponEventType);
   void Build();
   void BuildHighlightedSelection(int row, std::map<std::string, FireStats>::iterator aIt);
   void Connect();
   void ExportData();
   void FiringFilterComboBoxChanged();
   void FiringFilterDetailedComboBoxChanged();
   void HelperButtonClicked();
   void SetDefaultValues();
   void ShowContextMenu(const QPoint& pos);

   const QString ALL = "All";

   Ui::ScoreboardDialog             mUi;
   DataContainer&                   mScoreboardData;
   std::map<std::string, FireStats> mData;
   FilterOptions                    mCurrentFiringFilter;
   size_t                           mPreviousVectorSize;
   std::set<std::string>            mComboBoxOptions;
   bool                             mUserChange;
   QString                          mPreviousComboString;
};
} // namespace WkScoreboard

#endif
