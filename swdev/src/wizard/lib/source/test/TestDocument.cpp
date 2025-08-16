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

#include <QObject>
#include <QString>
#include <QtTest/QTest>

#include "UtTextDocument.hpp"

class TestDocument : public QObject
{
   Q_OBJECT

private slots:
   void testGetLine();
};

void TestDocument::testGetLine()
{
   const std::string fileContent("Begin Test File\n"
                                 "  This is the second Line\n"
                                 "  Third Line\n");
   UtTextDocument    doc;
   doc.Insert(0, fileContent);

   QCOMPARE(doc.GetLineCount(), 4);
   QCOMPARE(QString::fromStdString(doc.GetLineString(0)), QString("Begin Test File\n"));
   QCOMPARE(QString::fromStdString(doc.GetLineString(2)), QString("  Third Line\n"));

   doc.Erase(doc.GetLinePosition(1), doc.GetLineLength(1));
   QCOMPARE(QString::fromStdString(doc.GetLineString(1)), QString("  Third Line\n"));

   doc.Erase(doc.GetLinePosition(0), doc.GetLineLength(0));
   QCOMPARE(QString::fromStdString(doc.GetLineString(0)), QString("  Third Line\n"));
}

QTEST_APPLESS_MAIN(TestDocument)
#include "moc/TestDocument.moc"
