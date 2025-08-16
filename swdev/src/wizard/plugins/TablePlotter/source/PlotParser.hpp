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

#ifndef PLOT_IT_PARSER_HPP
#define PLOT_IT_PARSER_HPP

#include <QTextStream>

class UtQtGL2DPlot;

class PlotParser
{
public:
   PlotParser()
      : mSuccessfulParse(false)
   {
   }
   virtual ~PlotParser() = default;

   const QStringList& GetErrors() const { return mErrors; }

   const std::map<int, QString>& GetPlotIdsAndNames() const { return mPlotIdsAndNames; }

   virtual void PopulatePlot(int aPlotId, UtQtGL2DPlot* plot) = 0;

protected:
   bool                   mSuccessfulParse;
   std::map<int, QString> mPlotIdsAndNames;
   QStringList            mErrors;
};

#endif