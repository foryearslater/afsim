// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef RELATIVEGEOMETRYDIALOG_HPP
#define RELATIVEGEOMETRYDIALOG_HPP

#include <QDockWidget>

#include "WkfUpdater.hpp"
#include "ui_WkfRelativeGeometryDialog.h"

namespace RelativeGeometry
{
class Dialog : public QDockWidget
{
public:
   Dialog(QWidget* aWidget, const QString& aFrom, const QString& aTo, const wkf::SpatialDomain& aDomain, const size_t aPluginId);

   ~Dialog() override = default;
   void GuiUpdate();
private slots:
   void SwapPlatforms();

private:
   void Update();
   bool eventFilter(QObject* aObj, QEvent* aEvent) override;
   void PlotItem(QTreeWidgetItem* aItemPtr) const;
   void DragEvent(QTreeWidgetItem* aItemPtr);

   Ui::WkfRelativeGeometryDialogUi mUi;
   size_t                          mPluginId;

   std::unique_ptr<wkf::Updater> mLinearRangeUpdater;
   QTreeWidgetItem*              mLinearRangeItem{nullptr};
   std::unique_ptr<wkf::Updater> mRadialUpdater;
   QTreeWidgetItem*              mRadialItem{nullptr};
   std::unique_ptr<wkf::Updater> mInTrackUpdater;
   QTreeWidgetItem*              mInTrackItem{nullptr};
   std::unique_ptr<wkf::Updater> mCrossTrackUpdater;
   QTreeWidgetItem*              mCrossTrackItem{nullptr};
   std::unique_ptr<wkf::Updater> mBearingUpdater;
   QTreeWidgetItem*              mBearingItem{nullptr};
   std::unique_ptr<wkf::Updater> mElevationUpdater;
   QTreeWidgetItem*              mElevationItem{nullptr};
   std::unique_ptr<wkf::Updater> mDownRangeUpdater;
   QTreeWidgetItem*              mDownRangeItem{nullptr};
   std::unique_ptr<wkf::Updater> mCrossRangeUpdater;
   QTreeWidgetItem*              mCrossRangeItem{nullptr};
   std::unique_ptr<wkf::Updater> mRangeRateUpdater;
   QTreeWidgetItem*              mRangeRateItem{nullptr};

   bool   mDragPossible{false};
   QPoint mDragStart;
};
} // namespace RelativeGeometry

#endif
