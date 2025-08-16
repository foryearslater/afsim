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

#include "CommentDataContainer.hpp"

WkComment::CommentData::CommentData()
   : time(0)
   , message("")
{
}

WkComment::CommentData::CommentData(double aTime, const std::string& aMessage)
   : time(aTime)
   , message(aMessage)
{
}

void WkComment::DataContainer::Set(unsigned int aUniqueId, double aTime, const std::string& aMessage)
{
   mComments[aUniqueId] = CommentData(aTime, aMessage);
}

void WkComment::DataContainer::Add(unsigned int aUniqueId, double aTime, const std::string& aMessage)
{
   mComments.emplace(aUniqueId, CommentData(aTime, aMessage));
}

void WkComment::DataContainer::Remove(unsigned int aUniqueId)
{
   mComments.erase(aUniqueId);
}

void WkComment::DataContainer::Clear()
{
   mComments.clear();
}
