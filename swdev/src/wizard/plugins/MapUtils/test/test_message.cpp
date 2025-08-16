// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "test_message.hpp"

#include <cassert>
#include <iostream>
#include <string>

#include <QString>

//#include <gtest/gtest.h>

namespace test
{
namespace message
{

DerivedMessageForTest::DerivedMessageForTest()
   : Message(Level::NoIcon, "Placeholder Message Title", "Placeholder Message Body")
{
}

void Message::TestSetLevel(const Level aLevel) noexcept
{
   const_cast<Level&>(mMessageLevel) = aLevel;
}

bool operator==(const DerivedMessageForTest& aLhs, const DerivedMessageForTest& aRhs)
{
   return static_cast<MapUtils::Message>(aLhs) == static_cast<MapUtils::Message>(aRhs);
}

bool operator!=(const DerivedMessageForTest& aLhs, const DerivedMessageForTest& aRhs)
{
   return !(aLhs == aRhs);
}

std::vector<bool> Message_Constructor1()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor1;

   // test constructor 1
   // indirectly tests the static function StringToBody
   Message testCtor1(Message::Level::Critical,
                     "Message:  Test Constructor 1",
                     "This message tests the explicit constructor:\n" +
                        Message::cDEFAULT_INDENT + "Message(const Level aLevel, const QString& aTitle, const QString& aMessage, const Qt::TextFormat aFormat, const bool aShowType);",
                     Qt::TextFormat::PlainText,
                     true);
   resVec_testCtor1.push_back(testCtor1.GetLevel() == Message::Level::Critical);
   resVec_testCtor1.push_back(testCtor1.GetTitle() == "Message:  Test Constructor 1");
   resVec_testCtor1.push_back(testCtor1.GetBody().mData == "This message tests the explicit constructor:");
   if (testCtor1.GetBody().mNotes.size() == 1)
   {
      resVec_testCtor1.push_back(true);
      resVec_testCtor1.push_back(testCtor1.GetBody().mNotes[0].mData ==
                                 "Message(const Level aLevel, const QString& aTitle, const QString& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor1.push_back(testCtor1.GetBody().mNotes[0].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor1.push_back(false);
   }
   resVec_testCtor1.push_back(testCtor1.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testCtor1.push_back(testCtor1.GetShowType() == true);

   return resVec_testCtor1;
}

std::vector<bool> Message_Constructor2()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor2;

   // test constructor 2
   Message testCtor2(Message::Level::Information,
                     "Message:  Test Constructor 2",
                     Message::Body{"This message tests the explicit constructor:",
                                   std::vector<Message::Body>{
                                      Message::Body{"Message(const Level aLevel, const QString& aTitle, const Body& "
                                                    "aMessage, const Qt::TextFormat aFormat, const bool aShowType);",
                                                    {}},
                                      Message::Body{"struct Body { QString mData; std::vector<Body> mNotes; };", {}}}},
                     Qt::TextFormat::PlainText,
                     false);
   resVec_testCtor2.push_back(testCtor2.GetLevel() == Message::Level::Information);
   resVec_testCtor2.push_back(testCtor2.GetTitle() == "Message:  Test Constructor 2");
   resVec_testCtor2.push_back(testCtor2.GetBody().mData == "This message tests the explicit constructor:");
   if (testCtor2.GetBody().mNotes.size() == 2)
   {
      resVec_testCtor2.push_back(true);
      resVec_testCtor2.push_back(testCtor2.GetBody().mNotes[0].mData ==
                                 "Message(const Level aLevel, const QString& aTitle, const Body& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor2.push_back(testCtor2.GetBody().mNotes[0].mNotes.size() == 0);
      resVec_testCtor2.push_back(testCtor2.GetBody().mNotes[1].mData ==
                                 "struct Body { QString mData; std::vector<Body> mNotes; };");
      resVec_testCtor2.push_back(testCtor2.GetBody().mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor2.push_back(false);
   }
   resVec_testCtor2.push_back(testCtor2.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testCtor2.push_back(testCtor2.GetShowType() == false);

   return resVec_testCtor2;
}

std::vector<bool> Message_Constructor3()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor3;

   // test ut::log::Message
   ut::log::Message testMessage;
   testMessage.mTypes.Insert(ut::log::Message::Warning);
   testMessage.mData = "This message tests the explicit constructor:";
   testMessage.mNotes.push_back(ut::log::Message{"Message(const QString& aTitle, const ut::log::Message& aMessage, "
                                                 "const Qt::TextFormat aFormat, const bool aShowType);",
                                                 {}});
   testMessage.mNotes.push_back(ut::log::Message{
      "struct UT_EXPORT ut::log::Message final { "
      "static const std::string Fatal; static const std::string Error; static const std::string Warning; static const "
      "std::string Debug; static const std::string Info; static const std::string Developer; "
      "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList PresetRelease; "
      "static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
      "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ",
      {}});
   resVec_testCtor3.push_back(testMessage.mTypes.Contains(ut::log::Message::Warning));
   resVec_testCtor3.push_back(testMessage.mData == "This message tests the explicit constructor:");
   if (testMessage.mNotes.size() == 2)
   {
      resVec_testCtor3.push_back(true);
      resVec_testCtor3.push_back(testMessage.mNotes[0].mData ==
                                 "Message(const QString& aTitle, const ut::log::Message& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor3.push_back(testMessage.mNotes[0].mNotes.size() == 0);
      resVec_testCtor3.push_back(
         testMessage.mNotes[1].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testCtor3.push_back(testMessage.mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor3.push_back(false);
   }
   // test constructor 3
   // indirectly tests the static function LogMessageToBody
   Message testCtor3("Message:  Test Constructor 3", testMessage, Qt::TextFormat::PlainText, true);
   resVec_testCtor3.push_back(testCtor3.GetLevel() == Message::Level::Warning);
   resVec_testCtor3.push_back(testCtor3.GetTitle() == "Message:  Test Constructor 3");
   resVec_testCtor3.push_back(testCtor3.GetBody().mData == "This message tests the explicit constructor:");
   if (testCtor3.GetBody().mNotes.size() == 2)
   {
      resVec_testCtor3.push_back(true);
      resVec_testCtor3.push_back(testCtor3.GetBody().mNotes[0].mData ==
                                 "Message(const QString& aTitle, const ut::log::Message& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor3.push_back(testCtor3.GetBody().mNotes[0].mNotes.size() == 0);
      resVec_testCtor3.push_back(
         testCtor3.GetBody().mNotes[1].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testCtor3.push_back(testCtor3.GetBody().mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor3.push_back(false);
   }
   resVec_testCtor3.push_back(testCtor3.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testCtor3.push_back(testCtor3.GetShowType() == true);

   return resVec_testCtor3;
}

std::vector<bool> Message_Constructor4()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor4;

   // test constructor 4
   // indirectly tests constructor 1
   Message testCtor4(Message::Level::Question,
                     "Message:  Test Constructor 4",
                     "This message tests the explicit constructor:\n" + Message::cDEFAULT_INDENT +
                        "Message(const Level aLevel, const QString& aTitle, const Body& aMessage);");
   resVec_testCtor4.push_back(testCtor4.GetLevel() == Message::Level::Question);
   resVec_testCtor4.push_back(testCtor4.GetTitle() == "Message:  Test Constructor 4");
   if (testCtor4.GetBody().mNotes.size() == 1)
   {
      resVec_testCtor4.push_back(true);
      resVec_testCtor4.push_back(testCtor4.GetBody().mNotes[0].mData ==
                                 "Message(const Level aLevel, const QString& aTitle, const Body& aMessage);");
      resVec_testCtor4.push_back(testCtor4.GetBody().mNotes[0].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor4.push_back(false);
   }
   resVec_testCtor4.push_back(testCtor4.GetTextFormat() == Qt::TextFormat::AutoText);
   resVec_testCtor4.push_back(testCtor4.GetShowType() == true);

   return resVec_testCtor4;
}

std::vector<bool> Message_Constructor5()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor5;

   // test constructor 5
   // indirectly tests constructor 2
   Message testCtor5(
      Message::Level::NoIcon,
      "Message:  Test Constructor 5",
      Message::Body{"This message tests the explicit constructor:",
                    std::vector<Message::Body>{
                       Message::Body{"Message(const Level aLevel, const QString& aTitle, const Body& aMessage);", {}},
                       Message::Body{"struct Body { QString mData; std::vector<Body> mNotes; };", {}}}});
   resVec_testCtor5.push_back(testCtor5.GetLevel() == Message::Level::NoIcon);
   resVec_testCtor5.push_back(testCtor5.GetTitle() == "Message:  Test Constructor 5");
   if (testCtor5.GetBody().mNotes.size() == 2)
   {
      resVec_testCtor5.push_back(true);
      resVec_testCtor5.push_back(testCtor5.GetBody().mNotes[0].mData ==
                                 "Message(const Level aLevel, const QString& aTitle, const Body& aMessage);");
      resVec_testCtor5.push_back(testCtor5.GetBody().mNotes[0].mNotes.size() == 0);
      resVec_testCtor5.push_back(testCtor5.GetBody().mNotes[1].mData ==
                                 "struct Body { QString mData; std::vector<Body> mNotes; };");
      resVec_testCtor5.push_back(testCtor5.GetBody().mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor5.push_back(false);
   }
   resVec_testCtor5.push_back(testCtor5.GetTextFormat() == Qt::TextFormat::AutoText);
   resVec_testCtor5.push_back(testCtor5.GetShowType() == true);

   return resVec_testCtor5;
}

std::vector<bool> Message_Constructor6()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCtor6;

   // test ut::log::Message
   ut::log::Message testMessage;
   testMessage.mTypes.Insert(ut::log::Message::Error);
   testMessage.mData = "This message tests the explicit constructor:";
   testMessage.mNotes.push_back(ut::log::Message{"Message(const QString& aTitle, const ut::log::Message& aMessage, "
                                                 "const Qt::TextFormat aFormat, const bool aShowType);",
                                                 {}});
   testMessage.mNotes.push_back(ut::log::Message{
      "struct UT_EXPORT ut::log::Message final { "
      "static const std::string Fatal; static const std::string Error; static const std::string Warning; static const "
      "std::string Debug; static const std::string Info; static const std::string Developer; "
      "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList PresetRelease; "
      "static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
      "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ",
      {}});
   resVec_testCtor6.push_back(testMessage.mTypes.Contains(ut::log::Message::Error));
   resVec_testCtor6.push_back(testMessage.mData == "This message tests the explicit constructor:");
   if (testMessage.mNotes.size() == 2)
   {
      resVec_testCtor6.push_back(true);
      resVec_testCtor6.push_back(testMessage.mNotes[0].mData ==
                                 "Message(const QString& aTitle, const ut::log::Message& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor6.push_back(testMessage.mNotes[0].mNotes.size() == 0);
      resVec_testCtor6.push_back(
         testMessage.mNotes[1].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testCtor6.push_back(testMessage.mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor6.push_back(false);
   }
   // test constructor 6
   // indirectly tests constructor 3
   Message testCtor6("Message:  Test Constructor 6", testMessage, Qt::TextFormat::PlainText, true);
   resVec_testCtor6.push_back(testCtor6.GetLevel() == Message::Level::Critical);
   resVec_testCtor6.push_back(testCtor6.GetTitle() == "Message:  Test Constructor 6");
   resVec_testCtor6.push_back(testCtor6.GetBody().mData == "This message tests the explicit constructor:");
   if (testCtor6.GetBody().mNotes.size() == 2)
   {
      resVec_testCtor6.push_back(true);
      resVec_testCtor6.push_back(testCtor6.GetBody().mNotes[0].mData ==
                                 "Message(const QString& aTitle, const ut::log::Message& aMessage, const "
                                 "Qt::TextFormat aFormat, const bool aShowType);");
      resVec_testCtor6.push_back(testCtor6.GetBody().mNotes[0].mNotes.size() == 0);
      resVec_testCtor6.push_back(
         testCtor6.GetBody().mNotes[1].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testCtor6.push_back(testCtor6.GetBody().mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testCtor6.push_back(false);
   }
   resVec_testCtor6.push_back(testCtor6.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testCtor6.push_back(testCtor6.GetShowType() == true);

   return resVec_testCtor6;
}

std::vector<bool> Message_CopyConstructor()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testCopyCtor;

   // create a message
   Message testCtor1(Message::Level::Critical,
                     "Message:  Test Copy Constructor",
                     "This message tests the copy constructor:\n" + Message::cDEFAULT_INDENT +
                        "Message(const Message& /* aSrc */);",
                     Qt::TextFormat::PlainText,
                     true);

   // test copy constructor
   Message testCopyCtor(testCtor1);
   resVec_testCopyCtor.push_back(testCopyCtor.GetLevel() == Message::Level::Critical);
   resVec_testCopyCtor.push_back(testCopyCtor.GetTitle() == "Message:  Test Copy Constructor");
   resVec_testCopyCtor.push_back(testCopyCtor.GetBody().mData == "This message tests the copy constructor:");
   if (testCopyCtor.GetBody().mNotes.size() == 1)
   {
      resVec_testCopyCtor.push_back(true);
      resVec_testCopyCtor.push_back(testCopyCtor.GetBody().mNotes[0].mData == "Message(const Message& /* aSrc */);");
      resVec_testCopyCtor.push_back(testCopyCtor.GetBody().mNotes[0].mNotes.size() == 0);
   }
   else
   {
      resVec_testCopyCtor.push_back(false);
   }
   resVec_testCopyCtor.push_back(testCtor1.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testCopyCtor.push_back(testCtor1.GetShowType() == true);

   return resVec_testCopyCtor;
}

std::vector<bool> Message_MoveConstructor()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testMoveCtor;

   // create a message
   Message testCtor2(Message::Level::Information,
                     "Message:  Test Move Constructor",
                     Message::Body{"This message tests the move constructor:",
                                   std::vector<Message::Body>{Message::Body{"Message(Message&& /* aSrc */);", {}}}},
                     Qt::TextFormat::PlainText,
                     false);

   // test move constructor
   Message testMoveCtor(std::move(testCtor2));
   resVec_testMoveCtor.push_back(testMoveCtor.GetLevel() == Message::Level::Information);
   resVec_testMoveCtor.push_back(testMoveCtor.GetTitle() == "Message:  Test Move Constructor");
   resVec_testMoveCtor.push_back(testMoveCtor.GetBody().mData == "This message tests the move constructor:");
   if (testMoveCtor.GetBody().mNotes.size() == 1)
   {
      resVec_testMoveCtor.push_back(true);
      resVec_testMoveCtor.push_back(testMoveCtor.GetBody().mNotes[0].mData == "Message(Message&& /* aSrc */);");
      resVec_testMoveCtor.push_back(testMoveCtor.GetBody().mNotes[0].mNotes.size() == 0);
   }
   else
   {
      resVec_testMoveCtor.push_back(false);
   }
   resVec_testMoveCtor.push_back(testMoveCtor.GetTextFormat() == Qt::TextFormat::PlainText);
   resVec_testMoveCtor.push_back(testMoveCtor.GetShowType() == false);

   return resVec_testMoveCtor;
}

std::vector<bool> Message_Destructor()
{
   // the destructor is defaulted and there is no member pointers to clean up;
   // therefore, there is no easy way to test this
   // assume it works
   return {true};
}

std::vector<bool> Message_SetGetLevel()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testSetGetLevel;

   // construct a message
   DerivedMessageForTest testSetGetLevel;

   // test the mutator SetLevel
   resVec_testSetGetLevel.push_back(testSetGetLevel.GetLevel() == Message::Level::NoIcon);
   testSetGetLevel.UnsafeSetLevel(Message::Level::Critical);
   resVec_testSetGetLevel.push_back(testSetGetLevel.GetLevel() == Message::Level::Critical);

   return resVec_testSetGetLevel;
}

std::vector<bool> Message_SetGetTitle()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testSetGetTitle;

   // construct a message
   DerivedMessageForTest testSetGetTitle;

   // test the mutator SetLevel
   resVec_testSetGetTitle.push_back(testSetGetTitle.GetTitle() == "Placeholder Message Title");
   testSetGetTitle.SetTitle("This message tests the mutator SetTitle/accessor GetTitle");
   resVec_testSetGetTitle.push_back(testSetGetTitle.GetTitle() ==
                                    "This message tests the mutator SetTitle/accessor GetTitle");

   return resVec_testSetGetTitle;
}

std::vector<bool> Message_SetGetBody()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testSetGetBody;

   // construct a message
   DerivedMessageForTest testSetGetBody;

   // test the mutator SetLevel
   resVec_testSetGetBody.push_back(testSetGetBody.GetBody().mData == "Placeholder Message Body");

   // create a Message::Body
   Message::Body testBody{
      "This message tests the mutator SetBody/accessor GetBody:",
      std::vector<Message::Body>{
         Message::Body{"This test is for the overload that takes/returns a const Body&",
                       std::vector<Message::Body>{Message::Body{"void SetBody(const Body& aMessageBody);", {}},
                                                  Message::Body{"const Body& GetBody() const;", {}}}},
         Message::Body{"struct Body { QString mData; std::vector<Body> mNotes; };", {}}}};

   // test mutator SetBody overloaded for Message::Body/accessor GetBody
   testSetGetBody.SetBody(testBody);
   auto bodyAccess = testSetGetBody.GetBody();
   resVec_testSetGetBody.push_back(bodyAccess.mData == "This message tests the mutator SetBody/accessor GetBody:");
   if (bodyAccess.mNotes.size() == 2)
   {
      resVec_testSetGetBody.push_back(true);
      resVec_testSetGetBody.push_back(bodyAccess.mNotes[0].mData ==
                                      "This test is for the overload that takes/returns a const Body&");
      if (bodyAccess.mNotes[0].mNotes.size() == 2)
      {
         resVec_testSetGetBody.push_back(true);
         resVec_testSetGetBody.push_back(bodyAccess.mNotes[0].mNotes[0].mData ==
                                         "void SetBody(const Body& aMessageBody);");
         resVec_testSetGetBody.push_back(bodyAccess.mNotes[0].mNotes[0].mNotes.size() == 0);
         resVec_testSetGetBody.push_back(bodyAccess.mNotes[0].mNotes[1].mData == "const Body& GetBody() const;");
         resVec_testSetGetBody.push_back(bodyAccess.mNotes[0].mNotes[1].mNotes.size() == 0);
      }
      else
      {
         resVec_testSetGetBody.push_back(false);
      }
      resVec_testSetGetBody.push_back(bodyAccess.mNotes[1].mData ==
                                      "struct Body { QString mData; std::vector<Body> mNotes; };");
      resVec_testSetGetBody.push_back(bodyAccess.mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testSetGetBody.push_back(false);
   }

   testBody.mData     = "This message tests the accessor GetBodyAsLogMessage:";
   testBody.mNotes[0] = Message::Body{"ut::log::Message GetBodyAsLogMessage() const;", {}};
   testBody.mNotes[1] = Message::Body{
      "struct UT_EXPORT ut::log::Message final { "
      "static const std::string Fatal; static const std::string Error; static const std::string Warning; static const "
      "std::string Debug; static const std::string Info; static const std::string Developer; "
      "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList PresetRelease; "
      "static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
      "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ",
      {}};

   // test accessor GetBodyAsLogMessage
   // indirectly tests the static function BodyToLogMessage
   testSetGetBody.SetBody(testBody);
   auto bodyAsUtLogMessage = testSetGetBody.GetBodyAsLogMessage();
   resVec_testSetGetBody.push_back(
      bodyAsUtLogMessage.mTypes.Contains(Message::LevelToString(testSetGetBody.GetLevel()).toStdString()));
   resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mData == "This message tests the accessor GetBodyAsLogMessage:");
   if (bodyAccess.mNotes.size() == 2)
   {
      resVec_testSetGetBody.push_back(true);
      resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mNotes[0].mTypes.size() == 0);
      resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mNotes[0].mData ==
                                      "ut::log::Message GetBodyAsLogMessage() const;");
      resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mNotes[0].mNotes.size() == 0);
      resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mNotes[1].mTypes.size() == 0);
      resVec_testSetGetBody.push_back(
         bodyAsUtLogMessage.mNotes[1].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testSetGetBody.push_back(bodyAsUtLogMessage.mNotes[1].mNotes.size() == 0);
   }
   else
   {
      resVec_testSetGetBody.push_back(false);
   }

   // create a QString
   QString testString{"This message tests the mutator SetBody/accessor GetBodyAsString\n" + Message::cDEFAULT_INDENT +
                      "Test test is for the overload that takes/returns a QString\n" + Message::cDEFAULT_INDENT +
                      Message::cDEFAULT_INDENT + "void SetBody(const QString& aMessageBody);\n" +
                      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "QString GetBodyAsString() const;"};

   // test mutator SetBody overloaded for QString/accessor GetBodyAsString
   // indirectly tests the static function BodyToString
   testSetGetBody.SetBody(testString);
   auto bodyAsString = testSetGetBody.GetBodyAsString();
   resVec_testSetGetBody.push_back(bodyAsString == testString);

   return resVec_testSetGetBody;
}

std::vector<bool> Message_SetGetFormat()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testSetGetFormat;

   // construct a message
   DerivedMessageForTest testSetGetFormat;

   // test the mutator SetLevel
   resVec_testSetGetFormat.push_back(testSetGetFormat.GetTextFormat() == Qt::TextFormat::AutoText);
   testSetGetFormat.SetTextFormat(Qt::TextFormat::PlainText);
   resVec_testSetGetFormat.push_back(testSetGetFormat.GetTextFormat() == Qt::TextFormat::PlainText);

   return resVec_testSetGetFormat;
}

std::vector<bool> Message_SetGetShowType()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testSetGetShowType;

   // construct a message
   DerivedMessageForTest testSetGetShowType;

   // test the mutator SetLevel
   resVec_testSetGetShowType.push_back(testSetGetShowType.GetShowType() == true);
   testSetGetShowType.SetShowType(false);
   resVec_testSetGetShowType.push_back(testSetGetShowType.GetShowType() != true);

   return resVec_testSetGetShowType;
}

std::vector<bool> Message_Display()
{
   using MapUtils::Message;

   // create a DerivedMessageForTest
   DerivedMessageForTest testDisplay;
   testDisplay.Display();
   // create a Message
   Message testMessage(
      Message::Level::Critical,
      "Message:  Test Display Function",
      Message::Body{"This message tests the member function:",
                    std::vector<Message::Body>{Message::Body{"void Display() const;", {}},
                                               Message::Body{"NOTE:  It is really hard to unit test Qt things because "
                                                             "there are a lot of variables to take into consideration.",
                                                             {}}}});
   testMessage.Display();
   // change the body of testDisplay
   testDisplay.SetBody(testMessage.GetBody());
   testDisplay.Display();

   // probably should change definition of Display, so test can prove that the dialog shows
   return {true};
}

std::vector<bool> Message_BodyToString()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testBodyToString;

   // create a Message::Body
   Message::Body testBody{
      "This message tests the static function BodyToString.",
      std::vector<Message::Body>{
         Message::Body{"static QString BodyToString(const Body& aBody);", {}},
         Message::Body{"BodyToString uses the static function ParseBodyToStringP.",
                       std::vector<Message::Body>{
                          Message::Body{"static QString ParseBodyToStringP(const Body& aBody, const int aIndent);", {}},
                          Message::Body{"ParseBodyToStringP is used recursively.", {}}}},
         Message::Body{"struct Body { QString mData; std::vector<Body> mNotes; };", {}}}};
   // test BodyToString function
   QString testString{"This message tests the static function BodyToString.\n" + Message::cDEFAULT_INDENT +
                      "static QString BodyToString(const Body& aBody);\n" + Message::cDEFAULT_INDENT +
                      "BodyToString uses the static function ParseBodyToStringP.\n" + Message::cDEFAULT_INDENT +
                      Message::cDEFAULT_INDENT +
                      "static QString ParseBodyToStringP(const Body& aBody, const int aIndent);\n" +
                      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "ParseBodyToStringP is used recursively.\n" +
                      Message::cDEFAULT_INDENT + "struct Body { QString mData; std::vector<Body> mNotes; };"};
   auto    stringFromBody = Message::BodyToString(testBody);
   resVec_testBodyToString.push_back(stringFromBody == testString);

   return resVec_testBodyToString;
}

std::vector<bool> Message_StringToBody()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testStringToBody;

   // create a QString
   QString testString{
      "This message tests the static function StringToBody.\n" + Message::cDEFAULT_INDENT +
      "static Body StringToBody(const QString& aString);\n" + Message::cDEFAULT_INDENT +
      "StringToBody uses the static function ParseStringToBodyP.\n" + Message::cDEFAULT_INDENT +
      Message::cDEFAULT_INDENT + "static Body ParseStringToBodyP(QString& aString, const int aIndent);\n" +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "ParseStringToBodyP uses two functions:\n" +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT +
      "static int GetIndentOfCurrentLineP(const QString& aString);  (1)\n" + Message::cDEFAULT_INDENT +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "GetIndentOfCurrentLineP gets the current line indent level.\n" +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT +
      "static QString PopCurrentLineP(QString& aString, const int aIndent);  (2)\n" + Message::cDEFAULT_INDENT +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "PopCurrentLineP does two things:\n" +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT +
      "it removes the current line of the source string.  (1)\n" + Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "if it removed a string, then it returns it.  (2)\n" +
      Message::cDEFAULT_INDENT + Message::cDEFAULT_INDENT + "ParseStringToBodyP is used recursively."};

   // test StringToBody function
   auto bodyFromString = Message::StringToBody(testString);
   resVec_testStringToBody.push_back(bodyFromString.mData == "This message tests the static function StringToBody.");
   if (bodyFromString.mNotes.size() == 2)
   {
      resVec_testStringToBody.push_back(true);
      resVec_testStringToBody.push_back(bodyFromString.mNotes[0].mData ==
                                        "static Body StringToBody(const QString& aString);");
      resVec_testStringToBody.push_back(bodyFromString.mNotes[0].mNotes.size() == 0);
      resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mData ==
                                        "StringToBody uses the static function ParseStringToBodyP.");
      if (bodyFromString.mNotes[1].mNotes.size() == 3)
      {
         resVec_testStringToBody.push_back(true);
         resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[0].mData ==
                                           "static Body ParseStringToBodyP(QString& aString, const int aIndent);");
         resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[0].mNotes.size() == 0);
         resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mData ==
                                           "ParseStringToBodyP uses two functions:");
         if (bodyFromString.mNotes[1].mNotes[1].mNotes.size() == 4)
         {
            resVec_testStringToBody.push_back(true);
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[0].mData ==
                                              "static int GetIndentOfCurrentLineP(const QString& aString);  (1)");
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[0].mNotes.size() == 0);
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[1].mData ==
                                              "GetIndentOfCurrentLineP gets the current line indent level.");
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[1].mNotes.size() == 0);
            resVec_testStringToBody.push_back(
               bodyFromString.mNotes[1].mNotes[1].mNotes[2].mData ==
               "static QString PopCurrentLineP(QString& aString, const int aIndent);  (2)");
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[2].mNotes.size() == 0);
            resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[3].mData ==
                                              "PopCurrentLineP does two things:");
            if (bodyFromString.mNotes[1].mNotes[1].mNotes[3].mNotes.size() == 2)
            {
               resVec_testStringToBody.push_back(true);
               resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[3].mNotes[0].mData ==
                                                 "it removes the current line of the source string.  (1)");
               resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[3].mNotes[0].mNotes.size() == 0);
               resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[3].mNotes[1].mData ==
                                                 "if it removed a string, then it returns it.  (2)");
               resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[1].mNotes[3].mNotes[1].mNotes.size() == 0);
            }
            else
            {
               resVec_testStringToBody.push_back(false);
            }
         }
         else
         {
            resVec_testStringToBody.push_back(false);
         }
         resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[2].mData ==
                                           "ParseStringToBodyP is used recursively.");
         resVec_testStringToBody.push_back(bodyFromString.mNotes[1].mNotes[2].mNotes.size() == 0);
      }
      else
      {
         resVec_testStringToBody.push_back(false);
      }
   }
   else
   {
      resVec_testStringToBody.push_back(false);
   }

   return resVec_testStringToBody;
}

