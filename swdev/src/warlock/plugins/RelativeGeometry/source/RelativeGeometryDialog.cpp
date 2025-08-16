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

#include "RelativeGeometryDialog.hpp"

#include <QContextMenuEvent>
#include <QDrag>
#include <QMenu>
#include <QMimeData>

#include "RelativeGeometryPlotUpdater.hpp"
#include "RelativeGeometryUpdater.hpp"
#include "WkPlottingWidget.hpp"
#include "WkfDefines.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatformGrouping.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

RelativeGeometry::Dialog::Dialog(QWidget*                  aWidget,
                                 const QString&            aFrom,
                                 const QString&            aTo,
                                 const wkf::SpatialDomain& aDomain,
                                 const size_t              aPluginId)
   : QDockWidget(aWidget)
   , mPluginId(aPluginId)
{
   setAttribute(Qt::WA_DeleteOnClose);
   mUi.setupUi(this);
   mUi.mSwapPlatformsPushButton->setIcon(QIcon::fromTheme("swap"));

   // populate
   mUi.mFrom->SetPlatformName(aFrom);
   mUi.mTo->SetPlatformName(aTo);

   if (aDomain == wkf::SPATIAL_DOMAIN_SPACE)
   {
      mUi.mDomain->setCurrentText("Satellite");
   }
   else if (aDomain == wkf::SPATIAL_DOMAIN_LAND || aDomain == wkf::SPATIAL_DOMAIN_SURFACE)
   {
      mUi.mDomain->setCurrentText("Surface");
   }
   else
   {
      mUi.mDomain->setCurrentText("Standard");
   }
   mUi.mValues->setMouseTracking(true);

   mUi.mValues->viewport()->installEventFilter(this);
   Update();

   connect(mUi.mFrom, &wkf::PlatformWidget::PlatformChanged, [this]() { Update(); });
   connect(mUi.mTo, &wkf::PlatformWidget::PlatformChanged, [this]() { Update(); });
   connect(mUi.mDomain, &QComboBox::currentTextChanged, [this]() { Update(); });
   connect(mUi.mValues, &wkf::DraggableTreeWidget::DragEvent, this, &Dialog::DragEvent);
   connect(mUi.mSwapPlatformsPushButton, &QPushButton::clicked, this, &Dialog::SwapPlatforms);
}

