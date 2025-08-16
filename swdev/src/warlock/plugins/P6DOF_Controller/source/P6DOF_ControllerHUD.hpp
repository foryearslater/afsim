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
#ifndef WKP6DOF_CONTROLLERHUD_HPP
#define WKP6DOF_CONTROLLERHUD_HPP

#include <osg/Group>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Transform>
#include <osg/ref_ptr>
#include <osgText/Font>
#include <osgText/Text>

#include "P6DOF_ControllerDataContainer.hpp"
#include "UtoRawShape.hpp"
#include "VaOverlay.hpp"

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::HUD is an overlay on the tether window that
// provides a head-up display (HUD), offering insights into the
// current performance and weapon states for the aircraft/weapon
class HUD : public vespa::VaOverlay
{
public:
   HUD();
   explicit HUD(const std::string& aName);
   HUD(const std::string& aName, vespa::VaOverlay::OverlayType aType, double aWindowFullFovY_deg);
   HUD(const HUD& aSrc);
   HUD& operator=(const HUD& aSrc) = delete;
   ~HUD() override;

   // Provides a clone of the HUD
   HUD* Clone() const override;

   // This initializes the HUD and its data
   bool Initialize() override;

   // This updates the HUD
   void Update() override;

   // This sets the FOV for the HUD
   void SetupHudProjection(float aFovY);

   // Access the data directly (no need for protection)
   P6DOF_ControllerDataContainer::PlatformData mPlatData;

   // This sets the HUD mode using the specified HUD mode enum
   void SetHudMode(P6DOF_ControllerDataContainer::eHudMode aMode) { mHudMode = aMode; }

   // This return the current HUD mode enum
   enum P6DOF_ControllerDataContainer::eHudMode GetHudMode() const { return mHudMode; }

   // Planned improvements:
   // void LockRadarTarget(double& aLockX, double& aLockY);
   // void RadarTargetDesignationCoords(double& aX, double& aY);
   // double RadarDisplayRange_nm() { return mRadarRng_nm; }

   // This is called to lock the target closest to the target designation symbol.
   // It returns true if the lock-on was successful, providing the name of the
   // locked platform in aLockedTargetName, otherwise it returns false.
   bool LockRadarTarget(std::string& aLockedTargetName);

protected:
   class RegionExtents
   {
   public:
      RegionExtents()
         : mX1(0.0)
         , mY1(0.0)
         , mX2(0.0)
         , mY2(0.0)
      {
      }

      RegionExtents(double aX1, double aY1, double aX2, double aY2)
         : mX1(aX1)
         , mY1(aY1)
         , mX2(aX2)
         , mY2(aY2)
      {
      }

      void SetExtents(double aX1, double aY1, double aX2, double aY2)
      {
         mX1 = aX1;
         mY1 = aY1;
         mX2 = aX2;
         mY2 = aY2;
      }

      double X1() const { return mX1; }

      double Y1() const { return mY1; }

      double X2() const { return mX2; }

      double Y2() const { return mY2; }

   protected:
      double mX1;
      double mY1;
      double mX2;
      double mY2;
   };

   void DrawDebugRegion(const RegionExtents& aExtents);

   void Draw(double aHUD_HalfWidth,
             double aHUD_HalfHeight,
             double aFullFovY_deg,
             double aLookAngle_Az_deg,
             double aLookAngle_El_deg);

   void DrawHeadingRegion(double aPixelsPerDegree);
   void DrawWaterlineSymbol();

   void DrawSpeedRegion();
   void DrawLeftRegion();
   void DrawLeftRegionNavWaypt();
   void DrawLeftRegionNavLanding();
   void DrawLeftRegionNavTaxi();
   void DrawLeftRegionFuel();
   void DrawLeftRegionEngines();
   void DrawLeftRegionGround();
   void DrawLeftRegionGun();
   void DrawLeftRegionMissile();
   void DrawLeftRegionCommonWeapon();
   void DrawLeftRegionTest();
   void DrawLeftRegionControls();
   void DrawLeftRegionWeapon();

