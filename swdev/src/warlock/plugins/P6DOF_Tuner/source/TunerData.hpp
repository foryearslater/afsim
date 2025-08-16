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

#ifndef TUNERDATA_HPP
#define TUNERDATA_HPP

#include <map>

#include <QString>

#include "P6DofVehicleData.hpp"

namespace WkTuner
{
namespace Data
{
const std::map<P6DOF::Pid::Type, QString> PID_NameMap = {{P6DOF::Pid::Unknown, " "},
                                                         {P6DOF::Pid::Alpha, "Alpha"},
                                                         {P6DOF::Pid::VertSpeed, "Vertical Speed"},
                                                         {P6DOF::Pid::PitchAngle, "Pitch Angle"},
                                                         {P6DOF::Pid::PitchRate, "Pitch Rate"},
                                                         {P6DOF::Pid::FltPathAngle, "Flight Path Angle"},
                                                         {P6DOF::Pid::DeltaPitch, "Delta Pitch"},
                                                         {P6DOF::Pid::Altitude, "Altitude"},
                                                         {P6DOF::Pid::Beta, "Beta"},
                                                         {P6DOF::Pid::YawRate, "Yaw Rate"},
                                                         {P6DOF::Pid::YawHeading, "Yaw Heading"},
                                                         {P6DOF::Pid::TaxiHeading, "Taxi Heading"},
                                                         {P6DOF::Pid::RollRate, "Roll Rate"},
                                                         {P6DOF::Pid::DeltaRoll, "Delta Roll"},
                                                         {P6DOF::Pid::BankAngle, "Bank Angle"},
                                                         {P6DOF::Pid::RollHeading, "Roll Heading"},
                                                         {P6DOF::Pid::ForwardAccel, "Forward Accel"},
                                                         {P6DOF::Pid::Speed, "Speed"},
                                                         {P6DOF::Pid::TaxiForwardAccel, "Taxi Forward Accel"},
                                                         {P6DOF::Pid::TaxiSpeed, "Taxi Speed"},
                                                         {P6DOF::Pid::TaxiYawRate, "Taxi Yaw Rate"}};

// Note: this might end up the same as PID_NameMap, if so, remove this
const std::map<P6DOF::Pid::Type, QString> PID_SendCommandLabelMap = {{P6DOF::Pid::Unknown, " "},
                                                                     {P6DOF::Pid::Alpha, "Alpha"},
                                                                     {P6DOF::Pid::VertSpeed, "Vertical Speed"},
                                                                     {P6DOF::Pid::PitchAngle, "Pitch Angle"},
                                                                     {P6DOF::Pid::PitchRate, "Pitch Rate"},
                                                                     {P6DOF::Pid::FltPathAngle, "Flight Path Angle"},
                                                                     {P6DOF::Pid::DeltaPitch, "Delta Pitch"},
                                                                     {P6DOF::Pid::Altitude, "Altitude"},
                                                                     {P6DOF::Pid::Beta, "Beta"},
                                                                     {P6DOF::Pid::YawRate, "Yaw Rate"},
                                                                     {P6DOF::Pid::YawHeading, "Yaw Heading"},
                                                                     {P6DOF::Pid::TaxiHeading, "Taxi Heading"},
                                                                     {P6DOF::Pid::RollRate, "Roll Rate"},
                                                                     {P6DOF::Pid::DeltaRoll, "Delta Roll"},
                                                                     {P6DOF::Pid::BankAngle, "Bank Angle"},
                                                                     {P6DOF::Pid::RollHeading, "Roll Heading"},
                                                                     {P6DOF::Pid::ForwardAccel, "Forward Accel"},
                                                                     {P6DOF::Pid::Speed, "Speed"},
                                                                     {P6DOF::Pid::TaxiForwardAccel, "Taxi Forward Accel"},
                                                                     {P6DOF::Pid::TaxiSpeed, "Taxi Speed"},
                                                                     {P6DOF::Pid::TaxiYawRate, "Taxi Yaw Rate"}};

const std::map<P6DOF::Pid::Type, P6DOF::Pid::Type> PID_InnerMap = {{P6DOF::Pid::Unknown, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::Alpha, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::VertSpeed, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::PitchAngle, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::PitchRate, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::FltPathAngle, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::DeltaPitch, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::Altitude, P6DOF::Pid::Alpha},
                                                                   {P6DOF::Pid::Beta, P6DOF::Pid::Beta},
                                                                   {P6DOF::Pid::YawRate, P6DOF::Pid::Beta},
                                                                   {P6DOF::Pid::YawHeading, P6DOF::Pid::Beta},
                                                                   {P6DOF::Pid::TaxiHeading, P6DOF::Pid::TaxiYawRate},
                                                                   {P6DOF::Pid::RollRate, P6DOF::Pid::RollRate},
                                                                   {P6DOF::Pid::DeltaRoll, P6DOF::Pid::RollRate},
                                                                   {P6DOF::Pid::BankAngle, P6DOF::Pid::RollRate},
                                                                   {P6DOF::Pid::RollHeading, P6DOF::Pid::RollRate},
                                                                   {P6DOF::Pid::ForwardAccel, P6DOF::Pid::ForwardAccel},
                                                                   {P6DOF::Pid::Speed, P6DOF::Pid::Speed},
                                                                   {P6DOF::Pid::TaxiForwardAccel,
                                                                    P6DOF::Pid::TaxiForwardAccel},
                                                                   {P6DOF::Pid::TaxiSpeed, P6DOF::Pid::TaxiSpeed},
                                                                   {P6DOF::Pid::TaxiYawRate, P6DOF::Pid::TaxiYawRate}};

const std::map<P6DOF::Pid::Type, P6DOF::Pid::Type> PID_MiddleMap = {{P6DOF::Pid::Unknown, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::Alpha, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::VertSpeed, P6DOF::Pid::VertSpeed},
                                                                    {P6DOF::Pid::PitchAngle, P6DOF::Pid::PitchAngle},
                                                                    {P6DOF::Pid::PitchRate, P6DOF::Pid::PitchRate},
                                                                    {P6DOF::Pid::FltPathAngle, P6DOF::Pid::FltPathAngle},
                                                                    {P6DOF::Pid::DeltaPitch, P6DOF::Pid::DeltaPitch},
                                                                    {P6DOF::Pid::Altitude, P6DOF::Pid::VertSpeed},
                                                                    {P6DOF::Pid::Beta, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::YawRate, P6DOF::Pid::YawRate},
                                                                    {P6DOF::Pid::YawHeading, P6DOF::Pid::YawRate},
                                                                    {P6DOF::Pid::TaxiHeading, P6DOF::Pid::TaxiHeading},
                                                                    {P6DOF::Pid::RollRate, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::DeltaRoll, P6DOF::Pid::DeltaRoll},
                                                                    {P6DOF::Pid::BankAngle, P6DOF::Pid::BankAngle},
                                                                    {P6DOF::Pid::RollHeading, P6DOF::Pid::BankAngle},
                                                                    {P6DOF::Pid::ForwardAccel, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::Speed, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::TaxiForwardAccel, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::TaxiSpeed, P6DOF::Pid::Unknown},
                                                                    {P6DOF::Pid::TaxiYawRate, P6DOF::Pid::Unknown}};

const std::map<P6DOF::Pid::Type, P6DOF::Pid::Type> PID_OuterMap = {{P6DOF::Pid::Unknown, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::Alpha, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::VertSpeed, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::PitchAngle, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::PitchRate, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::FltPathAngle, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::DeltaPitch, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::Altitude, P6DOF::Pid::Altitude},
                                                                   {P6DOF::Pid::Beta, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::YawRate, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::YawHeading, P6DOF::Pid::YawHeading},
                                                                   {P6DOF::Pid::TaxiHeading, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::RollRate, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::DeltaRoll, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::BankAngle, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::RollHeading, P6DOF::Pid::RollHeading},
                                                                   {P6DOF::Pid::ForwardAccel, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::Speed, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::TaxiForwardAccel, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::TaxiSpeed, P6DOF::Pid::Unknown},
                                                                   {P6DOF::Pid::TaxiYawRate, P6DOF::Pid::Unknown}};

const std::map<P6DOF::Pid::Type, QString> PID_RelevantLimit1Map = {{P6DOF::Pid::Unknown, ""},
                                                                   {P6DOF::Pid::Alpha, "Min Alpha"},
                                                                   {P6DOF::Pid::VertSpeed, "Min Vertical Speed"},
                                                                   {P6DOF::Pid::PitchAngle, ""},
                                                                   {P6DOF::Pid::PitchRate, "Min Pitch Rate"},
                                                                   {P6DOF::Pid::FltPathAngle, "Min Pitch Rate"},
                                                                   {P6DOF::Pid::DeltaPitch, ""},
                                                                   {P6DOF::Pid::Altitude, ""},
                                                                   {P6DOF::Pid::Beta, "Beta Limit"},
                                                                   {P6DOF::Pid::YawRate, "Max Yaw Rate"},
                                                                   {P6DOF::Pid::YawHeading, ""},
                                                                   {P6DOF::Pid::TaxiHeading, ""},
                                                                   {P6DOF::Pid::RollRate, "Max Roll Rate"},
                                                                   {P6DOF::Pid::DeltaRoll, ""},
                                                                   {P6DOF::Pid::BankAngle, "Max Bank Angle"},
                                                                   {P6DOF::Pid::RollHeading, ""},
                                                                   {P6DOF::Pid::ForwardAccel, "Min Forward Accel"},
                                                                   {P6DOF::Pid::Speed, ""},
                                                                   {P6DOF::Pid::TaxiForwardAccel,
                                                                    "Taxi Forward Accel Limit 1"},
                                                                   {P6DOF::Pid::TaxiSpeed, ""},
                                                                   {P6DOF::Pid::TaxiYawRate, "Max Taxi Yaw Rate"}};

const std::map<P6DOF::Pid::Type, QString> PID_RelevantLimit2Map = {{P6DOF::Pid::Unknown, ""},
                                                                   {P6DOF::Pid::Alpha, "Max Alpha"},
                                                                   {P6DOF::Pid::VertSpeed, "Max Vertical Speed"},
                                                                   {P6DOF::Pid::PitchAngle, ""},
                                                                   {P6DOF::Pid::PitchRate, "Max Pitch Rate"},
                                                                   {P6DOF::Pid::FltPathAngle, "Max Pitch Rate"},
                                                                   {P6DOF::Pid::DeltaPitch, ""},
                                                                   {P6DOF::Pid::Altitude, ""},
                                                                   {P6DOF::Pid::Beta, ""},
                                                                   {P6DOF::Pid::YawRate, ""},
                                                                   {P6DOF::Pid::YawHeading, ""},
                                                                   {P6DOF::Pid::TaxiHeading, ""},
                                                                   {P6DOF::Pid::RollRate, ""},
                                                                   {P6DOF::Pid::DeltaRoll, ""},
                                                                   {P6DOF::Pid::BankAngle, ""},
                                                                   {P6DOF::Pid::RollHeading, ""},
                                                                   {P6DOF::Pid::ForwardAccel, "Max Forward Accel"},
                                                                   {P6DOF::Pid::Speed, ""},
                                                                   {P6DOF::Pid::TaxiForwardAccel,
                                                                    "Taxi Forward Accel Limit 2"},
                                                                   {P6DOF::Pid::TaxiSpeed, ""},
                                                                   {P6DOF::Pid::TaxiYawRate, ""}};

enum CustomCommand
{
   KTAS,
   KCAS,
   Mach,
   DynamicPressure,
   PitchG,
   YawG,
};

const std::map<CustomCommand, QString> SpeedTypeMap = {{KTAS, "True Air Speed"},
                                                       {KCAS, "Calibrated Air Speed"},
                                                       {Mach, "Mach"},
                                                       {DynamicPressure, "Dynamic Pressure"}};
} // namespace Data
} // namespace WkTuner

#endif
