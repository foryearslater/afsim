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
#ifndef FINDTEXTREQUEST_HPP
#define FINDTEXTREQUEST_HPP

namespace wizard
{
class TextSource;

class FindTextRequest
{
public:
   enum SearchLocation
   {
      cACTIVE_FILE,
      cPROJECT,
      cDIRECTORIES
   };
   FindTextRequest()
      : mSourcePtr()
      , mSearchLocation(cPROJECT)
      , mCaseSensitive(false)
   {
   }
   //! The single source file to search if non-null
   wizard::TextSource* mSourcePtr;
   SearchLocation      mSearchLocation;
   std::string         mText;
   bool                mCaseSensitive;
};
} // namespace wizard

#endif
