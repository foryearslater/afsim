// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <string>

#include <QObject>
#include <QtTest/QTest>

#include "UtTextDocument.hpp"
#include "ViRevisionChange.hpp"
#include "ViRevisionDb.hpp"

class TestViRevisionDb : public QObject
{
   Q_OBJECT

private slots:
   // To create a test, subclass QObject and add one or more private slots to
   // it. Each private slot is a test function in your test.
   //
   // In addition, there are four private slots that are not treated as test
   // functions. They will be executed by the testing framework and can be
   // used to initialize and clean up either the entire test or the current
   // test function:
   void initTestCase() {}    // Called before the first test function is executed.
   void init() {}            // Called before each test function is executed.
   void cleanup() {}         // Called after every test function.
   void cleanupTestCase() {} // Called after the last test function is executed.

   void testSerializeUtTextDocument();
   void testSerializeViTextSourceChange();
   void testDbInsertSelectChange();
   void testDbInsertSelectRevision();
   void testGetFileAtRevision();
   void testVersion();
};

// Tests that a UtTextDocument can be serialized and deserialized successfully.
void TestViRevisionDb::testSerializeUtTextDocument()
{
   const std::string testText = "test";
   UtTextDocument    origDoc;
   origDoc.Insert(0, testText);

   const char* dataPtr   = origDoc.GetPointer();
   QByteArray  origBytes = qCompress(dataPtr, origDoc.Size() + 1);
   QByteArray  newBytes  = qUncompress(origBytes);

   UtTextDocument newDoc;
   newDoc.Insert(0, newBytes.data(), newBytes.length());

   QVERIFY(origDoc == newDoc);
}

// Tests that a ViTextSourceChange can be serialized and deserialized successfully.
void TestViRevisionDb::testSerializeViTextSourceChange()
{
   ViTextSourceChange change;

   change.mCharsRemoved   = 0;
   change.mPos            = 1;
   change.mSequenceNumber = 2;
   change.mText           = "3, test test test! 4";

   QByteArray  bytes;
   QDataStream stream(&bytes, QIODevice::WriteOnly);
   stream << change;
   QByteArray compressedBytes = qCompress(bytes);

   ViTextSourceChange newChange;
   QDataStream        newStream(qUncompress(compressedBytes));
   newStream >> newChange;

   QVERIFY(change == newChange);
}

// Tests that a list of changes can be serialized, inserted into the database,
// retrieved from the database, and deserialized back into its original form.
void TestViRevisionDb::testDbInsertSelectChange()
{
   ViRevisionDb db;

   QVERIFY(db.OpenInMemory());

   ViTextSourceChange change;
   change.mCharsRemoved   = 2;
   change.mPos            = 3;
   change.mSequenceNumber = 1;
   change.mText           = "foo bar";

   QVector<ViTextSourceChange> changes;
   changes.push_back(change);

   const int revNo = db.NewRevisionNo("/test");
   QVERIFY(revNo != -1);

   ViRevisionChange rev(ViRevisionChange::cCHANGE_DELTA, "test_path", changes);
   QVERIFY(db.InsertChange(revNo, rev));

   ViRevisionChange dbRev = db.Changes(revNo).front();
   QVERIFY(dbRev == rev);

   QDataStream                 stream(dbRev.changeBytes);
   QVector<ViTextSourceChange> dbChanges;
   stream >> dbChanges;
   QVERIFY(dbChanges == changes);

   QVERIFY(db.Close());
}

// Tests that a revision and associated data can be inserted into and retrieved
// from the database.
void TestViRevisionDb::testDbInsertSelectRevision()
{
   ViRevisionDb db;

   QVERIFY(db.OpenInMemory());

   const int revNo = db.NewRevisionNo("/test");
   QVERIFY(revNo != -1);
   QVERIFY(db.InsertStartupFile(revNo, "/test/startup_file"));

   const ViRevisionList& revList = db.Revisions();
   QVERIFY(revList.size() == 1);

   const ViRevision& rev = revList.front();
   QVERIFY(rev.revNo == revNo);
   QVERIFY(rev.workingDir == "/test");
   QVERIFY(rev.startupFiles.size() == 1);
   QVERIFY(rev.startupFiles.front() == "/test/startup_file");

   QVERIFY(db.Close());
}

// Tests inserting and retrieving different revisions of a file from the database.
void TestViRevisionDb::testGetFileAtRevision()
{
   const std::string path = "testPath";
   UtTextDocument    rev1, rev2, rev3;

   rev1.Insert(0, "Test");
   rev2.Insert(0, std::string(rev1.GetPointer()) + " delta change");
   rev3.Insert(0, "Completely different");

   QVector<ViTextSourceChange> rev1Rev2changes;
   ViTextSource::DiffDocuments(rev1.GetPointer(), rev2.GetPointer(), rev1Rev2changes);
   QVERIFY(!rev1Rev2changes.empty());

   ViRevisionDb db;
   QVERIFY(db.OpenInMemory());

   int revNo1 = db.NewRevisionNo("/test");
   QVERIFY(revNo1 != -1);
   QVERIFY(db.InsertChange(revNo1, ViRevisionChange(ViRevisionChange::cNEW_FILE, path, rev1)));

   int revNo2 = db.NewRevisionNo("/test");
   QVERIFY(revNo2 != -1);
   QVERIFY(revNo2 != revNo1);
   QVERIFY(db.InsertChange(revNo2, ViRevisionChange(ViRevisionChange::cCHANGE_DELTA, path, rev1Rev2changes)));

   int revNo3 = db.NewRevisionNo("/test");
   QVERIFY(revNo3 != -1);
   QVERIFY(revNo3 != revNo1);
   QVERIFY(revNo3 != revNo2);
   QVERIFY(db.InsertChange(revNo3, ViRevisionChange(ViRevisionChange::cCHANGE_FULL, path, rev3)));

   UtTextDocument dbRev1, dbRev2, dbRev3;

   QVERIFY(db.GetFileAtRevision(path, revNo1, dbRev1));
   QVERIFY(dbRev1 == rev1);

   QVERIFY(db.GetFileAtRevision(path, revNo2, dbRev2));
   QVERIFY(dbRev2 == rev2);

   QVERIFY(db.GetFileAtRevision(path, revNo3, dbRev3));
   QVERIFY(dbRev3 == rev3);

   QVERIFY(db.Close());
}

// Tests that a newly created database reports an up-to-date version.
void TestViRevisionDb::testVersion()
{
   ViRevisionDb db;

   QVERIFY(db.OpenInMemory());
   QCOMPARE(db.Version(), db.LatestSupportedVersion());
   QVERIFY(!db.IsVersionOld());
   QVERIFY(db.Close());
}

QTEST_APPLESS_MAIN(TestViRevisionDb)
#include "moc/TestViRevisionDb.moc"
