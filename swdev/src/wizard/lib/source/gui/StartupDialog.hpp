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

#ifndef StartupDialog_HPP
#define StartupDialog_HPP

#include "ViExport.hpp"
#include "WkfStartupDialog.hpp"

namespace wizard
{
class VI_EXPORT StartupDialog : public wkf::StartupDialog<QStringList>
{
public:
   StartupDialog(QStringList& aInputFiles, QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());

private:
   QStringList               Browse() override;
   void                      RemoveRecent(const QStringList& aRecent) override;
   void                      ClearRecents() override;
   const QList<QStringList>& GetRecents() const override;
   QString GetRecentLabelDisplayName(const QStringList& aFileList, bool aDisplayFullPath) const override;
   QString RunActionText() const override { return "Open Project"; }
   void    BuildRecentContextMenu(QMenu& aMenu, const QStringList& aLink) override;
   void    PopulateLabels() override;

   bool        ValidateSelection() override;
   QStringList ParseDroppedUrls(const QList<QUrl>& urls) const override;
};
} // namespace wizard
#endif // StartupDialog_HPP