std::vector<bool> Message_BodyToLogMessage()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testBodyToLogMessage;

   // create a Message::Body
   Message::Body testBody{
      "This message tests the static function BodyToLogMessage.",
      std::vector<Message::Body>{
         Message::Body{"static ut::log::Message BodyToLogMessage(const Body& aBody);", {}},
         Message::Body{"BodyToLogMessage does two things:",
                       std::vector<Message::Body>{
                          Message::Body{"It stores the Body's main part into the ut::log::Message's main part.  (1)", {}},
                          Message::Body{"It stores the Body's notes into the ut::log::Message's notes.  (2)", {}},
                          Message::Body{"Of course, there is a necessary conversion from Body to ut::log::Message.", {}}}},
         Message::Body{"BodyToLogMessage is used recursively.", {}},
         Message::Body{
            "struct UT_EXPORT ut::log::Message final { "
            "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
            "const std::string Debug; static const std::string Info; static const std::string Developer; "
            "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
            "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
            "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ",
            {}}}};

   // test BodyToLogMessage function
   auto logMessageFromBody = Message::BodyToLogMessage(testBody);
   // note:  there is nothing in the TypeLists (BodyToLogMessage does not store the level of the message)
   resVec_testBodyToLogMessage.push_back(logMessageFromBody.mTypes.size() == 0);
   resVec_testBodyToLogMessage.push_back(logMessageFromBody.mData ==
                                         "This message tests the static function BodyToLogMessage.");
   if (logMessageFromBody.mNotes.size() == 4)
   {
      resVec_testBodyToLogMessage.push_back(true);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[0].mTypes.size() == 0);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[0].mData ==
                                            "static ut::log::Message BodyToLogMessage(const Body& aBody);");
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[0].mNotes.size() == 0);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mTypes.size() == 0);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mData == "BodyToLogMessage does two things:");
      if (logMessageFromBody.mNotes[1].mNotes.size() == 3)
      {
         resVec_testBodyToLogMessage.push_back(true);
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[0].mTypes.size() == 0);
         resVec_testBodyToLogMessage.push_back(
            logMessageFromBody.mNotes[1].mNotes[0].mData ==
            "It stores the Body's main part into the ut::log::Message's main part.  (1)");
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[0].mNotes.size() == 0);
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[1].mTypes.size() == 0);
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[1].mData ==
                                               "It stores the Body's notes into the ut::log::Message's notes.  (2)");
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[1].mNotes.size() == 0);
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[2].mTypes.size() == 0);
         resVec_testBodyToLogMessage.push_back(
            logMessageFromBody.mNotes[1].mNotes[2].mData ==
            "Of course, there is a necessary conversion from Body to ut::log::Message.");
         resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[1].mNotes[2].mNotes.size() == 0);
      }
      else
      {
         resVec_testBodyToLogMessage.push_back(false);
      }
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[2].mTypes.size() == 0);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[2].mData ==
                                            "BodyToLogMessage is used recursively.");
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[2].mNotes.size() == 0);
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[3].mTypes.size() == 0);
      resVec_testBodyToLogMessage.push_back(
         logMessageFromBody.mNotes[3].mData ==
         "struct UT_EXPORT ut::log::Message final { "
         "static const std::string Fatal; static const std::string Error; static const std::string Warning; static "
         "const std::string Debug; static const std::string Info; static const std::string Developer; "
         "static const TypeList PresetErrors; static const TypeList PresetWarnings; static const TypeList "
         "PresetRelease; static const TypeList PresetDebug; static const TypeList PresetDeveloper; "
         "std::string mData; TypeList mTypes; std::vector<Message> mNotes; } ");
      resVec_testBodyToLogMessage.push_back(logMessageFromBody.mNotes[3].mNotes.size() == 0);
   }
   else
   {
      resVec_testBodyToLogMessage.push_back(false);
   }

   return resVec_testBodyToLogMessage;
}

