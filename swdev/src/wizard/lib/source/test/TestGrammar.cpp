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

#include <memory>
#include <string>

#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QtTest/QTest>

#include "UtPath.hpp"
#include "ViAfnesExeManager.hpp"
#include "ViRunEnvManager.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxy.hpp"
#include "WsfPProxyDeserialize.hpp"
#include "WsfPProxyUndo.hpp"
#include "WsfParseDefinitions.hpp"
#include "WsfParseVerify.hpp"
#include "WsfParser.hpp"

class TestGrammar : public QObject
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
   void initTestCase();      // Called before the first test function is executed.
   void init() {}            // Called before each test function is executed.
   void cleanup() {}         // Called after every test function.
   void cleanupTestCase() {} // Called after the last test function is executed.

   void testLoadDefaultGrammar();
   void testCreateProxy();
};

void TestGrammar::initTestCase()
{
   UtPath baseDir = UtPath::GetExePath();
   baseDir.Up();
   baseDir += "wizard_install";

   ViRunEnvManager::InitDataDirs(baseDir);
}

void TestGrammar::testLoadDefaultGrammar()
{
   WsfParseDefinitions parseDefs;
   std::string         grammarText;

   ViAfnesExeManager::GetDefaultGrammar(parseDefs, grammarText);

   const bool parseDefsErrors = !parseDefs.GetDefinitionErrors().empty();
   const bool parseDefsInit   = parseDefs.Initialize();

   QVERIFY(!parseDefsErrors);
   QVERIFY(parseDefsInit);
   QVERIFY(parseDefs.GetRoot() != NULL);
   QVERIFY(!grammarText.empty());
}

const QString TEST_CREATE_PROXY_SCENARIO_FILE_NAME = "test_create_proxy_scenario.txt";
const QString TEST_CREATE_PROXY_SCENARIO("platform_type MY_PLAT_TYPE WSF_PLATFORM\n"
                                         "   side blue\n"
                                         "end_platform_type\n"
                                         "platform MyPlat MY_PLAT_TYPE\n"
                                         "   side green\n"
                                         "end_platform");

void TestGrammar::testCreateProxy()
{
   WsfParseDefinitions* parseDefs = new WsfParseDefinitions;
   std::string          grammarText;
   ViAfnesExeManager::GetDefaultGrammar(*parseDefs, grammarText);
   parseDefs->Initialize();

   ParseSourceProvider sourceProv;
   WsfParser           parser(&sourceProv, UtPath::WorkingDirectory(), parseDefs);

   QFile      testCreateProxyFile(TEST_CREATE_PROXY_SCENARIO_FILE_NAME);
   const bool testFileOpened = testCreateProxyFile.open(QFile::WriteOnly);
   QVERIFY(testFileOpened);
   if (testFileOpened)
   {
      QTextStream testCreateProxyFileText(&testCreateProxyFile);
      testCreateProxyFileText << TEST_CREATE_PROXY_SCENARIO;
      testCreateProxyFile.close();

      std::vector<UtPath> files;
      files.push_back(UtPath(TEST_CREATE_PROXY_SCENARIO_FILE_NAME.toStdString()));
      WsfParseNode*              treePtr = parser.ParseFiles(files, 0);
      std::vector<WsfParseNode*> fileTransitionNodes;
      parser.FinalizeParseTree(treePtr, fileTransitionNodes);

      auto proxyRegistry = std::make_shared<WsfPProxyRegistry>();
      proxyRegistry->Setup(parseDefs->GetRootStruct(), parseDefs->mBasicTypes);

      WsfPProxyIndex* index     = new WsfPProxyIndex;
      index->mRecordCurrentPath = true;

      WsfPProxyDeserialize deserialize(proxyRegistry.Pointer(), index);
      WsfPProxyValue       existingProxy = proxyRegistry->GetType("root")->ConstructNew();
      WsfPProxyUndoValue   existingProxyUndo(existingProxy);
      existingProxyUndo.DisableUndo();
      deserialize.Deserialize(existingProxyUndo, treePtr);

      WsfPProxy proxy;
      proxy.mIndex     = index;
      proxy.mRoot      = existingProxy;
      proxy.mBasicRoot = proxyRegistry->GetBasicRoot();
      proxy.mRegistry  = proxyRegistry;
      proxy.BuildPathMap();

      QVERIFY(proxy.mIndex);
      QVERIFY(proxy.mRoot);
      QVERIFY(proxy.mBasicRoot);
      QVERIFY(!proxy.mRegistry.IsNull());
      QVERIFY(proxy.mPathMap);

      WsfPM_Root root(&proxy);

      QVERIFY(root.platforms().Find("MyPlat").IsValid());
      QVERIFY(root.platformTypes().Find("MY_PLAT_TYPE").IsValid());
   }
}

QTEST_APPLESS_MAIN(TestGrammar)
#include "moc/TestGrammar.moc"
