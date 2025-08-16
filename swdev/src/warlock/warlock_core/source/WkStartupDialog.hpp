// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WkStartupDialog_HPP
#define WkStartupDialog_HPP

#include "warlock_core_export.h"

#include "WkfStartupDialog.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT StartupDialog : public wkf::StartupDialog<QStringList>
{
public:
   StartupDialog(QStringList&    aInputFiles,
                 bool            aDisableFileBrowsing,
                 QWidget*        aParent = nullptr,
                 Qt::WindowFlags aFlags  = Qt::WindowFlags());

private:
   QStringList               Browse() override;
   void                      RemoveRecent(const QStringList& aRecent) override;
   void                      ClearRecents() override;
   const QList<QStringList>& GetRecents() const override;
   QString     GetRecentLabelDisplayName(const QStringList& aFileList, bool aDisplayFullPath) const override;
   QString     RunActionText() const override { return "Run Scenario"; }
   void        BuildRecentContextMenu(QMenu& aMenu, const QStringList& aLink) override;
   void        PopulateLabels() override;
   bool        ValidateSelection() override;
   QStringList ParseDroppedUrls(const QList<QUrl>& urls) const override;
};
} // namespace warlock
#endif // WkStartupDialog_HPP
