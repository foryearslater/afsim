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

#include "ElementBase.hpp"

#include <iostream>
#include <limits>

#include "CrdTransition.hpp"

namespace CrdImporter
{
ElementBase::ElementBase() = default;

ElementBase::~ElementBase() = default;

const std::string& ElementBase::GetName() const
{
   return mName;
}

void ElementBase::SetName(const std::string& aName)
{
   mName = aName;
}

long ElementBase::GetId() const
{
   return mId;
}

void ElementBase::SetId(long aId)
{
   mId = aId;
}

std::vector<std::unique_ptr<CrdTransition>>* ElementBase::GetTransitionList() const
{
   return mTransitionList.get();
}

void ElementBase::SetTransitionList(std::vector<std::unique_ptr<CrdTransition>>* aTransitions)
{
   mTransitionList.reset(aTransitions);
}

size_t ElementBase::GetNextTraversalIndex() const
{
   return mNextTraversalIndex;
}

void ElementBase::SetNextTraversalIndex(size_t aIndex)
{
   mNextTraversalIndex = aIndex;
}
} // namespace CrdImporter
