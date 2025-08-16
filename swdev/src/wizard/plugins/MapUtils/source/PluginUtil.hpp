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

#ifndef MAP_UTILS_PLUGIN_UTIL_HPP
#define MAP_UTILS_PLUGIN_UTIL_HPP

#include <map>
#include <sstream>
#include <type_traits>
#include <typeinfo>

#include <QPalette>

#include "UtLatPos.hpp"
#include "UtLength.hpp"
#include "UtLonPos.hpp"
#include "UtUnitTypes.hpp"
#include "VaEntity.hpp"
#include "VaFwd.hpp"

namespace MapUtils
{

//! @brief namespace of common/helper objects, functions, and variables for the MapUtils plug-in
namespace PluginUtil
{

//! @brief namespace of common/helper objects, functions, and variables related to the Earth
namespace Earth
{

//! @brief denotes the equatorial circumference of the earth (in meters)
//! @note this is the World Geodetic System 1984 value
//! @see https://en.wikipedia.org/wiki/Earth
const UtLengthValue cEQUATORIAL_CIRCUMFERENCE_WGS84{40075017, UtUnitLength::BaseUnit::cMETERS};

//! @brief denotes the meridional circumference of the earth (in meters)
//! @note this is the World Geodetic System 1984 value
//! @see https://en.wikipedia.org/wiki/Earth
const UtLengthValue cMERIDIONAL_CIRCUMFERENCE_WGS84{40007860, UtUnitLength::BaseUnit::cMETERS};

//! @brief normalizes the longitudinal distance of a relative translation within one Earth::cEQUATORIAL_CIRCUMFERENCE_WGS84
//! @param aDifference is the actual difference
//! @return the normalized difference
double NormalizeEquatorialDistanceWGS84(const double aDifference);

//! @brief normalizes the latitudinal distance of a relative translation within one Earth::cMERIDIONAL_CIRCUMFERENCE_WGS84
//! @param aDifference is the actual difference
//! @return the normalized difference
double NormalizeMeridionalDistanceWGS84(const double aDifference);

} // namespace Earth

//! @brief denotes a range (a minimum and maximum value pair)
template<typename T>
struct RangeT
{
   //! @brief denotes the minimum allowed value
   T mMinimum;
   //! @brief denotes the maximum allowed value
   T mMaximum;
};

//! @brief denotes a property that has a value and a unit
template<typename VALUE, typename UNIT>
struct ValueAndUnitT
{
   //! @brief denotes the property value
   VALUE mValue;
   //! @brief denotes the property unit
   UNIT mUnit;
};

//! @brief denotes the palette
//! @see the QPalettes in MapUtils::PluginUtil for more information
enum class Palette
{
   cDISABLED, //!< denotes the "disabled" state
   cINVALID,  //!< denotes the "invalid" state
   cVALID     //!< denotes the "valid" state
};

//! @brief function to generate QPalettes
//! @note due to the nature of QPalettes, this is necessary to instantiate them
QPalette GeneratePalette(Qt::GlobalColor aColor);

//! @brief map of palettes used in the MapUtils plug-in
//! @note due to the nature of QPalettes, the lambdas are necessary to instantiate them
const std::map<Palette, QPalette> cPALETTES{
   {Palette::cDISABLED, GeneratePalette(Qt::gray)}, //!< color palette for disabled text in the line edit
   {Palette::cINVALID, GeneratePalette(Qt::red)},   //!< color palette for invalid text in the line edit
   {Palette::cVALID, GeneratePalette(Qt::white)}    //!< color palette for valid text in the line edit
};

//! @brief denotes what entities to get
enum class WhatEntities
{
   cNONE,     //!< denotes no entities
   cSCENARIO, //!< denotes all entities
   cSELECTION //!< denotes selected entities
};

//! @brief sets the entities
//! @param aEntitiesToAcquire is the acquisition type
//! @return an entity list
vespa::VaEntityList AcquireEntities(const WhatEntities aEntitiesToAcquire);

//! @brief gets the entities in the scenario
//! @return an entity list with the scenario entities
vespa::VaEntityList GetEntitiesOfScenario();

//! @brief gets the entities in the selection
//! @return an entity list with the selected entities
vespa::VaEntityList GetEntitiesOfSelection();

//! @brief determines whether to get all entities, some entities, or no entities
//! @return what entities are available for acquisition
WhatEntities GetAcquisitionType();

//! @brief takes a negative number that is out-of-range and makes it in-range
//! @param aValue is the value to wrap around
//! @param aNormalizationFactor is the number with which to use for wrapping around
//! @pre aValue and aNormalizationFactor are negative
//! return the in-range value
double WrapAroundNegative(const double aValue, const double aNormalizationFactor);

//! @brief takes a positive number that is out-of-range and makes it in-range
//! @param aValue is the value to wrap around
//! @param aNormalizationFactor is the number with which to use for wrapping around
//! @pre aValue and aNormalizationFactor are positive
//! return the in-range value
double WrapAroundPositive(const double aValue, const double aNormalizationFactor);

//! @name functions for checking differences if 6DOF
//@{

//! @brief function for checking wither or not two properties are different within a given tolerance
//! @tparam PROPERTY is the type of property that is being checked
//! @pre the PROPERTY type must be convertible to a double
//! @param aPrevious is the previous property value (and unit)
//! @param aCurrent is the current property value (and unit)
//! @param aEpsilon is the tolerance
//! @note uses UtMath::NearlyEqual under the hood, but provides a static_assert for extra protection
template<typename PROPERTY>
bool AreDifferent(const PROPERTY& aPrevious,
                  const PROPERTY& aCurrent,
                  const double    aEpsilon = std::numeric_limits<double>::epsilon())
{
   static_assert(std::is_convertible<PROPERTY, double>::value,
                 "The \"PROPERTY\" template parameter must be convertible to a double.");
   return !UtMath::NearlyEqual<double>(static_cast<double>(aPrevious), static_cast<double>(aCurrent), aEpsilon);
}

//! @brief checks the previous and current latitudes to make sure they are different
//! @param aPrevious is the previous latitude
//! @param aCurrent is the current latitude
//! @return whether or not the previous and current latitudes are different
bool AreLatitudesDifferent(const UtLatPos& aPrevious, const UtLatPos& aCurrent);

//! @brief checks the previous and current longitudes to make sure they are different
//! @param aPrevious is the previous longitude
//! @param aCurrent is the current longitude
//! @return whether or not the previous and current longitudes are different
bool AreLongitudesDifferent(const UtLonPos& aPrevious, const UtLonPos& aCurrent);

//! @brief checks the previous and current altitudes to make sure they are different
//! @param aPrevious is the previous altitude
//! @param aCurrent is the current altitude
//! @return whether or not the previous and current altitudes are different
bool AreAltitudesDifferent(const UtLength& aPrevious, const UtLength& aCurrent);

//! @brief checks the previous and current altitudes to make sure they are different
//! @param aPrevious is the previous altitude
//! @param aCurrent is the current altitude
//! @return whether or not the previous and current altitudes are different
bool AreAltitudesDifferent(const UtLengthValue& aPrevious, const UtLengthValue& aCurrent);

//! @brief checks the previous and current pitches to make sure they are different
//! @param aPrevious is the previous pitch
//! @param aCurrent is the current pitch
//! @return whether or not the previous and current pitches are different
bool ArePitchesDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent);

//! @brief checks the previous and current rolls to make sure they are different
//! @param aPrevious is the previous roll
//! @param aCurrent is the current roll
//! @return whether or not the previous and current rolls are different
bool AreRollsDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent);

//! @brief checks the previous and current yaws (headings) to make sure they are different
//! @param aPrevious is the previous yaw (heading)
//! @param aCurrent is the current yaw (heading)
//! @return whether or not the previous and current yaws (headings) are different
bool AreYawsDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent);

//@}

} // namespace PluginUtil
} // namespace MapUtils

#endif // MAP_UTILS_PLUGIN_UTIL_HPP
