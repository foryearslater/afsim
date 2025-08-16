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

#ifndef TUNERDATA_HPP
#define TUNERDATA_HPP

#include <map>

#include <QString>

#include "WsfSixDOF_VehicleData.hpp"

namespace WkSixDOF_Tuner
{
namespace Data
{
extern const std::map<wsf::six_dof::Pid::Type, QString> PID_NameMap;

// Note: this might end up the same as PID_NameMap, if so, remove this
extern const std::map<wsf::six_dof::Pid::Type, QString> PID_SendCommandLabelMap;

extern const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_InnerMap;

extern const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_MiddleMap;

extern const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_OuterMap;

extern const std::map<wsf::six_dof::Pid::Type, QString> PID_RelevantLimit1Map;

extern const std::map<wsf::six_dof::Pid::Type, QString> PID_RelevantLimit2Map;

enum CustomCommand
{
   KTAS,
   KCAS,
   Mach,
   DynamicPressure,
   PitchG,
   YawG,
};

extern const std::map<CustomCommand, QString> SpeedTypeMap;
} // namespace Data
} // namespace WkSixDOF_Tuner

#endif
