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

#include "AFSIM_Parser.hpp"

#include <QApplication>
#include <QDir>

#include "UtInputFile.hpp"
#include "UtInputString.hpp"
#include "UtQtGL2DPlot.hpp"

// Stupid stuff windows does, changing GetMessage() to GetMessageA()
#undef GetMessage

IV_Table::IV_Table()
   : mNumIVs(0)
{
}

IV_Table::IV_Table(const IV1_Table& aTable)
   : mNumIVs(1)
   , mTable1(aTable)
{
}

IV_Table::IV_Table(const IV2_Table& aTable)
   : mNumIVs(2)
   , mTable2(aTable)
{
}

AFSIM_Parser::AFSIM_Parser(const QString& aFilename)
   : PlotParser()
   , mNextPlotId(1)
{
   ReadUtCurveFile();
   ReadFile(aFilename.toStdString());
}

AFSIM_Parser::AFSIM_Parser(const std::string& aInputText)
   : PlotParser()
   , mNextPlotId(1)
{
   ReadUtCurveFile();
   ReadInput(aInputText);
}


void AFSIM_Parser::PopulatePlot(int aPlotId, UtQtGL2DPlot* plot)
{
   if (mDataMap.count(aPlotId) != 0)
   {
      mDataMap.at(aPlotId).SetUpPlot(plot);
   }
}

void AFSIM_Parser::ReadInput(const std::string& aInput)
{
   std::unique_ptr<UtInputString> inputPtr = nullptr;
   try
   {
      inputPtr = ut::make_unique<UtInputString>(aInput);
   }
   catch (std::exception&)
   {
      mErrors.push_back(QString("Unable to interpret input string"));
      return;
   }
   InterpretInput(std::move(inputPtr));
}

void AFSIM_Parser::ReadFile(const std::string& aFilename)
{
   std::unique_ptr<UtInputFile> inputPtr = nullptr;
   try
   {
      inputPtr = ut::make_unique<UtInputFile>(aFilename);
   }
   catch (std::exception&)
   {
      mErrors.push_back(QString("Unable to open input file ") + QString::fromStdString(aFilename) +
                        " in P6DofCommonController::ReadSupportFile()");
      return;
   }
   InterpretInput(std::move(inputPtr));
}

// TODO: If we change how files are allowed to be used in tables (to only store binary data instead of entire table
// format), extract the data when reading the input
void AFSIM_Parser::InterpretInput(std::unique_ptr<UtInputDocument> aInput)
{
   try
   {
      UtInput input;
      input.PushInput(std::move(aInput));

      std::string command         = "";
      std::string previousCommand = "";
      while (input.TryReadCommand(command))
      {
         if (command == "irregular_table")
         {
            ReadIrregularTable(input, previousCommand);
         }
         else if (command == "regular_table")
         {
            ReadRegularTable(input, previousCommand);
         }
         else if (command == "curve")
         {
            ReadCurve(input, command);
         }
         else
         {
            for (auto& str : mUtCurveStrings)
            {
               if (command == str)
               {
                  ReadCurve(input, command);
               }
            }
         }
         previousCommand = command;
      }
   }
   catch (UtInput::ExceptionBase& e)
   {
      mErrors.push_back(QString("Exception Thrown: ") + QString::fromStdString(e.GetMessage()));
   }
   catch (std::exception& e)
   {
      mErrors.push_back(QString("Exception Thrown: ") + e.what());
   }
}

void AFSIM_Parser::ReadUtCurveFile()
{
   mUtCurveStrings = {
      // P6DofAeroCoreObject
      "cmq_mach_table",
      "cnr_mach_table",
      "cnp_mach_table",
      "clp_mach_table",
      "clr_mach_table",
      // P6DofAtmosphere
      "density_altitude_table",
      "pressure_altitude_table",
      "temperature_altitude_table",
      "sonic_speed_altitude_table",
      // P6DofCommonController
      "cl_max_mach_table",
      "cl_min_mach_table",
      "alpha_max_mach_table",
      "alpha_min_mach_table",
      // P6DofFlightControlSystem
      "simple_table",
      "angle_mapping_table",
      "angle_mapping_auto_table",
      "angle_mapping_manual_table",
      // P6DofJetEngineObject
      "spin_up_table_mil_per_sec",
      "spin_down_table_mil_per_sec",
      "spin_up_table_ab_per_sec",
      "spin_down_table_ab_per_sec",
      "thrust_table_idle",
      "thrust_table_mil",
      "thrust_table_ab",
      // P6DofManualPilotAugmentedControls
      "pitch_control_mapping_table",
      "roll_control_mapping_table",
      "yaw_control_mapping_table",
      // P6DofRocketLiquidPropellantObject
      "normalized_thrust_vs_alt",
      "isp_vs_alt",
      // P6DofRocketSolidPropellantObject
      "thrust_vs_time_sealevel",
      "thrust_vs_time_vacuum",
      // WsfBrawlerProcessor
      "clmax_vs_mach_table",
      "alpha_at_clmax_vs_mach_table"
      // P6DofManualPilotSimpleControls and P6DofSyntheticPilot (Same as P6DofManualPilotAugmentedControls)
   };
}

