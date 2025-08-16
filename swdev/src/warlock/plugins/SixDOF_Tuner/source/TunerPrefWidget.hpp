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

#ifndef TUNERPREFWIDGET_HPP
#define TUNERPREFWIDGET_HPP

#include "TunerData.hpp"
#include "TunerPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_TunerPrefWidget.h"

namespace WkSixDOF_Tuner
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT

public:
   explicit PrefWidget();
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override { return "SixDOF"; }

   double GetPopSize(wsf::six_dof::Pid::Type aPid) const;
   double GetMaxGenerations(wsf::six_dof::Pid::Type aPid) const;
   double GetCarryOverFrac(wsf::six_dof::Pid::Type aPid) const;
   double GetCrossoverProb(wsf::six_dof::Pid::Type aPid) const;
   double GetMutationProb(wsf::six_dof::Pid::Type aPid) const;

   void   GetKpRange(wsf::six_dof::Pid::Type aPid, double& aMin, double& aMax);
   void   GetKiRange(wsf::six_dof::Pid::Type aPid, double& aMin, double& aMax);
   void   GetKdRange(wsf::six_dof::Pid::Type aPid, double& aMin, double& aMax);
   double GetRunTime(wsf::six_dof::Pid::Type aPid);
   double GetErrorWeight(wsf::six_dof::Pid::Type aPid);
   double GetOvershootWeight(wsf::six_dof::Pid::Type aPid);

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
} // namespace WkSixDOF_Tuner

#endif