void RelativeGeometry::Dialog::GuiUpdate()
{
   if (mLinearRangeUpdater)
   {
      if (!mLinearRangeItem)
      {
         mLinearRangeItem = new QTreeWidgetItem(mUi.mValues);
         mLinearRangeItem->setText(0, "Range (linear)");
         mLinearRangeItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mLinearRangeItem->setText(1, mLinearRangeUpdater->GetValueString());
   }
   else if (mLinearRangeItem)
   {
      delete mLinearRangeItem;
      mLinearRangeItem = nullptr;
   }
   if (mRadialUpdater)
   {
      if (!mRadialItem)
      {
         mRadialItem = new QTreeWidgetItem(mUi.mValues);
         mRadialItem->setText(0, "Radial");
         mRadialItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mRadialItem->setText(1, mRadialUpdater->GetValueString());
   }
   else
   {
      delete mRadialItem;
      mRadialItem = nullptr;
   }
   if (mInTrackUpdater)
   {
      if (!mInTrackItem)
      {
         mInTrackItem = new QTreeWidgetItem(mUi.mValues);
         mInTrackItem->setText(0, "In-Track");
         mInTrackItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mInTrackItem->setText(1, mInTrackUpdater->GetValueString());
   }
   else
   {
      delete mInTrackItem;
      mInTrackItem = nullptr;
   }
   if (mCrossTrackUpdater)
   {
      if (!mCrossTrackItem)
      {
         mCrossTrackItem = new QTreeWidgetItem(mUi.mValues);
         mCrossTrackItem->setText(0, "Cross-Track");
         mCrossTrackItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mCrossTrackItem->setText(1, mCrossTrackUpdater->GetValueString());
   }
   else
   {
      delete mCrossTrackItem;
      mCrossTrackItem = nullptr;
   }
   if (mBearingUpdater)
   {
      if (!mBearingItem)
      {
         mBearingItem = new QTreeWidgetItem(mUi.mValues);
         mBearingItem->setText(0, "Bearing");
         mBearingItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mBearingItem->setText(1, mBearingUpdater->GetValueString());
   }
   else
   {
      delete mBearingItem;
      mBearingItem = nullptr;
   }
   if (mElevationUpdater)
   {
      if (!mElevationItem)
      {
         mElevationItem = new QTreeWidgetItem(mUi.mValues);
         mElevationItem->setText(0, "Elevation");
         mElevationItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mElevationItem->setText(1, mElevationUpdater->GetValueString());
   }
   else
   {
      delete mElevationItem;
      mElevationItem = nullptr;
   }
   if (mDownRangeUpdater)
   {
      if (!mDownRangeItem)
      {
         mDownRangeItem = new QTreeWidgetItem(mUi.mValues);
         mDownRangeItem->setText(0, "Down Range");
         mDownRangeItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mDownRangeItem->setText(1, mDownRangeUpdater->GetValueString());
   }
   else
   {
      delete mDownRangeItem;
      mDownRangeItem = nullptr;
   }
   if (mCrossRangeUpdater)
   {
      if (!mCrossRangeItem)
      {
         mCrossRangeItem = new QTreeWidgetItem(mUi.mValues);
         mCrossRangeItem->setText(0, "Cross Range");
         mCrossRangeItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mCrossRangeItem->setText(1, mCrossRangeUpdater->GetValueString());
   }
   else
   {
      delete mCrossRangeItem;
      mCrossRangeItem = nullptr;
   }
   if (mRangeRateUpdater)
   {
      if (!mRangeRateItem)
      {
         mRangeRateItem = new QTreeWidgetItem(mUi.mValues);
         mRangeRateItem->setText(0, "Range Rate");
         mRangeRateItem->setData(0, wkf::UPDATER_ROLE, true);
      }
      mRangeRateItem->setText(1, mRangeRateUpdater->GetValueString());
   }
}

void RelativeGeometry::Dialog::SwapPlatforms()
{
   // swap the text in mFrom and mTo line edits
   QString temp = mUi.mFrom->GetPlatformName();
   mUi.mFrom->SetPlatformName(mUi.mTo->GetPlatformName());
   mUi.mTo->SetPlatformName(temp);

   // update the display
   Update();
}

void RelativeGeometry::Dialog::Update()
{
   mLinearRangeUpdater = wkf::make_updater<LinearRangeUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
   mRangeRateUpdater   = wkf::make_updater<RangeRateUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
   // clear then add updaters

   if (mUi.mDomain->currentText() == "Satellite")
   {
      mBearingUpdater    = nullptr;
      mElevationUpdater  = nullptr;
      mDownRangeUpdater  = nullptr;
      mCrossRangeUpdater = nullptr;

      mRadialUpdater     = wkf::make_updater<RadialUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mInTrackUpdater    = wkf::make_updater<InTrackUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mCrossTrackUpdater = wkf::make_updater<CrossTrackUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
   }
   else if (mUi.mDomain->currentText() == "Surface")
   {
      mRadialUpdater     = nullptr;
      mInTrackUpdater    = nullptr;
      mCrossTrackUpdater = nullptr;

      mBearingUpdater   = wkf::make_updater<BearingUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mElevationUpdater = wkf::make_updater<ElevationUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mDownRangeUpdater = wkf::make_updater<DownRangeUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mCrossRangeUpdater = wkf::make_updater<CrossRangeUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
   }
   else
   {
      mRadialUpdater     = nullptr;
      mInTrackUpdater    = nullptr;
      mCrossTrackUpdater = nullptr;
      mDownRangeUpdater  = nullptr;
      mCrossRangeUpdater = nullptr;

      mBearingUpdater   = wkf::make_updater<BearingUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
      mElevationUpdater = wkf::make_updater<ElevationUpdater>(mUi.mFrom->GetPlatformName(), mUi.mTo->GetPlatformName());
   }
}

bool RelativeGeometry::Dialog::eventFilter(QObject* aObj, QEvent* aEvent)
{
   if (aEvent->type() == QEvent::ContextMenu)
   {
      QContextMenuEvent* evt  = dynamic_cast<QContextMenuEvent*>(aEvent);
      QTreeWidgetItem*   item = mUi.mValues->itemAt(evt->pos());
      QMenu              ctxMenu(this);
      QString            text   = QString("Plot");
      QAction*           action = new QAction(QIcon::fromTheme("plot"), text, &ctxMenu);
      connect(action, &QAction::triggered, [this, item]() { PlotItem(item); });
      ctxMenu.addAction(action);
      ctxMenu.exec(QCursor::pos());
      return true;
   }
   return false;
}

void RelativeGeometry::Dialog::PlotItem(QTreeWidgetItem* aItemPtr) const
{
   QString fromName = mUi.mFrom->GetPlatformName();
   QString toName   = mUi.mTo->GetPlatformName();

   std::unique_ptr<PlotUpdater> updaterPtr =
      ut::make_unique<RelativeGeometry::PlotUpdater>(aItemPtr->text(0), fromName, toName);
   if (updaterPtr != nullptr)
   {
      warlock::PlottingWidget* plotWidget =
         vaEnv.GetFactory()->CreateDockWidget<warlock::PlottingWidget>("PlottingWidget",
                                                                       wkfEnv.GetMainWindow()->centralWidget());
      plotWidget->SetUpdater(std::move(updaterPtr));
      plotWidget->show();
   }
}

void RelativeGeometry::Dialog::DragEvent(QTreeWidgetItem* aItemPtr)
{
   QMimeData*  mimeData = new QMimeData;
   QByteArray  bytes;
   QDataStream out(&bytes, QIODevice::WriteOnly);

   unsigned int pluginId = static_cast<unsigned int>(mPluginId); // this is added to the item
   out << pluginId;
   QString title = mUi.mFrom->GetPlatformName() + " to " + mUi.mTo->GetPlatformName();
   out << title;
   out << aItemPtr->text(0);                                 // datum
   out << 0;                                                 // context
   mimeData->setData(wkf::Updater::cMIME_IDENTIFIER, bytes); // need to give an address to the updater here
   QDrag* drag = new QDrag(this);
   drag->setMimeData(mimeData);
   drag->exec(Qt::DropAction::LinkAction);
}
