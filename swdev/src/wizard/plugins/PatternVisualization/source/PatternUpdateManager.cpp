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

#include "PatternUpdateManager.hpp"

#include "Pattern.hpp"
#include "Session.hpp"

namespace PatternVisualizer
{

PatternUpdateManager::PatternUpdateManager(Session* aSessionPtr)
   : QObject(aSessionPtr)
   , mSessionPtr(aSessionPtr)
   , mUpdateThreadPtr(nullptr)
{
}

PatternUpdateManager::~PatternUpdateManager()
{
   mPendingUpdates.clear();
   if (mUpdateThreadPtr != nullptr)
   {
      mUpdateThreadPtr->quit();
   }
}

Pattern* PatternUpdateManager::GetNextRequestPattern()
{
   if (!mPendingUpdates.isEmpty())
   {
      return mPendingUpdates.begin().key();
   }
   return nullptr;
}

double PatternUpdateManager::GetRequestedFreq(Pattern* aPatternPtr)
{
   if (mPendingUpdates.contains(aPatternPtr))
   {
      return mPendingUpdates.take(aPatternPtr);
   }
   return -1;
}

void PatternUpdateManager::RequestUpdate(double aFreq, Pattern* aPatternPtr, bool aUserInitiated)
{
   if (aUserInitiated)
   {
      mPendingUpdates.insert(aPatternPtr, aFreq);
      if (mUpdateThreadPtr == nullptr)
      {
         mUpdateThreadPtr = new UpdatePatternDataThread(this);
         connect(mUpdateThreadPtr, &QThread::finished, mUpdateThreadPtr, &QThread::deleteLater);
         mUpdateThreadPtr->start();
      }
   }
   else
   {
      aPatternPtr->UpdatePatternData(aFreq);
      mSessionPtr->FinalizePatternUpdate(aPatternPtr, true);
   }
}

void PatternUpdateManager::StopRunningThread()
{
   if (mUpdateThreadPtr)
   {
      mUpdateThreadPtr->quit();
      mUpdateThreadPtr->wait();
   }
}

//##################################################################################################

UpdatePatternDataThread::UpdatePatternDataThread(PatternUpdateManager* aMgrPtr)
   : QThread(aMgrPtr)
   , mUpdateMgrPtr(aMgrPtr)
{
   connect(this,
           &UpdatePatternDataThread::FinalizePatternUpdate,
           static_cast<Session*>(aMgrPtr->parent()),
           &Session::FinalizePatternUpdate);
}

void UpdatePatternDataThread::run()
{
   ut::SetupThreadErrorHandling();

   // Update the pattern data for all patterns in the queue
   Pattern* patternPtr;
   while ((patternPtr = mUpdateMgrPtr->GetNextRequestPattern()) != nullptr)
   {
      double freq;
      bool   dataChanged = false;

      // Repeat the update for the current pattern until it is no longer in the queue
      // in case requests were added for another frequency
      while ((freq = mUpdateMgrPtr->GetRequestedFreq(patternPtr)) > 0)
      {
         dataChanged = (patternPtr->UpdatePatternData(freq));
      }

      // Emit signal to finalize pattern update; Session::FinalizePatternUpdate can only
      // be called directly from the GUI thread
      emit FinalizePatternUpdate(patternPtr, dataChanged);
   }

   mUpdateMgrPtr->mUpdateThreadPtr = nullptr;
}
} // namespace PatternVisualizer
