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

#include "CommandValuePair.hpp"

namespace wizard
{
namespace EventOutput
{
namespace editor
{
namespace token
{

CommandValuePair::CommandValuePair(QString aCommand, QString aValue)
   : mCommand(std::move(aCommand))
   , mValue(std::move(aValue))
{
}

CommandValuePair::CommandValuePair(QStringList aCommandValue)
   : mCommand(std::move(aCommandValue[0]))
   , mValue(std::move(aCommandValue[1]))
{
}

QString SettingCVP::ToStringP() const
{
   return {GetCommand() + ' ' + GetValue()};
}

QString EventCVP::ToStringP() const
{
   return {GetValue() + ' ' + GetCommand()};
}

} // namespace token
} // namespace editor
} // namespace EventOutput
} // namespace wizard
