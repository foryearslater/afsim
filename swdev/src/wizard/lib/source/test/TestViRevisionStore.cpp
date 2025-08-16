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

#include "UtPath.hpp"
#include "ViRevisionStore.hpp"

class TestViRevisionStore : public ViRevisionStore
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

   void testSaveRevision();
   void testStartupFilesOrdering();
};

// Tests queueing files and saving revisions.
void TestViRevisionStore::testSaveRevision()
{
   UtTextDocument doc;
   std::string    str("Hello World\nI am a file\nThis is the last line\n");

   const std::string&       workingDir = "myDir";
   std::vector<std::string> startupFiles;
   startupFiles.push_back("myFile");

   QVERIFY(OpenInMemory());
   QVERIFY(IsEmpty());
   QVERIFY(StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles)); // nothing queued

   doc.Insert(0, str.c_str(), str.size() + 1);
   QueueFile(&doc);
   QVERIFY(StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles));
   QueueFile(&doc);
   QVERIFY(!StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles)); // nothing changed
   QVERIFY(!IsEmpty());

   doc.Erase(10, 8);
   QueueFile(&doc);
   QVERIFY(StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles));
   QueueFile(&doc);
   QVERIFY(!StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles)); // nothing changed
   QVERIFY(!IsEmpty());

   doc.Insert(10, "This Here", 9);
   QueueFile(&doc);
   QVERIFY(StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles));
   QueueFile(&doc);
   QVERIFY(!StoreNewRevision(GenerateFileChanges(), workingDir, startupFiles)); // nothing changed
   QVERIFY(!IsEmpty());

   QVERIFY(Close());
}

void TestViRevisionStore::testStartupFilesOrdering()
{
   std::vector<std::string> startupFiles, reversedStartupFiles;
   startupFiles.push_back("myFile1");
   startupFiles.push_back("myFile2");
   reversedStartupFiles.push_back("myFile2");
   reversedStartupFiles.push_back("myFile1");

   QVERIFY(OpenInMemory());
   QVERIFY(StoreNewRevision(ViRevisionChangeList(), ".", startupFiles));
   QVERIFY(StoreNewRevision(ViRevisionChangeList(), ".", reversedStartupFiles));

   const int                latestRev = DB().LatestRevisionNo();
   const int                prevRev   = latestRev - 1;
   std::vector<std::string> retrievedStartupFiles, retrievedReversedStartupFiles;

   QVERIFY(prevRev != -1);
   QVERIFY(latestRev != -1);
   QVERIFY(DB().StartupFiles(prevRev, retrievedStartupFiles));
   QVERIFY(DB().StartupFiles(latestRev, retrievedReversedStartupFiles));
   QCOMPARE(retrievedStartupFiles, startupFiles);
   QCOMPARE(retrievedReversedStartupFiles, reversedStartupFiles);
   QVERIFY(Close());
}

QTEST_APPLESS_MAIN(TestViRevisionStore)
#include "moc/TestViRevisionStore.moc"
