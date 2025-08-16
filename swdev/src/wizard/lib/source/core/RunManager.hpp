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

#ifndef RUNMANAGER_HPP
#define RUNMANAGER_HPP

#include <QThread>

class UtPath;

#include "ViExport.hpp"

//! Convenience macro for accessing the wizard::RunManager instance
#define wizRunMgr wizard::RunManager::GetInstance()

namespace wizard
{
class VI_EXPORT RunManager
{
public:
   static RunManager& GetInstance();

   static void Create();

   const QList<QStringList>& GetRecentScenarios() const { return mRecentScenarios; }
   void                      RemoveRecentScenario(const QStringList& aFileList);
   void                      ClearRecentScenarios();
   QString                   GetMostRecentDirectory();

   bool ReloadScenario();
   bool LoadScenario(const QStringList& aFileList);

   void Shutdown();

   //! @note For window title and recent menu actions/links
   static QString GetScenarioDisplayName(const QStringList& aFileList, bool aDisplayFullPath);

   const QList<QStringList>& GetRecents() { return mRecentScenarios; }

   //! @name Application path routines
   //@{

   //! Get the application path
   //! @return the application path
   QString GetApplicationPath() const noexcept;
   //! Sets the working directory to where the application is
   //! @pre mApplicationPath contains the application path for Wizard
   //! @return whether or not the write-protection was removed
   bool SetWorkingDirectoryToApplication() noexcept;
   //! Determine whether or not the application path is set as the working directory
   //! @return whether or not the application path is set as the working directory
   bool IsWorkingDirectoryApplication() const noexcept;

   //@}

   //! @name Current project path routines
   //@{

   //! Get the current project's path
   QString GetCurrentProjectPath() const noexcept;
   //! Sets the working directory to where the "soon-to-be-current" project is
   //! @param aMainScenarioFile is the "soon-to-be-current" project (or the start-up file for the project)
   //! @post the application path for Wizard is stored in mApplicationPath
   //! @return whether or not the write-protection was set
   //! @note another side-effect of QDir::setCurrent on Windows is that the working directory is write-protected (just
   //! the directory, not the files in the directory)
   //! @see warlock::RunManager
   bool SetWorkingDirectoryToProject(const QString& aMainScenarioFile) noexcept;
   //! Overload of SetCurrentProjectDirectory for std::string
   bool SetWorkingDirectoryToProject(const std::string& aMainScenarioFile) noexcept;
   //! Overload of SetCurrentProjectDirectory for UtPath
   bool SetWorkingDirectoryToProject(const UtPath& aMainScenarioFile) noexcept;
   //! Determine whether or not the current project's path is set as the working directory
   //! @return whether or not the current project's path is set as the working directory
   bool IsWorkingDirectoryProject() const noexcept;

   //@}

protected:
   RunManager();
   virtual ~RunManager();

   void ReadSettings();
   void WriteHistory();
   void SetMostRecentScenario(const QStringList& aFileList);

   static QStringList StandardizeFilePaths(const QStringList& aFileList, bool* aFilesExist = nullptr);

   int                mMaxRecentScenarios;
   QList<QStringList> mRecentScenarios;
   QStringList        mFileList;
   //! the working directory for the current project
   QString mCurrentProjectPath;
   //! the working directory for the application
   const QString mApplicationPath;

   static RunManager* mInstancePtr;
};
} // namespace wizard

#endif