void AFSIM_Parser::ReadCurve(UtInput& aInput, const std::string& aCommand)
{
   PlotInfo info;
   info.iv1Label = "Independent Values";
   info.dvLabel  = "Dependent Values";
   info.table    = IV_Table(IV_Table::IV1_Table());
   info.plotName = QString::fromStdString(aCommand);
   std::string  ivName;
   UtInputBlock block(aInput, "end_" + aCommand);
   while (block.ReadCommand())
   {
      std::string command = block.GetCommand();
      if (command == "independent_variable")
      {
         aInput.ReadValue(ivName);
         ReadDummyCommands(aInput, block);
         info.iv1Label = QString::fromStdString(ivName);
      }
      else if (command == "dependent_variable")
      {
         ReadDummyCommands(aInput, block);
      }
      else
      {
         QString ivStr = QString::fromStdString(command);
         // QString ivStr = QString::fromStdString(block.GetCommand());
         if (block.ReadCommand())
         {
            QString dvStr = QString::fromStdString(block.GetCommand());

            bool   ok1 = false;
            bool   ok2 = false;
            double iv  = ivStr.toDouble(&ok1);
            double dv  = dvStr.toDouble(&ok2);
            if (ok1 && ok2)
            {
               info.table.mTable1.emplace_back(std::pair<double, double>(iv, dv));
            }
            else if (!ok1)
            {
               mErrors.push_back(ivStr + " is not a number.");
            }
            else //! ok2
            {
               mErrors.push_back(dvStr + " is not a number.");
            }
         }
      }
   }
   mPlotIdsAndNames[mNextPlotId] = info.plotName;
   mDataMap[mNextPlotId]         = info;
   mNextPlotId++;
}

// used to maintain alignment when reading commands that aren't necessary for the plot
void AFSIM_Parser::ReadDummyCommands(UtInput& aInput, UtInputBlock& aBlock)
{
   std::string command;
   std::string dummy;
   while (aBlock.ReadCommand(command))
   {
      if (command == "units" || command == "precision")
      {
         aInput.ReadValue(dummy);
      }
      else
      {
         aInput.PushBack(command);
         break;
      }
   }
}

void AFSIM_Parser::ReadIrregularTable(UtInput& aInput, const std::string& aTableName)
{
   int                           ivCount = 0;
   std::map<size_t, std::string> ivNames;

   UtInputBlock block(aInput, "end_irregular_table");
   while (block.ReadCommand())
   {
      std::string command = block.GetCommand();
      if (command == "independent_variable")
      {
         std::string ivName;
         aInput.ReadValue(ivName);
         ivNames[ivCount] = ivName;
         ivCount++;

         // We do not care about the units or precision, but we do need to read the lines to keep everything aligned.
         ReadDummyCommands(aInput, block);
      }
      else if (command == "dependent_variable")
      {
         // We do not care about the units or precision, but we do need to read the lines to keep everything aligned.
         ReadDummyCommands(aInput, block);
      }
      else // Data
      {
         size_t numIVs = ivNames.size();

         IV_Table::IV1_Table _1IV;
         IV_Table::IV2_Table _2IV;
         IV_Table::IV3_Table _3IV;

         std::map<size_t, int> ivIndices;
         ivIndices[0] = -1;
         ivIndices[1] = -1;
         ivIndices[2] = -1;

         std::vector<double> last_iv_values;

         aInput.PushBack(command);
         while (block.ReadCommand(command))
         {
            if (command == "values")
            {
               size_t count = 0;
               while (block.ReadCommand(command))
               {
                  bool   ok    = false;
                  double value = QString::fromStdString(command).toDouble(&ok);
                  if (!ok)
                  {
                     // Not meant for me, put command back on stack
                     aInput.PushBack(command);
                     break;
                  }
                  if (numIVs == 1)
                  {
                     (_1IV).emplace_back(std::pair<double, double>(last_iv_values[count], value));
                  }
                  else if (numIVs == 2)
                  {
                     (_2IV)[ivIndices[0]].second.emplace_back(std::pair<double, double>(last_iv_values[count], value));
                  }
                  else if (numIVs == 3)
                  {
                     (_3IV)[ivIndices[0]].second[ivIndices[1]].second.emplace_back(
                        std::pair<double, double>(last_iv_values[count], value));
                  }
                  count++;
                  if (count >= last_iv_values.size())
                  {
                     break;
                  }
               }
            }
            else
            {
               last_iv_values.clear();

               for (size_t i = 0; i < ivNames.size(); ++i)
               {
                  if (command == ivNames.at(i))
                  {
                     if (i == ivNames.size() - 1) // last IV
                     {
                        while (block.ReadCommand(command))
                        {
                           bool   ok    = false;
                           double value = QString::fromStdString(command).toDouble(&ok);
                           if (!ok)
                           {
                              // Not meant for me, put command back on stack
                              aInput.PushBack(command);
                              break;
                           }
                           last_iv_values.push_back(value);
                        }
                     }
                     else
                     {
                        // Read Single Value
                        std::string cmdValue;
                        block.ReadCommand(cmdValue);
                        bool   ok    = false;
                        double value = QString::fromStdString(cmdValue).toDouble(&ok);
                        if (ok)
                        {
                           // No need to check _1IV because it will be the last IV value if there is only 1
                           if (numIVs == 2)
                           {
                              (_2IV).emplace_back(std::pair<double, IV_Table::IV1_Table>(value, IV_Table::IV1_Table()));
                           }
                           else if (numIVs == 3)
                           {
                              if (command == ivNames.at(0))
                              {
                                 (_3IV).emplace_back(std::pair<double, IV_Table::IV2_Table>(value, IV_Table::IV2_Table()));
                                 ivIndices[1] = -1; // reset the 2nd index when the first index is changed.
                              }
                              else
                              {
                                 (_3IV)[ivIndices[0]].second.emplace_back(
                                    std::pair<double, IV_Table::IV1_Table>(value, IV_Table::IV1_Table()));
                              }
                           }
                           ivIndices[i]++;
                        }
                        else
                        {
                           mErrors.push_back(QString::fromStdString(command) + " is not a number.");
                        }
                     }
                  }
               }
            }
         }
         CreatePlots(aTableName, _1IV, _2IV, _3IV, ivNames);
         break;
      }
   }
}

