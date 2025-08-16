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

#ifndef POLARCOORDINATE_HPP
#define POLARCOORDINATE_HPP

#include "Angle.hpp"

namespace PatternVisualizer
{
class PolarCoordinate
{
public:
   constexpr PolarCoordinate() = default;

   constexpr PolarCoordinate(Angle aAngle, double aRadial)
      : mAngle{aAngle}
      , mRadial{aRadial}
   {
   }

   constexpr Angle  GetAngle() const noexcept { return mAngle; }
   constexpr double GetRadial() const noexcept { return mRadial; }

   void SetAngle(Angle aAngle) noexcept { mAngle = aAngle; }
   void SetRadial(double aRadial) noexcept { mRadial = aRadial; }

private:
   Angle  mAngle;
   double mRadial{};
};

constexpr bool operator==(PolarCoordinate a, PolarCoordinate b) noexcept
{
   return a.GetAngle() == b.GetAngle() && a.GetRadial() == b.GetRadial();
}

inline std::ostream& operator<<(std::ostream& aStream, PolarCoordinate aCoordinate)
{
   aStream << "(" << aCoordinate.GetAngle() << ", " << FloatToString(aCoordinate.GetRadial()) << ")";
   return aStream;
}

template<>
class ApproxImpl<PolarCoordinate>
{
public:
   using SelfType = ApproxImpl<PolarCoordinate>;

   explicit ApproxImpl(PolarCoordinate aValue)
      : mValue{aValue}
   {
   }

   friend bool operator==(const SelfType& a, PolarCoordinate b)
   {
      return Approx(a.mValue.GetAngle()).SetEpsilon(a.mEpsilon) == b.GetAngle() &&
             Approx(a.mValue.GetRadial()).SetEpsilon(a.mEpsilon) == b.GetRadial();
   }

   friend std::ostream& operator<<(std::ostream& aStream, const ApproxImpl<PolarCoordinate>& aApprox)
   {
      aStream << "Approx(" << aApprox.mValue << ")";
      return aStream;
   }

   SelfType& SetEpsilon(double aEpsilon)
   {
      mEpsilon = aEpsilon;
      return *this;
   }

private:
   double          mEpsilon{ApproxImpl<double>::DefaultEpsilon()};
   PolarCoordinate mValue;
};
} // namespace PatternVisualizer

#endif
