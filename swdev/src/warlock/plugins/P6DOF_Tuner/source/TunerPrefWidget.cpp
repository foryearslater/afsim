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

#include "TunerPrefWidget.hpp"

#include "TunerPrefObject.hpp"

WkTuner::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUi.setupUi(this);

   QDoubleValidator* validator = new QDoubleValidator(this);
   validator->setBottom(0.0);

   mUi.populationSizeLineEdit->setValidator(validator);
   mUi.maxGenerationsLineEdit->setValidator(validator);
   mUi.carryoverFracLineEdit->setValidator(validator);
   mUi.crossOverProbLineEdit->setValidator(validator);
   mUi.mutationProbLineEdit->setValidator(validator);
   mUi.minKpLineEdit->setValidator(validator);
   mUi.maxKpLineEdit->setValidator(validator);
   mUi.minKiLineEdit->setValidator(validator);
   mUi.maxKiLineEdit->setValidator(validator);
   mUi.minKdLineEdit->setValidator(validator);
   mUi.maxKdLineEdit->setValidator(validator);
   mUi.runtimeLineEdit->setValidator(validator);
   mUi.errorWeightLineEdit->setValidator(validator);
   mUi.overshootWeightLineEdit->setValidator(validator);

   for (auto& i : Data::PID_NameMap)
   {
      if (i.first != P6DOF::Pid::Unknown)
      {
         mUi.pidComboBox->addItem(i.second, i.first);
      }
   }

   connect(mUi.pidComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           [this]() { ReadPreferenceData(mPrefObjectPtr->GetPreferences()); });
   connect(mUi.resetGADefaultPushButton, &QPushButton::clicked, this, &PrefWidget::RestoreGADefaults);
   connect(mUi.resetPIDDefaultPushButton, &QPushButton::clicked, this, &PrefWidget::RestorePIDDefaults);
   connect(mUi.populationSizeLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckPopSize);
   connect(mUi.maxGenerationsLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckMaxGenerations);
   connect(mUi.carryoverFracLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckCarryOverFrac);
   connect(mUi.crossOverProbLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckCrossoverProb);
   connect(mUi.mutationProbLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckMutationProb);
   connect(mUi.runtimeLineEdit, &QLineEdit::textEdited, this, &PrefWidget::CheckRunTime);
}

double WkTuner::PrefWidget::GetPopSize(P6DOF::Pid::Type aPid) const
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).mPopulationSize;
}

double WkTuner::PrefWidget::GetMaxGenerations(P6DOF::Pid::Type aPid) const
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).mMaxGenerations;
}

double WkTuner::PrefWidget::GetCarryOverFrac(P6DOF::Pid::Type aPid) const
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).mCarryOverFrac;
}

double WkTuner::PrefWidget::GetCrossoverProb(P6DOF::Pid::Type aPid) const
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).mCrossoverProb;
}

double WkTuner::PrefWidget::GetMutationProb(P6DOF::Pid::Type aPid) const
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).mMutationProb;
}

void WkTuner::PrefWidget::GetKpRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;

   aMin = pidMap.at(aPid).minKp;
   aMax = pidMap.at(aPid).maxKp;
}

void WkTuner::PrefWidget::GetKiRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;

   aMin = pidMap.at(aPid).minKi;
   aMax = pidMap.at(aPid).maxKi;
}

void WkTuner::PrefWidget::GetKdRange(P6DOF::Pid::Type aPid, double& aMin, double& aMax)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;

   aMin = pidMap.at(aPid).minKd;
   aMax = pidMap.at(aPid).maxKd;
}

double WkTuner::PrefWidget::GetRunTime(P6DOF::Pid::Type aPid)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).runTime;
}

double WkTuner::PrefWidget::GetErrorWeight(P6DOF::Pid::Type aPid)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).errorWeight;
}

double WkTuner::PrefWidget::GetOvershootWeight(P6DOF::Pid::Type aPid)
{
   const auto& pidMap = mPrefObjectPtr->GetPreferences().mPidPrefMap;
   return pidMap.at(aPid).overshootWeight;
}

