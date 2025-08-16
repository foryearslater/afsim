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
#ifndef WkPlottingWidget_HPP
#define WkPlottingWidget_HPP

#include "warlock_core_export.h"

#include <memory>
#include <vector>

#include <QList>

#include "WkPlotUpdater.hpp"
#include "WkfPlotDockWidget.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT PlottingWidget : public wkf::PlotDockWidget
{
   Q_OBJECT

public:
   PlottingWidget(QMainWindow* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   PlottingWidget(const PlottingWidget&) = delete;

   ~PlottingWidget() override = default;

   // Takes ownership of the warlock::PlotUpdater and will delete it
   void SetUpdater(std::unique_ptr<warlock::PlotUpdater> aUpdater);
   // Releases ownership of the warlock::PlotUpdater, similar to "Take" functions in Qt
   void RemoveUpdater(PlotUpdater* aUpdater);

   void SetBaseLabelXAxis(const QString& aXAxisBaseName);
   void SetBaseLabelYAxis(const QString& aXAxisBaseName);

protected:
   void UpdaterUnitsChangedCB(const QString& aNewUnitName);

   void Update();

   void closeEvent(QCloseEvent* aEvent) override;

   QString                                            mXAxisBaseName;
   QString                                            mYAxisBaseName;
   QString                                            mXAxisUnitName;
   QString                                            mYAxisUnitName;
   std::vector<std::unique_ptr<warlock::PlotUpdater>> mUpdaterList;
};
} // namespace warlock
#endif
