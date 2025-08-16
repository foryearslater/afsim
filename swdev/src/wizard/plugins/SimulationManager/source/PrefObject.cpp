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


#include "PrefObject.hpp"

#include <QDateTime>
#include <QFileInfo>

#include "Signals.hpp"
#include "UtProcess.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfExe.hpp"

namespace
{
template<typename T>
QString FindAvailableAliasHelper(const QString& aSourceText, const T& aUnavailableAliases)
{
   if (aUnavailableAliases.find(aSourceText) == aUnavailableAliases.end())
   {
      return aSourceText;
   }
   int     check = 1;
   QString test  = aSourceText + "_" + QString::number(check);
   while (aUnavailableAliases.find(test) != aUnavailableAliases.end())
   {
      check++;
      test = aSourceText + "_" + QString::number(check);
   }
   return test;
}

SimulationManager::SimData ParseIdeOutput(std::istream& ideData)
{
   QString                            version;
   QString                            name;
   std::map<std::string, std::string> featuresAvailable;
   // if it is an afsim exe
   constexpr std::size_t cMAX_IDE_OUTPUT_LENGTH = 4000;
   char                  line[cMAX_IDE_OUTPUT_LENGTH];
   std::string           currentBlock;
   std::string           currentName;
   while (ideData.getline(line, cMAX_IDE_OUTPUT_LENGTH))
   {
      if (line[0] == '^' && line[1] == '^' && line[2] == '^')
      {
         char* endP = line + 3;
         while (*endP && *endP != '\n')
         {
            ++endP;
         }
         currentName.assign(line + 3, endP);
         continue;
      }
      else if (line[0] == '$' && line[1] == '$' && line[2] == '$')
      {
         if (currentName == "version")
         {
            version = currentBlock.c_str();
            version.remove('\n');
         }
         else if (currentName == "application")
         {
            name = currentBlock.c_str();
            name.remove('\n');
         }
         else if (currentName == "features")
         {
         }
         else if (currentName == "grammar")
         {
         }
         currentName.clear();
         currentBlock.clear();
      }
      else if (!currentName.empty())
      {
         if (currentName == "features")
         {
            const std::string lineStr(line);
            const size_t      splitPoint = lineStr.find_first_of(' ');
            if (splitPoint == std::string::npos)
            {
               featuresAvailable[lineStr] = "";
            }
            else
            {
               featuresAvailable[lineStr.substr(0, splitPoint)] = lineStr.substr(splitPoint + 1);
            }
         }
         else
         {
            currentBlock += line;
            currentBlock += '\n';
         }
      }
   }
   // get the aliases currently reflected in the UI, NOT from mCurrentPrefs.mSimList - other recent additions
   // count for the purposes of what's available.
   const bool profilingAvailable = (featuresAvailable.count("profiling") > 0);
   return SimulationManager::SimData(QString(), name, version, QString(), QString(), QString(), QString(), true, false, profilingAvailable);
}

} // namespace

ut::optional<SimulationManager::SimData> SimulationManager::QueryExecutableIdeOutput(const QString& aFile)
{
   QFileInfo fileInfo{aFile};
   bool      isWizExe = fileInfo.baseName() == "wizard";
   if (!aFile.isEmpty() && !isWizExe)
   {
      UtPath      exePath(aFile.toStdString());
      std::string tempFileName = wizard::Util::MakeTempFileName();
      auto        processPtr   = UtProcess::Execute(exePath.GetSystemPath(), "--ide-output " + tempFileName);
      if (processPtr)
      {
         processPtr->Wait();
         std::ifstream exeDataFile(tempFileName.c_str());
         if (!exeDataFile)
         {
            return ut::optional<SimulationManager::SimData>();
         }
         SimData simData  = ParseIdeOutput(exeDataFile);
         simData.mExeName = fileInfo.fileName();
         simData.mDate    = fileInfo.lastModified().toString();
         simData.mPath    = fileInfo.filePath();
         return simData;
      }
   }
   return ut::optional<SimData>();
}