std::vector<bool> Message_LogMessageToBody()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testLogMessageToBody;

   // create a ut::log::Message
   ut::log::Message testLogMessage{
      "This message tests the static function LogMessageToBody.",
      {},
      std::vector<ut::log::Message>{
         ut::log::Message{"static Body LogMessageToBody(const ut::log::Message& aMessage);", {}, {}},
         ut::log::Message{
            "LogMessageToBody does two things:",
            {},
            std::vector<ut::log::Message>{
               ut::log::Message{"It stores the ut::log::Message's main part into the Body's main part.  (1)", {}, {}},
               ut::log::Message{"It stores the ut::log::Message's notes into the Body's notes.  (2)", {}, {}},
               ut::log::Message{"Of course, there is a necessary conversion from ut::log::Message to Body.", {}, {}}}},
         ut::log::Message{"LogMessageToBody is used recursively.", {}, {}},
         ut::log::Message{"struct Body { QString mData; std::vector<Body> mNotes; };", {}, {}}}};

   // test BodyToLogMessage function
   auto bodyFromLogMessage = Message::LogMessageToBody(testLogMessage);
   // note:  there is nothing in the TypeLists (LogMessageToBody does not store the level of the message)
   resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mData ==
                                         "This message tests the static function LogMessageToBody.");
   if (bodyFromLogMessage.mNotes.size() == 4)
   {
      resVec_testLogMessageToBody.push_back(true);
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[0].mData ==
                                            "static Body LogMessageToBody(const ut::log::Message& aMessage);");
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[0].mNotes.size() == 0);
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[1].mData == "LogMessageToBody does two things:");
      if (bodyFromLogMessage.mNotes[1].mNotes.size() == 3)
      {
         resVec_testLogMessageToBody.push_back(true);
         resVec_testLogMessageToBody.push_back(
            bodyFromLogMessage.mNotes[1].mNotes[0].mData ==
            "It stores the ut::log::Message's main part into the Body's main part.  (1)");
         resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[1].mNotes[0].mNotes.size() == 0);
         resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[1].mNotes[1].mData ==
                                               "It stores the ut::log::Message's notes into the Body's notes.  (2)");
         resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[1].mNotes[1].mNotes.size() == 0);
         resVec_testLogMessageToBody.push_back(
            bodyFromLogMessage.mNotes[1].mNotes[2].mData ==
            "Of course, there is a necessary conversion from ut::log::Message to Body.");
         resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[1].mNotes[2].mNotes.size() == 0);
      }
      else
      {
         resVec_testLogMessageToBody.push_back(false);
      }
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[2].mData ==
                                            "LogMessageToBody is used recursively.");
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[2].mNotes.size() == 0);
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[3].mData ==
                                            "struct Body { QString mData; std::vector<Body> mNotes; };");
      resVec_testLogMessageToBody.push_back(bodyFromLogMessage.mNotes[3].mNotes.size() == 0);
   }
   else
   {
      resVec_testLogMessageToBody.push_back(false);
   }

   return resVec_testLogMessageToBody;
}

