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

#ifndef PREFOBJECT_HPP
#define PREFOBJECT_HPP

#include <set>

#include "UtOptional.hpp"
#include "WkfPrefObject.hpp"

namespace wizard
{
class WsfExe;
}

namespace SimulationManager
{
struct SimData
{
   SimData(const QString&     aPath,
           const QString&     aName,
           const QString&     aVersion,
           const QString&     aExeName,
           const QString&     aDate,
           const QString&     aProfilingOutputPath,
           const QString&     aExtraCommandLineArgs,
           bool               aGood,
           bool               aBuiltIn,
           ut::optional<bool> aSupportsProfiling)
      : mPath(aPath)
      , mName(aName)
      , mVersion(aVersion)
      , mProfilingOutputPath(aProfilingOutputPath)
      , mExtraCommandLineArgs(aExtraCommandLineArgs)
      , mGood(aGood)
      , mBuiltIn(aBuiltIn)
      , mSupportsProfiling(aSupportsProfiling)
   {
   }

   SimData() = default;

   bool operator==(const SimData& rhs) const
   {
      return (mPath == rhs.mPath) && (mName == rhs.mName) && (mVersion == rhs.mVersion) && (mExeName == rhs.mExeName) &&
             (mDate == rhs.mDate) && (mGood == rhs.mGood) && (mBuiltIn == rhs.mBuiltIn) &&
             (mProfilingOutputPath == rhs.mProfilingOutputPath) &&
             (mExtraCommandLineArgs == rhs.mExtraCommandLineArgs) && (mSupportsProfiling == rhs.mSupportsProfiling);
   }

   QString            mPath;
   QString            mName;
   QString            mVersion;
   QString            mExeName;
   QString            mDate;
   QString            mProfilingOutputPath;
   QString            mExtraCommandLineArgs;
   bool               mGood{false};
   bool               mBuiltIn{false};
   ut::optional<bool> mSupportsProfiling;
};

/// @brief Runs the executable specified by path in `exeFile` with the --ide-output argument, on success parses the
/// resulting output into a SimData structure.  On failure returns nullopt.
ut::optional<SimData> QueryExecutableIdeOutput(const QString& exeFile);

struct PrefData
{
   PrefData();

   const SimData& GetActiveSim() const;
   const QString& GetActiveSimName() const;
   QString        GetActiveSimPath() const;
   const QString& GetActiveSimProfilingOutput() const;
   bool           GetActiveSimSupportsProfiling() const;
   const QString& GetActiveSimExtraArguments() const;
   QString        ResolveActiveSimCompleteArguments() const;

   QString                    mActiveSim{"Mission"}; // default active to mission
   std::map<QString, SimData> mSimList;
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "SimulationManager";

   PrefObject(QObject* parent = nullptr);
   ~PrefObject() override = default;

   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   QString        FindAvailableAlias(const QString& aSourceText) const;
   static QString FindAvailableAlias(const QString& aSourceText, const std::set<QString>& aUnavailableAliases);
   QString        FindMatchingSim(const wizard::WsfExe& aExe) const;

signals:
   void ActiveSimChanged(const QString& aActiveSim, const QString& aPath, const QString& aCommandLineParams);

private:
   SimData GetSimData(const QString& name);
   void    ExeChanged(wizard::WsfExe* aExePtr);
   void    SetPreferenceDataP(const PrefData& aPrefData) override;

   bool                    mActiveSimChanged{false};
   QMetaObject::Connection mConnection;
};
} // namespace SimulationManager

Q_DECLARE_METATYPE(SimulationManager::PrefData)

#endif