SimulationManager::PrefData::PrefData()
{
#ifdef _WIN32
   QString mpath  = "mission.exe";
   QString epath  = "engage.exe";
   QString spath  = "sensor_plot.exe";
   QString wpath  = "warlock.exe";
   QString wtpath = "weapon_tools.exe";
#else
   QString mpath  = "mission";
   QString epath  = "engage";
   QString spath  = "sensor_plot";
   QString wpath  = "warlock";
   QString wtpath = "weapon_tools";
#endif
   // All builtin executables are forced as if profiling wasn't available. This helps ensure a "pristine" execution option.
   mSimList.emplace(std::piecewise_construct,
                    std::forward_as_tuple("Mission"),
                    std::forward_as_tuple(mpath, "Mission", "", "", "", "", "", true, true, true));
   mSimList.emplace(std::piecewise_construct,
                    std::forward_as_tuple("Engage"),
                    std::forward_as_tuple(epath, "Engage", "", "", "", "", "", true, true, false));
   mSimList.emplace(std::piecewise_construct,
                    std::forward_as_tuple("SensorPlot"),
                    std::forward_as_tuple(spath, "SensorPlot", "", "", "", "", "", true, true, false));
   mSimList.emplace(std::piecewise_construct,
                    std::forward_as_tuple("Warlock"),
                    std::forward_as_tuple(wpath, "Warlock", "", "", "", "", "", true, true, false));
   mSimList.emplace(std::piecewise_construct,
                    std::forward_as_tuple("WeaponTools"),
                    std::forward_as_tuple(wtpath, "WeaponTools", "", "", "", "", "", true, true, false));
}


const SimulationManager::SimData& SimulationManager::PrefData::GetActiveSim() const
{
   auto foundSim = mSimList.find(mActiveSim);
   if (foundSim == mSimList.end())
   {
      foundSim = mSimList.find("Mission");
      if (foundSim == mSimList.end())
      {
         foundSim = mSimList.begin();
      }
   }
   if (foundSim != mSimList.end())
   {
      return foundSim->second;
   }
   static const SimData kEmptySimData("", "", "", "", "", "", "", false, false, false);
   return kEmptySimData;
}

const QString& SimulationManager::PrefData::GetActiveSimName() const
{
   return mActiveSim;
}

QString SimulationManager::PrefData::GetActiveSimPath() const
{
   const auto& sim = GetActiveSim();
   if (sim.mBuiltIn)
   {
      UtPath p = UtPath::GetExePath();
      p.Up();
      return QString(p.GetSystemPath().c_str()) + '/' + sim.mPath;
   }
   else
   {
      return sim.mPath;
   }
}

bool SimulationManager::PrefData::GetActiveSimSupportsProfiling() const
{
   ut::optional<bool> supportsProfiling = GetActiveSim().mSupportsProfiling;
   return supportsProfiling.has_value() && *supportsProfiling;
}

const QString& SimulationManager::PrefData::GetActiveSimProfilingOutput() const
{
   return GetActiveSim().mProfilingOutputPath;
}

const QString& SimulationManager::PrefData::GetActiveSimExtraArguments() const
{
   return GetActiveSim().mExtraCommandLineArgs;
}

QString SimulationManager::PrefData::ResolveActiveSimCompleteArguments() const
{
   const auto& sim = GetActiveSim();
   const bool  profilingActive =
      sim.mSupportsProfiling.has_value() && *sim.mSupportsProfiling && !sim.mProfilingOutputPath.isEmpty();
   return (profilingActive ? "-profiling-output \"" + sim.mProfilingOutputPath + "\" " : "") + sim.mExtraCommandLineArgs;
}

SimulationManager::PrefObject::PrefObject(QObject* parent /*= nullptr*/)
   : wkf::PrefObjectT<PrefData>(parent, cNAME)
{
   mConnection = connect(wizSignals, &wizard::Signals::WsfExeChanged, this, &PrefObject::ExeChanged);
}

void SimulationManager::PrefObject::Apply()
{
   if (mActiveSimChanged)
   {
      mActiveSimChanged = false;
      emit ActiveSimChanged(mCurrentPrefs.GetActiveSimName(),
                            mCurrentPrefs.GetActiveSimPath(),
                            mCurrentPrefs.ResolveActiveSimCompleteArguments());
   }
}

