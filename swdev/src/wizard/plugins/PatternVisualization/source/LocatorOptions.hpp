// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef LOCATOROPTIONS_HPP
#define LOCATOROPTIONS_HPP

#include <QObject>

#include "Angle.hpp"

class QSettings;

namespace PatternVisualizer
{
class Session;

class LocatorOptions : public QObject
{
   Q_OBJECT
public:
   explicit LocatorOptions(const Session* aSessionPtr);
   ~LocatorOptions() = default;

   void LoadSettings(QSettings& aSettings);
   void SaveSettings(QSettings& aSettings);

   Angle GetAzimuth() const { return mAzimuth; }
   Angle GetElevation() const { return mElevation; }
   bool  GetShowLocator() const { return mLocatorShown; }

public slots:
   void SetAzEl(Angle aAzimuth, Angle aElevation);
   void RecomputeDockValue();
   void SetLocatorShown(bool aShown);

signals:
   void AzElChanged(Angle aAzimuth, Angle aElevation);
   void UpdateDockValue(float aValue, bool aValid);
   void ShowLocatorChanged(bool aIsShown);

private:
   Angle          mAzimuth;
   Angle          mElevation;
   const Session* mSessionPtr;
   bool           mLocatorShown;
};
} // namespace PatternVisualizer

#endif // LOCATOROPTIONS_HPP
