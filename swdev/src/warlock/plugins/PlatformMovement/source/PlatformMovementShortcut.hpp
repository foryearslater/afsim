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
#ifndef PLATFORMMOVEMENTSHORTCUT_HPP
#define PLATFORMMOVEMENTSHORTCUT_HPP

#include <QObject>

#include "PlatformMovementSimInterface.hpp"
#include "VaPosition.hpp"
#include "WkfViewer.hpp"

namespace vespa
{
class VaViewer;
}

namespace WkPlatformMovement
{
class Shortcut
{
public:
   Shortcut(SimInterface& aSimInterface, QObject* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());

   //! Initializing Turn To Heading user interface.
   void StartHeading();

   //! Initializes Go To Location user interface.
   void StartGoto();

private:
   //! If aSource == nullptr, uses PlatformOfInterest.
   //! Returns a pointer to the Rubberband that was created.
   //! Returns nullptr on failure.
   wkf::AttachmentRubberband* Start(void (Shortcut::*aFinishCB)(const wkf::AttachmentRubberband&));

   //! Required for Rubberband's interface.
   void RubberbandUpdate(wkf::AttachmentRubberband& aRubberband);

   //! Called when a Rubberband finishes.
   //! Calls TurnToHeading.
   void FinishHeading(const wkf::AttachmentRubberband& aRubberband);

   //! Called when a Rubberband finishes.
   //! Calls GotoLocation.
   void FinishGoto(const wkf::AttachmentRubberband& aRubberband);

   //! Helper function for issuing Turn To Heading commands.
   void TurnToHeading(double aHeadingDegrees) const;

   //! Helper function for issuing Go To Location commands.
   void GoToLocation(const vespa::VaPosition& aLocation) const;

   //! The name of the platform that the Rubberband originates from.
   std::string mSourceName;

   SimInterface& mSimInterface;
};
} // namespace WkPlatformMovement


#endif
