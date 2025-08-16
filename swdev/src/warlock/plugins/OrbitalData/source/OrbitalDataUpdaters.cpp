//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#include "OrbitalDataUpdaters.hpp"

#include "WsfSpaceMoverBase.hpp"

QString WkOrbitalData::EccentricityUpdater::GetValueString() const
{
   return QString::number(mValue);
}

bool WkOrbitalData::EccentricityUpdater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetEccentricity();
   return true;
}

double WkOrbitalData::SemiMajorAxisUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkOrbitalData::SemiMajorAxisUpdater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetSemiMajorAxis();
   return true;
}

double WkOrbitalData::InclinationUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkOrbitalData::InclinationUpdater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetInclination();
   return true;
}

double WkOrbitalData::RAAN_Updater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkOrbitalData::RAAN_Updater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetRAAN();
   return true;
}

double WkOrbitalData::ArgOfPeriapsisUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkOrbitalData::ArgOfPeriapsisUpdater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetArgumentOfPeriapsis();
   return true;
}

double WkOrbitalData::TrueAnomalyUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkOrbitalData::TrueAnomalyUpdater::ReadData(const UtOrbitalElements& aElements)
{
   mValue = aElements.GetTrueAnomaly();
   return true;
}
