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

#include "DataScale.hpp"

#include <cmath>

#include "Clamp.hpp"


namespace PatternVisualizer
{
namespace
{
const std::vector<QColor> gDefaultColors{
   QColor(55, 55, 255),  // dark blue
   QColor(50, 170, 255), // light blue
   QColor(50, 255, 225), // teal
   QColor(60, 255, 110), // green
   QColor(110, 255, 50), // yellow-green
   QColor(255, 255, 50), // yellow
   QColor(255, 170, 50), // orange
   QColor(255, 50, 50)   // red
};

const size_t gDefaultSteps = gDefaultColors.size();
} // namespace

// static
std::vector<std::pair<int, int>> DataScale::sDBIncAndRangeCutOff = {{1, 7},
                                                                    {2, 14},
                                                                    {3, 21},
                                                                    {5, 35},
                                                                    {10, 70},
                                                                    {15, 105},
                                                                    {20, 140},
                                                                    {25, 175},
                                                                    {30, 210},
                                                                    {40, 280},
                                                                    {50, 350},
                                                                    {75, 525},
                                                                    {100, 700}};


DataScale::DataScale()
   : mMinDB(0.0f)
   , mMaxDB(20.0f)
{
   mSteps.resize(gDefaultSteps);
   UpdateSteps();
}

void DataScale::SetMinMaxDB(float& aMinDB, float& aMaxDB)
{
   mMinDB = aMinDB;
   mMaxDB = aMaxDB;
   UpdateSteps();
   aMinDB = mMinDB;
   aMaxDB = mMaxDB;
}

bool DataScale::SetMinDBManual(float aValue)
{
   if (aValue == mMinDB)
   {
      return false;
   }
   mMinDB = aValue;

   // Update the color steps
   UpdateSteps();
   return true;
}

bool DataScale::SetMaxDBManual(float aValue)
{
   if (aValue == mMaxDB)
   {
      return false;
   }
   mMaxDB = aValue;

   // Update the color steps
   UpdateSteps();
   return true;
}

bool DataScale::SetIncDBManual(float aValue)
{
   if (aValue == mIncDB)
   {
      return false;
   }

   mIncDB = aValue;
   UpdateSteps();
   return true;
}

void DataScale::ComputeSteps()
{
   // Return if manually scaling
   if (mManualScaling && !mResettingScale)
   {
      return;
   }

   // If min/max dB values are equal, adjust them both so that the requested value
   // is in the middle of the new min/max
   if (mMinDB == mMaxDB)
   {
      if (!mManualScaling)
      {
         mIncDB = 5;
         mMinDB = floor(mMinDB) - mIncDB;
         mMaxDB = mMinDB + 2 * mIncDB;
      }
      return;
   }

   // Ensure that the min/max DB values are within [-300,300]
   mMinDB = std::max(-300.0f, std::floor(mMinDB));
   mMaxDB = std::min(300.0f, std::ceil(mMaxDB));

   // Use 0.5dB increments if the dB range is less that 2 dB
   if (mMaxDB - mMinDB <= 2)
   {
      mIncDB = 0.5;
      return;
   }

   // Otherwise set dB increments appropriate for the dB range required
   for (size_t i = 0; i < sDBIncAndRangeCutOff.size(); i++)
   {
      int   dbInc       = sDBIncAndRangeCutOff[i].first;
      int   rangeCutOff = sDBIncAndRangeCutOff[i].second;
      float dbNice      = dbInc * ceil(mMaxDB / dbInc);
      if (dbNice - mMinDB <= rangeCutOff)
      {
         mMaxDB     = std::min(300.0f, dbNice);
         int nsteps = (int)ceil((mMaxDB - mMinDB) / dbInc) + 1;
         mMinDB     = mMaxDB - dbInc * (nsteps - 1);
         mIncDB     = dbInc;
         return;
      }
   }
}

void DataScale::SetColorSteps()
{
   mSteps.clear();

   // Return if min/max are out of sequence
   if (mMinDB >= mMaxDB)
   {
      return;
   }

   // Define the color steps
   float db = mMinDB;
   while (db <= mMaxDB)
   {
      float indexF = (gDefaultSteps - 1) * (db - mMinDB) / (mMaxDB - mMinDB);
      int   index  = static_cast<int>(floor(indexF));
      float fract  = indexF - floor(indexF);

      QColor color;
      if (indexF >= gDefaultSteps)
      {
         color = gDefaultColors[gDefaultSteps - 1];
      }
      else if (fract == 0)
      {
         color = gDefaultColors[index];
      }
      else
      {
         int r1 = gDefaultColors[index].red();
         int r2 = gDefaultColors[index + 1].red();
         int g1 = gDefaultColors[index].green();
         int g2 = gDefaultColors[index + 1].green();
         int b1 = gDefaultColors[index].blue();
         int b2 = gDefaultColors[index + 1].blue();
         color  = QColor(static_cast<int>(r1 * (1 - fract) + r2 * fract),  // red component
                        static_cast<int>(g1 * (1 - fract) + g2 * fract),  // green component
                        static_cast<int>(b1 * (1 - fract) + b2 * fract)); // blue component
      }

      mSteps.emplace_back(db, color);
      db += mIncDB;
   }
}

void DataScale::UpdateSteps()
{
   ComputeSteps();
   SetColorSteps();
}
} // namespace PatternVisualizer
