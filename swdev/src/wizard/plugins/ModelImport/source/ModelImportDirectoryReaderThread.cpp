// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ModelImportDirectoryReaderThread.hpp"

#include <iostream>
#include <map>
#include <set>

#include <QDirIterator>
#include <QMessageBox>
#include <QProgressDialog>
#include <QString>

#include "ParserLite.hpp"
#include "UtException.hpp"

// Defined as bit locations to simplify implementation.
enum ParseLevel
{
   cIGNORE  = 1,
   cTITLE   = 2,
   cINHERIT = 4,
   cLABELS  = 8,

   cALL = cTITLE | cINHERIT | cLABELS
};

// ================================================================================================
// This list can be expanded
static const std::map<std::string, ParseLevel> cBLOCKS = {{"access_report", ParseLevel::cIGNORE},
                                                          {"acoustic_signature", ParseLevel::cTITLE},
                                                          {"aero", ParseLevel::cTITLE},
                                                          {"air_traffic", ParseLevel::cIGNORE},
                                                          {"antenna_pattern", ParseLevel::cTITLE},
                                                          {"atmosphere_calibration", ParseLevel::cIGNORE},
                                                          {"attenuation_model", ParseLevel::cTITLE},
                                                          {"behavior", ParseLevel::cTITLE},
                                                          {"callback", ParseLevel::cTITLE},
                                                          {"clutter_model", ParseLevel::cTITLE},
                                                          {"comm", ParseLevel::cTITLE},
                                                          {"csv_event_output", ParseLevel::cIGNORE},
                                                          {"cyber_attack", ParseLevel::cTITLE},
                                                          {"cyber_effect", ParseLevel::cTITLE},
                                                          {"cyber_protect", ParseLevel::cTITLE},
                                                          {"cyber_trigger", ParseLevel::cTITLE},
                                                          {"dis_interface", ParseLevel::cIGNORE},
                                                          {"draw", ParseLevel::cIGNORE},
                                                          {"electronic_warfare", ParseLevel::cTITLE},
                                                          {"electronic_warfare_effect", ParseLevel::cTITLE},
                                                          {"electronic_warfare_technique", ParseLevel::cTITLE},
                                                          {"error_model", ParseLevel::cTITLE},
                                                          {"event_pipe", ParseLevel::cIGNORE},
                                                          {"execute", ParseLevel::cIGNORE},
                                                          {"false_target", ParseLevel::cTITLE},
                                                          {"false_target_screener", ParseLevel::cTITLE},
                                                          {"filter", ParseLevel::cTITLE},
                                                          {"fuel", ParseLevel::cTITLE},
                                                          {"global_environment", ParseLevel::cIGNORE},
                                                          {"group", ParseLevel::cTITLE},
                                                          {"horizontal_map", ParseLevel::cIGNORE},
                                                          {"iff_mapping", ParseLevel::cIGNORE},
                                                          {"infrared_signature", ParseLevel::cTITLE},
                                                          {"inherent_contrast", ParseLevel::cTITLE},
                                                          {"launch_computer", ParseLevel::cTITLE},
                                                          {"message_table", ParseLevel::cIGNORE},
                                                          {"mover", ParseLevel::cIGNORE},
                                                          {"noise_cloud", ParseLevel::cTITLE},
                                                          {"observer", ParseLevel::cIGNORE},
                                                          {"optical_reflectivity", ParseLevel::cTITLE},
                                                          {"optical_signature", ParseLevel::cTITLE},
                                                          {"p6dof_object_types", ParseLevel::cIGNORE},
                                                          {"platform", ParseLevel::cTITLE},
                                                          {"platform_availability", ParseLevel::cIGNORE},
                                                          {"platform_type", ParseLevel::cALL},
                                                          {"processor", ParseLevel::cTITLE},
                                                          {"propagation_model", ParseLevel::cTITLE},
                                                          {"protocol", ParseLevel::cIGNORE},
                                                          {"radar_signature", ParseLevel::cTITLE},
                                                          {"random_seeds", ParseLevel::cIGNORE},
                                                          {"road_traffic", ParseLevel::cIGNORE},
                                                          {"route", ParseLevel::cTITLE},
                                                          {"route_network", ParseLevel::cTITLE},
                                                          {"script", ParseLevel::cIGNORE},
                                                          {"script_variables", ParseLevel::cIGNORE},
                                                          {"sea_traffic", ParseLevel::cIGNORE},
                                                          {"sensor", ParseLevel::cTITLE},
                                                          {"terrain", ParseLevel::cIGNORE},
                                                          {"track", ParseLevel::cIGNORE},
                                                          {"track_manager", ParseLevel::cIGNORE},
                                                          {"visual_part", ParseLevel::cTITLE},
                                                          {"weapon", ParseLevel::cTITLE},
                                                          {"weapon_effects", ParseLevel::cTITLE},
                                                          {"wsf_weapon_server", ParseLevel::cIGNORE},
                                                          {"xio_interface", ParseLevel::cIGNORE},
                                                          {"zone", ParseLevel::cTITLE},
                                                          {"zone_set", ParseLevel::cTITLE}};

