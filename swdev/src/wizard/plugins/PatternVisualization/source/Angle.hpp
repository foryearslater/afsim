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

#ifndef ANGLE_HPP
#define ANGLE_HPP

#include <cmath>
#include <type_traits>

#include "Approx.hpp"
#include "FloatToString.hpp"
#include "UtMath.hpp"

namespace PatternVisualizer
{

class Angle
{
public:
   constexpr Angle() = default;

   static constexpr Angle FromDegrees(double angleDegrees) { return Angle{angleDegrees * UtMath::cRAD_PER_DEG}; }
   static constexpr Angle FromRadians(double angleRadians) { return Angle{angleRadians}; }

   constexpr double GetDegrees() const noexcept { return mAngleRadians * UtMath::cDEG_PER_RAD; }
   constexpr double GetRadians() const noexcept { return mAngleRadians; }

   void SetDegrees(double angleDegrees) noexcept { mAngleRadians = angleDegrees * UtMath::cRAD_PER_DEG; }
   void SetRadians(double angleRadians) noexcept { mAngleRadians = angleRadians; }

   inline void Normalize(Angle start, Angle end);

   Angle& operator+=(Angle b)
   {
      mAngleRadians += b.mAngleRadians;
      return *this;
   }

   Angle& operator-=(Angle b)
   {
      mAngleRadians -= b.mAngleRadians;
      return *this;
   }

   Angle& operator*=(Angle b)
   {
      mAngleRadians *= b.mAngleRadians;
      return *this;
   }

   Angle& operator/=(Angle b)
   {
      mAngleRadians /= b.mAngleRadians;
      return *this;
   }

   Angle& operator*=(double b)
   {
      mAngleRadians *= b;
      return *this;
   }

   Angle& operator/=(double b)
   {
      mAngleRadians /= b;
      return *this;
   }

private:
   explicit constexpr Angle(double angleRadians) noexcept
      : mAngleRadians{angleRadians}
   {
   }

