// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SCENARIO_TRANSFORMATION_IO_TYPES_HPP
#define SCENARIO_TRANSFORMATION_IO_TYPES_HPP

#include <unordered_map>

#include "PluginUtil.hpp"
#include "UtEnumUtil.hpp"
#include "UtUnitTypes.hpp"

namespace MapUtils
{
namespace ScenarioTransformation
{
//! @brief namespace of common/helper objects, functions, and variables for input/output when doing scenario transformation
namespace IO_Types
{

//! @name convenient aliases for MapUtils::PluginUtil::RangeT
//@{

using RangeD = PluginUtil::RangeT<double>;

//@}

//! @name convenient aliases for MapUtils::PluginUtil::ValueAndUnitT
//@{

using ValueAndUnitDI = PluginUtil::ValueAndUnitT<double, int>;
using ValueAndUnitSS = PluginUtil::ValueAndUnitT<QString, QString>;

//@}

//! @brief denotes the direction characters of an absolute property
//! @note in particular, the absolute properties referred to are latitude and longitude
struct DirectionCharacters
{
   //! @brief the default direction characters
   static const DirectionCharacters cDEFAULT;

   //! @brief denotes the positive direction character
   char mPosChar;
   //! @brief denotes the negative direction character
   char mNegChar;
};

//! @name ranges for properties
//@{

//! @brief map of ranges for the property of angle
const std::unordered_map<UtUnitAngle::BaseUnit, RangeD, ut::EnumHash<UtUnitAngle::BaseUnit>> cANGLE_RANGES{
   {UtUnitAngle::BaseUnit::cRADIANS,
    {UtUnitAngle::ConvertToStandard(-360.0, UtUnitAngle::BaseUnit::cDEGREES),
     UtUnitAngle::ConvertToStandard(360.0, UtUnitAngle::BaseUnit::cDEGREES)}},
   {UtUnitAngle::BaseUnit::cDEGREES, {-360.0, 360.0}},
   {UtUnitAngle::BaseUnit::cMILS,
    {UtUnitAngle::ConvertFromStandard(UtUnitAngle::ConvertToStandard(-360.0, UtUnitAngle::BaseUnit::cDEGREES),
                                      UtUnitAngle::BaseUnit::cMILS),
     UtUnitAngle::ConvertFromStandard(UtUnitAngle::ConvertToStandard(360.0, UtUnitAngle::BaseUnit::cDEGREES),
                                      UtUnitAngle::BaseUnit::cMILS)}},
   {UtUnitAngle::BaseUnit::cARCSECONDS,
    {UtUnitAngle::ConvertFromStandard(UtUnitAngle::ConvertToStandard(-360.0, UtUnitAngle::BaseUnit::cDEGREES),
                                      UtUnitAngle::BaseUnit::cARCSECONDS),
     UtUnitAngle::ConvertFromStandard(UtUnitAngle::ConvertToStandard(360.0, UtUnitAngle::BaseUnit::cDEGREES),
                                      UtUnitAngle::BaseUnit::cARCSECONDS)}}};

//! @brief range for the property of latitude
const RangeD cLATITUDE_RANGE{-90.0, 90.0};

//! @brief range for the property of longitude
const RangeD cLONGITUDE_RANGE{-180.0, 180.0};

//! @brief range for the property of latitudinal (vertical) distance
const RangeD cVERTICAL_RANGE{-PluginUtil::Earth::cMERIDIONAL_CIRCUMFERENCE_WGS84,
                             PluginUtil::Earth::cMERIDIONAL_CIRCUMFERENCE_WGS84};

//! @brief range for the property of longitudinal (horizontal) distance
const RangeD cHORIZONTAL_RANGE{-PluginUtil::Earth::cEQUATORIAL_CIRCUMFERENCE_WGS84,
                               PluginUtil::Earth::cEQUATORIAL_CIRCUMFERENCE_WGS84};

//@}

} // namespace IO_Types

} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // SCENARIO_TRANSFORMATION_IO_TYPES_HPP
