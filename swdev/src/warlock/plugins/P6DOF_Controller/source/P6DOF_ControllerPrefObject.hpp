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

#ifndef P6DOF_CONTROLLERPREFOBJECT_HPP
#define P6DOF_CONTROLLERPREFOBJECT_HPP

#include "WkfPrefObject.hpp"

class QSettings;

namespace WkP6DOF_Controller
{
struct PrefData
{
   bool mShowHud{true};
   bool mFirstPerson{true};
   bool mHDD{true};
   bool mAutopilot{false};
   bool mManualPilot{true};
   bool mSyntheticPilot{false};
   bool mGuidancePilot{false};
   int  mVerticalFieldOfView{90};

   unsigned int mResX{960};
   unsigned int mResY{540};
};

// The WkP6DOF_Controller::PrefObject provides the preferences object with
// which the users can set their preferences for the P6DofController plug-in.
class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   explicit PrefObject(QObject* parent = nullptr);

   bool GetShowHud() const { return mCurrentPrefs.mShowHud; }
   void SetShowHud(bool aShow);

   bool GetFirstPerson() const { return mCurrentPrefs.mFirstPerson; }
   void SetFirstPerson(bool aFirstPerson);

   bool GetHDD() const { return mCurrentPrefs.mHDD; }
   void SetHDD(bool aHDD);

   bool GetAutopilotMode() const { return mCurrentPrefs.mAutopilot; }
   void SetAutopilotMode(bool mAutopilotMode);

   bool GetManualPilotMode() const { return mCurrentPrefs.mManualPilot; }
   void SetManualPilotMode(bool mManualPilotMode);

   bool GetSyntheticPilotMode() const { return mCurrentPrefs.mSyntheticPilot; }
   void SetSyntheticPilotMode(bool mSyntheticPilotMode);

   bool GetGuidancePilotMode() const { return mCurrentPrefs.mGuidancePilot; }
   void SetGuidancePilotMode(bool mGuidancePilotMode);

   unsigned int GetResolutionX() const { return mCurrentPrefs.mResX; }
   unsigned int GetResolutionY() const { return mCurrentPrefs.mResY; }
   void         SetResolution(unsigned int aResX, unsigned int aResY);

signals:
   void ShowHudChanged(bool aShow);
   void ShowCockpitChanged(bool aShow);
   void ShowKnotsChanged(bool aState);
   void ShowWeaponModeChanged(bool aState);
   void ShowFirstPersonChanged(bool aState);
   void ShowHDDChanged(bool aState);
   void AutopilotModeChanged(bool aState);
   void ManualPilotModeChanged(bool aState);
   void SyntheticPilotModeChanged(bool aState);
   void GuidancePilotModeChanged(bool aState);
   void ResolutionChanged(unsigned int aResX, unsigned int aResY);

private:
   void EmitAllSignals();

   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace WkP6DOF_Controller

Q_DECLARE_METATYPE(WkP6DOF_Controller::PrefData)
#endif
