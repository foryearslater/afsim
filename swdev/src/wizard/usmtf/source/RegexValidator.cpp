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

#include "RegexValidator.hpp"


namespace usmtf
{
bool RegexValidator::ParseAndValidate(const Field& aField)
{
   SetField(aField);
   if (!mRegexExpression.isValid())
   {
      AddError("Regex is invalid", mRegexString, mRegexExpression.errorString().toStdString());
      return false;
   }

   if (mRegexString == "No Regex Provided")
   {
      AddError("Regex is invalid", mRegexString, mErrorMessage);
      return false;
   }

   mMatchResults = mRegexExpression.match(QString::fromStdString(aField.GetContent()));
   if (!mMatchResults.hasMatch())
   {
      AddError("Validation Failed", aField.GetContent(), mErrorMessage);
      return false;
   }
   return true;
}

void RegexValidator::SetRegexExpression(const std::string& aExpression)
{
   mRegexString = aExpression;
   SetErrorMessage("Failed Regex Validation for: " + mRegexString);
   mRegexExpression = QRegularExpression(QString::fromStdString(aExpression));
}

void RegexValidator::SetErrorMessage(const std::string& aMessage)
{
   mErrorMessage = aMessage;
}

const std::string RegexValidator::cAny               = R"((.*))";
const std::string RegexValidator::cAtoZ              = R"((A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z))";
const std::string RegexValidator::c00to90            = R"(([0-8][0-9]|90))";
const std::string RegexValidator::c00to59            = R"(([0-5][0-9]))";
const std::string RegexValidator::c00to59dec000to999 = R"(([0-5][0-9])(.)([0-9]{3}))";
const std::string RegexValidator::c00to59dec0to9     = R"(([0-5][0-9])(.)([0-9]{1}))";
const std::string RegexValidator::c01to60            = R"(([0-5][1-9]|10|20|30|40|50|60))";
const std::string RegexValidator::cNorS              = R"((N|S))";
const std::string RegexValidator::c0to9              = R"(([0-9]))";
const std::string RegexValidator::cHyphenDelimeter   = R"((-))";
const std::string RegexValidator::c000to180          = R"((0[0-9]{2}|1[0-7][0-9]|180))";
const std::string RegexValidator::c000to359          = R"((0[0-9]{2}|1[0-9]{2}|2[0-9]{2}|3[0-5][0-9]))";
const std::string RegexValidator::c0to999            = R"(([0-9]{1,3}))";
const std::string RegexValidator::c1to999            = R"(([1-9][0-9]{0,2}))";
const std::string RegexValidator::c000to99999        = R"(([0-9]{3,5}))";
const std::string RegexValidator::cEorW              = R"((E|W))";
const std::string RegexValidator::cStart             = R"(^)";
const std::string RegexValidator::cEnd               = R"($)";
const std::string RegexValidator::cSpace             = R"( )";
} // namespace usmtf
