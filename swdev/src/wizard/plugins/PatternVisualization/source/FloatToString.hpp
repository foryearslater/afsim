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

#ifndef FLOATTOSTRING_HPP
#define FLOATTOSTRING_HPP

#include <iomanip>
#include <sstream>
#include <string>

namespace PatternVisualizer
{
template<typename T>
std::string FloatToString(T aValue, int aPrecision = 6)
{
   std::ostringstream s;
   s << std::setprecision(aPrecision) << std::fixed << aValue;

   std::string valueStr = s.str();

   // Remove trailing zeros.
   size_t i = valueStr.find_last_not_of('0');
   if (valueStr[i] == '.')
   {
      valueStr.erase(i + 2);
   } // Keep at least one zero after the decimal point.
   else
   {
      valueStr.erase(i + 1);
   }

   return valueStr;
}
} // namespace PatternVisualizer

#endif