std::vector<bool> Message_LevelToString()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testLevelToString;

   resVec_testLevelToString.push_back(Message::LevelToString(Message::Level::Critical) == "error");
   resVec_testLevelToString.push_back(Message::LevelToString(Message::Level::Information) == "info");
   resVec_testLevelToString.push_back(Message::LevelToString(Message::Level::NoIcon) == "");
   resVec_testLevelToString.push_back(Message::LevelToString(Message::Level::Question) == "question");
   resVec_testLevelToString.push_back(Message::LevelToString(Message::Level::Warning) == "warning");

   return resVec_testLevelToString;
}

std::vector<bool> Message_StringToLevel()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testStringToLevel;

   resVec_testStringToLevel.push_back(Message::StringToLevel("error") == Message::Level::Critical);
   resVec_testStringToLevel.push_back(Message::StringToLevel("info") == Message::Level::Information);
   resVec_testStringToLevel.push_back(Message::StringToLevel("") == Message::Level::NoIcon);
   resVec_testStringToLevel.push_back(Message::StringToLevel("question") == Message::Level::Question);
   resVec_testStringToLevel.push_back(Message::StringToLevel("warning") == Message::Level::Warning);
   resVec_testStringToLevel.push_back(Message::StringToLevel("custom") == Message::Level::NoIcon);

   return resVec_testStringToLevel;
}

