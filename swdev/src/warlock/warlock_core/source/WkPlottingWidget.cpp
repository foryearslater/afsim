// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "WkPlottingWidget.hpp"

#include <QMessageBox>

#include "UtQtGL2DPlot.hpp"
#include "UtQtGL2DPlotWidget.hpp"
#include "WkPlotUpdater.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

warlock::PlottingWidget::PlottingWidget(QMainWindow* aParent /*= 0*/, Qt::WindowFlags aFlags /*= 0*/)
   : wkf::PlotDockWidget(aParent, aFlags)
   , mXAxisUnitName("sec")
{
   wkfEnv.GetMainWindow()->AddDockWidgetToViewMenu(this);
   connect(&wkfEnv, &wkf::Environment::UpdateFrame, this, &PlottingWidget::Update);
   connect(&wkfEnv, &wkf::Environment::CloseDialogs, this, &PlottingWidget::deleteLater);
}

void warlock::PlottingWidget::SetUpdater(std::unique_ptr<warlock::PlotUpdater> aUpdater)
{
   if (aUpdater != nullptr)
   {
      connect(aUpdater.get(), &PlotUpdater::UnitChanged, this, &PlottingWidget::UpdaterUnitsChangedCB);
      mYAxisUnitName = aUpdater->GetUnitName();
      mUpdaterList.push_back(std::move(aUpdater));
   }
}

void warlock::PlottingWidget::RemoveUpdater(PlotUpdater* aUpdater)
{
   for (auto iter = mUpdaterList.begin(); iter != mUpdaterList.end(); iter++)
   {
      if (iter->get() == aUpdater)
      {
         disconnect(iter->get(), &PlotUpdater::UnitChanged, this, &PlottingWidget::UpdaterUnitsChangedCB);
         mUpdaterList.erase(iter);
         return;
      }
   }
}

void warlock::PlottingWidget::SetBaseLabelXAxis(const QString& aXAxisBaseName)
{
   mXAxisBaseName = aXAxisBaseName;
   // For now, the X axis only supports seconds
   GetPlot()->SetLabelXAxis(mXAxisBaseName + " (" + mXAxisUnitName + ")");
}

void warlock::PlottingWidget::SetBaseLabelYAxis(const QString& aYAxisBaseName)
{
   mYAxisBaseName = aYAxisBaseName;
   GetPlot()->SetLabelYAxis(mYAxisBaseName + " (" + mYAxisUnitName + ")");
}

void warlock::PlottingWidget::UpdaterUnitsChangedCB(const QString& aNewUnitName)
{
   mYAxisUnitName = aNewUnitName;
   GetPlot()->SetLabelYAxis(mYAxisBaseName + " (" + mYAxisUnitName + ")");
   GetPlot()->ClearData();
}

void warlock::PlottingWidget::Update()
{
   for (auto& iter : mUpdaterList)
   {
      iter->Update(*mPlot);
   }
   UpdatePlotBoundaries(false);
}

void warlock::PlottingWidget::closeEvent(QCloseEvent* aEvent)
{
   if (QMessageBox::No == QMessageBox::question(this,
                                                "Continue Recording?",
                                                "Do you want to continue recording data for this plot?",
                                                QMessageBox::Yes | QMessageBox::No))
   {
      // If the user does not indicate they want to continue recording, then go ahead and delete the plot.
      deleteLater();
   }
   wkf::PlotDockWidget::closeEvent(aEvent);
}
