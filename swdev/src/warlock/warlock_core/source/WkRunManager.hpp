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

#ifndef WKRUNMANAGER_HPP
#define WKRUNMANAGER_HPP

#include "warlock_core_export.h"

#include <memory>

#include <QProgressDialog>
#include <QString>
#include <QThread>

#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfStandardApplication.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT RunManager : public QObject
{
   Q_OBJECT

public:
   static RunManager& GetInstance();

   static void Create(WsfStandardApplication& aApp);

   const QList<QStringList>&              GetRecentScenarios() const { return mRecentScenarios; }
   const WsfStandardApplication::Options& GetApplicationOptions() const { return mOptions; }
   void                                   RemoveRecentScenario(const QStringList& aFileList);
   void                                   ClearRecentScenarios();
   QString                                GetMostRecentDirectory() const;

   void ReloadScenario();
   void LoadScenario(const QStringList& aFileList);
   // This function will take control of aOptions and delete it
   void LoadScenario(const WsfStandardApplication::Options& aOptions);

   void Shutdown();

   bool IsSimulationActive() const;

   // For window title and recent menu actions/links
   static QString GetScenarioDisplayName(const QStringList& aFileList, bool aDisplayFullPath);

signals:
   void RecentScenarioListChanged();

protected:
   RunManager(WsfStandardApplication& aApp);
   ~RunManager() override = default;

   void HandleScenarioLoadFinished();

   void StartLoading();
   void CreateSimThread(std::unique_ptr<WsfScenario> aScenarioPtr);

   void ReadSettings();
   void WriteHistory();
   void SetMostRecentScenario(const QStringList& aFileList);

   void AssignScenarioToSimThread(std::unique_ptr<WsfScenario> aScenarioPtr);

   static QStringList StandardizeFilePaths(const QStringList& aFileList, bool* aFilesExist = nullptr);

   class LoadThread : public QThread
   {
   public:
      LoadThread(QObject*                               aParent,
                 WsfStandardApplication&                aApplication,
                 const WsfStandardApplication::Options& aOptions,
                 const std::string&                     aAdditionalInput);

      ~LoadThread() override = default;

      bool                         WasSuccessful() const { return mSuccessful; }
      std::unique_ptr<WsfScenario> GetScenario()
      {
         return std::move(mScenarioPtr);
      } // Using mScenarioPtr after calling this is undefined behavior
      const QString& GetError() const { return mErrorStr; }

   private:
      void run() override;

      WsfStandardApplication&                mApplication;
      const WsfStandardApplication::Options& mOptions;
      const std::string                      mAdditionalInput;

      std::unique_ptr<WsfScenario> mScenarioPtr{nullptr};
      bool                         mSuccessful{false};
      QString                      mErrorStr{};
   };

   // This class executes the simulation event loop on a secondary thread, with the addition of some hooks for Warlock.
   class SimThread : public QThread
   {
   public:
      SimThread(QObject*                               aParent,
                WsfStandardApplication&                aApplication,
                std::unique_ptr<WsfScenario>           aScenarioPtr,
                const WsfStandardApplication::Options& aOptions);

      ~SimThread() override = default;

      void SetCurrentScenario(std::unique_ptr<WsfScenario> aScenarioPtr);
      void SetSimulationExternallyStarted(bool aExternallyStarted) { mExternallyStarted = aExternallyStarted; }
      void TerminateSimulation();

   private:
      void run() override;

      WsfStandardApplication&         mApplication;
      std::unique_ptr<WsfSimulation>  mSimulationPtr{nullptr};
      std::unique_ptr<WsfScenario>    mPendingScenarioPtr;
      WsfStandardApplication::Options mOptions;
      bool                            mExternallyStarted{false};
   };

   int                mMaxRecentScenarios{12};
   QList<QStringList> mRecentScenarios;

   static RunManager* mInstancePtr;

   bool                            mPendingLoading{false};
   WsfStandardApplication&         mApp;
   LoadThread*                     mLoadThread{nullptr};
   SimThread*                      mSimThread{nullptr};
   WsfStandardApplication::Options mOptions;

   QProgressDialog* mProgressDialog{nullptr};
};
} // namespace warlock
#endif