void WkTuner::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.populationSizeLineEdit->setText(QString::number(
      aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).mPopulationSize));
   mUi.maxGenerationsLineEdit->setText(QString::number(
      aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).mMaxGenerations));
   mUi.carryoverFracLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).mCarryOverFrac));
   mUi.crossOverProbLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).mCrossoverProb));
   mUi.mutationProbLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).mMutationProb));

   mUi.minKpLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).minKp));
   mUi.maxKpLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).maxKp));
   mUi.minKiLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).minKi));
   mUi.maxKiLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).maxKi));
   mUi.minKdLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).minKd));
   mUi.maxKdLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).maxKd));
   mUi.runtimeLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).runTime));
   mUi.errorWeightLineEdit->setText(
      QString::number(aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).errorWeight));
   mUi.overshootWeightLineEdit->setText(QString::number(
      aPrefData.mPidPrefMap.at((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt()).overshootWeight));
}

void WkTuner::PrefWidget::RestoreGADefaults()
{
   if (mPrefObjectPtr)
   {
      mPrefObjectPtr->RestoreGADefaults((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt());
   }
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void WkTuner::PrefWidget::RestorePIDDefaults()
{
   if (mPrefObjectPtr)
   {
      mPrefObjectPtr->RestorePIDDefaults((P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt());
   }
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void WkTuner::PrefWidget::CheckPopSize()
{
   if (mUi.populationSizeLineEdit->text().toInt() < 0)
   {
      mUi.populationSizeLineEdit->setText(QString::number(0));
   }
}

void WkTuner::PrefWidget::CheckMaxGenerations()
{
   if (mUi.maxGenerationsLineEdit->text().toInt() < 0)
   {
      mUi.maxGenerationsLineEdit->setText(QString::number(0));
   }
}

void WkTuner::PrefWidget::CheckCarryOverFrac()
{
   if (mUi.carryoverFracLineEdit->text().toDouble() < 0)
   {
      mUi.carryoverFracLineEdit->setText(QString::number(0));
   }

   if (mUi.carryoverFracLineEdit->text().toDouble() > 1.0)
   {
      mUi.carryoverFracLineEdit->setText(QString::number(1.0));
   }
}

void WkTuner::PrefWidget::CheckCrossoverProb()
{
   if (mUi.crossOverProbLineEdit->text().toDouble() < 0)
   {
      mUi.crossOverProbLineEdit->setText(QString::number(0));
   }

   if (mUi.crossOverProbLineEdit->text().toDouble() > 1.0)
   {
      mUi.crossOverProbLineEdit->setText(QString::number(1.0));
   }
}

void WkTuner::PrefWidget::CheckMutationProb()
{
   if (mUi.mutationProbLineEdit->text().toDouble() < 0)
   {
      mUi.mutationProbLineEdit->setText(QString::number(0));
   }

   if (mUi.mutationProbLineEdit->text().toDouble() > 1.0)
   {
      mUi.mutationProbLineEdit->setText(QString::number(1.0));
   }
}

void WkTuner::PrefWidget::CheckRunTime()
{
   if (mUi.runtimeLineEdit->text().toDouble() < 0)
   {
      mUi.runtimeLineEdit->setText(QString::number(0));
   }
}

void WkTuner::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mPidPrefMap.clear();
   auto       pid          = (P6DOF::Pid::Type)mUi.pidComboBox->currentData().toInt();
   PID_Prefs& pidPref      = aPrefData.mPidPrefMap[pid];
   pidPref.minKp           = mUi.minKpLineEdit->text().toDouble();
   pidPref.maxKp           = mUi.maxKpLineEdit->text().toDouble();
   pidPref.minKi           = mUi.minKiLineEdit->text().toDouble();
   pidPref.maxKi           = mUi.maxKiLineEdit->text().toDouble();
   pidPref.minKd           = mUi.minKdLineEdit->text().toDouble();
   pidPref.maxKd           = mUi.maxKdLineEdit->text().toDouble();
   pidPref.runTime         = mUi.runtimeLineEdit->text().toDouble();
   pidPref.errorWeight     = mUi.errorWeightLineEdit->text().toDouble();
   pidPref.overshootWeight = mUi.overshootWeightLineEdit->text().toDouble();
   pidPref.mPopulationSize = mUi.populationSizeLineEdit->text().toDouble();
   pidPref.mMaxGenerations = mUi.maxGenerationsLineEdit->text().toDouble();
   pidPref.mCarryOverFrac  = mUi.carryoverFracLineEdit->text().toDouble();
   pidPref.mCrossoverProb  = mUi.crossOverProbLineEdit->text().toDouble();
   pidPref.mMutationProb   = mUi.mutationProbLineEdit->text().toDouble();
}
