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

#include "RevisionSqlStmt.hpp"

#include <cassert>
#include <iostream>

#include <sqlite3.h>

#include "UtCast.hpp"

wizard::RevisionSqlStmt::RevisionSqlStmt()
   : mDb(nullptr)
   , mSqlStmt(nullptr)
{
}

wizard::RevisionSqlStmt::RevisionSqlStmt(sqlite3* aDb, const std::string& aSql)
{
   Prepare(aDb, aSql);
}

wizard::RevisionSqlStmt::~RevisionSqlStmt()
{
   Finalize();
}

bool wizard::RevisionSqlStmt::Prepare(sqlite3* aDb, const std::string& aSql)
{
   mDb    = aDb;
   int rc = sqlite3_prepare_v2(mDb, aSql.c_str(), ut::cast_to_int(aSql.size()), &mSqlStmt, nullptr);
   return CheckOk(rc);
}

void wizard::RevisionSqlStmt::Finalize()
{
   if (mSqlStmt)
   {
      sqlite3_finalize(mSqlStmt);
      mSqlStmt = nullptr;
   }
}

bool wizard::RevisionSqlStmt::Bind(int idx, int aInt)
{
   return CheckOk(sqlite3_bind_int(mSqlStmt, idx, aInt));
}

bool wizard::RevisionSqlStmt::Bind(int idx, const std::string& aString)
{
   return CheckOk(sqlite3_bind_text(mSqlStmt, idx, aString.c_str(), -1, SQLITE_TRANSIENT));
}

bool wizard::RevisionSqlStmt::Bind(int idx, const char* aString)
{
   const std::string str(aString);
   return Bind(idx, str);
}

bool wizard::RevisionSqlStmt::Bind(int idx, double aValue)
{
   return CheckOk(sqlite3_bind_double(mSqlStmt, idx, aValue));
}

bool wizard::RevisionSqlStmt::Bind(int idx, const QByteArray& aBlob)
{
   return CheckOk(sqlite3_bind_blob(mSqlStmt, idx, aBlob.constData(), aBlob.size(), SQLITE_TRANSIENT));
}

bool wizard::RevisionSqlStmt::BindBlobStatic(int idx, const char* aBlob, int aBlobSize)
{
   return CheckOk(sqlite3_bind_blob(mSqlStmt, idx, aBlob, aBlobSize, nullptr));
}

bool wizard::RevisionSqlStmt::BindZipped(int idx, const QByteArray& aBlob)
{
   const QByteArray compressed(qCompress(aBlob));
   return Bind(idx, compressed);
}

bool wizard::RevisionSqlStmt::Bind64(int idx, long long aInt64)
{
   return CheckOk(sqlite3_bind_int64(mSqlStmt, idx, aInt64));
}

bool wizard::RevisionSqlStmt::BindNull(int idx)
{
   return CheckOk(sqlite3_bind_null(mSqlStmt, idx));
}

bool wizard::RevisionSqlStmt::Reset()
{
   return CheckOk(sqlite3_reset(mSqlStmt));
}

bool wizard::RevisionSqlStmt::Step()
{
   return CheckStep(sqlite3_step(mSqlStmt));
}

bool wizard::RevisionSqlStmt::StepRow()
{
   const int resultCode = sqlite3_step(mSqlStmt);
   CheckStep(resultCode);
   return (resultCode == SQLITE_ROW);
}

bool wizard::RevisionSqlStmt::Execute()
{
   return Reset() && Step();
}

bool wizard::RevisionSqlStmt::ExecuteStepRow()
{
   return Reset() && StepRow();
}

std::string wizard::RevisionSqlStmt::GetParameterName(int idx)
{
   return sqlite3_bind_parameter_name(mSqlStmt, idx);
}

int wizard::RevisionSqlStmt::GetParameterCount()
{
   return sqlite3_bind_parameter_count(mSqlStmt);
}

const char* wizard::RevisionSqlStmt::ColCString(int col)
{
   return (const char*)sqlite3_column_text(mSqlStmt, col);
}

std::string wizard::RevisionSqlStmt::ColText(int col)
{
   const char* cStr = ColCString(col);
   return (cStr ? std::string(cStr) : std::string());
}

int wizard::RevisionSqlStmt::ColInt(int col)
{
   return sqlite3_column_int(mSqlStmt, col);
}

long long wizard::RevisionSqlStmt::ColInt64(int col)
{
   return sqlite3_column_int64(mSqlStmt, col);
}

QByteArray wizard::RevisionSqlStmt::ColBlob(int col)
{
   const void* ptr  = sqlite3_column_blob(mSqlStmt, col);
   const int   size = sqlite3_column_bytes(mSqlStmt, col);

   return QByteArray((const char*)ptr, size);
}

QByteArray wizard::RevisionSqlStmt::ColBlobUnzipped(int col)
{
   return qUncompress(ColBlob(col));
}

//! Executes a given SQL query.
//! @param aSqlDb The database to run the query on.
//! @param aSql The query to run.
//! @returns The SQLite result code of running the query.
int wizard::RevisionSqlStmt::Exec(sqlite3* aSqlDb, const char* aSql)
{
   return sqlite3_exec(aSqlDb, aSql, nullptr, nullptr, nullptr);
}

//! Executes a given SQL query and checks that the result code is SQLITE_OK.
//! @param aSqlDb The database to run the query on.
//! @param aSql The query to run.
//! @returns The SQLite result code of running the query.
int wizard::RevisionSqlStmt::ExecOk(sqlite3* aSqlDb, const char* aSql)
{
   const int  resultCode = Exec(aSqlDb, aSql);
   const bool success    = (resultCode == SQLITE_OK);
   assert(success);
   return success;
}

//! Used for handling successes/failures of database queries and automatically
//! asserting and/or printing database error messages. This is most useful for
//! debugging.
//! @param success Whether or not a database query was successful.
//! @returns The same value that was passed in.
bool wizard::RevisionSqlStmt::CheckSuccess(bool success)
{
   if (!success)
   {
      const char* errMsg = sqlite3_errmsg(mDb);
      if (errMsg)
      {
         std::cerr << errMsg << std::endl;
      }
   }
   // assert(success);
   return success;
}

//! Checks that the given SQLite result code is SQLITE_OK.
//! @param resultCode A result code from executing an SQLite function.
//! @returns Whether or not the given result code is SQLITE_OK.
bool wizard::RevisionSqlStmt::CheckOk(int resultCode)
{
   return CheckSuccess(resultCode == SQLITE_OK);
}

//! Checks that the given SQLite result code is a valid result code of sqlite3_step.
//! @param resultCode A result code from running sqlite3_step.
//! @returns Whether or not the given result code is a non-error value.
bool wizard::RevisionSqlStmt::CheckStep(int resultCode)
{
   return CheckSuccess((resultCode == SQLITE_DONE) || (resultCode == SQLITE_ROW));
}