   void DrawAlphaSymbol(osg::ref_ptr<osg::Group>& aGroup, double aX, double aY, double aWidth, double aHeight);

   void DrawAltitudeRegion();
   void DrawRightRegion();
   void DrawRightRegionNavWaypt();
   void DrawRightRegionNavLanding();
   void DrawRightRegionNavTaxi();
   void DrawRightRegionFuel();
   void DrawRightRegionEngines();
   void DrawRightRegionGround();
   void DrawRightRegionGun();
   void DrawRightRegionMissile();
   void DrawRightRegionTest();
   void DrawRightRegionControls();
   void DrawRightRegionWeaponTest();

   void DrawTargetBoxOnHud();

   std::string GetStringForHudMode() const;
   std::string GetPilotTypeString() const;
   std::string GetTimeToString(double aTimeRemaining_hr) const;

   void DrawRollPointer();
   void DrawVelocityVector();
   void DrawPitchLadder();
   void DrawWeaponSelected(std::string aWeaponString, bool aDrawX);
   void DrawBingo();
   void DrawShoot();

   void DrawCenterCircle();

   void DrawStencilBufferRegion(const RegionExtents& aExtents, int aStencilValue);
   void DrawStencilBufferRegion(const osg::ref_ptr<osg::Vec2Array> aArray, int aStencilValue);

   void DrawLftMFD(P6DOF_ControllerDataContainer::eMfdMode aMfdMode, bool aActive);
   void DrawRgtMFD(P6DOF_ControllerDataContainer::eMfdMode aMfdMode, bool aActive);