SimulationManager::PrefData SimulationManager::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mActiveSim = aSettings.value("active", mDefaultPrefs.GetActiveSimName()).toString();
   if (!aSettings.contains("sims/size"))
   {
      pData = mDefaultPrefs;
   }
   else
   {
      int simCount = aSettings.beginReadArray("sims");
      for (int i = 0; i < simCount; ++i)
      {
         aSettings.setArrayIndex(i);
         const QString      alias                = aSettings.value("alias").toString();
         const QString      path                 = aSettings.value("path").toString();
         const QString      name                 = aSettings.value("name").toString();
         const QString      version              = aSettings.value("version").toString();
         const QString      filename             = aSettings.value("filename").toString();
         const QString      date                 = aSettings.value("date").toString();
         const bool         builtIn              = aSettings.value("builtIn").toBool();
         const QString      profilingOutputPath  = aSettings.value("profilingOutputPath").toString();
         const QString      commandLineParam     = aSettings.value("commandLineParams").toString();
         QVariant           supportsProfilingVar = aSettings.value("supportsProfiling");
         ut::optional<bool> supportsProfiling;

         const bool needToUpdateSettings = !supportsProfilingVar.isValid();
         if (needToUpdateSettings)
         {
            const ut::optional<SimData> queriedSettings = QueryExecutableIdeOutput(path);
            if (queriedSettings.has_value())
            {
               supportsProfiling = queriedSettings->mSupportsProfiling;
            }
            // else we couldn't query the settings to update them.  Nothing we can do.
         }
         else
         {
            supportsProfiling = (supportsProfilingVar.isValid() ? supportsProfilingVar.toBool() : ut::optional<bool>());
         }

         const std::string spath = path.toStdString();
         const QFileInfo   qpath(path);
         if (builtIn)
         {
            UtPath op = UtPath::GetExePath();
            op.Up();
            op += path.toStdString();

            QFileInfo bip(op.GetSystemPath().c_str());
            pData.mSimList[alias] =
               SimData(path, name, version, filename, date, profilingOutputPath, commandLineParam, bip.exists(), builtIn, supportsProfiling);
         }
         else if (qpath.exists())
         {
            pData.mSimList[alias] =
               SimData(path, name, version, filename, date, profilingOutputPath, commandLineParam, true, builtIn, supportsProfiling);
         }
         else
         {
            QString message = "The simulation " + alias + " cannot be found at " + path +
                              " would you like to remove it from your simulation list?";
            if (QMessageBox::No == QMessageBox::question(wkfEnv.GetMainWindow(), "Missing Simulation", message))
            {
               pData.mSimList[alias] =
                  SimData(path, name, version, filename, date, profilingOutputPath, commandLineParam, false, builtIn, supportsProfiling);
            }
         }
      }
      aSettings.endArray();
   }
   return pData;
}

void SimulationManager::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("active", mCurrentPrefs.mActiveSim);
   aSettings.beginWriteArray("sims", (int)(mCurrentPrefs.mSimList.size()));
   unsigned int i = 0;
   for (auto&& it : mCurrentPrefs.mSimList)
   {
      aSettings.setArrayIndex(i++);
      aSettings.setValue("alias", it.first);
      aSettings.setValue("path", it.second.mPath);
      aSettings.setValue("name", it.second.mName);
      aSettings.setValue("version", it.second.mVersion);
      aSettings.setValue("filename", it.second.mExeName);
      aSettings.setValue("date", it.second.mDate);
      aSettings.setValue("builtIn", it.second.mBuiltIn);
      aSettings.setValue("profilingOutputPath", it.second.mProfilingOutputPath);
      aSettings.setValue("commandLineParams", it.second.mExtraCommandLineArgs);
      if (it.second.mSupportsProfiling.has_value())
      {
         aSettings.setValue("supportsProfiling", *it.second.mSupportsProfiling);
      }
   }
   aSettings.endArray();
}

QString SimulationManager::PrefObject::FindAvailableAlias(const QString& aSourceText) const
{
   return FindAvailableAliasHelper(aSourceText, mCurrentPrefs.mSimList);
}

QString SimulationManager::PrefObject::FindAvailableAlias(const QString&           aSourceText,
                                                          const std::set<QString>& aUnavailableAliases)
{
   return FindAvailableAliasHelper(aSourceText, aUnavailableAliases);
}