std::vector<bool> Message_LevelToType()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testLevelToType;

   resVec_testLevelToType.push_back(Message::LevelToType(Message::Level::Critical).Contains("error"));
   resVec_testLevelToType.push_back(Message::LevelToType(Message::Level::Information).Contains("info"));
   resVec_testLevelToType.push_back(Message::LevelToType(Message::Level::NoIcon).Contains(""));
   resVec_testLevelToType.push_back(Message::LevelToType(Message::Level::Question).Contains("question"));
   resVec_testLevelToType.push_back(Message::LevelToType(Message::Level::Warning).Contains("warning"));

   return resVec_testLevelToType;
}

std::vector<bool> Message_TypeToLevel()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testTypeToLevel;

   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{"error"}) == Message::Level::Critical);
   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{"info"}) == Message::Level::Information);
   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{""}) == Message::Level::NoIcon);
   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{"question"}) == Message::Level::Question);
   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{"warning"}) == Message::Level::Warning);
   resVec_testTypeToLevel.push_back(Message::TypeToLevel(ut::log::TypeList{"custom"}) == Message::Level::NoIcon);

   return resVec_testTypeToLevel;
}

std::vector<bool> Message_EqualityOperator()
{
   using MapUtils::Message;

   std::vector<bool> resVec_testEqualityOperator;

   // create two default DerivedMessageForTests
   DerivedMessageForTest testLhs;
   DerivedMessageForTest testRhs;

   // test equality operator
   resVec_testEqualityOperator.push_back(testLhs == testRhs);

   return resVec_testEqualityOperator;

   // create a Message::Body
   Message::Body body{
      "This message tests the equality operator.",
      std::vector<Message::Body>{
         Message::Body{"bool operator==(const Message& aLhs, const Message& aRhs;", {}},
         Message::Body{"The operator== for Message uses the operator== for Body.",
                       std::vector<Message::Body>{Message::Body{"bool operator==(const Body& aLhs, const Body& aRhs);", {}},
                                                  Message::Body{"This operator uses recursion ", {}}}},
      }};
   testRhs.SetBody(body);

   // test equality operator
   if (testLhs != testRhs)
   {
      resVec_testEqualityOperator.push_back(true);
      resVec_testEqualityOperator.push_back(testLhs.GetLevel() == testRhs.GetLevel());
      resVec_testEqualityOperator.push_back(testLhs.GetTitle() == testRhs.GetTitle());
      resVec_testEqualityOperator.push_back(testLhs.GetBody() != testRhs.GetBody());
      resVec_testEqualityOperator.push_back(testLhs.GetTextFormat() == testRhs.GetTextFormat());
      resVec_testEqualityOperator.push_back(testLhs.GetShowType() == testRhs.GetShowType());
   }
   else
   {
      resVec_testEqualityOperator.push_back(false);
   }

   return resVec_testEqualityOperator;
}

} // namespace message