   void DrawMFD_Radar(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_RWR(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_IRST(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_FLIR(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_Datalink(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_Weapons(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_Fuel(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);
   void DrawMFD_Engines(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);

   void DrawCenterTextForUnsupportedDisplay(osg::ref_ptr<osg::Geode>&     aVectorGeode,
                                            bool                          aActive,
                                            osg::ref_ptr<osg::Vec4Array>& aGridColor,
                                            const std::string&            aString1,
                                            const std::string&            aString2);

   void DrawMfdHighlightBox(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive);

   void DrawRadarTarget(osg::ref_ptr<osg::Geode>&          aVectorGeode,
                        double                             aCx,
                        double                             aCy,
                        double                             aOwnshipHeading_deg,
                        double                             aTargetHeading_deg,
                        int                                aAlt_kft,
                        const osg::ref_ptr<osg::Vec4Array> aColor,
                        bool                               aPrimaryTgt   = false,
                        bool                               aSecondaryTgt = false);

   void DrawUFC();

   osg::ref_ptr<osg::Geode> CreateHudGeometry(osg::ref_ptr<osg::Vec2Array>       aArray,
                                              int                                aDrawMode,
                                              const osg::ref_ptr<osg::Vec4Array> aColor);

   osg::ref_ptr<osg::StateSet> GetStencilDrawWhereMatch();

private:
   void SetHudColors(float aHudRedColor, float aHudGreenColor, float aHudBlueColor, float aHudAlphaColor);

   // Alignment types include:  LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM,
   //                           CENTER_TOP, CENTER_CENTER, CENTER_BOTTOM,
   //                           RIGHT_TOP, RIGHT_CENTER, RIGHT_BOTTOM,
   //                           LEFT_BASE_LINE, CENTER_BASE_LINE, RIGHT_BASE_LINE,
   //                           LEFT_BOTTOM_BASE_LINE, CENTER_BOTTOM_BASE_LINE, and RIGHT_BOTTOM_BASE_LINE
   osg::ref_ptr<osg::Geode> AddTextItem(std::string                   aStr,
                                        float                         aXPos,
                                        float                         aYPos,
                                        int                           aFontSize,
                                        osg::ref_ptr<osg::Vec4Array>& aColor,
                                        osgText::Text::AlignmentType  aAlignment = osgText::Text::LEFT_BASE_LINE);

   // Alignment types include:  LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM,
   //                           CENTER_TOP, CENTER_CENTER, CENTER_BOTTOM,
   //                           RIGHT_TOP, RIGHT_CENTER, RIGHT_BOTTOM,
   //                           LEFT_BASE_LINE, CENTER_BASE_LINE, RIGHT_BASE_LINE,
   //                           LEFT_BOTTOM_BASE_LINE, CENTER_BOTTOM_BASE_LINE, and RIGHT_BOTTOM_BASE_LINE
   osg::ref_ptr<osg::Geode> AddTextItem(std::string                  aStr,
                                        float                        aXPos,
                                        float                        aYPos,
                                        int                          aFontSize,
                                        osgText::Text::AlignmentType aAlignment = osgText::Text::LEFT_BASE_LINE);

   void DrawCircle(double                        aCx,
                   double                        aCy,
                   double                        aRadius,
                   osg::ref_ptr<osg::Vec4Array>& aColor,
                   osg::ref_ptr<osg::Geode>&     aGeode,
                   int                           aNumPts = 20,
                   bool                          aFilled = false);

   osgText::Font*                     mFont               = nullptr;
   osg::ref_ptr<osg::Geode>           mHudGeode           = nullptr;
   osg::ref_ptr<osg::StateSet>        mHudState           = nullptr;
   osg::ref_ptr<osg::Vec4Array>       mHudColor           = nullptr;
   osg::ref_ptr<osg::Projection>      mHudProjection      = nullptr;
   osg::ref_ptr<osg::MatrixTransform> mHudModelViewMatrix = nullptr;

   UtoRawShape* mRawShapePtr        = nullptr;
   double       mHUD_HalfWidth      = 0.0;
   double       mHUD_HalfHeight     = 0.0;
   double       mWindowFullFovY_deg = 0.0;
   double       mHUDFullFovX_deg    = 0.0;
   double       mHUDFullFovY_deg    = 0.0;

   RegionExtents mHeadingRegion;
   RegionExtents mSpeedRegion;
   RegionExtents mLeftRegion;
   RegionExtents mAltitudeRegion;
   RegionExtents mRightRegion;

   bool mDrawMFDs = true;

   RegionExtents mMfdLftRegion;
   RegionExtents mMfdRgtRegion;
   RegionExtents mUfcRegion;

   float mHudRedColor     = 0.0f;
   float mHudGreenColor   = 1.0f;
   float mHudBlueColor    = 0.0f;
   float mHudAlphaColor   = 1.0f;
   bool  mDebugFlag       = false;
   float mPixelsPerDegree = 0.0f;

   P6DOF_ControllerDataContainer::eHudMode mHudMode = P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT;

   int   mFontSizeMult  = 1;
   float mLineWidthMult = 1.0f;

   osg::ref_ptr<osg::Vec2Array> mHudPolygonRegion = nullptr;

   bool mSlowBlink      = false;
   bool mMediumBlink    = false;
   bool mFastBlink      = false;
   bool mSuperFastBlink = false;

   double mRadarTgtDesignatorX = 0.0;
   double mRadarTgtDesignatorY = 0.0;
   int    mRadarRng_nm         = 20;

   double mMfdRadarTime_sec     = 0.0;
   bool   mMfdRadarUpdateActive = false;
   double mMfdRwrTime_sec       = 0.0;
   bool   mMfdRwrUpdateActive   = false;
   double mMfdIrstTime_sec      = 0.0;
   bool   mMfdIrstUpdateActive  = false;
   double mMfdFlirTime_sec      = 0.0;
   bool   mMfdFlirUpdateActive  = false;
};
} // namespace WkP6DOF_Controller

VA_DECLARE_OBJECT_TYPE(WkP6DOF_Controller::HUD)
#endif
