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

#ifndef REVISIONCHANGE_HPP
#define REVISIONCHANGE_HPP

#include <string>

#include <QByteArray>
#include <QVector>

#include "TextSourceChange.hpp"
#include "ViExport.hpp"

namespace wizard
{
//! A struct that represents the changes made to a file in a revision in the
//! Change History database.
struct VI_EXPORT RevisionChange
{
   // These integer values are stored in the database and used in SQL queries.
   // Since they are stored in the database, they should not be changed.
   enum Type
   {
      cNO_CHANGE    = 0,
      cNEW_FILE     = 1,
      cREMOVED_FILE = 2,
      cCHANGE_DELTA = 3,
      cCHANGE_FULL  = 4
   };

   RevisionChange();
   RevisionChange(Type aKind, const std::string& aFilePath, const QByteArray& aChangeBytes);
   RevisionChange(Type aKind, const std::string& aFilePath, const QVector<TextSourceChange>& aChanges);
   RevisionChange(Type aKind, const std::string& aFilePath, const char* aDataPtr, int aDataSize = -1);
   RevisionChange(Type aKind, const std::string& aFilePath, const UtTextDocument& aDoc);
   RevisionChange(Type aKind, const std::string& aFilePath);

   bool operator==(const RevisionChange& aRhs);

   Type        kind;
   std::string filePath;
   QByteArray  changeBytes;
};

using RevisionChangeList = std::vector<RevisionChange>;
} // namespace wizard
#endif // REVISIONCHANGE_HPP