QString SimulationManager::PrefObject::FindMatchingSim(const wizard::WsfExe& aExe) const
{
   for (auto&& it : mCurrentPrefs.mSimList)
   {
      if (it.second.mPath == aExe.GetPath().GetSystemPath().c_str()) // we don't check versions, because there is
                                                                     // nothing we could do even if they didn't match!
      {
         return it.first;
      }
   }
   return "";
}

void SimulationManager::PrefObject::ExeChanged(wizard::WsfExe* aExePtr)
{
   QString alias = "";
   if (!aExePtr->GetPath().GetSystemPath().empty())
   {
      QString     newpath = aExePtr->GetPath().GetSystemPath().c_str();
      std::string asim    = mCurrentPrefs.mActiveSim.toStdString();
      std::string reqsim  = aExePtr->GetDescriptiveName();
      SimData     simData = GetSimData(QString::fromStdString(asim));
      if (simData.mPath != nullptr)
      {
         alias = simData.mName;
      }
      else if (asim != reqsim)
      {
         // NOTE on QByteArray:
         // QByteArray is implicitly convertible to QString, so this is functionally equivalent to
         // QString::fromStdString. However, using QByteArray as an intermediate masks a string
         // termination vulnerability identified by static code analysis, with sink in QString's
         // use of 'strlen'. The source is std::string input from a file stream in WsfExe, which
         // should be null-terminated, so this is apparently a false positive, but this workaround
         // might be unnecessary in a future version of Qt.
         alias = QByteArray::fromStdString(reqsim);
         alias = FindMatchingSim(*aExePtr);
         if (alias.isEmpty())
         {
            QString newname    = QByteArray::fromStdString(aExePtr->GetAlias());
            QString newversion = QByteArray::fromStdString(aExePtr->GetVersion());
            QString newexename = QByteArray::fromStdString(aExePtr->GetApplication());

            QFileInfo* file    = new QFileInfo(newexename);
            QString    newdate = file->lastModified().toString();

            alias                         = FindAvailableAlias(newname);
            const auto& activeSim         = mCurrentPrefs.GetActiveSim();
            mCurrentPrefs.mSimList[alias] = SimData(newpath,
                                                    newname,
                                                    newversion,
                                                    newexename,
                                                    newdate,
                                                    activeSim.mProfilingOutputPath,
                                                    activeSim.mExtraCommandLineArgs,
                                                    true,
                                                    false,
                                                    activeSim.mSupportsProfiling);
         }
      }
      else
      {
         alias = QByteArray::fromStdString(reqsim);
      }
   }
   mCurrentPrefs.mActiveSim = alias;

   disconnect(mConnection);
   emit ActiveSimChanged(mCurrentPrefs.mActiveSim,
                         aExePtr->GetPath().GetSystemPath().c_str(),
                         mCurrentPrefs.ResolveActiveSimCompleteArguments());
   mConnection = connect(wizSignals, &wizard::Signals::WsfExeChanged, this, &PrefObject::ExeChanged);
}

void SimulationManager::PrefObject::SetPreferenceDataP(const PrefData& aPrefData)
{
   // If the currently selected simulation executive changed
   if (aPrefData.mActiveSim != mCurrentPrefs.mActiveSim)
   {
      mActiveSimChanged = true;
   }
   else
   {
      const auto newData = aPrefData.mSimList.find(aPrefData.mActiveSim);
      const auto oldData = mCurrentPrefs.mSimList.find(mCurrentPrefs.mActiveSim);
      // If one of the elements could not be located, then obviously a change occurred.
      if (newData == aPrefData.mSimList.end() || oldData == mCurrentPrefs.mSimList.end())
      {
         mActiveSimChanged = true;
      }
      // If the data for the currently select executable changed
      else if (!(newData->second == oldData->second))
      {
         mActiveSimChanged = true;
      }
   }
   BasePrefObject::SetPreferenceDataP(aPrefData);
}

SimulationManager::SimData SimulationManager::PrefObject::GetSimData(const QString& name)
{
   for (auto&& jt : mCurrentPrefs.mSimList)
   {
      if (jt.second.mName == name)
      {
         return jt.second;
      }
   }
   return SimData();
}
