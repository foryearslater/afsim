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

#include "TunerData.hpp"

#include <map>

#include <QString>

namespace WkSixDOF_Tuner
{
namespace Data
{
// std::map variables in this block are definitions of extern-declared variables in the header file

const std::map<wsf::six_dof::Pid::Type, QString> PID_NameMap = {{wsf::six_dof::Pid::Unknown, " "},
                                                                {wsf::six_dof::Pid::Alpha, "Alpha"},
                                                                {wsf::six_dof::Pid::VerticalSpeed, "Vertical Speed"},
                                                                {wsf::six_dof::Pid::PitchAngle, "Pitch Angle"},
                                                                {wsf::six_dof::Pid::PitchRate, "Pitch Rate"},
                                                                {wsf::six_dof::Pid::FlightPathAngle, "Flight Path Angle"},
                                                                {wsf::six_dof::Pid::DeltaPitch, "Delta Pitch"},
                                                                {wsf::six_dof::Pid::Altitude, "Altitude"},
                                                                {wsf::six_dof::Pid::Beta, "Beta"},
                                                                {wsf::six_dof::Pid::YawRate, "Yaw Rate"},
                                                                {wsf::six_dof::Pid::YawHeading, "Yaw Heading"},
                                                                {wsf::six_dof::Pid::TaxiHeading, "Taxi Heading"},
                                                                {wsf::six_dof::Pid::RollRate, "Roll Rate"},
                                                                {wsf::six_dof::Pid::DeltaRoll, "Delta Roll"},
                                                                {wsf::six_dof::Pid::BankAngle, "Bank Angle"},
                                                                {wsf::six_dof::Pid::RollHeading, "Roll Heading"},
                                                                {wsf::six_dof::Pid::ForwardAccel, "Forward Accel"},
                                                                {wsf::six_dof::Pid::Speed, "Speed"},
                                                                {wsf::six_dof::Pid::TaxiForwardAccel, "Taxi Forward Accel"},
                                                                {wsf::six_dof::Pid::TaxiSpeed, "Taxi Speed"},
                                                                {wsf::six_dof::Pid::TaxiYawRate, "Taxi Yaw Rate"}};

const std::map<wsf::six_dof::Pid::Type, QString> PID_SendCommandLabelMap = PID_NameMap;

const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_InnerMap = {
   {wsf::six_dof::Pid::Unknown, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Alpha, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::VerticalSpeed, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::PitchAngle, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::PitchRate, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::FlightPathAngle, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::DeltaPitch, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::Altitude, wsf::six_dof::Pid::Alpha},
   {wsf::six_dof::Pid::Beta, wsf::six_dof::Pid::Beta},
   {wsf::six_dof::Pid::YawRate, wsf::six_dof::Pid::Beta},
   {wsf::six_dof::Pid::YawHeading, wsf::six_dof::Pid::Beta},
   {wsf::six_dof::Pid::TaxiHeading, wsf::six_dof::Pid::TaxiYawRate},
   {wsf::six_dof::Pid::RollRate, wsf::six_dof::Pid::RollRate},
   {wsf::six_dof::Pid::DeltaRoll, wsf::six_dof::Pid::RollRate},
   {wsf::six_dof::Pid::BankAngle, wsf::six_dof::Pid::RollRate},
   {wsf::six_dof::Pid::RollHeading, wsf::six_dof::Pid::RollRate},
   {wsf::six_dof::Pid::ForwardAccel, wsf::six_dof::Pid::ForwardAccel},
   {wsf::six_dof::Pid::Speed, wsf::six_dof::Pid::Speed},
   {wsf::six_dof::Pid::TaxiForwardAccel, wsf::six_dof::Pid::TaxiForwardAccel},
   {wsf::six_dof::Pid::TaxiSpeed, wsf::six_dof::Pid::TaxiSpeed},
   {wsf::six_dof::Pid::TaxiYawRate, wsf::six_dof::Pid::TaxiYawRate}};

const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_MiddleMap = {
   {wsf::six_dof::Pid::Unknown, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Alpha, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::VerticalSpeed, wsf::six_dof::Pid::VerticalSpeed},
   {wsf::six_dof::Pid::PitchAngle, wsf::six_dof::Pid::PitchAngle},
   {wsf::six_dof::Pid::PitchRate, wsf::six_dof::Pid::PitchRate},
   {wsf::six_dof::Pid::FlightPathAngle, wsf::six_dof::Pid::FlightPathAngle},
   {wsf::six_dof::Pid::DeltaPitch, wsf::six_dof::Pid::DeltaPitch},
   {wsf::six_dof::Pid::Altitude, wsf::six_dof::Pid::VerticalSpeed},
   {wsf::six_dof::Pid::Beta, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::YawRate, wsf::six_dof::Pid::YawRate},
   {wsf::six_dof::Pid::YawHeading, wsf::six_dof::Pid::YawRate},
   {wsf::six_dof::Pid::TaxiHeading, wsf::six_dof::Pid::TaxiHeading},
   {wsf::six_dof::Pid::RollRate, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::DeltaRoll, wsf::six_dof::Pid::DeltaRoll},
   {wsf::six_dof::Pid::BankAngle, wsf::six_dof::Pid::BankAngle},
   {wsf::six_dof::Pid::RollHeading, wsf::six_dof::Pid::BankAngle},
   {wsf::six_dof::Pid::ForwardAccel, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Speed, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiForwardAccel, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiSpeed, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiYawRate, wsf::six_dof::Pid::Unknown}};

const std::map<wsf::six_dof::Pid::Type, wsf::six_dof::Pid::Type> PID_OuterMap = {
   {wsf::six_dof::Pid::Unknown, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Alpha, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::VerticalSpeed, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::PitchAngle, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::PitchRate, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::FlightPathAngle, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::DeltaPitch, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Altitude, wsf::six_dof::Pid::Altitude},
   {wsf::six_dof::Pid::Beta, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::YawRate, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::YawHeading, wsf::six_dof::Pid::YawHeading},
   {wsf::six_dof::Pid::TaxiHeading, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::RollRate, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::DeltaRoll, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::BankAngle, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::RollHeading, wsf::six_dof::Pid::RollHeading},
   {wsf::six_dof::Pid::ForwardAccel, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::Speed, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiForwardAccel, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiSpeed, wsf::six_dof::Pid::Unknown},
   {wsf::six_dof::Pid::TaxiYawRate, wsf::six_dof::Pid::Unknown}};

const std::map<wsf::six_dof::Pid::Type, QString> PID_RelevantLimit1Map = {
   {wsf::six_dof::Pid::Unknown, ""},
   {wsf::six_dof::Pid::Alpha, "Min Alpha"},
   {wsf::six_dof::Pid::VerticalSpeed, "Min Vertical Speed"},
   {wsf::six_dof::Pid::PitchAngle, ""},
   {wsf::six_dof::Pid::PitchRate, "Min Pitch Rate"},
   {wsf::six_dof::Pid::FlightPathAngle, ""},
   {wsf::six_dof::Pid::DeltaPitch, ""},
   {wsf::six_dof::Pid::Altitude, ""},
   {wsf::six_dof::Pid::Beta, "Beta Limit"},
   {wsf::six_dof::Pid::YawRate, "Max Yaw Rate"},
   {wsf::six_dof::Pid::YawHeading, ""},
   {wsf::six_dof::Pid::TaxiHeading, ""},
   {wsf::six_dof::Pid::RollRate, "Max Roll Rate"},
   {wsf::six_dof::Pid::DeltaRoll, ""},
   {wsf::six_dof::Pid::BankAngle, "Max Bank Angle"},
   {wsf::six_dof::Pid::RollHeading, ""},
   {wsf::six_dof::Pid::ForwardAccel, "Min Forward Accel"},
   {wsf::six_dof::Pid::Speed, ""},
   {wsf::six_dof::Pid::TaxiForwardAccel, "Taxi Forward Accel Limit 1"},
   {wsf::six_dof::Pid::TaxiSpeed, ""},
   {wsf::six_dof::Pid::TaxiYawRate, "Max Taxi Yaw Rate"}};

const std::map<wsf::six_dof::Pid::Type, QString> PID_RelevantLimit2Map = {
   {wsf::six_dof::Pid::Unknown, ""},
   {wsf::six_dof::Pid::Alpha, "Max Alpha"},
   {wsf::six_dof::Pid::VerticalSpeed, "Max Vertical Speed"},
   {wsf::six_dof::Pid::PitchAngle, ""},
   {wsf::six_dof::Pid::PitchRate, "Max Pitch Rate"},
   {wsf::six_dof::Pid::FlightPathAngle, ""},
   {wsf::six_dof::Pid::DeltaPitch, ""},
   {wsf::six_dof::Pid::Altitude, ""},
   {wsf::six_dof::Pid::Beta, ""},
   {wsf::six_dof::Pid::YawRate, ""},
   {wsf::six_dof::Pid::YawHeading, ""},
   {wsf::six_dof::Pid::TaxiHeading, ""},
   {wsf::six_dof::Pid::RollRate, ""},
   {wsf::six_dof::Pid::DeltaRoll, ""},
   {wsf::six_dof::Pid::BankAngle, ""},
   {wsf::six_dof::Pid::RollHeading, ""},
   {wsf::six_dof::Pid::ForwardAccel, "Max Forward Accel"},
   {wsf::six_dof::Pid::Speed, ""},
   {wsf::six_dof::Pid::TaxiForwardAccel, "Taxi Forward Accel Limit 2"},
   {wsf::six_dof::Pid::TaxiSpeed, ""},
   {wsf::six_dof::Pid::TaxiYawRate, ""}};

const std::map<CustomCommand, QString> SpeedTypeMap = {{KTAS, "True Air Speed"},
                                                       {KCAS, "Calibrated Air Speed"},
                                                       {Mach, "Mach"},
                                                       {DynamicPressure, "Dynamic Pressure"}};
} // namespace Data
} // namespace WkSixDOF_Tuner