int AFSIM_Parser::ReadRegularCommands(UtInput& aInput, std::vector<double>& aValues)
{
   UtInputBlock block(aInput);
   std::string  command;
   std::string  units;
   int          currentSize = 0;
   // TODO: Do we need to handle extrapolation?
   while (block.ReadCommand(command))
   {
      if (command == "units")
      {
         aInput.ReadValue(units);
      }
      else if (command == "precision")
      {
         std::string str;
         aInput.ReadValue(str);
         if (str != "double" && str != "float")
         {
            throw UtInput::BadValue(aInput);
         }
      }
      else
      {
         // Start of the values... If units are required make sure units were specified
         aInput.PushBack(command);
         {
            std::string blockCommand;
            UtInput&    input = block.GetInput();
            while (block.ReadCommand(blockCommand))
            {
               input.PushBack(blockCommand);
               double value;
               input.ReadValue(value);
               aValues.push_back(value);
               currentSize++;
            }
         }
         // We should have read up to the end of the block, so just exit the while loop.
         break;
      }
   }
   return currentSize;
}

void AFSIM_Parser::CreatePlots(const std::string&                   aTableName,
                               const IV_Table::IV1_Table&           a_1IV,
                               const IV_Table::IV2_Table&           a_2IV,
                               const IV_Table::IV3_Table&           a_3IV,
                               const std::map<size_t, std::string>& aIvNames)
{
   // Create the plots
   if (!a_1IV.empty())
   {
      PlotInfo info;
      if (!aTableName.empty())
      {
         info.plotName = QString::fromStdString(aTableName);
      }
      else
      {
         info.plotName = QString::fromStdString(aIvNames.at(0));
      }
      info.iv1Label = QString::fromStdString(aIvNames.at(0));
      info.dvLabel  = "values";
      info.table    = IV_Table(a_1IV);

      mPlotIdsAndNames[mNextPlotId] = info.plotName;
      mDataMap[mNextPlotId]         = info;
      mNextPlotId++;
   }
   else if (!a_2IV.empty())
   {
      PlotInfo info;
      if (!aTableName.empty())
      {
         info.plotName = QString::fromStdString(aTableName);
      }
      else
      {
         info.plotName = QString::fromStdString(aIvNames.at(0)) + " & " + QString::fromStdString(aIvNames.at(1));
      }
      info.iv1Label = QString::fromStdString(aIvNames.at(0));
      info.iv2Label = QString::fromStdString(aIvNames.at(1));
      info.dvLabel  = "values";
      info.table    = IV_Table(a_2IV);

      mPlotIdsAndNames[mNextPlotId] = info.plotName;
      mDataMap[mNextPlotId]         = info;
      mNextPlotId++;
   }
   else if (!a_3IV.empty())
   {
      for (auto& i : (a_3IV))
      {
         PlotInfo info;
         if (!aTableName.empty())
         {
            info.plotName = QString::fromStdString(aTableName) + " " + QString::fromStdString(aIvNames.at(0)) + "=" +
                            QString::number(i.first);
         }
         else
         {
            info.plotName = QString::fromStdString(aIvNames.at(0)) + "=" + QString::number(i.first);
         }
         info.iv1Label                 = QString::fromStdString(aIvNames.at(1));
         info.iv2Label                 = QString::fromStdString(aIvNames.at(2));
         info.dvLabel                  = "values";
         info.table                    = IV_Table(i.second);
         mPlotIdsAndNames[mNextPlotId] = info.plotName;
         mDataMap[mNextPlotId]         = info;
         mNextPlotId++;
      }
   }
   else // numIVs == 4
   {
      mErrors.push_back("Too many independent values.");
   }
}

