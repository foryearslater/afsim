// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef REGEXVALIDATOR_HPP
#define REGEXVALIDATOR_HPP

#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>

#include "FFIRN_Validator.hpp"

namespace usmtf
{

//! RegexValidator is intended to hand complex cases which cause
//! headaches with traditional rule building Validators. If this
//! works out it will probably become status quo, we didn't build
//! it early naively thinking that it was unnecessary. But now were
//! spending lots of time codifying parse and extraction variances.
//! It was thought they would be more generically reusable.

//! were using whole value matching because we want parsing after matching
//! to be simplified. If its valid, you should be able to use static string positions.
//! We can if we need move to search for sub-expressions etc.
class USMTF_EXPORT RegexValidator : public FFIRN_Validator
{
public:
   // It would be nice to have better names and some convenience functions for building regex ranges.
   // However, ranges are pretty difficulty in regex, I highly suggest using RangedValidator if the
   // field is just the range value. That is much easier. Regex is easier when values have complicated
   // sub components in one value.
   static const std::string cAny;
   static const std::string c00to90;
   static const std::string c00to59;
   static const std::string c00to59dec000to999;
   static const std::string c00to59dec0to9;
   static const std::string c01to60;
   static const std::string cNorS;
   static const std::string cAtoZ;
   static const std::string c0to9;
   static const std::string cHyphenDelimeter;
   static const std::string c000to180;
   static const std::string c000to359;
   static const std::string c0to999;
   static const std::string c1to999;
   static const std::string c000to99999;
   static const std::string cEorW;
   static const std::string cStart;
   static const std::string cEnd;
   static const std::string cSpace;

   RegexValidator() = default;
   //! Run against whole field content. If Matches, safely extract portions.
   //! I see this be used in conjunction with variant validator quite a bit
   //! to solve complex extraction. Lets see!!
   bool ParseAndValidate(const usmtf::Field& aField) override;
   //! Configure to run against regex needs.
   void SetRegexExpression(const std::string& aExpression);

   //! Help your users understand by providing them something nice to read
   //! when validation doesnt pass.
   void SetErrorMessage(const std::string& aMessage);

protected:
   QRegularExpressionMatch mMatchResults;
   QRegularExpression      mRegexExpression;
   std::string             mRegexString{"No Regex Provided"};
   std::string             mErrorMessage{"Failed Regex Validation for: " + mRegexString};
};
} // namespace usmtf

#endif