   double mAngleRadians{};
};

// The checks help to verify that Angle is light weight like a double.
// static_assert(std::is_default_constructible<Angle>::value, "Angle is_default_constructible check failed");
// static_assert(std::is_trivially_assignable<Angle, Angle>::value, "Angle is_trivially_assignable check failed");
// static_assert(std::is_trivially_copyable<Angle>::value, "Angle is_trivially_copyable check failed");
// static_assert(std::is_trivially_move_assignable<Angle>::value, "Angle is_trivially_move_assignable check failed");
// static_assert(std::is_trivially_move_constructible<Angle>::value, "Angle is_trivially_move_constructible check failed");
static_assert(sizeof(Angle) == sizeof(double), "Angle is not the proper size");

// Ensure that the Angle class doesn't decay into an unitless double.
static_assert(!std::is_constructible<Angle, double>::value, "Angle is_constructible from double check failed");
static_assert(!std::is_assignable<Angle, double>::value, "Angle is_assignable from double check failed");


constexpr Angle operator"" _deg(long double angleDegrees) noexcept
{
   return Angle::FromDegrees(static_cast<double>(angleDegrees));
}

constexpr Angle operator"" _rad(long double angleRadians) noexcept
{
   return Angle::FromRadians(static_cast<double>(angleRadians));
}

constexpr Angle operator"" _deg(unsigned long long angleDegrees) noexcept
{
   return Angle::FromDegrees(static_cast<double>(angleDegrees));
}

constexpr Angle operator"" _rad(unsigned long long angleRadians) noexcept
{
   return Angle::FromRadians(static_cast<double>(angleRadians));
}

constexpr Angle operator+(Angle a) noexcept
{
   return a;
}
constexpr Angle operator-(Angle a) noexcept
{
   return Angle::FromRadians(-a.GetRadians());
}

constexpr Angle operator+(Angle a, Angle b) noexcept
{
   return Angle::FromRadians(a.GetRadians() + b.GetRadians());
}
constexpr Angle operator-(Angle a, Angle b) noexcept
{
   return Angle::FromRadians(a.GetRadians() - b.GetRadians());
}
constexpr Angle operator*(Angle a, Angle b) noexcept
{
   return Angle::FromRadians(a.GetRadians() * b.GetRadians());
}
constexpr Angle operator/(Angle a, Angle b) noexcept
{
   return Angle::FromRadians(a.GetRadians() / b.GetRadians());
}
constexpr Angle operator*(Angle a, double b) noexcept
{
   return Angle::FromRadians(a.GetRadians() * b);
}
constexpr Angle operator/(Angle a, double b) noexcept
{
   return Angle::FromRadians(a.GetRadians() / b);
}
constexpr Angle operator*(double a, Angle b) noexcept
{
   return Angle::FromRadians(a * b.GetRadians());
}
constexpr Angle operator/(double a, Angle b) noexcept
{
   return Angle::FromRadians(a / b.GetRadians());
}

constexpr bool operator==(Angle a, Angle b) noexcept
{
   return a.GetRadians() == b.GetRadians();
}
constexpr bool operator!=(Angle a, Angle b) noexcept
{
   return a.GetRadians() != b.GetRadians();
}
constexpr bool operator<(Angle a, Angle b) noexcept
{
   return a.GetRadians() < b.GetRadians();
}
constexpr bool operator>(Angle a, Angle b) noexcept
{
   return a.GetRadians() > b.GetRadians();
}
constexpr bool operator<=(Angle a, Angle b) noexcept
{
   return a.GetRadians() <= b.GetRadians();
}
constexpr bool operator>=(Angle a, Angle b) noexcept
{
   return a.GetRadians() >= b.GetRadians();
}

constexpr Angle AngleZero      = Angle();
constexpr Angle AnglePi        = Angle::FromRadians(UtMath::cPI);
constexpr Angle AngleTwoPi     = 2.0 * AnglePi;
constexpr Angle AnglePiOverTwo = AnglePi / 2.0;

inline double cos(Angle angle)
{
   return std::cos(angle.GetRadians());
}
inline double sin(Angle angle)
{
   return std::sin(angle.GetRadians());
}
inline double tan(Angle angle)
{
   return std::tan(angle.GetRadians());
}

inline void Angle::Normalize(Angle start, Angle end)
{
   const double width       = end.mAngleRadians - start.mAngleRadians;
   const double offsetValue = mAngleRadians - start.mAngleRadians;

   mAngleRadians = offsetValue - floor(offsetValue / width) * width + start.mAngleRadians;
}

// Linear interpolates a value that is between [valueMin,valueMax] to an angle that is between
// [minAngle,maxAngle].
template<typename T>
constexpr Angle LerpAngle(T value, T valueMin, T valueMax, Angle minAngle, Angle maxAngle)
{
   return minAngle + (value - valueMin) * (maxAngle - minAngle) / (valueMax - valueMin);
}

inline std::ostream& operator<<(std::ostream& os, Angle angle)
{
   const char degreeSymbol[2] = "\xB0";
   os << FloatToString(angle.GetDegrees()) << degreeSymbol;
   return os;
}

template<>
class ApproxImpl<Angle>
{
public:
   using SelfType = ApproxImpl<Angle>;

   explicit ApproxImpl(Angle value)
      : mValue{value}
   {
   }

   friend bool operator<(Angle a, const SelfType& b) { return a < b.mValue; }
   friend bool operator<(const SelfType& a, Angle b) { return a.mValue < b; }
   friend bool operator==(const SelfType& a, Angle b)
   {
      return Approx(a.mValue.GetRadians()).SetEpsilon(a.mEpsilon) == b.GetRadians();
   }

   friend std::ostream& operator<<(std::ostream& os, const ApproxImpl<Angle>& approx)
   {
      os << "Approx(" << approx.mValue << ")";
      return os;
   }

   SelfType& SetEpsilon(double epsilon)
   {
      mEpsilon = epsilon;
      return *this;
   }

private:
   double mEpsilon{ApproxImpl<double>::DefaultEpsilon()};
   Angle  mValue;
};

} // namespace PatternVisualizer

#endif
