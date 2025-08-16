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

#ifndef ASTROLABEEVENTINTERFACE_HPP
#define ASTROLABEEVENTINTERFACE_HPP

#include <memory>

#include <QObject>

class WsfOrbitalMissionSequence;

namespace wsfg
{
namespace spaceg
{

class MissionVerificationResults;

} // namespace spaceg
} // namespace wsfg

namespace WkAstrolabe
{

class DockWidget;

class EventInterface : public QObject
{
public:
   explicit EventInterface(DockWidget& aDockWidget, QObject* aParentPtr = nullptr)
      : QObject(aParentPtr)
      , mDockWidget(aDockWidget)
   {
   }

   void SetPlatformAvailability(const QString& aPlatformName, bool aAvailability);
   void SetMissionVerificationResults(const wsfg::spaceg::MissionVerificationResults& aResults);
   void FinishVerify(std::unique_ptr<WsfOrbitalMissionSequence> aSequencePtr);
   void FinishApply(std::unique_ptr<WsfOrbitalMissionSequence> aSequencePtr);

private:
   DockWidget& mDockWidget;
};

} // namespace WkAstrolabe

#endif // ASTROLABEEVENTINTERFACE_HPP
