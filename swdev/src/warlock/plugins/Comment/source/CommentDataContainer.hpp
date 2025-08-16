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

#ifndef COMMENTDATACONTAINER_HPP
#define COMMENTDATACONTAINER_HPP

#include <map>
#include <string>

namespace WkComment
{
struct CommentData
{
   CommentData();
   CommentData(double aTime, const std::string& aMessage);

   double      time;
   std::string message;
};

class DataContainer
{
public:
   const std::map<unsigned int, CommentData>& GetComments() const { return mComments; }

   void Set(unsigned int aUniqueId, double aTime, const std::string& aMessage);
   void Add(unsigned int aUniqueId, double aTime, const std::string& aMessage);
   void Remove(unsigned int aUniqueId);
   void Clear();

private:
   std::map<unsigned int, CommentData> mComments;
};
} // namespace WkComment


#endif
