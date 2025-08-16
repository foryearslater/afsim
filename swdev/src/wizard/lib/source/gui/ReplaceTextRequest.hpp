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
#ifndef VIREPLACETEXTREQUEST_HPP
#define VIREPLACETEXTREQUEST_HPP

#include "FindTextRequest.hpp"

namespace wizard
{
class ReplaceTextRequest : public FindTextRequest
{
public:
   ReplaceTextRequest()
      : FindTextRequest()
   {
   }

   std::string mReplaceText;
};
} // namespace wizard

#endif