// ================================================================================================

void ModelImport::DirectoryReaderThread::SetFields(const QString&     aDirectoryRootPath,
                                                   ParseData&         aDataBase,
                                                   int                aMaxFiles,
                                                   const std::string& aMessagesFile)
{
   mRootPath     = aDirectoryRootPath;
   mDataBase     = &aDataBase;
   mUpdatePeriod = aMaxFiles / 99 + 1;
   mMessagesFile = aMessagesFile;
   mWarningCount = 0;
   mErrorCount   = 0;
   mFileCount    = 0;
}

// ================================================================================================
// virtual
void ModelImport::DirectoryReaderThread::run()
{
   ut::SetupThreadErrorHandling();

   std::ofstream ferr;

   std::ostream& log = [&]() -> std::ostream&
   {
      if (mMessagesFile.empty())
      {
         return std::cerr;
      }
      ferr.open(mMessagesFile);
      return ferr;
   }();

   ParseDirectory("", log);

   // Key = definition
   // Value = file
   std::map<QString, QString> defines;

   for (const ModelImport::FileData& file : *mDataBase)
   {
      for (const ModelImport::ItemData& item : file.mDefines)
      {
         if (defines.count(item.mName))
         {
            log << "WARNING: Redefinition of \"" << item.mName << "\".\n";
            log << "\tFound in file: " << file.PathToFile() << "\n";
            log << "\tFirst defined in: " << defines[item.mName] << "\n";
            mWarningCount++;
         }
         else
         {
            defines[item.mName] = file.PathToFile();
         }
      }
   }
}

// ================================================================================================

int ModelImport::DirectoryReaderThread::Warnings() const noexcept
{
   return mWarningCount;
}

// ================================================================================================

int ModelImport::DirectoryReaderThread::Errors() const noexcept
{
   return mErrorCount;
}

// ================================================================================================

