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

#ifndef REVISIONSQLSTMT_HPP
#define REVISIONSQLSTMT_HPP

#include <string>

#include <QByteArray>

#include "ViExport.hpp"
struct sqlite3;
struct sqlite3_stmt;

namespace wizard
{
// Wrapper around an sqlite statement
class VI_EXPORT RevisionSqlStmt
{
public:
   RevisionSqlStmt();
   RevisionSqlStmt(sqlite3* aDb, const std::string& aSql);
   virtual ~RevisionSqlStmt();

   bool Prepare(sqlite3* aDb, const std::string& aSql);
   void Finalize();

   bool Bind(int idx, int aInt);
   bool Bind(int idx, const std::string& aString);
   bool Bind(int idx, const char* aString);
   bool Bind(int idx, double aValue);
   bool Bind(int idx, const QByteArray& aBlob);
   bool BindBlobStatic(int idx, const char* aBlob, int aBlobSize);
   bool BindZipped(int idx, const QByteArray& aBlob);
   bool Bind64(int idx, long long aInt64);
   bool BindNull(int idx);

   bool Reset();
   bool Step();
   bool StepRow();
   bool Execute();
   bool ExecuteStepRow();

   std::string GetParameterName(int idx);
   int         GetParameterCount();

   const char* ColCString(int col);
   std::string ColText(int col);
   int         ColInt(int col);
   long long   ColInt64(int col);
   QByteArray  ColBlob(int col);
   QByteArray  ColBlobUnzipped(int col);

   static int Exec(sqlite3* aSqlDb, const char* aSql);
   static int ExecOk(sqlite3* aSqlDb, const char* aSql);

private:
   bool CheckSuccess(bool success);
   bool CheckOk(int resultCode);
   bool CheckStep(int resultCode);

   sqlite3*      mDb;
   sqlite3_stmt* mSqlStmt;
};
} // namespace wizard

#endif // REVISIONSQLSTMT_HPP
