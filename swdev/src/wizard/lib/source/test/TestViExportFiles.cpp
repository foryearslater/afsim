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

#include <QDir>
#include <QObject>
#include <QString>
#include <QtTest/QTest>

#include "UtPath.hpp"
#include "ViExportFiles.hpp"

class TestViExportFiles : public QObject
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

   void testWindowsAbsExtExportPath();
   void testWindowsAbsExtNetworkExportPath();
   void testLinuxAbsExtExportPath();

private:
   void testExportPath(const QString& aExportDirPath,
                       const QString& aFilePath,
                       const QString& aExpectedExportPath,
                       const QString& aProjectDir = "")
   {
      const UtPath        projDir(aProjectDir.toStdString());
      const ViExportFiles exportFiles(projDir);
      const QString&      actualExportPath = exportFiles.GetExportPath(aExportDirPath, aFilePath);

      const UtPath actual(actualExportPath.toStdString());
      const UtPath expected(aExpectedExportPath.toStdString());

      QCOMPARE(actual, expected);
   }
};

void TestViExportFiles::testWindowsAbsExtExportPath()
{
   const QString exportDirPath("\\\\networkdrive\\Some\\Other\\");
   const QString externalFilePath("C:\\\\Programs\\hey.txt");
   const QString expectedExportPath("\\\\networkdrive\\Some\\Other\\_external_items\\C\\Programs\\hey.txt");

   testExportPath(exportDirPath, externalFilePath, expectedExportPath);
}

void TestViExportFiles::testWindowsAbsExtNetworkExportPath()
{
   const QString exportDirPath("C:\\\\Some\\Other\\");
   const QString externalFilePath("\\\\networkdrive\\Programs\\hey.txt");
   const QString expectedExportPath("C:\\\\Some\\Other\\_external_items\\networkdrive\\Programs\\hey.txt");

   testExportPath(exportDirPath, externalFilePath, expectedExportPath);
}

void TestViExportFiles::testLinuxAbsExtExportPath()
{
   const QString exportDirPath("/home/me/exportdir");
   const QString externalFilePath("/home/you/somefile.txt");
   const QString expectedExportPath("/home/me/exportdir/_external_items/home/you/somefile.txt");

   testExportPath(exportDirPath, externalFilePath, expectedExportPath);
}

QTEST_MAIN(TestViExportFiles)
#include "moc/TestViExportFiles.moc"
