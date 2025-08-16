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

#include "Session.hpp"

#include <QDir>
#include <QMessageBox>

#include "Canvas.hpp"
#include "Debug.hpp"
#include "GlobalPlotSettings.hpp"
#include "Pattern.hpp"
#include "PatternUpdateManager.hpp"
#include "PatternsTreeModel.hpp"
#include "UtInput.hpp"
#include "UtInputFile.hpp"
#include "UtSleep.hpp"
#include "WsfAcousticSignatureTypes.hpp"
#include "WsfAntennaPatternTypes.hpp"
#include "WsfComponentList.hpp"
#include "WsfComponentRoles.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Types.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfEM_XmtrRcvr.hpp"
#include "WsfInfraredSignatureTypes.hpp"
#include "WsfMilRadarSignature.hpp"
#include "WsfMode.hpp"
#include "WsfOpticalSignatureTypes.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfRadarSignatureTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorComponent.hpp"
#include "WsfSensorModeList.hpp"
#include "WsfSimulation.hpp"
#include "WsfStandardAcousticSignature.hpp"
#include "WsfStandardInfraredSignature.hpp"
#include "WsfStandardOpticalSignature.hpp"

namespace PatternVisualizer
{
Session::Session()
   : mPatternsTreeModelPtr{new PatternsTreeModel{this, nullptr}}
   , mPatternUpdateManagerPtr{new PatternUpdateManager(this)}
   , mGlobalPlotOptionsPtr{new GlobalPlotOptions{this}}
   , mPolarPlotOptionsPtr{new PolarPlotOptions{}}
   , mLocatorOptionsPtr{new LocatorOptions{this}}
   , mCameraPtr{new Camera{}}
{
   connect(mGlobalPlotOptionsPtr.get(), &GlobalPlotOptions::ManualScalingChanged, this, &Session::OnManualScalingChanged);
   connect(mGlobalPlotOptionsPtr.get(), &GlobalPlotOptions::FrequencyChanged, this, &Session::OnFrequencyChanged);
   connect(mGlobalPlotOptionsPtr.get(), &GlobalPlotOptions::FrequencyUnitsChanged, this, &Session::OnFrequencyUnitsChanged);
}

void Session::FinalizePatternUpdate(Pattern* aPatternPtr, bool aDataChanged)
{
   if (mCanvasPtr == nullptr)
   {
      return;
   }

   // Store flag to indicate that the input pattern is done updating its data
   SetDisplayedPatternStatus(true, aPatternPtr);

   // Update the canvas with the added/changed pattern
   mCanvasPtr->AddOrUpdatePattern(*aPatternPtr, aDataChanged);
   UpdateScaleDBRange();

   // Set the update state if all patterns are done updating their data
   {
      QMutexLocker locker(&mDisplayedPatternsMutex);
      for (Pattern* patternPtr : mDisplayedPatterns.keys())
      {
         if (!mDisplayedPatterns[patternPtr])
         {
            return;
         }
      }
   }

   // Update the Locator plane value
   mLocatorOptionsPtr->RecomputeDockValue();
}

bool Session::RemovePatternFromList(Pattern* aPatternPtr)
{
   QMutexLocker locker(&mDisplayedPatternsMutex);
   if (mDisplayedPatterns.contains(aPatternPtr))
   {
      mDisplayedPatterns.remove(aPatternPtr);
      return true;
   }
   return false;
}

void Session::RequestPatternUpdate(bool aUserInitiated, Pattern* aPatternPtr)
{
   if (aPatternPtr == nullptr)
   {
      QMutexLocker locker(&mDisplayedPatternsMutex);
      auto         keys = mDisplayedPatterns.keys();
      if (keys.size())
      {
         for (Pattern* patternPtr : keys)
         {
            mPatternUpdateManagerPtr->RequestUpdate(mGlobalPlotOptionsPtr->GetFrequencyHz(), patternPtr, aUserInitiated);
         }
      }
   }
   else
   {
      mPatternUpdateManagerPtr->RequestUpdate(mGlobalPlotOptionsPtr->GetFrequencyHz(), aPatternPtr, aUserInitiated);
   }
}

void Session::SetDisplayedPatternStatus(bool aIsUpdated, Pattern* aPatternPtr)
{
   QMutexLocker locker(&mDisplayedPatternsMutex);
   if (aPatternPtr == nullptr)
   {
      for (Pattern* patternPtr : mDisplayedPatterns.keys())
      {
         mDisplayedPatterns.insert(patternPtr, aIsUpdated);
      }
   }
   else
   {
      mDisplayedPatterns.insert(aPatternPtr, aIsUpdated);
   }
}

void Session::ShowHidePattern(Pattern* aPatternPtr, bool aShow, bool aUserInitiated)
{
   if (aShow)
   {
      // Request pattern update on separate thread so that GUI remains responsive
      // to user input
      RequestPatternUpdate(aUserInitiated, aPatternPtr);
   }
   else if (RemovePatternFromList(aPatternPtr))
   {
      // Remove the pattern from the canvas
      if (mCanvasPtr != nullptr)
      {
         mCanvasPtr->RemovePattern(*aPatternPtr);
         UpdateScaleDBRange();
      }
   }
}

void Session::SetCanvas(Canvas* aCanvasPtr)
{
   mCanvasPtr = aCanvasPtr;
}

void Session::LoadPatterns(const QString& aFileName, WsfScenario& aScenarioPtr, const QString& aSigViewFile)
{
   mNameGeneratorPtr.reset(new PatternNameGenerator(aFileName));

   // Get the antenna pattern types from the scenario
   WsfAntennaPatternTypes&    antennaPatternTypes = WsfAntennaPatternTypes::Get(aScenarioPtr);
   WsfRadarSignatureTypes&    radarSigTypes       = WsfRadarSignatureTypes::Get(aScenarioPtr);
   WsfOpticalSignatureTypes&  opticalSigTypes     = WsfOpticalSignatureTypes::Get(aScenarioPtr);
   WsfInfraredSignatureTypes& infraredSigTypes    = WsfInfraredSignatureTypes::Get(aScenarioPtr);
   WsfAcousticSignatureTypes& acousticSigTypes    = WsfAcousticSignatureTypes::Get(aScenarioPtr);

   mSimulationPtr = ut::make_unique<WsfSimulation>(aScenarioPtr, 0);

   TreeItem* parentPtr  = mTreeRootPtr;
   mTreeItemToSelectPtr = nullptr;
   mIsTreeComplete      = false;

   if (!aSigViewFile.isEmpty())
   {
      UtInput input;
      input.PushInput(ut::make_unique<UtInputFile>(aSigViewFile.toStdString()));
      std::string command, expectedEndCmd, categoryName, patternType;
      QString     errorMsg;
      QStringList expectedCmds = QStringList() << "sigview_category"
                                               << "end_sigview_category"
                                               << "antenna_pattern"
                                               << "radar_signature"
                                               << "optical_signature"
                                               << "infrared_signature"
                                               << "accoustic_signature";

      while (input.TryReadCommand(command))
      {
         if (!expectedEndCmd.empty())
         {
            // Ignore information within pattern commands since we're getting it from the scenario;
            // this was needed for SigView since it has no knowledge of the scenario
            if (command == expectedEndCmd)
            {
               expectedEndCmd = "";
            }
            else if (expectedCmds.contains(command.c_str()))
            {
               errorMsg = QString{R"(
                  Missing %1 command in SigView file:
                  <br>
                  <pre>%2</pre>
               )"}
                             .arg(expectedEndCmd.c_str(), input.GetLocation().c_str());
               break;
            }
         }

         else if (command == "sigview_category")
         {
            input.ReadValue(categoryName);
            parentPtr = parentPtr->appendChild(categoryName.c_str());
         }
         else if (command == "end_sigview_category")
         {
            parentPtr = parentPtr->parent();
         }

         else if (command == "antenna_pattern")
         {
            try
            {
               input.ReadValue(patternType);
               expectedEndCmd = "end_antenna_pattern";

               // Add the pattern to the list
               WsfAntennaPattern* wsfAntPat = antennaPatternTypes.Find(patternType);
               if (wsfAntPat->Initialize(mSimulationPtr.get()))
               {
                  AddAntPatternToTree(aFileName, wsfAntPat, parentPtr);
               }
               else
               {
                  errorMsg = QString{R"(
                     Could not initialize antenna pattern, %1, in SigView file:
                     <br>
                     <pre>%2</pre>
                     )"}
                                .arg(QString::fromStdString(patternType), QString::fromStdString(input.GetLocation()));
               }
            }
            catch (std::exception& e)
            {
               throw UtException(e.what());
            }
         }

         else if (command == "radar_signature")
         {
            input.ReadValue(patternType);
            expectedEndCmd = "end_radar_signature";

            // Add the signature for each state to the list
            auto* wsfSig = radarSigTypes.Find(patternType);
            AddSignatureToTree(aFileName, Pattern::Type::RADAR_SIG, wsfSig, parentPtr);
         }

         else if (command == "optical_signature")
         {
            input.ReadValue(patternType);
            expectedEndCmd = "end_optical_signature";

            // Add the signature for each state to the list
            auto* wsfSig = opticalSigTypes.Find(patternType);
            AddSignatureToTree(aFileName, Pattern::Type::OPTICAL_SIG, wsfSig, parentPtr);
         }

         else if (command == "infrared_signature")
         {
            input.ReadValue(patternType);
            expectedEndCmd = "end_infrared_signature";

            // Add the pattern to the list
            auto* wsfSig = infraredSigTypes.Find(patternType);
            AddSignatureToTree(aFileName, Pattern::Type::INFRARED_SIG, wsfSig, parentPtr);
         }

         else if (command == "acoustic_signature")
         {
            input.ReadValue(patternType);
            expectedEndCmd = "end_acoustic_signature";

            // Add the pattern to the list
            auto* wsfSig = acousticSigTypes.Find(patternType);
            AddSignatureToTree(aFileName, Pattern::Type::ACOUSTIC_SIG, wsfSig, parentPtr);
         }

         else
         {
            errorMsg = QString{R"(
               Error in SigView file:
               <br>
               <pre>%1</pre>
            )"}
                          .arg(input.GetLocation().c_str());
         }

         if (!errorMsg.isEmpty())
         {
            Debug::ShowError(errorMsg);
            break;
         }
      }
   }
   else
   {
      // Create a pattern data instance for each antenna in the file
      if (antennaPatternTypes.Size() > 0)
      {
         std::vector<WsfAntennaPattern*> wsfAntennaPatterns;
         antennaPatternTypes.GetCurrentUserTypes(wsfAntennaPatterns);
         parentPtr = parentPtr->appendChild("Antenna Patterns");
         for (WsfAntennaPattern* wsfAntPat : wsfAntennaPatterns)
         {
            wsfAntPat->Initialize(mSimulationPtr.get());
            AddAntPatternToTree(aFileName, wsfAntPat, parentPtr);
         }
         parentPtr = parentPtr->parent();
      }

      // Create a pattern data instance for each radar signature in the file
      if (radarSigTypes.Size() > 0)
      {
         std::vector<WsfRadarSignature*> wsfRadarSigs;
         radarSigTypes.GetCurrentUserTypes(wsfRadarSigs);
         parentPtr = parentPtr->appendChild("Radar Signatures");
         for (WsfRadarSignature* wsfSig : wsfRadarSigs)
         {
            AddSignatureToTree(aFileName, Pattern::Type::RADAR_SIG, wsfSig, parentPtr);
         }
         parentPtr = parentPtr->parent();
      }

      // Create a pattern data instance for each optical signature in the file
      if (opticalSigTypes.Size() > 0)
      {
         std::vector<WsfOpticalSignature*> wsfOpticalSigs;
         opticalSigTypes.GetCurrentUserTypes(wsfOpticalSigs);
         parentPtr = parentPtr->appendChild("Optical Signatures");
         for (WsfOpticalSignature* wsfSig : wsfOpticalSigs)
         {
            AddSignatureToTree(aFileName, Pattern::Type::OPTICAL_SIG, wsfSig, parentPtr);
         }
         parentPtr = parentPtr->parent();
      }

      // Create a pattern data instance for each infrared signature in the file
      if (infraredSigTypes.Size() > 0)
      {
         std::vector<WsfInfraredSignature*> wsfInfraredSigs;
         infraredSigTypes.GetCurrentUserTypes(wsfInfraredSigs);
         parentPtr = parentPtr->appendChild("Infrared Signatures");
         for (WsfInfraredSignature* wsfSig : wsfInfraredSigs)
         {
            AddSignatureToTree(aFileName, Pattern::Type::INFRARED_SIG, wsfSig, parentPtr);
         }
         parentPtr = parentPtr->parent();
      }

      // Create a pattern data instance for each acoustic signature in the file
      if (acousticSigTypes.Size() > 0)
      {
         std::vector<WsfAcousticSignature*> wsfAcousticSigs;
         acousticSigTypes.GetCurrentUserTypes(wsfAcousticSigs);
         parentPtr = parentPtr->appendChild("Acoustic Signatures");
         for (WsfAcousticSignature* wsfSig : wsfAcousticSigs)
         {
            AddSignatureToTree(aFileName, Pattern::Type::ACOUSTIC_SIG, wsfSig, parentPtr);
         }
         parentPtr = parentPtr->parent();
      }
   }

   // Populate the Patterns dock widget
   mIsTreeComplete = true;
   PopulatePatternsDock(aFileName);
}

void Session::PopulatePatternsDock(const QString& aFileName)
{
   if (mTreeRootPtr->childCount() == 0)
   {
      if (mDockWidgetPtr != nullptr)
      {
         mDockWidgetPtr->close();
      }

      QMessageBox msg;
      msg.setText("No patterns were found.");
      msg.exec();
   }
   else
   {
      if (mDockWidgetPtr != nullptr)
      {
         mDockWidgetPtr->show();
         mTreeItemToSelectPtr->SetChecked(true, false);
         mWidgetPtr->ExpandAll();

         // Update the dB scale as appropriate for the selected pattern or as specified manually
         UpdateScaleDBRange();

         // Raise the dock widget so it immediately becomes visible.
         mDockWidgetPtr->raise();
      }
   }
}

void Session::AddAntPatternToTree(const QString& aFileName, WsfAntennaPattern* aWsfPatternPtr, TreeItem* aParentPtr)
{
   if (aWsfPatternPtr == nullptr)
   {
      throw UtException("Pattern object not found.");
   }
   QString   itemName   = mNameGeneratorPtr->GetPatternName(*aWsfPatternPtr);
   Pattern*  patternPtr = new AntPattern(aFileName, *aWsfPatternPtr, this, itemName);
   TreeItem* childPtr   = new TreeItem(this, QList<QVariant>() << itemName, aParentPtr, patternPtr);
   aParentPtr->appendChild(childPtr);
   if (childPtr->isCheckable() && mTreeItemToSelectPtr == nullptr)
   {
      mTreeItemToSelectPtr = childPtr;
   }
}

void Session::AddSignatureToTree(const QString& aFileName, Pattern::Type aSigType, WsfSignature* aWsfSigPtr, TreeItem* aParentPtr)
{
   if (aWsfSigPtr == nullptr)
   {
      Debug::ShowError("failed to find/create/initialize signature");
      return;
   }
   QString sigName = mNameGeneratorPtr->GetPatternName(*aWsfSigPtr);
   if (aWsfSigPtr->RequiresPlatform())
   {
      Debug::ShowError("Cannot visualize " + sigName +
                       ".  This signature requires contextual information about a platform's location and time.");
      return;
   }

   // Create/append the signature node
   TreeItem* sigItem = new TreeItem(this, QList<QVariant>() << sigName, aParentPtr);
   aParentPtr->appendChild(sigItem);

   // Create/append the state and/or polarization nodes
   Pattern*  patternPtr;
   TreeItem* stateItemPtr;
   if (aSigType == Pattern::RADAR_SIG)
   {
      for (auto stateNameId : aWsfSigPtr->GetStateNames())
      {
         QString stateName = QString::fromStdString(stateNameId);
         stateItemPtr      = new TreeItem(this, QList<QVariant>() << "State: " + stateName, sigItem);
         sigItem->appendChild(stateItemPtr);
         for (int j = 0; j < WsfEM_Types::Polarization::cPOL_COUNT; j++)
         {
            patternPtr = new RadarSigPattern(aFileName,
                                             *dynamic_cast<WsfRadarSignature*>(aWsfSigPtr),
                                             this,
                                             sigName,
                                             stateName,
                                             static_cast<WsfEM_Types::Polarization>(j));
            TreeItem* polItemPtr;
            polItemPtr = new TreeItem(this,
                                      QList<QVariant>() << "Polarization: " + RadarSigPattern::sPolLabels[j],
                                      stateItemPtr,
                                      patternPtr);
            if (mTreeItemToSelectPtr == nullptr)
            {
               mTreeItemToSelectPtr = polItemPtr;
            }
            stateItemPtr->appendChild(polItemPtr);
         }
      }
   }
   else
   {
      for (auto stateNameId : aWsfSigPtr->GetStateNames())
      {
         QString stateName = QString::fromStdString(stateNameId);

         switch (aSigType)
         {
         case Pattern::OPTICAL_SIG:
            patternPtr =
               new OpticalSigPattern(aFileName, *dynamic_cast<WsfOpticalSignature*>(aWsfSigPtr), this, sigName, stateName);
            break;
         case Pattern::INFRARED_SIG:
            patternPtr =
               new InfraredSigPattern(aFileName, *dynamic_cast<WsfInfraredSignature*>(aWsfSigPtr), this, sigName, stateName);
            break;
         case Pattern::ACOUSTIC_SIG:
            patternPtr =
               new AcousticSigPattern(aFileName, *dynamic_cast<WsfAcousticSignature*>(aWsfSigPtr), this, sigName, stateName);
            break;
         default:
            return;
         }
         stateItemPtr = new TreeItem(this, QList<QVariant>() << "State: " + stateName, sigItem, patternPtr);
         if (mTreeItemToSelectPtr == nullptr)
         {
            mTreeItemToSelectPtr = stateItemPtr;
         }
         sigItem->appendChild(stateItemPtr);
      }
   }
}

void Session::OnManualScalingChanged(bool aIsManual)
{
   if (!aIsManual)
   {
      UpdateScaleDBRange();
   }
}

void Session::OnFrequencyChanged(float aValue)
{
   // Set the update flags for all displayed patterns; this must be done before any pattern is
   // commanded to update in case it would get done before all the flags were set
   SetDisplayedPatternStatus(false);

   // Update the displayed patterns
   RequestPatternUpdate(true);
}

void Session::OnFrequencyUnitsChanged(FrequencyUnits aUnits)
{
   // Set the update flags for all displayed patterns; this must be done before any pattern is
   // commanded to update in case it would get done before all the flags were set
   SetDisplayedPatternStatus(false);

   // Update the displayed patterns
   RequestPatternUpdate(true);
}

void Session::UpdateScaleDBRange()
{
   if (mCanvasPtr == nullptr)
   {
      return;
   }

   float minDB, maxDB, incDB = -1;

   const DataScale& scale = mGlobalPlotOptionsPtr->GetDataScale();
   if (scale.IsManualScaling() && !scale.IsResettingScale())
   {
      minDB = scale.GetMinDB();
      maxDB = scale.GetMaxDB();
      incDB = scale.GetIncDB();
      mCanvasPtr->UpdateScale(minDB, maxDB, incDB);
   }
   else if (HasPatterns())
   {
      minDB = std::numeric_limits<float>::max();
      maxDB = std::numeric_limits<float>::min();

      QMutexLocker locker(&mDisplayedPatternsMutex);
      for (Pattern* pattern : mDisplayedPatterns.keys())
      {
         if (mDisplayedPatterns[pattern])
         {
            minDB = std::min(minDB, pattern->GetMinDBAllFreqs());
            maxDB = std::max(maxDB, pattern->GetMaxDBAllFreqs());
         }
      }
      locker.unlock();

      incDB = scale.GetIncDB();
      mGlobalPlotOptionsPtr->SetScale(minDB, maxDB, incDB);
   }
}

void Session::Clear()
{
   GetPatternsTreeModel()->ClearTree();
   mSimulationPtr.reset(); // don't allow the simulation to outlive the scenario
}

float Session::GetCurrentValueAt(Angle aAzimuth, Angle aElevation, bool& aValid) const
{
   float retval{-1.0};
   aValid = false;

   if (mWidgetPtr != nullptr)
   {
      Pattern* patternPtr = mWidgetPtr->GetCurrentPattern();
      auto     iter       = mDisplayedPatterns.find(patternPtr);
      if (iter != mDisplayedPatterns.end() && iter.value())
      {
         PatternData* dataPtr = patternPtr->GetPatternData();
         if (dataPtr != nullptr)
         {
            retval = dataPtr->GetValueAtAngles(aAzimuth, aElevation);
            aValid = true;
         }
      }
   }

   return retval;
}

bool Session::HasPatterns()
{
   QMutexLocker locker(&mDisplayedPatternsMutex);
   return (mDisplayedPatterns.size() > 0);
}
} // namespace PatternVisualizer
