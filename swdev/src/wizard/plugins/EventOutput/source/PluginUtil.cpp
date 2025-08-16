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

#include "PluginUtil.hpp"

#include "CommandValuePair.hpp"

namespace wizard
{
namespace EventOutput
{
namespace PluginUtil
{

Signals*                 Signals::mInstancePtr{nullptr};
std::unique_ptr<Signals> mSignals{ut::make_unique<Signals>()};

Signals::Signals(QObject* aParent /* = nullptr */)
   : QObject(aParent)
{
   assert(mInstancePtr == nullptr);
   mInstancePtr = this;
}

Signals::~Signals()
{
   assert(mInstancePtr != nullptr);
   mInstancePtr = nullptr;
}

void SetBoolCheckBox(QCheckBox* aCheckBoxPtr, const QString& aValue, const QString& aCheckedValue /* = "true" */)
{
   if (aValue == aCheckedValue)
   {
      aCheckBoxPtr->setChecked(true);
   }
   else
   {
      aCheckBoxPtr->setChecked(false);
   }
}

void SetBoolToken(editor::token::CommandValuePair& aToken,
                  bool                             aValue,
                  const QString&                   aFalseValue /* = "false" */,
                  const QString&                   aTrueValue /* = "true"  */)
{
   if (aValue)
   {
      aToken.SetValue(aTrueValue);
   }
   else
   {
      aToken.SetValue(aFalseValue);
   }
}

void SetBoolToken(editor::token::CommandValuePair& aToken,
                  const QString&                   aValue,
                  const QString&                   aFalseValue /* = "false" */,
                  const QString&                   aTrueValue /* = "true"  */)
{
   SetBoolToken(aToken, aValue == aTrueValue, aFalseValue, aTrueValue);
}

bool SetBoolToken(editor::token::CommandValuePair& aToken,
                  event::Value                     aState,
                  const QString&                   aFalseValue /* = "false" */,
                  const QString&                   aTrueValue /* = "true"  */)
{
   switch (aState)
   {
   case event::Value::cFALSE:
      aToken.SetValue(aFalseValue);
      return true;
   case event::Value::cTRUE:
      aToken.SetValue(aTrueValue);
      return true;
   default:
      return false;
   }
}

} // namespace PluginUtil
} // namespace EventOutput
} // namespace wizard