void AFSIM_Parser::ReadRegularTable(UtInput& aInput, const std::string& aTableName)
{
   size_t                                ivCount = 0;
   std::map<size_t, std::string>         ivNames;
   std::map<size_t, std::vector<double>> IVs;
   std::vector<double>                   DVs;

   UtInputBlock block(aInput, "end_regular_table");
   std::string  command;
   while (block.ReadCommand(command))
   {
      if (command == "independent_variable")
      {
         if (ivCount >= 4)
         {
            throw UtInput::BadValue(aInput, "A maximum of 4 independent_variables may be specified");
         }
         // Read the variable name and make sure it is a name known by the caller.
         std::string name;
         aInput.ReadValue(name);
         ivNames[ivCount] = name;

         ReadRegularCommands(aInput, IVs[ivCount]);

         ++ivCount;
      }
      else if (command == "dependent_variable")
      {
         size_t expectedSize = 1;
         for (const auto& iv : IVs)
         {
            expectedSize *= iv.second.size();
         }

         size_t currentSize = static_cast<size_t>(ReadRegularCommands(aInput, DVs));

         if (currentSize < expectedSize)
         {
            throw UtInput::BadValue(aInput, "Not enough dependent_variable values provided");
         }
         else if (currentSize > expectedSize)
         {
            throw UtInput::BadValue(aInput, "Too many dependent_variables values provided");
         }
      }
   }

   // populate table values
   IV_Table::IV1_Table _1IV;
   IV_Table::IV2_Table _2IV;
   IV_Table::IV3_Table _3IV;

   if (IVs.size() == 1)
   {
      for (size_t i = 0; i < IVs[0].size(); ++i)
      {
         double iv_value = IVs[0][i];
         _1IV.emplace_back(iv_value, DVs[i]);
      }
   }
   else if (IVs.size() == 2)
   {
      for (size_t i = 0; i < IVs[0].size(); ++i)
      {
         IV_Table::IV1_Table vec;
         for (size_t j = 0; j < IVs[1].size(); ++j)
         {
            vec.emplace_back(IVs[1][j], DVs[i * IVs[1].size() + j]);
         }
         _2IV.emplace_back(IVs[0][i], vec);
      }
   }
   else if (IVs.size() == 3)
   {
      int count = 0;
      for (auto& z : IVs[0])
      {
         IV_Table::IV2_Table tmp_2IV;
         for (size_t i = 0; i < IVs[1].size(); ++i)
         {
            IV_Table::IV1_Table vec;
            for (size_t j = 0; j < IVs[2].size(); ++j)
            {
               vec.emplace_back(IVs[2][j], DVs[count]);
               count++;
            }
            tmp_2IV.emplace_back(IVs[1][i], vec);
         }
         _3IV.emplace_back(z, tmp_2IV);
      }
   }
   else
   {
      mErrors.push_back("Too many independent values.");
   }
   CreatePlots(aTableName, _1IV, _2IV, _3IV, ivNames);
}


void AFSIM_Parser::PlotInfo::SetUpPlot(UtQtGL2DPlot* plot, double aIV_table3 /*= 0*/) const
{
   plot->Reset();
   if (!table.mTable1.empty() || !table.mTable2.empty()) // we display 3D as multiple 2D tables
   {
      if (!table.mTable1.empty())
      {
         plot->SetTitle(plotName);
         plot->SetLabelYAxis(dvLabel);
         plot->SetLabelXAxis(iv1Label);
         plot->AddPoints(table.mTable1);
      }
      else if (!table.mTable2.empty())
      {
         plot->SetTitle(plotName);
         plot->SetLabelYAxis(dvLabel);
         plot->SetLabelXAxis(iv2Label);
         plot->SetShowLegend(true);

         int count = 0;
         for (auto& i : (table.mTable2))
         {
            plot->AddPoints(i.second, count + 1);
            plot->SetSeriesName(iv1Label + "_" + QString::number(i.first), count + 1);
            count++;
         }
      }
   }
}