ModelImport::FileData ModelImport::DirectoryReaderThread::ParseFile(const QString& aFileRelativePath,
                                                                    std::ostream&  aErrorStream)
{
   ModelImport::FileData retval;
   retval.SetPathToFile(aFileRelativePath);

   QString completePath = Path(mRootPath, aFileRelativePath);

   ParserLite::FileInput file(completePath.toStdString());
   ParserLite::Input     inputReader(file);

   while (!inputReader.AtEnd())
   {
      std::string nextString = inputReader.GetString();

      if (nextString == "include" || nextString == "include_once")
      {
         inputReader.SeekNext();

         // Paths listed relative to aFileRootPath
         retval.mDependencies << Path(retval.mPath, inputReader.GetString());

         if (!QFile::exists(Path(mRootPath, retval.mDependencies.back())))
         {
            aErrorStream << "ERROR: Dependency \"" << retval.mDependencies.back() << "\" not found.\n";
            aErrorStream << "\tIn file: " << aFileRelativePath << " : " << inputReader.GetRow() << " : "
                         << inputReader.GetColumn() << "\n";
            mErrorCount++;
         }
      }
      else if (nextString == "edit")
      {
         inputReader.SeekNext();
         ParserLite::BlockInput block(inputReader);
      }
      else if (nextString == "file_path")
      {
         aErrorStream << "WARNING: Command \"file_path\" ignored.\n";
         aErrorStream << "\tIn file: " << aFileRelativePath << " : " << inputReader.GetRow() << " : "
                      << inputReader.GetColumn() << "\n";
         mWarningCount++;
         inputReader.SeekNext();
      }
      else if (nextString == "ignore_block")
      {
         inputReader.SeekNext();
      }
      else if (cBLOCKS.count(nextString) > 0) // if (cBLOCKS.contains(nextString))
      {
         ParserLite::BlockInput block(inputReader);
         ParseLevel             blockType = cBLOCKS.at(nextString);

         if (blockType & ParseLevel::cIGNORE)
         {
            continue;
         }

         ModelImport::ItemData nextItem;

         nextItem.mType = QString::fromStdString(nextString);

         if (blockType & ParseLevel::cTITLE)
         {
            nextItem.mName = QString::fromStdString(inputReader.ReadString());
         }

         if (blockType & ParseLevel::cINHERIT)
         {
            nextItem.mInherits = QString::fromStdString(inputReader.ReadString());
         }

         if (blockType & ParseLevel::cLABELS)
         {
            while (inputReader.FindNext("category"))
            {
               inputReader.SeekNext();
               nextItem.mLabels << QString::fromStdString(inputReader.ReadString());
            }
         }

         retval.mDefines.push_back(std::move(nextItem));
      }

      inputReader.SeekNext();
   }

   mFileCount++;
   if (!(mFileCount % mUpdatePeriod))
      FinishedFile(mFileCount);

   return retval;
}

// ================================================================================================

void ModelImport::DirectoryReaderThread::ParseDirectory(const QString& aDirectoryRelativePath, std::ostream& aErrorStream)
{
   QFileInfoList range =
      QDir(Path(mRootPath, aDirectoryRelativePath))
         .entryInfoList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot | QDir::Filter::Readable);
   for (const QFileInfo& info : range)
   {
      QString relativePath = Path(aDirectoryRelativePath, info.fileName());

      if (info.fileName().startsWith(".")) // Ignores stuff like .git
      {
         continue;
      }

      if (info.isDir())
      {
         ParseDirectory(relativePath, aErrorStream);
      }
      else if (info.isFile() && info.fileName().endsWith(".txt") &&
               !info.fileName().contains("README", Qt::CaseSensitivity::CaseInsensitive))
      {
         try
         {
            mDataBase->push_back(ParseFile(relativePath, aErrorStream));
         }
         catch (const ParserLite::Token& errorLocation)
         {
            aErrorStream << "ERROR: Unclosed block \"" << errorLocation.mText << "\".\n";
            aErrorStream << "\tIn file: " << relativePath << " : " << errorLocation.mRow << " : "
                         << errorLocation.mColumn << "\n";
            mErrorCount++;
         }
      }
   }
}

// ================================================================================================

int ModelImport::TotalDirectorySize(const QString& aPath)
{
   int counter = 0;

   QFileInfoList range =
      QDir(aPath).entryInfoList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot | QDir::Filter::Readable);

   for (const QFileInfo& info : range)
   {
      if (info.fileName().startsWith(".")) // Ignores stuff like .git
      {
         continue;
      }

      if (info.isDir())
      {
         counter += ModelImport::TotalDirectorySize(Path(aPath, info.fileName()));
      }
      else if (info.isFile() && info.fileName().endsWith(".txt") &&
               !info.fileName().contains("README", Qt::CaseSensitivity::CaseInsensitive))
      {
         counter++;
      }
   }

   return counter;
}
