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
#include "ModelImportUtility.hpp"

std::ostream& ModelImport::operator<<(std::ostream& aOut, const QString& aString)
{
   aOut << aString.toStdString();
   return aOut;
}

// ================================================================================================

void ModelImport::FileData::SetPathToFile(const QString& aPath)
{
   // Formats aPath to be easier to work with.
   QString path = Path(aPath);

   int delimIndex = path.lastIndexOf('/');

   if (delimIndex < 0)
   {
      mFile = path;
      mPath.clear();
   }
   else
   {
      mFile = path.mid(delimIndex + 1);
      mPath = path.left(delimIndex);
   }
}

// ================================================================================================

QString ModelImport::FileData::PathToDir() const
{
   return Path(mPath);
}

// ================================================================================================

QString ModelImport::FileData::PathToFile() const
{
   return Path(mPath, mFile);
}

// ================================================================================================

const QString& ModelImport::detail::MakePath(const QString& aPath)
{
   return aPath;
}

// ================================================================================================

QString ModelImport::detail::MakePath(const std::string& aPath)
{
   return QString::fromStdString(aPath);
}

// ================================================================================================

QString ModelImport::detail::MakePath(const char* aPath)
{
   return aPath;
}
