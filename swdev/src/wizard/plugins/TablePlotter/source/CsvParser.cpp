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

#include "CsvParser.hpp"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include "UtQtGL2DPlotTP.hpp"

CsvParser::CsvParser(const QString& aFilename)
   : PlotParser()
   , mTitle()
   , mY_Label()
   , mTP_SpeedUnit(UtUnitSpeed::cFEET_PER_SECOND)
   , mTP_SpeedMultiplier(0.0)
   , mTurnPerformance(false)
{
   QFile file(aFilename);
   if (!file.open(QIODevice::ReadOnly))
   {
      QMessageBox::information(nullptr, "error", file.errorString());
   }
   else
   {
      QTextStream in(&file);

      if (!in.atEnd())
      {
         //************************
         // Check for Meta Data
         // Read Top Line for Headers
         QString     line   = in.readLine();
         QStringList fields = line.split(",");

         // If there was Meta Data present, skip to next line, this line has been parsed.
         if (CheckForMetaData(fields))
         {
            if (in.atEnd())
            {
               // error
            }
            else // read next line
            {
               fields = in.readLine().split(",");
            }
         }

         //************************
         // Check for Headers
         bool headerPresent = false;
         // Identify which columns have data in them
         int                    col = 0;
         std::map<int, QString> columns;
         for (const auto& str : fields)
         {
            if (str != "")
            {
               bool ok = false;
               str.toDouble(&ok);

               if (ok) // First Row is data and not strings, No Headers
               {
                  columns[col] = "";
               }
               else // Header located
               {
                  headerPresent = true;
                  columns[col]  = str;
               }
            }
            col++;
         }

         //************************
         // Create Series Pairs by grouping columns together that contain data
         // independent var column, dependent var column
         std::vector<std::pair<int, int>> series;
         int                              previousKey       = -10; // Some bogus value
         int                              independentVarCol = -10; // Another bogus value
         for (auto i : columns)
         {
            // If the previous column was empty, then this column is treated as an Independent variable
            if (i.first - 1 != previousKey)
            {
               independentVarCol = i.first;
            }
            else // This column is treated as an Dependent variable
            {
               series.emplace_back(independentVarCol, i.first);
            }
            previousKey = i.first;
         }

         //************************
         // Parse all data present in the file and store in column vectors
         // If there is no header, Read the first line as if it is data
         if (!headerPresent)
         {
            // Read the first line as if it is data (in case there was no header)
            ParseStrings(fields);
         }

         while (!in.atEnd())
         {
            ParseStrings(in.readLine().split(","));
         }

         //************************
         // Construct the plot object

         // Use the column as the Plot Key
         for (auto i : series)
         {
            // If this X value is not part of an exist plot, create new plot
            if (mPlotInfoMap.count(i.first) == 0)
            {
               PlotInfo info;
               info.plotname = columns[i.second] + " v " + columns[i.first];
               info.xColumn  = i.first;
               info.xLabel   = columns[i.first];
               info.yColumns.push_back(i.second);
               info.yLabels.push_back(columns[i.second]);
               mPlotInfoMap[i.first]     = info;
               mPlotIdsAndNames[i.first] = info.plotname;
            }
            else // If this X column is part of an exist plot append it
            {
               mPlotInfoMap[i.first].plotname = "Values v " + mPlotInfoMap[i.first].xLabel;
               mPlotIdsAndNames[i.first]      = mPlotInfoMap[i.first].plotname;

               mPlotInfoMap[i.first].yColumns.push_back(i.second);
               mPlotInfoMap[i.first].yLabels.push_back(columns[i.second]);
            }
         }
         mSuccessfulParse = true;
      }
      file.close();
   }
}

void CsvParser::PopulatePlot(int aPlotId, UtQtGL2DPlot* plot)
{
   plot->Reset();

   if (mPlotInfoMap.count(aPlotId) > 0)
   {
      PlotInfo& info = mPlotInfoMap.at(aPlotId);

      plot->SetLabelXAxis(info.xLabel);
      if (mTitle != "")
      {
         plot->SetTitle(mTitle);
      }
      else
      {
         plot->SetTitle(info.plotname);
      }

      if (mY_Label != "")
      {
         plot->SetLabelYAxis(mY_Label);
      }
      else
      {
         plot->SetLabelYAxis("Values");
      }

      auto* plotTP = dynamic_cast<UtQtGL2DPlotTP*>(plot);
      if (plotTP != nullptr)
      {
         plotTP->SetShowBackgroundCurves(mTurnPerformance);

         if (mTP_SpeedMultiplier != 0.0)
         {
            plotTP->SetSpeedMultiplier(mTP_SpeedMultiplier);
         }
         else
         {
            plotTP->SetSpeedUnits(mTP_SpeedUnit);
         }
      }

      if (info.yLabels.size() > 1)
      {
         int count = 1;
         for (auto& yLabel : info.yLabels)
         {
            plot->SetSeriesName(yLabel, count);
            count++;
         }

         count = 1;
         for (auto& yColumn : info.yColumns)
         {
            plot->AddPoints(mColumnData[info.xColumn], mColumnData[yColumn], count);
            count++;
         }

         plot->SetShowLegend(true);
      }
      else if (info.yLabels.size() == 1)
      {
         plot->SetLabelYAxis(info.yLabels.front());
         plot->AddPoints(mColumnData[info.xColumn], mColumnData[info.yColumns.front()], 1);
         plot->SetShowLegend(false);
      }
      else
      {
         // Error
      }
      plot->MakePlotPretty();
   }
}

bool CsvParser::CheckForMetaData(const QStringList& aStrings)
{
   if (!aStrings.empty() && aStrings.at(0) == "Meta")
   {
      for (auto& str : aStrings)
      {
         if (str.startsWith("title="))
         {
            mTitle = str;
            mTitle.remove(0, 6); // 6 = length of "title="
         }
         else if (str.startsWith("y_label="))
         {
            mY_Label = str;
            mY_Label.remove(0, 8); // 8 = length of "y_label="
         }
         else if (str.startsWith("tp_unit="))
         {
            QString temp = str;
            temp         = temp.remove(0, 8); // 8 = length of "tp_unit="
            int unit     = UtUnitSpeed::ReadUnit(temp.toStdString());
            if (UtUnitSpeed::IsUnitValid(unit))
            {
               mTP_SpeedUnit    = (UtUnitSpeed::BaseUnit)unit;
               mTurnPerformance = true;
            }
            else
            {
               // Warning: unit not valid, so disable background lines.
               mTurnPerformance = false;
               mTP_SpeedUnit    = UtUnitSpeed::cFEET_PER_SECOND;
            }
         }
         else if (str.startsWith("tp_multiplier="))
         {
            QString temp      = str;
            temp              = temp.remove(0, 14); // 8 = length of "tp_multiplier="
            bool   ok         = false;
            double multiplier = temp.toDouble(&ok);
            if (ok)
            {
               mTurnPerformance    = true;
               mTP_SpeedMultiplier = multiplier;
            }
            else
            {
               mTurnPerformance = false;
            }
         }
      }
      return true;
   }
   return false;
}

void CsvParser::ParseStrings(const QStringList& aStrings)
{
   bool   ok    = false;
   double value = 0;

   int col = 0;
   for (auto& i : aStrings)
   {
      value = i.toDouble(&ok);
      if (ok)
      {
         mColumnData[col].emplace_back(value);
      }
      else if (i != "")
      {
         mErrors.push_back(QString("Warning: ") + i + " is not a number.");
      }
      col++;
   }
}
