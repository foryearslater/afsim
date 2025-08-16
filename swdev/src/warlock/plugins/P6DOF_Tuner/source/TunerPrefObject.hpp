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

#ifndef TUNERPREFERENCESOBJECT_HPP
#define TUNERPREFERENCESOBJECT_HPP

#include "TunerData.hpp"
#include "WkfPrefObject.hpp"

namespace WkTuner
{
struct PID_Prefs
{
   double minKp{0.0};
   double maxKp{100.0};
   double minKi{0.0};
   double maxKi{100.0};
   double minKd{0.0};
   double maxKd{100.0};
   double runTime{20.0};
   double errorWeight{1.0};
   double overshootWeight{1.0};
   double mPopulationSize{25.0};
   double mMaxGenerations{50.0};
   double mCarryOverFrac{0.7};
   double mCrossoverProb{0.5};
   double mMutationProb{0.02};
};

struct PrefData
{
   PrefData();

   std::map<P6DOF::Pid::Type, PID_Prefs> mPidPrefMap;
   QString                               mMoverCreatorFileStr;
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

   friend class PrefWidget;

public:
   explicit PrefObject(QObject* parent = nullptr);

   const QString& GetLaunchedFromMoverCreatorFile() const;

signals:
   void MoverCreatorFileSet();

private:
   void     SetPreferenceDataP(const PrefData& aPrefData) override;
   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   void RestoreGADefaults(P6DOF::Pid::Type aPid);
   void RestorePIDDefaults(P6DOF::Pid::Type aPid);
};
} // namespace WkTuner

Q_DECLARE_METATYPE(WkTuner::PrefData)
#endif // TUNERPREFERENCESOBJECT_HPP
