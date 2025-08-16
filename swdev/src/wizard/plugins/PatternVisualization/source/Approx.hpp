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

#ifndef APPROX_HPP
#define APPROX_HPP

#include <iostream>
#include <limits>
#include <type_traits>

#include "FloatToString.hpp"

namespace PatternVisualizer
{
// These definitions define an approximate comparison operators for various types. It is
// used as follows:
//
//    float someValue;
//    ...
//    if (Approx(someValue) == 1.234f)
//      ...
//
// Approx() supports the same comparison operators that the underlying type supports.
//
// It is possible to extend Approx() to support custom types. To do so, define a custom
// ApproxImpl<T> specialization for the specified type. At a minimum, ApproxImpl<T> must
// support the EqualityComparable concept, and this function:
//
//    bool operator==(const ApproxImpl<T>& a, const T& b);
//
// If the comparison operators are also desired for the given type, then ApproxImpl<T>
// must also support the LessThanComparable concept and these functions:
//
//    bool operator<(const T& a, const ApproxImpl<T>& b);
//    bool operator<(const ApproxImpl<T>& a, const T& b);
//
// The following function is also recommended to support debug logging with Catch:
//
//    std::ostream& operator<<(std::ostream& os, const ApproxImpl<T>& approx)
//
// See ApproxImpl<double> and ApproxImpl<float> for examples.
template<typename T>
class ApproxImpl
{
public:
   // We don't really care if T is a null pointer. We just need the test to expand to
   // false for all types approx will be defined for so this error is generated should
   // this template be instantiated.
   //      static_assert(std::is_null_pointer<T>::value, "no approx specialization defined for this type");
};

template<typename T>
bool operator>(const ApproxImpl<T>& a, const T& b)
{
   return b < a;
}
template<typename T>
bool operator<=(const ApproxImpl<T>& a, const T& b)
{
   return !(a > b);
}
template<typename T>
bool operator>=(const ApproxImpl<T>& a, const T& b)
{
   return !(a < b);
}
template<typename T>
bool operator!=(const ApproxImpl<T>& a, const T& b)
{
   return !(a == b);
}

template<typename T>
bool operator>(const T& a, const ApproxImpl<T>& b)
{
   return b < a;
}
template<typename T>
bool operator<=(const T& a, const ApproxImpl<T>& b)
{
   return !(b < a);
}
template<typename T>
bool operator>=(const T& a, const ApproxImpl<T>& b)
{
   return !(b > a);
}
template<typename T>
bool operator==(const T& a, const ApproxImpl<T>& b)
{
   return b == a;
}
template<typename T>
bool operator!=(const T& a, const ApproxImpl<T>& b)
{
   return !(b == a);
}

namespace Detail
{
template<typename T>
bool IsFloatApprox(T a, T b, T epsilon)
{
   return fabs(a - b) <= epsilon;
}
} // namespace Detail

template<>
class ApproxImpl<double>
{
public:
   using SelfType = ApproxImpl<double>;

   explicit ApproxImpl(double value)
      : mValue{value}
   {
   }

   friend bool operator<(double a, const SelfType& b) { return a < b.mValue; }
   friend bool operator<(const SelfType& a, double b) { return a.mValue < b; }
   friend bool operator==(const SelfType& a, double b) { return Detail::IsFloatApprox(a.mValue, b, a.mEpsilon); }

   friend std::ostream& operator<<(std::ostream& os, const ApproxImpl<double>& approx)
   {
      os << "Approx(" << FloatToString(approx.mValue, 12) << ")";
      return os;
   }

   static constexpr double DefaultEpsilon() noexcept { return 100.0 * std::numeric_limits<double>::epsilon(); }
   SelfType&               SetEpsilon(double epsilon)
   {
      mEpsilon = epsilon;
      return *this;
   }

private:
   double mEpsilon{DefaultEpsilon()};
   double mValue{};
};

template<>
class ApproxImpl<float>
{
public:
   using SelfType = ApproxImpl<float>;

   explicit ApproxImpl(float value)
      : mValue{value}
   {
   }

   friend bool operator<(float a, const SelfType& b) { return a < b.mValue; }
   friend bool operator<(const SelfType& a, float b) { return a.mValue < b; }
   friend bool operator==(const SelfType& a, float b) { return Detail::IsFloatApprox(a.mValue, b, a.mEpsilon); }

   friend std::ostream& operator<<(std::ostream& os, const ApproxImpl<float>& approx)
   {
      os << "Approx(" << FloatToString(approx.mValue, 6) << ")";
      return os;
   }

   static constexpr float DefaultEpsilon() noexcept { return 100.0f * std::numeric_limits<float>::epsilon(); }
   SelfType&              SetEpsilon(float epsilon)
   {
      mEpsilon = epsilon;
      return *this;
   }

private:
   float mEpsilon{DefaultEpsilon()};
   float mValue{};
};

template<typename T>
ApproxImpl<T> Approx(const T& value)
{
   return ApproxImpl<T>{value};
}
} // namespace PatternVisualizer

#endif