bool CheckTest(const std::vector<bool>& aTestResults)
{
   for (auto res : aTestResults)
   {
      if (res == false)
      {
         return false;
      }
   }
   return true;
}

void RunTests()
{
   using MapUtils::Message;

   std::vector<std::pair<std::string, std::vector<bool> (*)()>> testFns;
   testFns.push_back({"Message_Constructor1", &message::Message_Constructor1});
   testFns.push_back({"Message_Constructor2", &message::Message_Constructor2});
   testFns.push_back({"Message_Constructor3", &message::Message_Constructor3});
   testFns.push_back({"Message_Constructor4", &message::Message_Constructor4});
   testFns.push_back({"Message_Constructor5", &message::Message_Constructor5});
   testFns.push_back({"Message_Constructor6", &message::Message_Constructor6});
   testFns.push_back({"Message_CopyConstructor", &message::Message_CopyConstructor});
   testFns.push_back({"Message_MoveConstructor", &message::Message_MoveConstructor});
   testFns.push_back({"Message_Destructor", &message::Message_Destructor});
   testFns.push_back({"Message_SetGetLevel", &message::Message_SetGetLevel});
   testFns.push_back({"Message_SetGetTitle", &message::Message_SetGetTitle});
   testFns.push_back({"Message_SetGetBody", &message::Message_SetGetBody});
   testFns.push_back({"Message_SetGetFormat", &message::Message_SetGetFormat});
   testFns.push_back({"Message_SetGetShowType", &message::Message_SetGetShowType});
   testFns.push_back({"Message_Display", &message::Message_Display});
   testFns.push_back({"Message_BodyToString", &message::Message_BodyToString});
   testFns.push_back({"Message_StringToBody", &message::Message_StringToBody});
   testFns.push_back({"Message_BodyToLogMessage", &message::Message_BodyToLogMessage});
   testFns.push_back({"Message_LogMessageToBody", &message::Message_LogMessageToBody});
   testFns.push_back({"Message_LevelToString", &message::Message_LevelToString});
   testFns.push_back({"Message_StringToLevel", &message::Message_StringToLevel});
   testFns.push_back({"Message_LevelToString", &message::Message_LevelToType});
   testFns.push_back({"Message_StringToLevel", &message::Message_TypeToLevel});
   testFns.push_back({"Message_EqualityOperator", &message::Message_EqualityOperator});

   for (auto elem : testFns)
   {
      if (!CheckTest(elem.second()))
      {
         std::cout << "TEST \"" + elem.first + "\" FAILED" << std::endl;
      }
      else
      {
         std::cout << "TEST \"" + elem.first + "\" PASSED" << std::endl;
      }
   }
}

} // namespace test
