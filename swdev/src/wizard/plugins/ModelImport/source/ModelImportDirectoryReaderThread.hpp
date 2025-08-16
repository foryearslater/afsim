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
#ifndef MODELIMPORT_PARSEFILE_HPP
#define MODELIMPORT_PARSEFILE_HPP

#include <vector>

#include <QString>
#include <QThread>
#include <QTimer>

#include "ModelImportUtility.hpp"

class QProgressDialog;

namespace ModelImport
{
//! A worker thread for parsing a directory and files.
//! Emits a FinishedFile signal approximately every 1% of the files finished.
class DirectoryReaderThread : public QThread
{
   Q_OBJECT

public:
   //! Sets / Resets all the arguments required for parsing a directory.
   //! @param aDirectoryRootPath This is the root directory to begin parsing at.
   //! @param aDataBase This is where the data should be stored.
   //! @param aMaxFiles This is the total number of files in the directory.
   //! @param aMessagesFile This is the file to write all messages (errors and warnings) to.
   //!        If an empty string is passed, uses std::cerr.
   //! @note THIS MUST BE CALLED BEFORE CALLING start()!
   void SetFields(const QString& aDirectoryRootPath, ParseData& aDataBase, int aMaxFiles, const std::string& aMessagesFile);

   //! Called by start().
   //! Begins the process of parsing the directory.
   virtual void run() override;

   //! Returns the number of warnings generated.
   int Warnings() const noexcept;

   //! Returns the number of errors generated.
   int Errors() const noexcept;

signals:
   void FinishedFile(int aCount);

private:
   QString     mRootPath;
   ParseData*  mDataBase;
   int         mUpdatePeriod;
   std::string mMessagesFile;
   int         mWarningCount;
   int         mErrorCount;
   int         mFileCount;

   //! Parses a file.
   //! @param aFileRelativePath This is the path relative to the root path (see SetFields) where the file is located.
   //! @param aErrorStream This is the stream to write messages (errors and warings) to.
   //! @returns The meta-data for the file parsed.
   ModelImport::FileData ParseFile(const QString& aFileRelativePath, std::ostream& aErrorStream);

   //! Recursively parses a directory.
   //! @param aDirectoryRelativePath This is the path relative to the root path (see SetFields) where the directory is located.
   //! @param aErrorStream This is the stream to write messages (errors and warings) to.
   void ParseDirectory(const QString& aDirectoryRelativePath, std::ostream& aErrorStream);
};

//! Returns the total number of AFSIM files are in a directory.
//! AFSIM files must meet the following criteria to be counted:
//! 1) Must be a *.txt file.
//! 2) Name cannot contain the string README, case insensitive.
//! 3) No part of its path can begin with a period.
//! @param aPath This is the path of the directory to scan.
int TotalDirectorySize(const QString& aPath);
} // namespace ModelImport

#endif
