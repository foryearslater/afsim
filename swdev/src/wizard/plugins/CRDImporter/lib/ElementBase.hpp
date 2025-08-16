// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ELEMENTBASE_HPP
#define ELEMENTBASE_HPP

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CrdImporter
{
class CrdTransition;

class ElementBase
{
public:
   ElementBase();
   virtual ~ElementBase();

   ElementBase(const ElementBase& aRhs) = delete;
   ElementBase& operator=(const ElementBase& aRhs) = delete;

   long GetId() const;
   void SetId(long aId);

   const std::string& GetName() const;
   void               SetName(const std::string& aName);

   std::vector<std::unique_ptr<CrdTransition>>* GetTransitionList() const;
   void SetTransitionList(std::vector<std::unique_ptr<CrdTransition>>* aTransitions);

   size_t GetNextTraversalIndex() const;
   void   SetNextTraversalIndex(size_t aIndex);

private:
   long   mId{std::numeric_limits<long>::max()};
   size_t mNextTraversalIndex{0};

   std::string                                                  mName{};
   std::unique_ptr<std::vector<std::unique_ptr<CrdTransition>>> mTransitionList;
};
} // namespace CrdImporter

#endif
