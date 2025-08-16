// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef EXPORTFILES_HPP
#define EXPORTFILES_HPP

#include <QMessageBox>
#include <QString>

#include "ParseResults.hpp"
#include "RevisionDb.hpp"
#include "ViExport.hpp"

class QWidget;
class UtPath;

namespace wizard
{
class RevisionStore;

//! This class is used for exporting project/scenario files to a user-chosen
//! directory. The methods will attempt to preserve the relative directory
//! structure of the project and any external items it may include. However, it
//! cannot perfectly handle the case where external items are found on different
//! drives on Windows; e.g. the project is on C:\ and an included external item
//! is on D:\. In that case, the external item will be added to a folder at the
//! root of the export folder called "_external_items" and scenario files that
//! included such external items will need to be updated to look for them there
//! instead. Linux does not have this problem.
class VI_EXPORT ExportFiles
{
public:
   ExportFiles(const UtPath& aProjectDir, QWidget* aRequestingWidget = nullptr);

   bool    ExportProject(const ParseIncludes& aIncludes);
   bool    ExportRevision(RevisionStore& aRevStore, int aRevNo);
   QString GetExportPath(const QString& aExportDirPath, const QString& aFilePath) const;

private:
   QString                     PromptForExportDir() const;
   QMessageBox::StandardButton PromptForOverwrite(const QString& aFilePath) const;
   QString                     AdjustExportDirPath(const QString& aExportDirPath, const FileMap& aFileMap);
   bool                        Export(const QString& aExportDirPath, const FileMap& aFileMap);
   void                        OpenDir(const QString& aExportDirPath) const;

   const UtPath mProjectDir;       //! The root directory of the project.
   QWidget*     mRequestingWidget; //! The widget requesting the export.
};
} // namespace wizard

#endif // VIEXPORTFILES_HPP
