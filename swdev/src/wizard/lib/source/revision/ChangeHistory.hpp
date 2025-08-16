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

#ifndef CHANGEHISTORY_HPP
#define CHANGEHISTORY_HPP

#include <memory>

#include <QDialog>

#include "UtPath.hpp"
#include "ui_ChangeHistory.h"

class QDateTime;
class QListWidgetItem;

namespace wizard
{
class RevisionStore;

//! A class representing the Change History window. This is the main user
//! interface for accessing the backup/restore backend.
class ChangeHistory : public QDialog
{
   Q_OBJECT
public:
   explicit ChangeHistory(RevisionStore& aRevisionStore, const UtPath& aProjectDir, QWidget* aParent = nullptr);
   ChangeHistory(const ChangeHistory&) = delete;
   ChangeHistory(ChangeHistory&&)      = default;
   ChangeHistory& operator=(const ChangeHistory&) = delete;
   ChangeHistory& operator=(ChangeHistory&&) = default;
   ~ChangeHistory() override                 = default;

public slots:
   void RestoreSelectedRevision();
   void ExportSelectedRevision();
   void DeleteAllRevisions();
   void UpdateWidgets();
   void UpdateDiff();
   void NavigateDiff();

private:
   QString Anchor(size_t listIndex);
   int     SelectedRevNo() const;
   bool    IsRevSelected() const;
   bool    RestoreConfirmation();
   bool    DeleteAllConfirmation();
   void    SetDiffTabWidth();
   void    PopulateRevisions();

   QString GetWorkingDirChangesHtml(int aRevNo);
   QString GetStartupFilesChangesHtml(int aRevNo);
   QString GetFileDiffsHtml(int aRevNo);

   QString GetAbsoluteDateTimeText(const QDateTime& time);
   QString GetDateTimeText(const QDateTime& time);

   bool IsYesterday(const QDateTime& time) const;
   int  SecondsAgo(const QDateTime& time) const;
   int  MinutesAgo(const QDateTime& time) const;
   int  HoursAgo(const QDateTime& time) const;
   int  DaysAgo(const QDateTime& time) const;

   RevisionStore&    mRevisionStore;
   UtPath            mProjectDir;
   Ui::ChangeHistory mUi;
};
} // namespace wizard

#endif // CHANGEHISTORY_HPP
