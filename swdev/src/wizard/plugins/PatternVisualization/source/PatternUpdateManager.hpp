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

#ifndef PATTERNUPDATEMANAGER_HPP
#define PATTERNUPDATEMANAGER_HPP

#include <QMap>
#include <QObject>
#include <QThread>

namespace PatternVisualizer
{
class Session;
class Pattern;
class UpdatePatternDataThread;

class PatternUpdateManager : public QObject
{
   Q_OBJECT
   friend class UpdatePatternDataThread;

public:
   explicit PatternUpdateManager(Session* aSessionPtr);
   ~PatternUpdateManager() override;

   PatternUpdateManager(const PatternUpdateManager&) = delete;
   PatternUpdateManager(PatternUpdateManager&&)      = delete;
   PatternUpdateManager& operator=(const PatternUpdateManager&) = delete;

   Pattern* GetNextRequestPattern();
   double   GetRequestedFreq(Pattern* aPatternPtr);
   Session* GetSession() { return mSessionPtr; }
   void     RequestUpdate(double aFreq, Pattern* aPatternPtr, bool aUserInitiated);
   void     StopRunningThread();

private:
   Session*                 mSessionPtr;
   UpdatePatternDataThread* mUpdateThreadPtr;
   QMap<Pattern*, double>   mPendingUpdates;
};

class UpdatePatternDataThread : public QThread
{
   Q_OBJECT
public:
   explicit UpdatePatternDataThread(PatternUpdateManager* aMgrPtr);
   ~UpdatePatternDataThread() {}

   void run();

signals:
   void FinalizePatternUpdate(Pattern* aPatternPtr, bool aDataChanged);

private:
   PatternUpdateManager* mUpdateMgrPtr;
};
} // namespace PatternVisualizer
#endif // PATTERNUPDATEMANAGER_HPP
