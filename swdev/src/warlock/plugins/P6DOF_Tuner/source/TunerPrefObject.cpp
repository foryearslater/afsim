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

#include "TunerPrefObject.hpp"

#include <QSettings>

WkTuner::PrefData::PrefData()
{
   for (const auto& pid : Data::PID_NameMap)
   {
      if (pid.first != P6DOF::Pid::Unknown)
      {
         auto it = mPidPrefMap.emplace(pid.first, PID_Prefs{}).first;

         switch (pid.first)
         {
         case P6DOF::Pid::Alpha:
         case P6DOF::Pid::Beta:
         case P6DOF::Pid::BankAngle:
            it->second.runTime         = 10;
            it->second.overshootWeight = 10.0;
            break;

         case P6DOF::Pid::Altitude:
         case P6DOF::Pid::Speed:
         case P6DOF::Pid::RollHeading:
         case P6DOF::Pid::YawHeading:
            it->second.runTime = 90;
            break;

         default:
            break;
         }
      }
   }
}

WkTuner::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "P6DOFTuner")
{
}

const QString& WkTuner::PrefObject::GetLaunchedFromMoverCreatorFile() const
{
   return mCurrentPrefs.mMoverCreatorFileStr;
}

void WkTuner::PrefObject::SetPreferenceDataP(const PrefData& aPrefData)
{
   for (auto& pd : aPrefData.mPidPrefMap)
   {
      mCurrentPrefs.mPidPrefMap.erase(pd.first);
      mCurrentPrefs.mPidPrefMap.insert(pd);
   }

   mCurrentPrefs.mMoverCreatorFileStr = aPrefData.mMoverCreatorFileStr;
}

void WkTuner::PrefObject::Apply()
{
   if (mCurrentPrefs.mMoverCreatorFileStr != "")
   {
      emit MoverCreatorFileSet();
   }
}

WkTuner::PrefData WkTuner::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   for (const auto& i : Data::PID_NameMap)
   {
      if (i.first != P6DOF::Pid::Unknown)
      {
         aSettings.beginGroup(i.second);

         pData.mPidPrefMap[i.first].minKp =
            aSettings.value("minKp", mDefaultPrefs.mPidPrefMap.at(i.first).minKp).toDouble();
         pData.mPidPrefMap[i.first].maxKp =
            aSettings.value("maxKp", mDefaultPrefs.mPidPrefMap.at(i.first).maxKp).toDouble();
         pData.mPidPrefMap[i.first].minKi =
            aSettings.value("minKi", mDefaultPrefs.mPidPrefMap.at(i.first).minKi).toDouble();
         pData.mPidPrefMap[i.first].maxKi =
            aSettings.value("maxKi", mDefaultPrefs.mPidPrefMap.at(i.first).maxKi).toDouble();
         pData.mPidPrefMap[i.first].minKd =
            aSettings.value("minKd", mDefaultPrefs.mPidPrefMap.at(i.first).minKd).toDouble();
         pData.mPidPrefMap[i.first].maxKd =
            aSettings.value("maxKd", mDefaultPrefs.mPidPrefMap.at(i.first).maxKd).toDouble();

         pData.mPidPrefMap[i.first].runTime =
            aSettings.value("runTime", mDefaultPrefs.mPidPrefMap.at(i.first).runTime).toDouble();
         pData.mPidPrefMap[i.first].errorWeight =
            aSettings.value("errorWeight", mDefaultPrefs.mPidPrefMap.at(i.first).errorWeight).toDouble();
         pData.mPidPrefMap[i.first].overshootWeight =
            aSettings.value("overshootWeight", mDefaultPrefs.mPidPrefMap.at(i.first).overshootWeight).toDouble();

         // Get the genetic algorithm values
         aSettings.beginGroup("GeneticAlgorithm");
         pData.mPidPrefMap[i.first].mPopulationSize =
            aSettings.value("populationSize", mDefaultPrefs.mPidPrefMap.at(i.first).mPopulationSize).toDouble();
         pData.mPidPrefMap[i.first].mMaxGenerations =
            aSettings.value("maxGenerations", mDefaultPrefs.mPidPrefMap.at(i.first).mMaxGenerations).toDouble();
         pData.mPidPrefMap[i.first].mCarryOverFrac =
            aSettings.value("carryoverFraction", mDefaultPrefs.mPidPrefMap.at(i.first).mCarryOverFrac).toDouble();
         pData.mPidPrefMap[i.first].mCrossoverProb =
            aSettings.value("crossoverProbability", mDefaultPrefs.mPidPrefMap.at(i.first).mCrossoverProb).toDouble();
         pData.mPidPrefMap[i.first].mMutationProb =
            aSettings.value("mutationProbability", mDefaultPrefs.mPidPrefMap.at(i.first).mMutationProb).toDouble();

         aSettings.endGroup();
         aSettings.endGroup();
      }
   }
   pData.mMoverCreatorFileStr = aSettings.value("moverCreatorFile").toString();

   return pData;
}

void WkTuner::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   for (auto& i : mCurrentPrefs.mPidPrefMap)
   {
      aSettings.beginGroup(Data::PID_NameMap.at(i.first));

      aSettings.setValue("minKp", i.second.minKp);
      aSettings.setValue("maxKp", i.second.maxKp);
      aSettings.setValue("minKi", i.second.minKi);
      aSettings.setValue("maxKi", i.second.maxKi);
      aSettings.setValue("minKd", i.second.minKd);
      aSettings.setValue("maxKd", i.second.maxKd);
      aSettings.setValue("runTime", i.second.runTime);
      aSettings.setValue("errorWeight", i.second.errorWeight);
      aSettings.setValue("overshootWeight", i.second.overshootWeight);

      aSettings.beginGroup("GeneticAlgorithm");
      aSettings.setValue("populationSize", i.second.mPopulationSize);
      aSettings.setValue("maxGenerations", i.second.mMaxGenerations);
      aSettings.setValue("carryoverFraction", i.second.mCarryOverFrac);
      aSettings.setValue("crossoverProbability", i.second.mCrossoverProb);
      aSettings.setValue("mutationProbability", i.second.mMutationProb);

      aSettings.endGroup();
      aSettings.endGroup();
   }
}

void WkTuner::PrefObject::RestoreGADefaults(P6DOF::Pid::Type aPid)
{
   mCurrentPrefs.mPidPrefMap.at(aPid).mPopulationSize = mDefaultPrefs.mPidPrefMap.at(aPid).mPopulationSize;
   mCurrentPrefs.mPidPrefMap.at(aPid).mMaxGenerations = mDefaultPrefs.mPidPrefMap.at(aPid).mMaxGenerations;
   mCurrentPrefs.mPidPrefMap.at(aPid).mCarryOverFrac  = mDefaultPrefs.mPidPrefMap.at(aPid).mCarryOverFrac;
   mCurrentPrefs.mPidPrefMap.at(aPid).mCrossoverProb  = mDefaultPrefs.mPidPrefMap.at(aPid).mCrossoverProb;
   mCurrentPrefs.mPidPrefMap.at(aPid).mMutationProb   = mDefaultPrefs.mPidPrefMap.at(aPid).mMutationProb;
}

void WkTuner::PrefObject::RestorePIDDefaults(P6DOF::Pid::Type aPid)
{
   mCurrentPrefs.mPidPrefMap.at(aPid) = mDefaultPrefs.mPidPrefMap.at(aPid);
}
