// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WSFSCRIPTPROMPT_HPP
#define WSFSCRIPTPROMPT_HPP

#include "UtScriptClass.hpp"
#include "UtScriptClassDefine.hpp"

class WsfScriptPromptClass : public UtScriptClass
{
public:
   using ScriptButtons = std::vector<std::pair<std::string, std::string>>;

   struct PromptData
   {
      std::string   platformName;
      std::string   title;
      std::string   message;
      ScriptButtons scriptButtons;
      bool          block;
   };

   WsfScriptPromptClass(const std::string& aClassName, UtScriptTypes* aScriptTypesPtr);

   UT_DECLARE_SCRIPT_METHOD(Display_1);
   UT_DECLARE_SCRIPT_METHOD(Display_2);
   UT_DECLARE_SCRIPT_METHOD(DisplayAndPause_1);
   UT_DECLARE_SCRIPT_METHOD(DisplayAndPause_2);
};

#endif
