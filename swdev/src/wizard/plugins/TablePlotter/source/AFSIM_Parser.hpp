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

#ifndef PLOT_IT_AFSIM_PARSER_HPP
#define PLOT_IT_AFSIM_PARSER_HPP

#include <memory>

#include <QTextStream>

#include "PlotParser.hpp"
#include "UtInputBlock.hpp"

class UtInputDocument;

class IV_Table
{
public:
   using IV1_Table = std::vector<std::pair<double, double>>;
   using IV2_Table = std::vector<std::pair<double, IV1_Table>>;
   using IV3_Table = std::vector<std::pair<double, IV2_Table>>;

   IV_Table();
   IV_Table(const IV1_Table& aTable);
   IV_Table(const IV2_Table& aTable);
   // IV_Table(const IV3_Table& aTable);

   unsigned int GetNumIVs() { return mNumIVs; }
   unsigned int mNumIVs;
   IV1_Table    mTable1;
   IV2_Table    mTable2;
   // IV3_Table mTable3;
};

class AFSIM_Parser : public PlotParser
{
public:
   AFSIM_Parser(const QString& aFilename);
   AFSIM_Parser(const std::string& aInputText);

   void PopulatePlot(int aPlotId, UtQtGL2DPlot* plot) override;

private:
   struct PlotInfo
   {
      PlotInfo() = default;
      void SetUpPlot(UtQtGL2DPlot* plot, double aIV_table3 = 0) const;
      int  xColumn;
      int  yColumn;

      QString  plotName;
      QString  iv1Label;
      QString  iv2Label;
      QString  dvLabel;
      IV_Table table;
   };

   void ReadFile(const std::string& aFilename);
   void ReadUtCurveFile();

   void InterpretInput(std::unique_ptr<UtInputDocument> aInput);
   void ReadInput(const std::string& aInput);

   void ReadCurve(UtInput& aInput, const std::string& aCommand);
   void ReadIrregularTable(UtInput& aInput, const std::string& aTableName);
   void ReadRegularTable(UtInput& aInput, const std::string& aTableName);

   int  ReadRegularCommands(UtInput& aInput, std::vector<double>& aValues);
   void ReadDummyCommands(UtInput& aInput, UtInputBlock& aBlock);
   void CreatePlots(const std::string&                   aTableName,
                    const IV_Table::IV1_Table&           a_1IV,
                    const IV_Table::IV2_Table&           a_2IV,
                    const IV_Table::IV3_Table&           a_3IV,
                    const std::map<size_t, std::string>& aIvNames);

   int                      mNextPlotId;
   std::map<int, PlotInfo>  mDataMap;
   std::vector<std::string> mUtCurveStrings;
   PlotInfo                 mCurrentPlotInfo;
};

#endif
