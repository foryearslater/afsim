// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKINTERACTIONSSIMEVENTS_HPP
#define WKINTERACTIONSSIMEVENTS_HPP

#include <string>

#include "WkSimInterface.hpp"

namespace wkf
{
class InteractionPrefObject;
}

namespace vespa
{
class VaViewer;
}

namespace WkInteractions
{
class InteractionEvent : public warlock::SimEvent
{
public:
   InteractionEvent(size_t             aSourcePlatformIndex,
                    size_t             aTargetPlatformIndex,
                    bool               aStart,
                    const std::string& aType,
                    unsigned int       aId,
                    const std::string& aAuxText = "")
      : mSourcePlatformIndex(aSourcePlatformIndex)
      , mTargetPlatformIndex(aTargetPlatformIndex)
      , mStart(aStart)
      , mType(aType)
      , mId(aId)
      , mAuxText(aAuxText)
   {
   }

   bool Process(vespa::VaViewer& aViewer, const wkf::InteractionPrefObject* aPrefObjectPtr);

protected:
   size_t       mSourcePlatformIndex;
   size_t       mTargetPlatformIndex;
   bool         mStart;   // True if adding interaction, false if removing interaction
   std::string  mType;    // Type of interaction (see wkf::AttachmentInteraction)
   unsigned int mId;      // A (possibly unique) identifier for this interaction, to allow it to be removed later
   std::string  mAuxText; // Auxiliary text to display when hovering over interaction line
};
} // namespace WkInteractions

#endif
