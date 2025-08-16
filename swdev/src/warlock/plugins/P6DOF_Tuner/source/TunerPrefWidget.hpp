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

#ifndef TUNERPREFWIDGET_HPP
#define TUNERPREFWIDGET_HPP

#include "TunerData.hpp"
#include "TunerPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_TunerPrefWidget.h"

namespace WkTuner
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT

public:
   explicit PrefWidget();
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override { return "P6DOF"; }

   double GetPopSize(P6DOF::Pid::Type aPid) const;
   double GetMaxGenerations(P6DOF::Pid::Type aPid) const;
   double GetCarryOverFrac(P6DOF::Pid::Type aPid) const;
   double GetCrossoverProb(P6DOF::Pid::Type aPid) const;
   double GetMutationProb(P6DOF::Pid::Type aPid) const;

   void   GetKpRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax);
   void   GetKiRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax);
   void   GetKdRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax);
   double GetRunTime(P6DOF::Pid::Type aPid);
   double GetErrorWeight(P6DOF::Pid::Type aPid);
   double GetOvershootWeight(P6DOF::Pid::Type aPid);

signals:
   void TestActionActivated();

private:
   void RestoreGADefaults();
   void RestorePIDDefaults();
   void CheckPopSize();
   void CheckMaxGenerations();
   void CheckCarryOverFrac();
   void CheckCrossoverProb();
   void CheckMutationProb();
   void CheckRunTime();

   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::TunerPrefWidget mUi;
};
} // namespace WkTuner

#endif
