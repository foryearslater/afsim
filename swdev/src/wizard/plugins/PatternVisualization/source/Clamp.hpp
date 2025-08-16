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

#ifndef CLAMP_HPP
#define CLAMP_HPP

namespace PatternVisualizer
{
/**
 * \brief Clamps the given value between the lower and upper bounds inclusively.
 * \tparam T type
 * \param aValue value
 * \param aLower inclusive lower bound
 * \param aUpper inclusive upper bound
 * \return Returns the clamped value.
 */
template<typename T>
constexpr T Clamp(T aValue, T aLower, T aUpper)
{
   return std::max(std::min(aValue, aUpper), aLower);
}
} // namespace PatternVisualizer

#endif
