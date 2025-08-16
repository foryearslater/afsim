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

#include "RevisionChange.hpp"

#include <QDataStream>

wizard::RevisionChange::RevisionChange()
   : kind(cNO_CHANGE)
   , filePath()
   , changeBytes()
{
}

wizard::RevisionChange::RevisionChange(RevisionChange::Type aKind, const std::string& aFilePath, const QByteArray& aChangeBytes)
   : kind(aKind)
   , filePath(aFilePath)
   , changeBytes(aChangeBytes)
{
}

wizard::RevisionChange::RevisionChange(RevisionChange::Type             aKind,
                                       const std::string&               aFilePath,
                                       const QVector<TextSourceChange>& aChanges)
   : kind(aKind)
   , filePath(aFilePath)
{
   QDataStream stream(&changeBytes, QIODevice::WriteOnly);
   stream << aChanges;
}

wizard::RevisionChange::RevisionChange(RevisionChange::Type aKind, const std::string& aFilePath, const char* aDataPtr, int aDataSize)
   : kind(aKind)
   , filePath(aFilePath)
   , changeBytes(aDataPtr, aDataSize)
{
}

wizard::RevisionChange::RevisionChange(RevisionChange::Type aKind, const std::string& aFilePath, const UtTextDocument& aDoc)
   : kind(aKind)
   , filePath(aFilePath)
   , changeBytes(aDoc.GetPointer())
{
}

// For change types that have no associated data; e.g., cREMOVED_FILE.
wizard::RevisionChange::RevisionChange(RevisionChange::Type aKind, const std::string& aFilePath)
   : kind(aKind)
   , filePath(aFilePath)
   , changeBytes()
{
}

bool wizard::RevisionChange::operator==(const RevisionChange& aRhs)
{
   return (kind == aRhs.kind) && (filePath == aRhs.filePath) && (changeBytes == aRhs.changeBytes);
}
