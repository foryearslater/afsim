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
#ifndef WKCOMMENTSIMEVENTS_HPP
#define WKCOMMENTSIMEVENTS_HPP

#include "CommentDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkComment
{
class CommentSimEvent : public warlock::SimEvent
{
public:
   virtual void Process(WkComment::DataContainer& aComments) = 0;
};

class SimIntializedEvent : public CommentSimEvent
{
public:
   SimIntializedEvent() = default;
   void Process(WkComment::DataContainer& aComments) override;
};

class AddEvent : public CommentSimEvent
{
public:
   AddEvent(double aTime, const std::string& aPlatformName, const std::string& aMessage);
   void Process(WkComment::DataContainer& aComments) override;

private:
   double      mCreationTime;
   std::string mPlatformName;
   std::string mMessage;
};
} // namespace WkComment

#endif
