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

#ifndef TEST_MESSAGE_HPP
#define TEST_MESSAGE_HPP

#include <vector>

#include "Message.hpp"

namespace test
{
namespace message
{
//! @brief derived class of MapUtils::Message used solely for unit testing purposes
class DerivedMessageForTest final : public MapUtils::Message
{
public:
   //! @note cannot default this because MapUtils::Message default constructor is deleted
   DerivedMessageForTest();
   ~DerivedMessageForTest() override = default;

   //! @brief sets the severity of the message
   //! @param aLevel is the severity with which to set the corresponding message
   //! @note this function exists ONLY for testing purposes!
   void TestSetLevel(const Level aLevel) noexcept;
};

//! @name conditional operators
//@{

bool operator==(const DerivedMessageForTest& aLhs, const DerivedMessageForTest& aRhs);
bool operator!=(const DerivedMessageForTest& aLhs, const DerivedMessageForTest& aRhs);

//@}

//! @name unit tests for the explicit constructors
//!@{

std::vector<bool> Message_Constructor1();
std::vector<bool> Message_Constructor2();
std::vector<bool> Message_Constructor3();
std::vector<bool> Message_Constructor4();

//@}

//! @name unit tests for copy constructor, move constructor, and destructor
//@{

std::vector<bool> Message_CopyConstructor();
std::vector<bool> Message_MoveConstructor();
std::vector<bool> Message_Destructor();

//@}

//! @name unit tests for the accessor and mutator methods
//@{

std::vector<bool> Message_SetGetLevel();
std::vector<bool> Message_SetGetTitle();
std::vector<bool> Message_SetGetBody();
std::vector<bool> Message_SetGetFormat();
std::vector<bool> Message_SetGetShowType();

//@}

//! @note not really sure how to test this
std::vector<bool> Message_Display();

//! @name unit tests for the static conversion methods
//@{

std::vector<bool> Message_StringToBody();
std::vector<bool> Message_BodyToString();
std::vector<bool> Message_BodyToLogMessage();
std::vector<bool> Message_LogMessageToBody();
std::vector<bool> Message_LevelToString();
std::vector<bool> Message_StringToLevel();
std::vector<bool> Message_LevelToType();
std::vector<bool> Message_TypeToLevel();

//@}

//! @name unit tests for comparison operators
//@{

//! @note this also tests the inequality operator because inequality is not(equality)
std::vector<bool> Message_EqualityOperator();

//@}
} // namespace message

bool CheckTest(const std::vector<bool>& aTestResults);

void RunTests();
} // namespace test

#endif // TEST_MESSAGE_HPP
