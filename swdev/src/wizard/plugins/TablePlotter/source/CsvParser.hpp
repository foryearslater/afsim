// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLOT_IT_CSV_PARSER_HPP
#define PLOT_IT_CSV_PARSER_HPP

#include <QTextStream>

#include "PlotParser.hpp"
#include "UtUnitTypes.hpp"

class CsvParser : public PlotParser
{
public:
   CsvParser(const QString& aFilename);

   void PopulatePlot(int aPlotId, UtQtGL2DPlot* plot) override;

protected:
   struct PlotInfo
   {
      QString              plotname;
      int                  xColumn;
      QString              xLabel;
      std::vector<int>     yColumns;
      std::vector<QString> yLabels;
   };

   bool CheckForMetaData(const QStringList& aStrings);

   void ParseStrings(const QStringList& aStrings);

   std::map<int, PlotInfo>            mPlotInfoMap;
   std::map<int, std::vector<double>> mColumnData;

   // Meta Data information
   QString               mTitle;
   QString               mY_Label;
   UtUnitSpeed::BaseUnit mTP_SpeedUnit;
   double                mTP_SpeedMultiplier;
   bool                  mTurnPerformance;
};

#endif