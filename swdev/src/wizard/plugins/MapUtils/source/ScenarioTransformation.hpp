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

#ifndef MAP_UTILS_SCENARIO_TRANSFORMATION_HPP
#define MAP_UTILS_SCENARIO_TRANSFORMATION_HPP

#include <QAbstractSlider>
#include <QLineEdit>
#include <QString>

#include "UtColor.hpp"
class UtLLAPos;
#include "UtUnitTypes.hpp"
#include "UtVec2.hpp"
namespace vespa
{
class VaAttachment;
}
#include "VaEntity.hpp"
#include "WkfPointOfInterest.hpp"

namespace MapUtils
{
//! @brief namespace of common/helper objects, functions, and variables for scenario transformation
//! @par details
//!    A "scenario transformation" is any geometric transformation done to a scenario.
//!    Geometric transformations include rotation, translation, reflection*, and dialation*.
//!    This namespace also houses the methods to apply said transformation to entity attachments.
//! @note an asterisk (*) by the transformation means it is not implemented yet.
namespace ScenarioTransformation
{

//! @brief denotes what type of transformation is occurring
//! @note see MapUtils::PluginUtil::LineEditSliderManager for more information
enum class Type
{
   cANGLE,    //!< denotes angle transformation
   cLATITUDE, //!< denotes latitude transformation
   cLENGTH,   //!< denotes length transformation
   cLONGITUDE //!< denotes longitude transformation
};

//! @brief denotes how relative distance to absolute position is extrapolated
enum class ExtrapolationMode
{
   cGREAT_CIRCLE,  //!< mode to extrapolate position on a great circle
   cGREAT_ELLIPSE, //!< mode to extrapolate position on a great ellipse
};

//! @brief converts relative XY distance into absolute LL coordinates via the extrapolated great ellipse
//! @param aXY_Distance is the relative XY distance to convert to absolute LL coordinates
//! @param aCenterRef is a reference to the center from which to do the conversion
//! @param aMode is the extrapolation mode
//! @return the absolute LL coordinates corresponding to the relative XY distance
UtLLAPos DistanceToPosition(const UtVec2d& aXY_Distance, const vespa::VaPosition& aCenterRef, const ExtrapolationMode aMode);

//! @brief denotes what distances to preserve when translating
enum class TranslationMode
{
   cRELATIVE_LLA, //!< mode to preserve lla spacing during translation
   cRELATIVE_XYA  //!< mode to preserve xya spacing during translation
};

//! @brief translates an entity from one position to another position
//! @param aOldRefLLA is the LLA of the reference position from which to translate
//! @param aNewRefLLA is the LLA of the reference position to which to translate
//! @param aPositionPtr is the position to translate
//! @param aMode is the translation mode
//! @return a boolean denoting whether the translation succeeded
//! @note if expanding on MapUtils, please use this function for translation
bool Translate(const UtLLAPos&        aOldRefLLA,
               const UtLLAPos&        aNewRefLLA,
               vespa::VaPosition*     aPositionPtr,
               const TranslationMode& aMode);

//! @brief computes the position of the centroid based on the reference entity list
//! @return the centroid of the list of entities
vespa::VaPosition ComputePositionOfCentroid(const vespa::VaEntityList& aSrc);

//! @name transformation application functions
//@{

//! @brief applies any change in the 6DOF of one entity to another entity
//! @param aSourceEntityPtr is the entity from which to extract the 6DOF information
//! @param aDestinationEntityPtr is the entity to which to apply the 6DOF information
void ApplyChangesToEntity6DOF(vespa::VaEntity* aSourceEntityPtr, vespa::VaEntity* aDestinationEntityPtr);

//! @brief apply the moved changes from the ghost route to the real route
//! @param aSourceRoutePtr is the real route
//! @param aDestinationRoutePtr is the ghost route
void ApplyRouteChange(vespa::VaAttachment* aSourceRoutePtr, vespa::VaAttachment* aDestinationRoutePtr);

//! @brief apply the changes from the ghost circle zone to the real circle zone
//! @param aSourceZonePtr is the real circle zone
//! @param aDestinationZonePtr is the ghost circle zone
void ApplyCircleZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr);

//! @brief apply the changes from the ghost ellipse zone to the real ellipse zone
//! @param aSourceZonePtr is the real ellipse zone
//! @param aDestinationZonePtr is the ghost ellipse zone
void ApplyEllipseZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr);

//! @brief apply the changes from the ghost polygon zone to the real polygon zone
//! @param aSourceZonePtr is the real polygon zone
//! @param aDestinationZonePtr is the ghost polygon zone
void ApplyPolygonZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr);

//@}

} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // MAP_UTILS_SCENARIO_TRANSFORMATION_HPP
