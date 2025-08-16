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

#include "P6DOF_ControllerHUD.hpp"

#include <chrono>
#include <iostream>
#include <math.h>

#include <QApplication>
#include <QDesktopWidget>
#include <osg/Depth>
#include <osg/LineStipple>
#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>
#include <osg/PrimitiveSet>
#include <osg/StateSet>
#include <osg/Stencil>

#include "UtMath.hpp"
#include "UtPath.hpp"
#include "UtSphericalEarth.hpp"
#include "Uto.hpp"
#include "VaCamera.hpp"
#include "VaObserver.hpp"
#include "VaUtils.hpp"
#include "VaViewer.hpp"

// Set this to true when debugging
bool debugFlagHUD = false;

WkP6DOF_Controller::HUD::HUD()
   : HUD("hud", cOVERLAY_WORLD, 50.0)
{
}

WkP6DOF_Controller::HUD::HUD(const std::string& aName)
   : HUD(aName, cOVERLAY_WORLD, 50.0)
{
}

WkP6DOF_Controller::HUD::HUD(const std::string& aName, vespa::VaOverlay::OverlayType aType, double aWindowFullFovY_deg)
   : vespa::VaOverlay(aName, aType)
   , mWindowFullFovY_deg(aWindowFullFovY_deg)
{
}

WkP6DOF_Controller::HUD::HUD(const WkP6DOF_Controller::HUD& aSrc)
   : vespa::VaOverlay(aSrc)
   , mFont(nullptr)
   , mHudGeode(aSrc.mHudGeode)
   , mHudState(aSrc.mHudState)
   , mHudColor(aSrc.mHudColor)
   , mHudProjection(aSrc.mHudProjection)
   , mHudModelViewMatrix(aSrc.mHudModelViewMatrix)
   , mRawShapePtr(aSrc.mRawShapePtr)
   , mHUD_HalfWidth(aSrc.mHUD_HalfWidth)
   , mHUD_HalfHeight(aSrc.mHUD_HalfHeight)
   , mWindowFullFovY_deg(aSrc.mWindowFullFovY_deg)
   , mHUDFullFovX_deg(aSrc.mHUDFullFovX_deg)
   , mHUDFullFovY_deg(aSrc.mHUDFullFovY_deg)
   , mHudRedColor(aSrc.mHudRedColor)
   , mHudGreenColor(aSrc.mHudGreenColor)
   , mHudBlueColor(aSrc.mHudBlueColor)
   , mHudAlphaColor(aSrc.mHudAlphaColor)
   , mDebugFlag(aSrc.mDebugFlag)
   , mPixelsPerDegree(aSrc.mPixelsPerDegree)
   , mHudMode(aSrc.mHudMode)
{
}

WkP6DOF_Controller::HUD::~HUD()
{
   UnbindShape("hud");
}

WkP6DOF_Controller::HUD* WkP6DOF_Controller::HUD::Clone() const
{
   return new WkP6DOF_Controller::HUD(*this);
}

bool WkP6DOF_Controller::HUD::Initialize()
{
   bool initialized = false;

   UtoRawShape rawShapePtr;
   mRawShapePtr = dynamic_cast<UtoRawShape*>(BindShape("hud", rawShapePtr));
   if (mRawShapePtr == nullptr)
   {
      std::cout << "***** WARNING: Couldn't bind shape: hud" << std::endl;
   }

   if (mRawShapePtr != nullptr)
   {
      mRawShapePtr->SetColor(UtoColor(0, 0, 0, 128));
      SetVisible(true);
      initialized = true;
   }

   if (!mHudGeode)
   {
      mHudGeode = new osg::Geode;
   }

   if (!mHudState)
   {
      mHudState = new osg::StateSet;
      // Disable depth testing so geometry is draw regardless of depth values
      // of geometry already draw.
      mHudState->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
      mHudState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

      // Need to make sure this geometry is draw last. RenderBins are handled
      // in numerical order so set bin number to 11
      mHudState->setRenderBinDetails(11, "RenderBin");

      mHudGeode->setStateSet(mHudState);
   }

   if (!mHudProjection)
   {
      mHudProjection = new osg::Projection;
   }

   if (!mHudColor)
   {
      mHudColor = new osg::Vec4Array;
      mHudColor->push_back(osg::Vec4(mHudRedColor, mHudGreenColor, mHudBlueColor, mHudAlphaColor));
   }

   if (mHudProjection != nullptr && GetViewer() != nullptr)
   {
      SetupHudProjection(mWindowFullFovY_deg);

      if (mHudModelViewMatrix == nullptr)
      {
         mHudModelViewMatrix = new osg::MatrixTransform;
         mHudModelViewMatrix->setMatrix(osg::Matrix::identity());
         mHudProjection->addChild(mHudModelViewMatrix.get());
         mHudModelViewMatrix->addChild(mHudGeode.get());

         mRawShapePtr->GetTree()->addChild(mHudProjection.get());
      }
   }

   return initialized;
}

void WkP6DOF_Controller::HUD::SetupHudProjection(float aFovY)
{
   // Set window full FOV member attribute
   mWindowFullFovY_deg = aFovY;

   int viewHeight = GetViewer()->GetHeight() / QApplication::desktop()->devicePixelRatio();
   int viewWidth  = GetViewer()->GetWidth() / QApplication::desktop()->devicePixelRatio();

   mPixelsPerDegree = viewHeight / aFovY;
   double ratio     = (double)viewWidth / (double)viewHeight;

   // NOTE: The HUD uses an "extent" of roughly +/-14 deg off bore-sight in both the
   // vertical and horizontal directions. Consider a 1080 vertical window with a 44 deg
   // vertical FOV. In this case, the mPixelsPerDegree is roughly 24.5 pixel/deg. Hence,
   // the HUD extents are approx 343 pixels (14x24.5). We determine the number of pixels
   // for the current configuration based on the pixels per degree and the HUD vertical
   // extent.

   const double hudVertExtent_deg = 14.0;
   float        pixels            = mPixelsPerDegree * hudVertExtent_deg;
   pixels                         = (float)viewHeight * 0.5f;

   mHudProjection->setMatrix(osg::Matrix::ortho2D(-ratio * pixels, ratio * pixels, -pixels, pixels));
}

void WkP6DOF_Controller::HUD::Update()
{
   double lookAngle_Az_deg = 0.0;
   double lookAngle_El_deg = 0.0;

   double time = mPlatData.simTime;

   double result = fmod(time, 2.0);
   if (result < 1.0)
   {
      mSlowBlink = true;
   }
   else
   {
      mSlowBlink = false;
   }

   result = fmod(time, 1.0);
   if (result < 0.5)
   {
      mMediumBlink = true;
   }
   else
   {
      mMediumBlink = false;
   }

   result = fmod(time, 0.5);
   if (result < 0.25)
   {
      mFastBlink = true;
   }
   else
   {
      mFastBlink = false;
   }

   result = fmod(time, 0.2);
   if (result < 0.1)
   {
      mSuperFastBlink = true;
   }
   else
   {
      mSuperFastBlink = false;
   }

   vespa::VaCamera* camera = GetViewer()->GetCamera();
   if (camera)
   {
      double dist_m = 0.0;
      lookAngle_El_deg *= -1.0; // Convention is strange -- negate it
      camera->GetAzElDist(lookAngle_Az_deg, lookAngle_El_deg, dist_m);
   }

   // Set font size and line width, based on window dimensions
   int  height    = GetViewer()->GetHeight() / QApplication::desktop()->devicePixelRatio();
   int  width     = GetViewer()->GetWidth() / QApplication::desktop()->devicePixelRatio();
   bool valuesSet = false;

   if (height == 540)
   {
      if (width == 960)
      {
         mFontSizeMult  = 1;
         mLineWidthMult = 1.0f;
         valuesSet      = true;
      }
   }
   else if (height == 1080)
   {
      if ((width == 960) || (width == 1920))
      {
         mFontSizeMult  = 2;
         mLineWidthMult = 2.0f;
         valuesSet      = true;
      }
   }
   else if (height == 1200)
   {
      if ((width == 960) || (width = 1920))
      {
         mFontSizeMult  = 2;
         mLineWidthMult = 2.2f;
         valuesSet      = true;
      }
   }
   else if (height == 2160)
   {
      if ((width == 1920) || (width == 3840))
      {
         mFontSizeMult  = 4;
         mLineWidthMult = 4.0f;
         valuesSet      = true;
      }
   }

   Draw(0.5 * (double)width, 0.5 * (double)height, mWindowFullFovY_deg, lookAngle_Az_deg, lookAngle_El_deg);

   if (!valuesSet)
   {
      std::cout << "*********************************************************************************" << std::endl;
      std::cout << "  ERROR:  P6DOF_Controller does not support a window resolution of " << width << "x" << height
                << std::endl;
      std::cout << "*********************************************************************************" << std::endl
                << std::endl;
   }
}

void WkP6DOF_Controller::HUD::Draw(double aHUD_HalfWidth,
                                   double aHUD_HalfHeight,
                                   double aFullFovY_deg,
                                   double aLookAngle_Az_deg,
                                   double aLookAngle_El_deg)
{
   double ppd = mPixelsPerDegree;

   // Clear out the geodes drawables
   mHudGeode->removeDrawables(0, mHudGeode->getNumDrawables());

   // Set regions
   mHeadingRegion.SetExtents(-8.0 * ppd, 5.0 * ppd, 8.0 * ppd, 7.0 * ppd);

   mSpeedRegion.SetExtents(-8.0 * ppd, 0.5 * ppd, -5.0 * ppd, 2.0 * ppd);
   mLeftRegion.SetExtents(-9.0 * ppd, -11.0 * ppd, -5.0 * ppd, 0.5 * ppd);

   mAltitudeRegion.SetExtents(5.0 * ppd, 0.5 * ppd, 9.0 * ppd, 2.0 * ppd);
   mRightRegion.SetExtents(5.0 * ppd, -11.0 * ppd, 9.0 * ppd, 0.5 * ppd);

   mMfdLftRegion.SetExtents(-36.0 * ppd, -20.0 * ppd, -16.0 * ppd, 0.0 * ppd);
   mMfdRgtRegion.SetExtents(16.0 * ppd, -20.0 * ppd, 36.0 * ppd, 0.0 * ppd);
   mUfcRegion.SetExtents(-14.0 * ppd, -22.0 * ppd, 14.0 * ppd, -14.0 * ppd);

   // Set HUD background color
   osg::ref_ptr<osg::Vec4Array> hudBackgroundColor(new osg::Vec4Array);
   hudBackgroundColor->push_back(osg::Vec4(0.0, 0.15, 0.0, 0.55));

   osg::ref_ptr<osg::Vec4Array> hudEdgeColor;
   hudEdgeColor = new osg::Vec4Array;
   hudEdgeColor->push_back(osg::Vec4(0.0f, 0.6f, 0.1f, 0.65f));

   // HUD Outer Border
   osg::ref_ptr<osg::Vec2Array> vertsHudEdge(new osg::Vec2Array);
   vertsHudEdge->push_back(osg::Vec2d(5.0 * ppd, 12.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(11.0 * ppd, 6.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(11.0 * ppd, -12.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(7.0 * ppd, -14.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(-7.0 * ppd, -14.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(-11.0 * ppd, -12.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(-11.0 * ppd, 6.0 * ppd));
   vertsHudEdge->push_back(osg::Vec2d(-5.0 * ppd, 12.0 * ppd));

   mHudGeode->addChild(CreateHudGeometry(vertsHudEdge, GL_POLYGON, hudBackgroundColor));
   mHudGeode->addChild(CreateHudGeometry(vertsHudEdge, GL_LINE_LOOP, hudEdgeColor).get());

   // HUD Inner Border
   if (mHudPolygonRegion == nullptr)
   {
      mHudPolygonRegion = new osg::Vec2Array;
   }
   else
   {
      mHudPolygonRegion->clear();
   }

   mHudPolygonRegion->push_back(osg::Vec2d(4.0 * ppd, 9.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(10.0 * ppd, 6.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(10.0 * ppd, -11.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(6.0 * ppd, -13.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(-6.0 * ppd, -13.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(-10.0 * ppd, -11.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(-10.0 * ppd, 6.0 * ppd));
   mHudPolygonRegion->push_back(osg::Vec2d(-4.0 * ppd, 9.0 * ppd));

   // NOTE: This flag can be used for testing. It should be false for release.
   mDebugFlag = false;

   if (mDebugFlag)
   {
      osg::ref_ptr<osg::Vec4Array> debugColor(new osg::Vec4Array);
      debugColor->push_back(osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f));
      mHudGeode->addChild(CreateHudGeometry(mHudPolygonRegion, GL_LINE_LOOP, debugColor));

      DrawDebugRegion(mHeadingRegion);
      DrawDebugRegion(mSpeedRegion);
      DrawDebugRegion(mAltitudeRegion);
      DrawDebugRegion(mLeftRegion);
      DrawDebugRegion(mRightRegion);
   }

   // Start Heading Region -----------------------------------------
   DrawStencilBufferRegion(mHeadingRegion, 1);
   DrawHeadingRegion(mPixelsPerDegree);
   DrawStencilBufferRegion(mHeadingRegion, 0);
   // End Heading Region

   // Draw Waterline and velocity vector ---------------------------
   DrawWaterlineSymbol();
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawVelocityVector();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);

   // Draw CenterCircle --------------------------------------------
   DrawCenterCircle();

   // Start Speed Region -------------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawSpeedRegion();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Speed Region

   // Start Altitude Region ----------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawAltitudeRegion();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Altitude Region

   // Start Left Region --------------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawLeftRegion();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Left Region

   // Start Right Region -------------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawRightRegion();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Right Region

   // Start Pitch Ladder -------------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawStencilBufferRegion(mHeadingRegion, 0);  // Clear-out the heading region
   DrawStencilBufferRegion(mSpeedRegion, 0);    // Clear-out the speed region
   DrawStencilBufferRegion(mAltitudeRegion, 0); // Clear-out the altitude region
   DrawStencilBufferRegion(mLeftRegion, 0);     // Clear-out the left region
   DrawStencilBufferRegion(mRightRegion, 0);    // Clear-out the right region
   DrawPitchLadder();
   DrawRollPointer();

   // Draw selected weapon data
   if (!mPlatData.activeWeapon.empty())
   {
      std::string weapnStr = mPlatData.activeWeapon;
      DrawWeaponSelected(weapnStr, !mPlatData.masterArmEnabled);
   }

   // Draw bingo warning
   if (mPlatData.bingoFlagActive)
   {
      DrawBingo();
   }

   // Draw shoot flag
   if (mPlatData.shootFlagActive)
   {
      DrawShoot();
   }

   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Pitch Ladder

   // Start Target Box ---------------------------------------------
   DrawStencilBufferRegion(mHudPolygonRegion, 1);
   DrawTargetBoxOnHud();
   DrawStencilBufferRegion(mHudPolygonRegion, 0);
   // End Target Box

   if (mDrawMFDs)
   {
      bool MFD1_IsActive = true;
      bool MFD2_IsActive = false;

      if (mPlatData.mfdActive == 1)
      {
         MFD1_IsActive = false;
         MFD2_IsActive = true;
      }

      bool noSensorsActive = false;

      switch (mPlatData.mfdLftMode)
      {
      case P6DOF_ControllerDataContainer::eMFD_RADAR:
         // mMfdRadarUpdateActive = true;
         mMfdRwrUpdateActive  = false;
         mMfdIrstUpdateActive = false;
         mMfdFlirUpdateActive = false;
         break;
      case P6DOF_ControllerDataContainer::eMFD_RWR:
         mMfdRadarUpdateActive = false;
         // mMfdRwrUpdateActive   = true;
         mMfdIrstUpdateActive = false;
         mMfdFlirUpdateActive = false;
         break;
      case P6DOF_ControllerDataContainer::eMFD_IRST:
         mMfdRadarUpdateActive = false;
         mMfdRwrUpdateActive   = false;
         // mMfdIrstUpdateActive  = true;
         mMfdFlirUpdateActive = false;
         break;
      case P6DOF_ControllerDataContainer::eMFD_FLIR:
         mMfdRadarUpdateActive = false;
         mMfdRwrUpdateActive   = false;
         // mMfdIrstUpdateActive  = false;
         mMfdFlirUpdateActive = true;
         break;
      case P6DOF_ControllerDataContainer::eMFD_DATALINK:
         noSensorsActive = true;
         break;
      case P6DOF_ControllerDataContainer::eMFD_WEAPONS:
         noSensorsActive = true;
         break;
      case P6DOF_ControllerDataContainer::eMFD_FUEL:
         noSensorsActive = true;
         break;
      case P6DOF_ControllerDataContainer::eMFD_ENGINES:
         noSensorsActive = true;
         break;
      default:
         std::cout << "ERROR - Unknown MFD mode in WkP6DOF_Controller::HUD::DrawLftMFD()" << std::endl;
         noSensorsActive = true;
         break;
      }

      if (noSensorsActive)
      {
         mMfdRadarUpdateActive = false;
         mMfdRwrUpdateActive   = false;
         mMfdIrstUpdateActive  = false;
         mMfdFlirUpdateActive  = false;
      }

      // Lft MFD
      DrawStencilBufferRegion(mMfdLftRegion, 1); // Draw region
      DrawLftMFD(mPlatData.mfdLftMode, MFD1_IsActive);
      DrawStencilBufferRegion(mMfdLftRegion, 0); // Restore region

      // Rgt MFD
      DrawStencilBufferRegion(mMfdRgtRegion, 1); // Draw region
      DrawRgtMFD(mPlatData.mfdRgtMode, MFD2_IsActive);
      DrawStencilBufferRegion(mMfdRgtRegion, 0); // Restore region

      // UFC
      DrawStencilBufferRegion(mUfcRegion, 1); // Draw region
      DrawUFC();
      DrawStencilBufferRegion(mUfcRegion, 0); // Restore region
   }
}

void WkP6DOF_Controller::HUD::SetHudColors(float aHudRedColor, float aHudGreenColor, float aHudBlueColor, float aHudAlphaColor)
{
   mHudRedColor   = aHudRedColor;
   mHudGreenColor = aHudGreenColor;
   mHudBlueColor  = aHudBlueColor;
   mHudAlphaColor = aHudAlphaColor;
}

void WkP6DOF_Controller::HUD::DrawDebugRegion(const RegionExtents& aExtents)
{
   double                       hudHalfWidth  = 0;
   double                       hudHalfHeight = 0;
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);
   verts->push_back(osg::Vec2d(aExtents.X1() + hudHalfWidth, aExtents.Y1() + hudHalfHeight));
   verts->push_back(osg::Vec2d(aExtents.X2() + hudHalfWidth, aExtents.Y1() + hudHalfHeight));
   verts->push_back(osg::Vec2d(aExtents.X2() + hudHalfWidth, aExtents.Y2() + hudHalfHeight));
   verts->push_back(osg::Vec2d(aExtents.X1() + hudHalfWidth, aExtents.Y2() + hudHalfHeight));

   osg::ref_ptr<osg::Vec4Array> debugColor(new osg::Vec4Array);
   debugColor->push_back(osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f));

   mHudGeode->addChild(CreateHudGeometry(verts, GL_LINE_LOOP, debugColor).get());
}

void WkP6DOF_Controller::HUD::DrawWaterlineSymbol()
{
   float ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Geometry>  geom(new osg::Geometry);
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);
   verts->push_back(osg::Vec2d(-0.8 * ppd, 0.0));
   verts->push_back(osg::Vec2d(-0.4 * ppd, 0.0));
   verts->push_back(osg::Vec2d(-0.2 * ppd, -0.3 * ppd));
   verts->push_back(osg::Vec2d(0.0 * ppd, 0.0));
   verts->push_back(osg::Vec2d(0.2 * ppd, -0.3 * ppd));
   verts->push_back(osg::Vec2d(0.4 * ppd, 0.0));
   verts->push_back(osg::Vec2d(0.8 * ppd, 0.0));
   osg::ref_ptr<osg::DrawArrays> lineArray(new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()));
   mHudGeode->addChild(CreateHudGeometry(verts, GL_LINE_STRIP, mHudColor).get());
}

void WkP6DOF_Controller::HUD::DrawVelocityVector()
{
   float ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Geode>     vectorGeode(new osg::Geode);
   osg::ref_ptr<osg::Group>     vectorGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   // Beta convention is opposite of alpha convention
   osg::ref_ptr<osg::PositionAttitudeTransform> vectorXform(new osg::PositionAttitudeTransform());
   vectorXform->setPosition(osg::Vec3d(mPlatData.beta_deg * ppd, -mPlatData.alpha_deg * ppd, 0.0));

   const int numPts = 12;
   double    dang   = 2.0 * 3.1415927 / (double)numPts;
   for (int i = 0; i < numPts; ++i)
   {
      double     ang = (double)i * dang;
      double     x   = 0.3 * ppd * cos(ang);
      double     y   = 0.3 * ppd * sin(ang);
      osg::Vec2d v(x, y);
      verts->push_back(v);
   }

   osg::ref_ptr<osg::DrawArrays> circleArray(new osg::DrawArrays(GL_LINE_LOOP, 0, verts->size()));
   osg::ref_ptr<osg::Vec2Array>  vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-0.6 * ppd, 0.0));
   vertArray->push_back(osg::Vec2d(-0.3 * ppd, 0.0));
   vertArray->push_back(osg::Vec2d(0.3 * ppd, 0.0));
   vertArray->push_back(osg::Vec2d(0.6 * ppd, 0.0));
   vertArray->push_back(osg::Vec2d(0.0, 0.3 * ppd));
   vertArray->push_back(osg::Vec2d(0.0, 0.6 * ppd));

   vectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINES, mHudColor).get());
   vectorGeode->addChild(CreateHudGeometry(verts, GL_LINE_LOOP, mHudColor).get());
   vectorXform->addChild(vectorGeode.get());
   vectorGroup->addChild(vectorXform.get());
   vectorGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(vectorGroup.get());
}

void WkP6DOF_Controller::HUD::DrawCenterCircle()
{
   if (mPlatData.shootFlagActive)
   {
      osg::ref_ptr<osg::Group>     circleGroup(new osg::Group);
      osg::ref_ptr<osg::Geometry>  geom(new osg::Geometry);
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

      for (int i = 0; i < 20; ++i)
      {
         double ang_rad = 2.0 * 3.1415927 * ((double)i / 20.0);
         double x       = 20.0 * cos(ang_rad);
         double y       = 20.0 * sin(ang_rad);
         vertArray->push_back(osg::Vec2d(x, y));
      }

      circleGroup->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, mHudColor).get());
      mHudGeode->addChild(circleGroup.get());
   }
}

void WkP6DOF_Controller::HUD::DrawStencilBufferRegion(const RegionExtents& aExtents, int aStencilValue)
{
   osg::ref_ptr<osg::StateSet> stencilStateSet(new osg::StateSet);
   osg::ref_ptr<osg::Stencil>  stencilStateAttribute(new osg::Stencil);
   ;

   stencilStateAttribute->setFunction(osg::Stencil::Function::ALWAYS,
                                      aStencilValue,
                                      aStencilValue); // Always passes, 1 bit plane, 1 as mask
   stencilStateAttribute->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE); // Set stencil to
                                                                                                       // 1 where we draw
   stencilStateSet->setAttributeAndModes(stencilStateAttribute.get(), osg::StateAttribute::ON);

   // Disable drawing into the color buffer
   osg::ref_ptr<osg::ColorMask> cMask(new osg::ColorMask);
   cMask->setMask(false, false, false, false); // Similar to glColorMask(0, 0, 0, 0);
   stencilStateSet->setAttributeAndModes(cMask.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

   // Disable the depth buffer
   mHudState->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

   // Draw region
   double x1 = aExtents.X1();
   double y1 = aExtents.Y1();
   double x2 = aExtents.X2();
   double y2 = aExtents.Y2();

   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   verts->push_back(osg::Vec2d(x1, y1));
   verts->push_back(osg::Vec2d(x2, y1));
   verts->push_back(osg::Vec2d(x2, y2));
   verts->push_back(osg::Vec2d(x1, y2));

   osg::ref_ptr<osg::Geode> stencilGeode = CreateHudGeometry(verts, GL_QUADS, mHudColor).get();
   stencilGeode->setStateSet(stencilStateSet.get());
   mHudGeode->addChild(stencilGeode.get());
}

void WkP6DOF_Controller::HUD::DrawStencilBufferRegion(const osg::ref_ptr<osg::Vec2Array> aArray, int aStencilValue)
{
   osg::ref_ptr<osg::StateSet> stencilStateSet(new osg::StateSet);
   osg::ref_ptr<osg::Stencil>  stencilStateAttribute(new osg::Stencil);
   ;

   stencilStateAttribute->setFunction(osg::Stencil::Function::ALWAYS,
                                      aStencilValue,
                                      aStencilValue); // Always passes, 1 bit plane, 1 as mask
   stencilStateAttribute->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE); // Set stencil to
                                                                                                       // 1 where we draw
   stencilStateSet->setAttributeAndModes(stencilStateAttribute.get(), osg::StateAttribute::ON);

   // Disable drawing into the color buffer
   osg::ref_ptr<osg::ColorMask> cMask(new osg::ColorMask);
   cMask->setMask(false, false, false, false); // Similar to glColorMask(0, 0, 0, 0);
   stencilStateSet->setAttributeAndModes(cMask.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

   // Disable the depth buffer
   mHudState->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

   // Draw region
   osg::ref_ptr<osg::Geode> stencilGeode = CreateHudGeometry(aArray, GL_POLYGON, mHudColor).get();
   stencilGeode->setStateSet(stencilStateSet.get());
   mHudGeode->addChild(stencilGeode.get());
}

osg::ref_ptr<osg::StateSet> WkP6DOF_Controller::HUD::GetStencilDrawWhereMatch()
{
   osg::ref_ptr<osg::StateSet> stencilStateSet(new osg::StateSet);
   osg::ref_ptr<osg::Stencil>  stencilStateAttribute(new osg::Stencil);
   ;

   stencilStateAttribute->setFunction(osg::Stencil::Function::EQUAL, 1, 1); // Always passes, 1 bit plane, 1 as mask
   stencilStateAttribute->setOperation(osg::Stencil::KEEP,
                                       osg::Stencil::KEEP,
                                       osg::Stencil::KEEP); // Set stencil to 1 where we draw

   stencilStateSet->setAttributeAndModes(stencilStateAttribute.get(), osg::StateAttribute::ON);
   osg::ref_ptr<osg::ColorMask> cMask(new osg::ColorMask);
   cMask->setMask(true, true, true, true);
   stencilStateSet->setAttributeAndModes(cMask.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
   return stencilStateSet;
}

void WkP6DOF_Controller::HUD::DrawHeadingRegion(double aPixelsPerDegree)
{
   double ppd = aPixelsPerDegree;

   int fontSize = 11 * mFontSizeMult;

   // Heading
   double tmpHdg_deg = mPlatData.heading_deg;

   if (tmpHdg_deg < 0.0)
   {
      tmpHdg_deg += 360.0;
   }
   if (tmpHdg_deg > 360.0)
   {
      tmpHdg_deg -= 360.0;
   }

   int hdg_deg = (int)(tmpHdg_deg + 0.5);
   if (hdg_deg == 0)
   {
      hdg_deg = 360.0;
   }
   if (hdg_deg > 360)
   {
      hdg_deg -= 360.0;
   }

   // We first draw the main heading "box"

   // Top heading text
   std::stringstream ss;
   ss << std::setw(3) << std::setfill('0') << hdg_deg;
   mHudGeode->addChild(AddTextItem(ss.str(), 0.0f, 6.3f * ppd, fontSize, osgText::Text::CENTER_CENTER).get());

   // Border around top heading
   osg::ref_ptr<osg::Group>     headingGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-1.0 * ppd, 5.8 * ppd));
   vertArray->push_back(osg::Vec2d(1.0 * ppd, 5.8 * ppd));
   vertArray->push_back(osg::Vec2d(1.0 * ppd, 6.8 * ppd));
   vertArray->push_back(osg::Vec2d(-1.0 * ppd, 6.8 * ppd));
   mHudGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, mHudColor).get());

   // Now, we prevent drawing into the box
   RegionExtents boxRegion;
   boxRegion.SetExtents(-1.1 * ppd, 5.7 * ppd, 1.1 * ppd, 6.9 * ppd);
   DrawStencilBufferRegion(boxRegion, 0);

   // Multiplier for moving the heading tape side-to-side, based on heading. Note that
   // we don't use the standard aPixelsPerDegree, we use the headingPixelsPerDegree
   double headingPixelsPerDegree = 0.5 * aPixelsPerDegree;

   // Set the font size for all other text for the heading region
   fontSize = 9 * mFontSizeMult;

   osg::ref_ptr<osg::PositionAttitudeTransform> xform(new osg::PositionAttitudeTransform());
   xform->setPosition(osg::Vec3d(-tmpHdg_deg * headingPixelsPerDegree, 6.3 * ppd, 0.0));

   // Heading numerals
   std::string curHdgStr = "";

   // This needs to have an extra 30 deg on either side
   for (int i = -4; i < 40; i++)
   {
      curHdgStr = "";
      if (i < 0)
      {
         int num = 36 + i;
         curHdgStr.append(std::to_string(num));
      }
      else if (i < 10)
      {
         curHdgStr.append("0");
         curHdgStr.append(std::to_string(i));
      }
      else if (i > 36)
      {
         int num = i - 36;
         curHdgStr.append("0");
         curHdgStr.append(std::to_string(num));
      }
      else
      {
         curHdgStr.append(std::to_string(i));
      }

      xform->addChild(
         AddTextItem(curHdgStr, (10.0 * i) * headingPixelsPerDegree, 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
   }

   // Tick lines

   // Center tick (Note that this does not use the xform)
   osg::ref_ptr<osg::Vec2Array> centerTickVerts(new osg::Vec2Array);
   centerTickVerts->push_back(osg::Vec2d(0.0f, 5.4 * ppd));
   centerTickVerts->push_back(osg::Vec2d(0.0f, 5.1 * ppd));
   mHudGeode->addChild(CreateHudGeometry(centerTickVerts, GL_LINES, mHudColor).get());

   // Heading bug
   if (mPlatData.headingBugValid)
   {
      // We draw the bug twice, since it may not be seen depending on the ownship heading

      // Standard draw
      osg::ref_ptr<osg::Vec2Array> hdgBugVerts1(new osg::Vec2Array);
      float                        bugX = (float)(headingPixelsPerDegree * mPlatData.headingBug_deg);
      hdgBugVerts1->push_back(osg::Vec2d(bugX - 0.2 * ppd, -1.2 * ppd));
      hdgBugVerts1->push_back(osg::Vec2d(bugX, -0.9 * ppd));
      hdgBugVerts1->push_back(osg::Vec2d(bugX + 0.2 * ppd, -1.2 * ppd));
      xform->addChild(CreateHudGeometry(hdgBugVerts1, GL_LINE_STRIP, mHudColor).get());

      // Draw with 360 deg offset
      osg::ref_ptr<osg::Vec2Array> hdgBugVerts2(new osg::Vec2Array);
      bugX = (float)(headingPixelsPerDegree * (mPlatData.headingBug_deg + 360.0));
      hdgBugVerts2->push_back(osg::Vec2d(bugX - 0.2 * ppd, -1.2 * ppd));
      hdgBugVerts2->push_back(osg::Vec2d(bugX, -0.9 * ppd));
      hdgBugVerts2->push_back(osg::Vec2d(bugX + 0.2 * ppd, -1.2 * ppd));
      xform->addChild(CreateHudGeometry(hdgBugVerts2, GL_LINE_STRIP, mHudColor).get());
   }

   // Multiple tick lines
   int                          count = 0;
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   for (int i = -60; i <= 400; i += 5)
   {
      double num  = (double)i;
      double xpos = num * headingPixelsPerDegree;
      if (count == 0)
      {
         verts->push_back(osg::Vec2d(xpos, -0.5 * ppd));
         verts->push_back(osg::Vec2d(xpos, -0.8 * ppd));
      }
      else if (count == 1)
      {
         verts->push_back(osg::Vec2d(xpos, -0.55 * ppd));
         verts->push_back(osg::Vec2d(xpos, -0.75 * ppd));
      }
      ++count;
      if (count > 1)
      {
         count = 0;
      }
   }
   xform->addChild(CreateHudGeometry(verts, GL_LINES, mHudColor).get());
   headingGroup->addChild(xform.get());

   headingGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(headingGroup.get());
}

// ---------------------------------------------------------------------------------------------------

void WkP6DOF_Controller::HUD::DrawSpeedRegion()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group>     speedGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

   vertArray->push_back(osg::Vec2(-8.0 * ppd, 0.5 * ppd));
   vertArray->push_back(osg::Vec2(-5.0 * ppd, 0.5 * ppd));
   vertArray->push_back(osg::Vec2(-5.0 * ppd, 2.0 * ppd));
   vertArray->push_back(osg::Vec2(-8.0 * ppd, 2.0 * ppd));

   speedGroup->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, mHudColor).get());

   int fontSizeLarger  = 13 * mFontSizeMult;
   int fontSizeSmaller = 8 * mFontSizeMult;

   std::stringstream kcas;
   kcas << std::setw(4) << (int)mPlatData.kcas;
   speedGroup->addChild(AddTextItem(kcas.str(), -5.3 * ppd, 1.25 * ppd, fontSizeLarger, osgText::Text::RIGHT_CENTER).get());

   std::stringstream ktas;
   ktas << "GS " << std::setw(4) << (int)mPlatData.ktas;
   speedGroup->addChild(AddTextItem(ktas.str(), -5.3 * ppd, 2.5 * ppd, fontSizeSmaller, osgText::Text::RIGHT_CENTER).get());

   speedGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(speedGroup.get());
}

void WkP6DOF_Controller::HUD::DrawAltitudeRegion()
{
   double ppd = mPixelsPerDegree;

   double sy = 80.0;

   osg::ref_ptr<osg::Group>     altitudeGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

   vertArray->push_back(osg::Vec2(5.0 * ppd, 0.5 * ppd));
   vertArray->push_back(osg::Vec2(9.0 * ppd, 0.5 * ppd));
   vertArray->push_back(osg::Vec2(9.0 * ppd, 2.0 * ppd));
   vertArray->push_back(osg::Vec2(5.0 * ppd, 2.0 * ppd));

   altitudeGroup->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, mHudColor).get());

   int fontSizeLarge  = 13 * mFontSizeMult;
   int fontSizeMiddle = 11 * mFontSizeMult;
   int fontSizeSmall  = 9 * mFontSizeMult;

   int ft  = (int)mPlatData.altitudeMSL_ft;
   int kft = ft / 1000;
   ft -= (kft * 1000);

   // Note: If altitude is >= 100,000 feet it will only display xx,xxx (hundred-thousands will not be shown)
   sy -= 5;
   if (kft >= 1)
   {
      std::stringstream kftstr;
      kftstr << std::setw(3) << kft;
      std::stringstream ftstr;
      ftstr << std::setw(3) << std::setfill('0') << ft;

      altitudeGroup->addChild(
         AddTextItem(kftstr.str(), 6.7 * ppd, 1.25 * ppd, fontSizeLarge, osgText::Text::RIGHT_CENTER).get());

      altitudeGroup->addChild(
         AddTextItem(ftstr.str(), 8.6 * ppd, 1.25 * ppd, fontSizeMiddle, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }
   else
   {
      std::stringstream ftstr;
      ftstr << std::setw(3) << ft;
      altitudeGroup->addChild(
         AddTextItem(ftstr.str(), 8.6 * ppd, 1.25 * ppd, fontSizeMiddle, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   std::stringstream vs;
   vs << "VS" << std::showpos << std::setw(6) << (int)mPlatData.verticalSpeed_fpm << std::noshowpos;
   altitudeGroup->addChild(
      AddTextItem(vs.str(), 8.6 * ppd, 2.5 * ppd, fontSizeSmall, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Planned improvement: (if desiring an explicit plus sign)
   //
   //   if (mPlatData.verticalSpeed_fpm > 0)
   //   {
   //      vs << "VS +" << std::setw(5) << (int)mPlatData.verticalSpeed_fpm;
   //      altitudeGroup->addChild(AddTextItem(vs.str(), 8.6*ppd, 1.5*ppd, fontSizeSmall,
   //      osgText::Text::AlignmentType::RIGHT_CENTER).get());
   //   }
   //   else
   //   {
   //      vs << "VS " << std::setw(6) << (int)mPlatData.verticalSpeed_fpm;
   //      altitudeGroup->addChild(AddTextItem(vs.str(), 8.6*ppd, 1.5*ppd, fontSizeSmall,
   //      osgText::Text::AlignmentType::RIGHT_CENTER).get());
   //   }

   // Planned improvement: (this will be for radar altitude, in addition to nominal baro altitude)
   //   std::stringstream ra;
   //   ra << "RA " << std::setw(6) << (int)mPlatData.altitudeMSL_ft;
   //   altitudeGroup->addChild(AddTextItem(ra.str(), sx, sy - 20.0, fontSizeSmall).get());

   altitudeGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(altitudeGroup.get());
}

void WkP6DOF_Controller::HUD::DrawLeftRegion()
{
   switch (mHudMode)
   {
   case P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT:
      DrawLeftRegionNavWaypt();
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_LAND:
      DrawLeftRegionNavLanding();
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_TAXI:
      DrawLeftRegionNavTaxi();
      break;
   case P6DOF_ControllerDataContainer::eHUD_FUEL:
      DrawLeftRegionFuel();
      break;
   case P6DOF_ControllerDataContainer::eHUD_ENG:
      DrawLeftRegionEngines();
      break;
   case P6DOF_ControllerDataContainer::eHUD_GND:
      DrawLeftRegionGround();
      break;
   case P6DOF_ControllerDataContainer::eHUD_GUN:
      DrawLeftRegionGun();
      break;
   case P6DOF_ControllerDataContainer::eHUD_MSL:
      DrawLeftRegionMissile();
      break;
   case P6DOF_ControllerDataContainer::eHUD_TEST:
      DrawLeftRegionTest();
      break;
   case P6DOF_ControllerDataContainer::eHUD_CONTROLS:
      DrawLeftRegionControls();
      break;
   case P6DOF_ControllerDataContainer::eHUD_WEAPON:
      DrawLeftRegionWeapon();
      break;
   case P6DOF_ControllerDataContainer::eHUD_UNDEFINED:
   default:
      // Do nothing
      break;
   }
}

void WkP6DOF_Controller::HUD::DrawLeftRegionNavWaypt()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   if (mPlatData.ecmXmit && mMediumBlink)
   {
      std::stringstream ecm;
      ecm << "ECM XMIT";
      leftGroup->addChild(AddTextItem(ecm.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream wp;
   wp << "NAV WAYPT";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream cm;
   cm << "C" << std::setw(3) << mPlatData.numChaff << " F" << std::setw(3) << mPlatData.numFlares;
   leftGroup->addChild(AddTextItem(cm.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionNavLanding()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   if (mPlatData.markerValid && mMediumBlink)
   {
      std::stringstream marker;
      marker << mPlatData.markerSymbol;
      leftGroup->addChild(AddTextItem(marker.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream wp;
   wp << "NAV LAND";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   double flaps_deg = mPlatData.flapLft_deg;
   if (mPlatData.flapRgt_deg > flaps_deg)
   {
      flaps_deg = mPlatData.flapRgt_deg;
   }
   if (flaps_deg > 0.0)
   {
      std::stringstream flaps;
      flaps << "FLAPS " << std::setw(2) << flaps_deg;
      leftGroup->addChild(AddTextItem(flaps.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionNavTaxi()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd * 2; // Two blank lines

   y -= ppd;
   std::stringstream wp;
   wp << "NAV TAXI";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   double flaps_deg = mPlatData.flapLft_deg;
   if (mPlatData.flapRgt_deg > flaps_deg)
   {
      flaps_deg = mPlatData.flapRgt_deg;
   }
   if (flaps_deg > 0.0)
   {
      std::stringstream flaps;
      flaps << "FLAPS " << std::setw(2) << flaps_deg;
      leftGroup->addChild(AddTextItem(flaps.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionFuel()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd * 2; // Two blank lines

   y -= ppd;
   std::stringstream wp;
   wp << "FUEL MODE";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   double flaps_deg = mPlatData.flapLft_deg;
   if (mPlatData.flapRgt_deg > flaps_deg)
   {
      flaps_deg = mPlatData.flapRgt_deg;
   }
   if (flaps_deg > 0.0)
   {
      std::stringstream flaps;
      flaps << "FLAPS " << std::setw(2) << flaps_deg;
      leftGroup->addChild(AddTextItem(flaps.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionEngines()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd * 2; // Two blank lines

   y -= ppd;
   std::stringstream wp;
   wp << "ENG MODE";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   double flaps_deg = mPlatData.flapLft_deg;
   if (mPlatData.flapRgt_deg > flaps_deg)
   {
      flaps_deg = mPlatData.flapRgt_deg;
   }
   if (flaps_deg > 0.0)
   {
      std::stringstream flaps;
      flaps << "FLAPS " << std::setw(2) << flaps_deg;
      leftGroup->addChild(AddTextItem(flaps.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionGround()
{
   DrawLeftRegionCommonWeapon();
}

void WkP6DOF_Controller::HUD::DrawLeftRegionGun()
{
   DrawLeftRegionCommonWeapon();
}

void WkP6DOF_Controller::HUD::DrawLeftRegionMissile()
{
   DrawLeftRegionCommonWeapon();
}

void WkP6DOF_Controller::HUD::DrawLeftRegionCommonWeapon()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   if (mPlatData.ecmXmit && mMediumBlink)
   {
      std::stringstream ecm;
      ecm << "ECM XMIT";
      leftGroup->addChild(AddTextItem(ecm.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sensorMode;
   sensorMode << mPlatData.radarMode;
   leftGroup->addChild(AddTextItem(sensorMode.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream cm;
   cm << "C" << std::setw(3) << mPlatData.numChaff << " F" << std::setw(3) << mPlatData.numFlares;
   leftGroup->addChild(AddTextItem(cm.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionTest()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ga;
   ga << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Allowable;
   leftGroup->addChild(AddTextItem(ga.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream gAvail;
   gAvail << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Available;
   leftGroup->addChild(AddTextItem(gAvail.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream gAttain;
   gAttain << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Attained;
   leftGroup->addChild(AddTextItem(gAttain.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream wp;
   wp << "TEST MODE";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   double flaps_deg = mPlatData.flapLft_deg;
   if (mPlatData.flapRgt_deg > flaps_deg)
   {
      flaps_deg = mPlatData.flapRgt_deg;
   }
   if (flaps_deg > 0.0)
   {
      std::stringstream flaps;
      flaps << "FLAPS " << std::setw(2) << flaps_deg;
      leftGroup->addChild(AddTextItem(flaps.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream ab;
   int               iAB = (int)(mPlatData.engineThrottleLeverAfterburner * 100.0);
   if (iAB > 0)
   {
      ab << "AB " << std::setw(3) << iAB;
      leftGroup->addChild(AddTextItem(ab.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   y -= ppd;
   std::stringstream sb;
   int               iSB = (int)mPlatData.speedBrakes_deg;
   if (iSB > 0)
   {
      sb << "SB " << std::setfill('0') << std::setw(2) << iSB;
      leftGroup->addChild(AddTextItem(sb.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());
   }

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionControls()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream stickRgt;
   stickRgt << "SR:" << std::fixed << std::setprecision(3) << std::setw(6) << mPlatData.stickRightControllerPosition;
   leftGroup->addChild(AddTextItem(stickRgt.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream stickBack;
   stickBack << "SB:" << std::fixed << std::setprecision(3) << std::setw(6) << mPlatData.stickBackControllerPosition;
   leftGroup->addChild(AddTextItem(stickBack.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream rudderRgt;
   rudderRgt << "RR:" << std::fixed << std::setprecision(3) << std::setw(6) << mPlatData.rudderRightControllerPosition;
   leftGroup->addChild(AddTextItem(rudderRgt.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream throttleMil;
   throttleMil << "TM:" << std::fixed << std::setprecision(3) << std::setw(6)
               << mPlatData.throttleMilitaryControllerPosition;
   leftGroup->addChild(AddTextItem(throttleMil.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream throttleAB;
   throttleAB << "TA:" << std::fixed << std::setprecision(3) << std::setw(6)
              << mPlatData.throttleAfterburnerControllerPosition;
   leftGroup->addChild(AddTextItem(throttleAB.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream speedBrake;
   speedBrake << "SB:" << std::fixed << std::setprecision(3) << std::setw(6) << mPlatData.speedBrakesControllerPosition;
   leftGroup->addChild(AddTextItem(speedBrake.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream wp;
   wp << "CONTROLS";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawLeftRegionWeapon()
{
   double ppd = mPixelsPerDegree;

   osg::ref_ptr<osg::Group> leftGroup(new osg::Group);

   int fontSize = 8 * mFontSizeMult;

   double y = -0.5 * ppd;

   DrawAlphaSymbol(leftGroup, -5.3 * ppd - 2.5 * ppd, y, ppd * 0.5, ppd * 0.5);
   std::stringstream alpha;
   alpha << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.alpha_deg;
   leftGroup->addChild(AddTextItem(alpha.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream beta;
   beta << "B" << std::fixed << std::setprecision(1) << std::setw(6) << mPlatData.beta_deg;
   leftGroup->addChild(AddTextItem(beta.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream mach;
   mach << "M " << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.mach;
   leftGroup->addChild(AddTextItem(mach.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream g;
   g << "G " << std::fixed << std::setprecision(1) << std::setw(5) << -mPlatData.nz_g;
   leftGroup->addChild(AddTextItem(g.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream gAvail;
   gAvail << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Available;
   leftGroup->addChild(AddTextItem(gAvail.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream gAttain;
   gAttain << std::fixed << std::setprecision(1) << std::setw(5) << mPlatData.maxG_Attained;
   leftGroup->addChild(AddTextItem(gAttain.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream powerCmd;
   powerCmd << "POW:" << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.throttleCommand;
   leftGroup->addChild(AddTextItem(powerCmd.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream yawCmd;
   yawCmd << "YAW:" << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.yawCommandG;
   leftGroup->addChild(AddTextItem(yawCmd.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream pitCmd;
   pitCmd << "PIT:" << std::fixed << std::setprecision(2) << std::setw(5) << mPlatData.pitchCommandG;
   leftGroup->addChild(AddTextItem(pitCmd.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   y -= ppd;
   std::stringstream wp;
   wp << "*WEAPON* ";
   leftGroup->addChild(AddTextItem(wp.str(), -5.3 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   leftGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(leftGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegion()
{
   switch (mHudMode)
   {
   case P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT:
      DrawRightRegionNavWaypt();
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_LAND:
      DrawRightRegionNavLanding();
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_TAXI:
      DrawRightRegionNavTaxi();
      break;
   case P6DOF_ControllerDataContainer::eHUD_FUEL:
      DrawRightRegionFuel();
      break;
   case P6DOF_ControllerDataContainer::eHUD_ENG:
      DrawRightRegionEngines();
      break;
   case P6DOF_ControllerDataContainer::eHUD_GND:
      DrawRightRegionGround();
      break;
   case P6DOF_ControllerDataContainer::eHUD_GUN:
      DrawRightRegionGun();
      break;
   case P6DOF_ControllerDataContainer::eHUD_MSL:
      DrawRightRegionMissile();
      break;
   case P6DOF_ControllerDataContainer::eHUD_TEST:
      DrawRightRegionTest();
      break;
   case P6DOF_ControllerDataContainer::eHUD_CONTROLS:
      DrawRightRegionControls();
      break;
   case P6DOF_ControllerDataContainer::eHUD_WEAPON:
      DrawRightRegionWeaponTest();
      break;
   case P6DOF_ControllerDataContainer::eHUD_UNDEFINED:
   default:
      // Do nothing
      break;
   }
}

void WkP6DOF_Controller::HUD::DrawAlphaSymbol(osg::ref_ptr<osg::Group>& aGroup, double aX, double aY, double aWidth, double aHeight)
{
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

   double dx = aWidth * 0.5;
   double dy = aHeight * 0.5;

   vertArray->push_back(osg::Vec2(aX + dx, aY + dy));
   vertArray->push_back(osg::Vec2(aX + 0.250 * dx, aY - 0.667 * dy));
   vertArray->push_back(osg::Vec2(aX - 0.250 * dx, aY - dy));
   vertArray->push_back(osg::Vec2(aX - 0.750 * dx, aY - 0.667 * dy));
   vertArray->push_back(osg::Vec2(aX - dx, aY));
   vertArray->push_back(osg::Vec2(aX - 0.750 * dx, aY + 0.667 * dy));
   vertArray->push_back(osg::Vec2(aX - 0.250 * dx, aY + dy));
   vertArray->push_back(osg::Vec2(aX + 0.250 * dx, aY + 0.667 * dy));
   vertArray->push_back(osg::Vec2(aX + dx, aY - dy));

   aGroup->addChild(CreateHudGeometry(vertArray, GL_LINE_STRIP, mHudColor).get());
}

void WkP6DOF_Controller::HUD::DrawRightRegionNavWaypt()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double            y = -0.5 * ppd;
   std::stringstream rdrAlt;
   rdrAlt << "RA " << std::setw(6) << (int)mPlatData.radarAltitude_ft;
   rightGroup->addChild(
      AddTextItem(rdrAlt.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd * 2; // Two blank lines

   if ((mPlatData.numWaypoints > 0) && (mPlatData.currentWaypoint < mPlatData.numWaypoints))
   {
      y -= ppd;
      std::stringstream waypt;
      waypt << "WPT " << std::setw(3) << (int)mPlatData.currentWaypoint;
      rightGroup->addChild(
         AddTextItem(waypt.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

      y -= ppd;
      if (mPlatData.headingBugValid)
      {
         int hdg = (int)mPlatData.headingBug_deg;
         if (hdg <= 0)
         {
            hdg += 360;
         }
         if (hdg > 360)
         {
            hdg -= 360;
         }

         std::stringstream wayHdg;
         wayHdg << "HDG " << std::setw(3) << hdg;
         rightGroup->addChild(
            AddTextItem(wayHdg.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }

      y -= ppd;
      if (mPlatData.wayptDataValid)
      {
         std::stringstream wayRng;
         wayRng << "RNG " << std::fixed << std::setw(5) << std::setprecision(1) << mPlatData.distanceToWaypoint_nm;
         rightGroup->addChild(
            AddTextItem(wayRng.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
         y -= ppd;
         std::string timeStr = GetTimeToString(mPlatData.timeToWaypoint_sec / 3600.0); // Convert to hours
         rightGroup->addChild(
            AddTextItem(timeStr, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }
   else
   {
      y -= ppd * 4;
   }

   y -= ppd * 2; // Two blank lines

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionNavLanding()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double            y = -0.5 * ppd;
   std::stringstream rdrAlt;
   rdrAlt << "RA " << std::setw(6) << (int)mPlatData.radarAltitude_ft;
   rightGroup->addChild(
      AddTextItem(rdrAlt.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd; // Blank line

   bool gearPresent = false;
   if (mPlatData.noseGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
   }
   if (mPlatData.leftGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
   }
   if (mPlatData.rightGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
   }

   y -= ppd;
   if (gearPresent)
   {
      std::string engage = "LO";
      if (mPlatData.nwsEnabled)
      {
         engage = "HI";
      }
      std::stringstream nws;
      nws << "NWS-" << engage << std::setw(3) << (int)mPlatData.nws_angle_deg;
      rightGroup->addChild(
         AddTextItem(nws.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   // Only draw if we have a non-zero compression
   if (mPlatData.noseGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
      std::stringstream ng;
      int               percentCompression =
         (int)(100.0 * mPlatData.noseGearCurrentCompression_ft / mPlatData.noseGearMaximumCompression_ft);
      if (percentCompression > 99)
      {
         ng << "NG **   _";
         rightGroup->addChild(
            AddTextItem(ng.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
      else
      {
         ng << "NG " << std::setw(2) << percentCompression << "   _";
         rightGroup->addChild(
            AddTextItem(ng.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }

   y -= ppd;
   // Only draw if we have a non-zero compression
   if (mPlatData.leftGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
      std::stringstream lg;
      int               percentCompression =
         (int)(100.0 * mPlatData.leftGearCurrentCompression_ft / mPlatData.leftGearMaximumCompression_ft);
      if (percentCompression > 99)
      {
         lg << "LG ** " << std::setw(3) << (int)(mPlatData.leftGearNormalizedBrakingValue * 100.0);
         rightGroup->addChild(
            AddTextItem(lg.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
      else
      {
         lg << "LG " << std::setw(2) << percentCompression << " " << std::setw(3)
            << (int)(mPlatData.leftGearNormalizedBrakingValue * 100.0);
         rightGroup->addChild(
            AddTextItem(lg.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }

   y -= ppd;
   // Only draw if we have a non-zero compression
   if (mPlatData.rightGearMaximumCompression_ft > 0.0)
   {
      gearPresent = true;
      std::stringstream rg;
      int               percentCompression =
         (int)(100.0 * mPlatData.rightGearCurrentCompression_ft / mPlatData.rightGearMaximumCompression_ft);
      if (percentCompression > 99)
      {
         rg << "RG ** " << std::setw(3) << (int)(mPlatData.rightGearNormalizedBrakingValue * 100.0);
         rightGroup->addChild(
            AddTextItem(rg.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
      else
      {
         rg << "RG " << std::setw(2) << percentCompression << " " << std::setw(3)
            << (int)(mPlatData.rightGearNormalizedBrakingValue * 100.0);
         rightGroup->addChild(
            AddTextItem(rg.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }

   y -= ppd;
   if (gearPresent)
   {
      std::stringstream gear;
      gear << "LGEAR " << std::setw(3) << (int)(mPlatData.landingGear_deg / 90.0);
      rightGroup->addChild(
         AddTextItem(gear.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (mPlatData.wayptDataValid)
   {
      std::stringstream wayRng;
      wayRng << "DST " << std::fixed << std::setw(5) << std::setprecision(1) << mPlatData.distanceToWaypoint_nm;
      rightGroup->addChild(
         AddTextItem(wayRng.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

      y -= ppd;
      std::string timeStr = GetTimeToString(mPlatData.timeToWaypoint_sec / 3600.0); // Convert to hours
      rightGroup->addChild(AddTextItem(timeStr, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }
   else
   {
      y -= ppd; // Blank line
   }

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionNavTaxi()
{
   // This is the same as for Nav-Landing mode
   DrawRightRegionNavLanding();
}

void WkP6DOF_Controller::HUD::DrawRightRegionFuel()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double y = -0.5 * ppd; // Blank line

   bool fuelCapable = false;
   if (mPlatData.internalTotalFuelTankCapacity_lbs > 0.0)
   {
      fuelCapable = true;
   }

   y -= ppd;
   if (fuelCapable)
   {
      std::stringstream intFuel;
      int               internalFuel_lbs = (int)mPlatData.currentInternalTotalFuelTankQuantity_lbs;
      intFuel << "IN " << std::setw(6) << internalFuel_lbs;
      rightGroup->addChild(
         AddTextItem(intFuel.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (fuelCapable)
   {
      std::stringstream extFuel;
      int               externalFuel_lbs =
         (int)(mPlatData.currentTotalFuelTankQuantity_lbs - mPlatData.currentInternalTotalFuelTankQuantity_lbs);
      extFuel << "EX " << std::setw(6) << externalFuel_lbs;
      rightGroup->addChild(
         AddTextItem(extFuel.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (fuelCapable)
   {
      std::stringstream totFuel;
      int               totalFuel_lbs = (int)mPlatData.currentTotalFuelTankQuantity_lbs;
      totFuel << "TF " << std::setw(6) << totalFuel_lbs;
      rightGroup->addChild(
         AddTextItem(totFuel.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (fuelCapable)
   {
      std::stringstream bingoFuel;
      int               bingoFuel_lbs = (int)mPlatData.bingoFuelQuantity_lbs;
      bingoFuel << "B   " << std::setw(5) << bingoFuel_lbs;
      rightGroup->addChild(
         AddTextItem(bingoFuel.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (fuelCapable)
   {
      std::stringstream fuelFlow;
      int               bingoFuel_lbs = (int)mPlatData.engineFuelFlow_pph;
      fuelFlow << "FF " << std::setw(6) << bingoFuel_lbs;
      rightGroup->addChild(
         AddTextItem(fuelFlow.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd;
   if (fuelCapable)
   {
      if (mPlatData.currentTotalFuelTankQuantity_lbs > 0.0)
      {
         if (mPlatData.engineFuelFlow_pph > 0.0)
         {
            double      time_hrs = mPlatData.currentTotalFuelTankQuantity_lbs / mPlatData.engineFuelFlow_pph;
            std::string timeStr  = GetTimeToString(time_hrs);
            rightGroup->addChild(
               AddTextItem(timeStr, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
         }
         else
         {
            // Infinite time
            std::stringstream timeStr;
            timeStr << "TXX:XX:XX";
            rightGroup->addChild(
               AddTextItem(timeStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
         }
      }
      else
      {
         std::stringstream timeStr;
         timeStr << "T00:00:00";
         rightGroup->addChild(
            AddTextItem(timeStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }

   y -= ppd;
   if (fuelCapable)
   {
      if (mPlatData.currentTotalFuelTankQuantity_lbs > 0.0)
      {
         if (mPlatData.engineFuelFlow_pph > 0.0)
         {
            double            time_hrs = mPlatData.currentTotalFuelTankQuantity_lbs / mPlatData.engineFuelFlow_pph;
            double            range_nm = time_hrs * mPlatData.ktas;
            std::stringstream rngStr;
            rngStr << "RNG " << std::setw(5) << (int)range_nm;
            rightGroup->addChild(
               AddTextItem(rngStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
         }
         else
         {
            // Infinite range
            std::stringstream rngStr;
            rngStr << "RNG XXXXX";
            rightGroup->addChild(
               AddTextItem(rngStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
         }
      }
      else
      {
         // Zero range - out of gas
         std::stringstream rngStr;
         rngStr << "RNG     0";
         rightGroup->addChild(
            AddTextItem(rngStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }

   y -= ppd; // Blank line

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionEngines()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double y = -0.5 * ppd; // Blank line

   y -= ppd;
   std::stringstream mil;
   mil << "M:" << std::setfill('0') << std::setw(3) << (int)(100.0 * mPlatData.engineThrottleLeverMilitary) << "|"
       << std::setfill('0') << std::setw(3) << (int)(100.0 * mPlatData.throttleMilitaryControllerPosition);
   rightGroup->addChild(AddTextItem(mil.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ab;
   ab << "A:" << std::setfill('0') << std::setw(3) << (int)(100.0 * mPlatData.engineThrottleLeverAfterburner) << "|"
      << std::setfill('0') << std::setw(3) << (int)(100.0 * mPlatData.throttleAfterburnerControllerPosition);
   rightGroup->addChild(AddTextItem(ab.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream lbs;
   lbs << "T " << std::setw(7) << (int)mPlatData.engineThrust_lbs;
   rightGroup->addChild(AddTextItem(lbs.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream qty;
   qty << "TF " << std::setw(6) << (int)mPlatData.currentTotalFuelTankQuantity_lbs;
   rightGroup->addChild(AddTextItem(qty.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream bingoFuel;
   int               bingoFuel_lbs = (int)mPlatData.bingoFuelQuantity_lbs;
   bingoFuel << "B   " << std::setw(5) << bingoFuel_lbs;
   rightGroup->addChild(
      AddTextItem(bingoFuel.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream pph;
   pph << "FF " << std::setw(6) << (int)mPlatData.engineFuelFlow_pph;
   rightGroup->addChild(AddTextItem(pph.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   if (mPlatData.currentTotalFuelTankQuantity_lbs > 0.0)
   {
      if (mPlatData.engineFuelFlow_pph > 0.0)
      {
         double      time_hrs = mPlatData.currentTotalFuelTankQuantity_lbs / mPlatData.engineFuelFlow_pph;
         std::string timeStr  = GetTimeToString(time_hrs);
         rightGroup->addChild(
            AddTextItem(timeStr, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
      else
      {
         // Infinite time
         std::stringstream timeStr;
         timeStr << "TXX:XX:XX";
         rightGroup->addChild(
            AddTextItem(timeStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
      }
   }
   else
   {
      std::stringstream timeStr;
      timeStr << "T00:00:00";
      rightGroup->addChild(
         AddTextItem(timeStr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd; // Blank line

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionGround()
{
   // Planned improvement
}

void WkP6DOF_Controller::HUD::DrawRightRegionGun()
{
   // Planned improvement
}

void WkP6DOF_Controller::HUD::DrawRightRegionMissile()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double            y = -0.5 * ppd;
   std::stringstream jam;
   if (mPlatData.jammingDetected)
   {
      jam << "**JAM**" << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.aileronLft_deg;
      rightGroup->addChild(
         AddTextItem(jam.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd * 2.0; // Two blank lines

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionTest()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double y = -0.5 * ppd; // Blank line

   y -= ppd;
   std::stringstream nx;
   nx << "NX " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.nx_g;
   rightGroup->addChild(AddTextItem(nx.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ny;
   ny << "NY " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.ny_g;
   rightGroup->addChild(AddTextItem(ny.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream nz;
   nz << "NZ " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.nz_g;
   rightGroup->addChild(AddTextItem(nz.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream rollRate;
   rollRate << "P " << std::fixed << std::setw(7) << std::setprecision(2) << mPlatData.rollRate_dps;
   rightGroup->addChild(
      AddTextItem(rollRate.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream pitchRate;
   pitchRate << "Q " << std::fixed << std::setw(7) << std::setprecision(2) << mPlatData.pitchRate_dps;
   rightGroup->addChild(
      AddTextItem(pitchRate.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream yawRate;
   yawRate << "R " << std::fixed << std::setw(7) << std::setprecision(2) << mPlatData.yawRate_dps;
   rightGroup->addChild(
      AddTextItem(yawRate.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream ad;
   ad << "AD" << std::fixed << std::setw(7) << std::setprecision(2) << mPlatData.alphaDot_dps;
   rightGroup->addChild(AddTextItem(ad.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream bd;
   bd << "BD" << std::fixed << std::setw(7) << std::setprecision(2) << mPlatData.betaDot_dps;
   rightGroup->addChild(AddTextItem(bd.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionControls()
{
   double ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::Group> rightGroup(new osg::Group);

   double y = -0.5 * ppd; // Blank line

   if (mPlatData.aileronsValid)
   {
      y -= ppd;
      std::stringstream la;
      la << "LA " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.aileronLft_deg;
      rightGroup->addChild(AddTextItem(la.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

      y -= ppd;
      std::stringstream ra;
      ra << "RA " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.aileronRgt_deg;
      rightGroup->addChild(AddTextItem(ra.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }
   else
   {
      y -= ppd * 2.0; // Two blank lines
   }

   y -= ppd;
   std::stringstream ls;
   ls << "LS " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.stabLeft_deg;
   rightGroup->addChild(AddTextItem(ls.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream rs;
   rs << "RS " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.stabRight_deg;
   rightGroup->addChild(AddTextItem(rs.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   if (mPlatData.twinRuddersValid)
   {
      y -= ppd;
      std::stringstream lr;
      lr << "LR " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.rudderLft_deg;
      rightGroup->addChild(AddTextItem(lr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

      y -= ppd;
      std::stringstream rr;
      rr << "RR " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.rudderRgt_deg;
      rightGroup->addChild(AddTextItem(rr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }
   else
   {
      if (mPlatData.singleRudderValid)
      {
         y -= ppd;
         std::stringstream sr;
         sr << "R  " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.rudder_deg;
         rightGroup->addChild(
            AddTextItem(sr.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

         y -= ppd; // Blank line
      }
      else
      {
         y -= ppd * 2.0; // Two blank lines
      }
   }

   y -= ppd;
   if (mPlatData.speedbrakesValid)
   {
      std::stringstream sb;
      sb << "SB " << std::fixed << std::setw(6) << std::setprecision(2) << mPlatData.speedBrakes_deg;
      rightGroup->addChild(AddTextItem(sb.str(), 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());
   }

   y -= ppd; // Blank line

   y -= ppd;
   std::string mode = GetStringForHudMode();
   rightGroup->addChild(AddTextItem(mode, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   // Control Mode
   y -= ppd;
   std::string pilotType = GetPilotTypeString();
   rightGroup->addChild(AddTextItem(pilotType, 8.6 * ppd, y, fontSize, osgText::Text::AlignmentType::RIGHT_CENTER).get());

   rightGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(rightGroup);
}

void WkP6DOF_Controller::HUD::DrawRightRegionWeaponTest()
{
   DrawRightRegionTest();
}

void WkP6DOF_Controller::HUD::DrawTargetBoxOnHud()
{
   if (mPlatData.currentTargetValid)
   {
      double ppd = mPixelsPerDegree;

      double cx = mPlatData.currentTarget.tgtAz_rad * UtMath::cDEG_PER_RAD * ppd;
      double cy = mPlatData.currentTarget.tgtEl_rad * UtMath::cDEG_PER_RAD * ppd;

      osg::ref_ptr<osg::Group>     boxGroup(new osg::Group);
      osg::ref_ptr<osg::Vec2Array> boxArray(new osg::Vec2Array);
      boxArray->push_back(osg::Vec2(cx - 0.6 * ppd, cy - 0.6 * ppd));
      boxArray->push_back(osg::Vec2(cx + 0.6 * ppd, cy - 0.6 * ppd));
      boxArray->push_back(osg::Vec2(cx + 0.6 * ppd, cy + 0.6 * ppd));
      boxArray->push_back(osg::Vec2(cx - 0.6 * ppd, cy + 0.6 * ppd));
      boxGroup->addChild(CreateHudGeometry(boxArray, GL_LINE_LOOP, mHudColor).get());

      boxGroup->setStateSet(GetStencilDrawWhereMatch().get());
      mHudGeode->addChild(boxGroup.get());
   }
}

void WkP6DOF_Controller::HUD::DrawPitchLadder()
{
   float ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
   xform->postMult(osg::Matrix::translate(0.0, -mPlatData.pitch_deg * ppd, 0.0));
   xform->postMult(osg::Matrix::rotate(osg::Quat(osg::DegreesToRadians(mPlatData.roll_deg), osg::Vec3(0, 0, 1))));

   // --------------------------------------------------
   // Horizon line
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

   vertArray->push_back(osg::Vec2(-1.0 * ppd, 0.0));
   vertArray->push_back(osg::Vec2(-20.0 * ppd, 0.0));
   vertArray->push_back(osg::Vec2(1.0 * ppd, 0.0));
   vertArray->push_back(osg::Vec2(20.0 * ppd, 0.0));

   xform->addChild(CreateHudGeometry(vertArray, GL_LINES, mHudColor).get());

   // --------------------------------------------------
   // Upper lines
   osg::ref_ptr<osg::Group> upperLineGroup(new osg::Group);
   std::string              numString[8] = {"10", "20", "30", "40", "50", "60", "70", "80"};
   for (int i = 1; i < 9; ++i)
   {
      double hud_pitchAngle_deg = 10.0 * (double)i;
      double yVal               = hud_pitchAngle_deg * ppd;

      osg::ref_ptr<osg::Vec2Array> vertsOne(new osg::Vec2Array);
      vertsOne->push_back(osg::Vec2(1.0 * ppd, yVal));
      vertsOne->push_back(osg::Vec2(3.5 * ppd, yVal));
      vertsOne->push_back(osg::Vec2(3.5 * ppd, yVal - 0.5 * ppd));
      upperLineGroup->addChild(CreateHudGeometry(vertsOne, GL_LINE_STRIP, mHudColor).get());

      osg::ref_ptr<osg::Vec2Array> vertsTwo(new osg::Vec2Array);
      vertsTwo->push_back(osg::Vec2(-1.0 * ppd, yVal));
      vertsTwo->push_back(osg::Vec2(-3.5 * ppd, yVal));
      vertsTwo->push_back(osg::Vec2(-3.5 * ppd, yVal - 0.5 * ppd));
      upperLineGroup->addChild(CreateHudGeometry(vertsTwo, GL_LINE_STRIP, mHudColor).get());

      upperLineGroup->addChild(
         AddTextItem(numString[i - 1], -4.0 * ppd, yVal - 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
      upperLineGroup->addChild(
         AddTextItem(numString[i - 1], 4.0 * ppd, yVal - 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
   }

   xform->addChild(upperLineGroup.get());

   // Extra 5-deg line
   double      hud_pitchAngle_deg = 5.0;
   double      yVal               = hud_pitchAngle_deg * ppd;
   std::string numStr             = "5";

   osg::ref_ptr<osg::Group>     fiveDegLine(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> firstSet(new osg::Vec2Array);
   firstSet->push_back(osg::Vec2(1.0 * ppd, yVal));
   firstSet->push_back(osg::Vec2(3.5 * ppd, yVal));
   firstSet->push_back(osg::Vec2(3.5 * ppd, yVal - 0.5 * ppd));
   fiveDegLine->addChild(CreateHudGeometry(firstSet, GL_LINE_STRIP, mHudColor));

   osg::ref_ptr<osg::Vec2Array> secondSet(new osg::Vec2Array);
   secondSet->push_back(osg::Vec2(-1.0 * ppd, yVal));
   secondSet->push_back(osg::Vec2(-3.5 * ppd, yVal));
   secondSet->push_back(osg::Vec2(-3.5 * ppd, yVal - 0.5 * ppd));
   fiveDegLine->addChild(CreateHudGeometry(secondSet, GL_LINE_STRIP, mHudColor));

   fiveDegLine->addChild(AddTextItem(numStr, -4.0 * ppd, yVal - 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
   fiveDegLine->addChild(AddTextItem(numStr, 4.0 * ppd, yVal - 0.0, fontSize, osgText::Text::CENTER_CENTER).get());

   xform->addChild(fiveDegLine.get());

   // --------------------------------------------------
   // Lower lines
   osg::ref_ptr<osg::Group>       lowerLineGroup(new osg::Group);
   osg::ref_ptr<osg::Geode>       lowerLineStateGeode(new osg::Geode);
   osg::ref_ptr<osg::StateSet>    lowerLineStateSet(new osg::StateSet);
   osg::ref_ptr<osg::LineStipple> lowerLineStipple(new osg::LineStipple);
   lowerLineStipple->setPattern((GLushort)0x0F0F);
   lowerLineStateSet->setAttribute(lowerLineStipple.get());
   lowerLineStateSet->setMode(GL_LINE_STIPPLE, osg::StateAttribute::ON);
   lowerLineStateGeode->setStateSet(lowerLineStateSet.get());

   for (int i = 1; i < 9; ++i)
   {
      double hud_pitchAngle_deg = 10.0 * (double)i;
      double yVal               = -hud_pitchAngle_deg * ppd;

      osg::ref_ptr<osg::Vec2Array> vertsOne(new osg::Vec2Array);
      vertsOne->push_back(osg::Vec2(1.0 * ppd, yVal));
      vertsOne->push_back(osg::Vec2(3.5 * ppd, yVal));
      vertsOne->push_back(osg::Vec2(3.5 * ppd, yVal + 0.5 * ppd));
      lowerLineGroup->addChild(CreateHudGeometry(vertsOne, GL_LINE_STRIP, mHudColor).get());

      osg::ref_ptr<osg::Vec2Array> vertsTwo(new osg::Vec2Array);
      vertsTwo->push_back(osg::Vec2(-1.0 * ppd, yVal));
      vertsTwo->push_back(osg::Vec2(-3.5 * ppd, yVal));
      vertsTwo->push_back(osg::Vec2(-3.5 * ppd, yVal + 0.5 * ppd));
      lowerLineGroup->addChild(CreateHudGeometry(vertsTwo, GL_LINE_STRIP, mHudColor).get());

      lowerLineGroup->addChild(
         AddTextItem(numString[i - 1], -4.0 * ppd, yVal + 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
      lowerLineGroup->addChild(
         AddTextItem(numString[i - 1], 4.0 * ppd, yVal + 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
   }

   lowerLineStateGeode->addChild(lowerLineGroup.get());

   // Extra 5-deg line
   hud_pitchAngle_deg = -5.0;
   yVal               = hud_pitchAngle_deg * ppd;

   osg::ref_ptr<osg::Group>     bottomFiveDegLine(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> bottomFirstSet(new osg::Vec2Array);
   bottomFirstSet->push_back(osg::Vec2(1.0 * ppd, yVal));
   bottomFirstSet->push_back(osg::Vec2(3.5 * ppd, yVal));
   bottomFirstSet->push_back(osg::Vec2(3.5 * ppd, yVal + 0.5 * ppd));
   bottomFiveDegLine->addChild(CreateHudGeometry(bottomFirstSet, GL_LINE_STRIP, mHudColor));

   osg::ref_ptr<osg::Vec2Array> bottomSecondSet(new osg::Vec2Array);
   bottomSecondSet->push_back(osg::Vec2(-1.0 * ppd, yVal));
   bottomSecondSet->push_back(osg::Vec2(-3.5 * ppd, yVal));
   bottomSecondSet->push_back(osg::Vec2(-3.5 * ppd, yVal + 0.5 * ppd));
   bottomFiveDegLine->addChild(CreateHudGeometry(bottomSecondSet, GL_LINE_STRIP, mHudColor));

   bottomFiveDegLine->addChild(AddTextItem(numStr, -4.0 * ppd, yVal + 0.0, fontSize, osgText::Text::CENTER_CENTER).get());
   bottomFiveDegLine->addChild(AddTextItem(numStr, 4.0 * ppd, yVal + 0.0, fontSize, osgText::Text::CENTER_CENTER).get());

   lowerLineStateGeode->addChild(bottomFiveDegLine.get());

   xform->addChild(lowerLineStateGeode.get());
   xform->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(xform.get());
}

std::string WkP6DOF_Controller::HUD::GetStringForHudMode() const
{
   std::string mode;

   switch (mHudMode)
   {
   case P6DOF_ControllerDataContainer::eUNDEFINED:
      mode = "UNDEFINED";
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_WAYPT:
      mode = "NAV-WAYPT";
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_LAND:
      mode = "NAV-LNDNG";
      break;
   case P6DOF_ControllerDataContainer::eHUD_NAV_TAXI:
      mode = "NAV-TAXI";
      break;
   case P6DOF_ControllerDataContainer::eHUD_FUEL:
      mode = "NAV-FUEL";
      break;
   case P6DOF_ControllerDataContainer::eHUD_ENG:
      mode = "ENGINES";
      break;
   case P6DOF_ControllerDataContainer::eHUD_GND:
      mode = "GROUND";
      break;
   case P6DOF_ControllerDataContainer::eHUD_GUN:
      mode = "GUN MODE";
      break;
   case P6DOF_ControllerDataContainer::eHUD_MSL:
      mode = "MSL MODE";
      break;
   case P6DOF_ControllerDataContainer::eHUD_TEST:
      mode = "TEST MODE";
      break;
   case P6DOF_ControllerDataContainer::eHUD_CONTROLS:
      mode = "CONTROLS";
      break;
   case P6DOF_ControllerDataContainer::eHUD_WEAPON:
      mode = "WPN TEST";
      break;
   default:
      mode = "ERROR";
      break;
   }

   return mode;
}

std::string WkP6DOF_Controller::HUD::GetPilotTypeString() const
{
   std::string pilotType = "ERROR";

   if (mPlatData.pilotType == "Guidance_BTT")
   {
      pilotType = "Guidance";
   }
   else if (mPlatData.pilotType == "Guidance_STT")
   {
      pilotType = "Guidance";
   }
   else if (mPlatData.pilotType == "Autopilot_BTT")
   {
      pilotType = "Autopilot";
   }
   else if (mPlatData.pilotType == "Autopilot_STT")
   {
      pilotType = "Autopilot";
   }
   else if (mPlatData.pilotType == "ManualPilot")
   {
      pilotType = "Manual";
   }
   else if (mPlatData.pilotType == "Manual-CAS")
   {
      pilotType = "CAS";
   }
   else if (mPlatData.pilotType == "Manual-Simple")
   {
      pilotType = "FCS-S";
   }
   else if (mPlatData.pilotType == "SyntheticPilot")
   {
      pilotType = "Synthetic";
   }

   return pilotType;
}

std::string WkP6DOF_Controller::HUD::GetTimeToString(double aTimeRemaining_hr) const
{
   int iHours   = 0;
   int iMinutes = 0;
   int iSeconds = 0;

   if (mPlatData.engineFuelFlow_pph > 0.0)
   {
      iHours                   = (int)aTimeRemaining_hr;
      double timeRemaining_min = (aTimeRemaining_hr - (double)iHours) * 60.0;

      iMinutes                 = (int)timeRemaining_min;
      double timeRemaining_sec = (timeRemaining_min - (double)iMinutes) * 60.0;

      iSeconds = (int)timeRemaining_sec;
   }

   std::stringstream timeStr;
   timeStr << "T" << std::setw(2) << iHours << ":" << std::setw(2) << std::setfill('0') << iMinutes << ":"
           << std::setw(2) << std::setfill('0') << iSeconds;

   std::string retString = timeStr.str();
   return retString;
}

void WkP6DOF_Controller::HUD::DrawRollPointer()
{
   float ppd = mPixelsPerDegree;

   // Fixed roll ticks
   osg::ref_ptr<osg::MatrixTransform> xformTicks(new osg::MatrixTransform());
   osg::ref_ptr<osg::Vec2Array>       vertArrayTicks(new osg::Vec2Array);
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(-60.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(-60.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(-60.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(-60.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(-40.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(-40.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(-40.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(-40.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(-20.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(-20.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(-20.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(-20.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(-10.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(-10.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.4 * sin(-10.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.4 * cos(-10.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(
      osg::Vec2(8.0 * sin(0.0 * UtMath::cRAD_PER_DEG) * ppd, -4.0 * ppd - 8.0 * cos(0.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(
      osg::Vec2(8.7 * sin(0.0 * UtMath::cRAD_PER_DEG) * ppd, -4.0 * ppd - 8.7 * cos(0.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(10.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(10.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.4 * sin(10.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.4 * cos(10.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(20.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(20.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(20.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(20.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(40.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(40.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(40.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(40.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.0 * sin(60.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.0 * cos(60.0 * UtMath::cRAD_PER_DEG) * ppd));
   vertArrayTicks->push_back(osg::Vec2(8.7 * sin(60.0 * UtMath::cRAD_PER_DEG) * ppd,
                                       -4.0 * ppd - 8.7 * cos(60.0 * UtMath::cRAD_PER_DEG) * ppd));
   xformTicks->addChild(CreateHudGeometry(vertArrayTicks, GL_LINES, mHudColor).get());
   xformTicks->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(xformTicks.get());

   // Movable roll pointer
   double roll_deg = mPlatData.roll_deg;

   // Only draw pointer with limited roll (60 deg)
   if (fabs(roll_deg) < 60.0)
   {
      osg::ref_ptr<osg::MatrixTransform> xformArrow(new osg::MatrixTransform());
      xformArrow->postMult(osg::Matrix::rotate(osg::Quat(osg::DegreesToRadians(mPlatData.roll_deg), osg::Vec3(0, 0, 1))));
      xformArrow->postMult(osg::Matrix::translate(0.0, -4.0 * ppd, 0.0));
      osg::ref_ptr<osg::Vec2Array> vertArrayArrow(new osg::Vec2Array);
      vertArrayArrow->push_back(osg::Vec2(0.00 * ppd, -8.0 * ppd));
      vertArrayArrow->push_back(osg::Vec2(0.15 * ppd, -7.6 * ppd));
      vertArrayArrow->push_back(osg::Vec2(-0.15 * ppd, -7.6 * ppd));
      xformArrow->addChild(CreateHudGeometry(vertArrayArrow, GL_LINE_LOOP, mHudColor).get());
      xformArrow->setStateSet(GetStencilDrawWhereMatch().get());
      mHudGeode->addChild(xformArrow.get());
   }
}

void WkP6DOF_Controller::HUD::DrawWeaponSelected(std::string aWeaponString, bool aDrawX)
{
   // This is only drawn when in eGUN or eMSL mode
   if ((mHudMode == P6DOF_ControllerDataContainer::eHUD_GUN) || (mHudMode == P6DOF_ControllerDataContainer::eHUD_MSL))
   {
      float ppd = mPixelsPerDegree;

      int fontSize = 12 * mFontSizeMult;

      osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
      xform->postMult(osg::Matrix::translate(0.0, -9.0 * ppd, 0.0));

      osg::ref_ptr<osg::Group> weaponGroup(new osg::Group);
      weaponGroup->addChild(AddTextItem(aWeaponString, 0.0, 0.0, fontSize, osgText::Text::CENTER_CENTER).get());

      if (aDrawX)
      {
         osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);
         verts->push_back(osg::Vec2(-2.0 * ppd, 0.6 * ppd));
         verts->push_back(osg::Vec2(2.0 * ppd, -0.6 * ppd));
         verts->push_back(osg::Vec2(-2.0 * ppd, -0.6 * ppd));
         verts->push_back(osg::Vec2(2.0 * ppd, 0.6 * ppd));
         weaponGroup->addChild(CreateHudGeometry(verts, GL_LINES, mHudColor).get());
      }

      xform->addChild(weaponGroup.get());

      xform->setStateSet(GetStencilDrawWhereMatch().get());
      mHudGeode->addChild(xform.get());
   }
}

void WkP6DOF_Controller::HUD::DrawBingo()
{
   if (mSlowBlink)
   {
      float ppd = mPixelsPerDegree;

      int fontSize = 12 * mFontSizeMult;

      osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
      xform->postMult(osg::Matrix::translate(0.0, -10.5 * ppd, 0.0));

      osg::ref_ptr<osg::Group> bingoGroup(new osg::Group);
      std::string              bingoStr = "BINGO";
      bingoGroup->addChild(AddTextItem(bingoStr, 0.0, 0.0, fontSize, osgText::Text::CENTER_CENTER).get());

      xform->addChild(bingoGroup.get());

      xform->setStateSet(GetStencilDrawWhereMatch().get());
      mHudGeode->addChild(xform.get());
   }
}

void WkP6DOF_Controller::HUD::DrawShoot()
{
   if (mFastBlink)
   {
      float ppd = mPixelsPerDegree;

      int fontSize = 12 * mFontSizeMult;

      osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
      xform->postMult(osg::Matrix::translate(0.0, 4.0 * ppd, 0.0));

      osg::ref_ptr<osg::Group> shootGroup(new osg::Group);
      std::string              shootStr = "SHOOT";
      shootGroup->addChild(AddTextItem(shootStr, 0.0, 0.0, fontSize, osgText::Text::CENTER_CENTER).get());

      xform->addChild(shootGroup.get());

      xform->setStateSet(GetStencilDrawWhereMatch().get());
      mHudGeode->addChild(xform.get());
   }
}

osg::ref_ptr<osg::Geode> WkP6DOF_Controller::HUD::CreateHudGeometry(osg::ref_ptr<osg::Vec2Array>       aArray,
                                                                    int                                aDrawMode,
                                                                    const osg::ref_ptr<osg::Vec4Array> aColor)
{
   osg::ref_ptr<osg::Geometry> geom(new osg::Geometry);
   osg::ref_ptr<osg::Geode>    retGeode(new osg::Geode);

   osg::ref_ptr<osg::DrawArrays> geomArray(new osg::DrawArrays(aDrawMode, 0, aArray->size()));
   geom->setVertexArray(aArray.get());
   geom->addPrimitiveSet(geomArray.get());

   geom->setColorArray(aColor);
   geom->setColorBinding(osg::Geometry::BIND_OVERALL);

   osg::ref_ptr<osg::StateSet>  lineState(new osg::StateSet);
   osg::ref_ptr<osg::LineWidth> lineWidthAttr(new osg::LineWidth);
   lineWidthAttr->setWidth(1.0 * mLineWidthMult);
   lineState->setAttribute(lineWidthAttr.get());
   lineState->setMode(GL_LINE_SMOOTH, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
   lineState->setMode(GL_BLEND, osg::StateAttribute::ON);
   retGeode->setStateSet(lineState.get());

   retGeode->addDrawable(geom.get());
   return retGeode;
}

osg::ref_ptr<osg::Geode> WkP6DOF_Controller::HUD::AddTextItem(std::string                   aStr,
                                                              float                         aXPos,
                                                              float                         aYPos,
                                                              int                           aFontSize,
                                                              osg::ref_ptr<osg::Vec4Array>& aColor,
                                                              osgText::Text::AlignmentType  aAlignment)
{
   osg::ref_ptr<osgText::Text> textItem(new osgText::Text());
   textItem->setCharacterSize(aFontSize);

   // Set alignment
   textItem->setAlignment(aAlignment);

   if (mFont == nullptr)
   {
      std::string resourceDir = "";
      vespa::VaUtils::GetPathPrioritizedFilename("fonts/DejaVuSansMono.ttf", resourceDir);
      UtPath path(resourceDir);
      path.Up();

      // Find the font in the available resource directories
      // If an empty string is returned, OSG will find some font to use

      UtPath wd = UtPath::WorkingDirectory();
      path.SetWorkingDirectory();
      mFont = osgText::readFontFile("DejaVuSansMono.ttf");
      wd.SetWorkingDirectory();
   }

   textItem->setFont(mFont);
   textItem->setText(aStr);
   textItem->setAxisAlignment(osgText::Text::SCREEN);
   textItem->setPosition(osg::Vec3d(aXPos, aYPos, 0));
   textItem->setColor((*aColor)[0]);

   osg::ref_ptr<osg::Geode> geoNode(new osg::Geode);
   geoNode->addDrawable(textItem);
   return geoNode;
}

osg::ref_ptr<osg::Geode> WkP6DOF_Controller::HUD::AddTextItem(std::string                  aStr,
                                                              float                        aXPos,
                                                              float                        aYPos,
                                                              int                          aFontSize,
                                                              osgText::Text::AlignmentType aAlignment)
{
   osg::ref_ptr<osgText::Text> textItem(new osgText::Text());
   textItem->setCharacterSize(aFontSize);

   // Set alignment
   textItem->setAlignment(aAlignment);

   if (mFont == nullptr)
   {
      std::string resourceDir = "";
      vespa::VaUtils::GetPathPrioritizedFilename("fonts/DejaVuSansMono.ttf", resourceDir);
      UtPath path(resourceDir);
      path.Up();

      // Find the font in the available resource directories
      // If an empty string is returned, OSG will find some font to use

      UtPath wd = UtPath::WorkingDirectory();
      path.SetWorkingDirectory();
      mFont = osgText::readFontFile("DejaVuSansMono.ttf");
      wd.SetWorkingDirectory();
   }

   textItem->setFont(mFont);
   textItem->setText(aStr);
   textItem->setAxisAlignment(osgText::Text::SCREEN);
   textItem->setPosition(osg::Vec3d(aXPos, aYPos, 0));
   textItem->setColor((*mHudColor)[0]);

   osg::ref_ptr<osg::Geode> geoNode(new osg::Geode);
   geoNode->addDrawable(textItem);
   return geoNode;
}

void WkP6DOF_Controller::HUD::DrawLftMFD(P6DOF_ControllerDataContainer::eMfdMode aMfdMode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   double cx = -26.0 * ppd;
   double cy = -10.0 * ppd;

   osg::ref_ptr<osg::Geode>     vectorGeode(new osg::Geode);
   osg::ref_ptr<osg::Group>     vectorGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   // Transformation
   osg::ref_ptr<osg::PositionAttitudeTransform> vectorXform(new osg::PositionAttitudeTransform());
   vectorXform->setPosition(osg::Vec3d(cx, cy, 0.0));

   // Set background color
   osg::ref_ptr<osg::Vec4Array> backgroundColor(new osg::Vec4Array);
   backgroundColor->push_back(osg::Vec4(0.0, 0.0, 0.0, 0.4));

   // HUD Outer Border
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-10.0 * ppd, -10.0 * ppd));
   vertArray->push_back(osg::Vec2d(10.0 * ppd, -10.0 * ppd));
   vertArray->push_back(osg::Vec2d(10.0 * ppd, 10.0 * ppd));
   vertArray->push_back(osg::Vec2d(-10.0 * ppd, 10.0 * ppd));

   vectorGeode->addChild(CreateHudGeometry(vertArray, GL_POLYGON, backgroundColor).get());

   bool thisMfdIsActive = false;
   if (mPlatData.mfdActive == 0)
   {
      thisMfdIsActive = true;
   }

   switch (mPlatData.mfdLftMode)
   {
   case P6DOF_ControllerDataContainer::eMFD_RADAR:
      DrawMFD_Radar(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_RWR:
      DrawMFD_RWR(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_IRST:
      DrawMFD_IRST(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_FLIR:
      DrawMFD_FLIR(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_DATALINK:
      DrawMFD_Datalink(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_WEAPONS:
      DrawMFD_Weapons(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_FUEL:
      DrawMFD_Fuel(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_ENGINES:
      DrawMFD_Engines(vectorGeode, thisMfdIsActive);
      break;
   default:
      std::cout << "ERROR - Unknown MFD mode in WkP6DOF_Controller::HUD::DrawLftMFD()" << std::endl;
      break;
   }

   vectorXform->addChild(vectorGeode.get());
   vectorGroup->addChild(vectorXform.get());
   vectorGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(vectorGroup.get());
}

void WkP6DOF_Controller::HUD::DrawRgtMFD(P6DOF_ControllerDataContainer::eMfdMode aMfdMode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   double cx = 26.0 * ppd;
   double cy = -10.0 * ppd;

   osg::ref_ptr<osg::Geode>     vectorGeode(new osg::Geode);
   osg::ref_ptr<osg::Group>     vectorGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   // Transformation
   osg::ref_ptr<osg::PositionAttitudeTransform> vectorXform(new osg::PositionAttitudeTransform());
   vectorXform->setPosition(osg::Vec3d(cx, cy, 0.0));

   // Set background color
   osg::ref_ptr<osg::Vec4Array> backgroundColor(new osg::Vec4Array);
   backgroundColor->push_back(osg::Vec4(0.0, 0.0, 0.0, 0.4));

   // HUD Outer Border
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-10.0 * ppd, -10.0 * ppd));
   vertArray->push_back(osg::Vec2d(10.0 * ppd, -10.0 * ppd));
   vertArray->push_back(osg::Vec2d(10.0 * ppd, 10.0 * ppd));
   vertArray->push_back(osg::Vec2d(-10.0 * ppd, 10.0 * ppd));

   vectorGeode->addChild(CreateHudGeometry(vertArray, GL_POLYGON, backgroundColor).get());

   bool thisMfdIsActive = false;
   if (mPlatData.mfdActive == 1)
   {
      thisMfdIsActive = true;
   }

   switch (mPlatData.mfdRgtMode)
   {
   case P6DOF_ControllerDataContainer::eMFD_RADAR:
      DrawMFD_Radar(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_RWR:
      DrawMFD_RWR(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_IRST:
      DrawMFD_IRST(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_FLIR:
      DrawMFD_FLIR(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_DATALINK:
      DrawMFD_Datalink(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_WEAPONS:
      DrawMFD_Weapons(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_FUEL:
      DrawMFD_Fuel(vectorGeode, thisMfdIsActive);
      break;
   case P6DOF_ControllerDataContainer::eMFD_ENGINES:
      DrawMFD_Engines(vectorGeode, thisMfdIsActive);
      break;
   default:
      std::cout << "ERROR - Unknown MFD mode in WkP6DOF_Controller::HUD::DrawRgtMFD()" << std::endl;
      break;
   }

   vectorXform->addChild(vectorGeode.get());
   vectorGroup->addChild(vectorXform.get());
   vectorGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(vectorGroup.get());
}

void WkP6DOF_Controller::HUD::DrawMFD_Radar(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   // If this radar display is active, handle target designator symbol movement
   if (aActive)
   {
      // Detect TD movement
      if (mPlatData.tgtDesignationRgt || mPlatData.tgtDesignationLft || mPlatData.tgtDesignationUp ||
          mPlatData.tgtDesignationDn)
      {
         // The user is moving the TD symbol
         if (!mMfdRadarUpdateActive)
         {
            // Set the local active flag. This is used to adjust the speed
            // that the TD symbol moves at, based on how long it has been
            // moving.
            mMfdRadarUpdateActive = true;

            // Just became active - save the time
            mMfdRadarTime_sec = mPlatData.simTime;
         }
      }
      else
      {
         // The user is not moving the TD symbol
         mMfdRadarUpdateActive = false;
      }

      // We use a multiplier to make the TD symbol move faster, based on how
      // long it has been moving.
      double tdMovementMult = 1.0;
      if (mMfdRadarUpdateActive)
      {
         double deltaTimeSinceActive_sec = mPlatData.simTime - mMfdRadarTime_sec;
         if (deltaTimeSinceActive_sec > 0.25)
         {
            if (deltaTimeSinceActive_sec < 0.5)
            {
               tdMovementMult = 2.0;
            }
            else if (deltaTimeSinceActive_sec < 1.0)
            {
               tdMovementMult = 3.0;
            }
            else
            {
               tdMovementMult = 4.0;
            }
         }
      }

      // Now that we've adjusted the multiplier, we can actually
      // move the target designation symbol.

      // We use a baseline step size that can be adjusted by the multiplier
      const double tdStepSize = 0.01;

      if (mPlatData.tgtDesignationRgt)
      {
         mRadarTgtDesignatorX += tdStepSize * tdMovementMult;
      }
      else if (mPlatData.tgtDesignationLft)
      {
         mRadarTgtDesignatorX -= tdStepSize * tdMovementMult;
      }

      if (mPlatData.tgtDesignationUp)
      {
         mRadarTgtDesignatorY += tdStepSize * tdMovementMult;
      }
      else if (mPlatData.tgtDesignationDn)
      {
         mRadarTgtDesignatorY -= tdStepSize * tdMovementMult;
      }

      // Limit the TD symbol movement range in X to +/-1
      if (mRadarTgtDesignatorX > 1.0)
      {
         mRadarTgtDesignatorX = 1.0;
      }
      else if (mRadarTgtDesignatorX < -1.0)
      {
         mRadarTgtDesignatorX = -1.0;
      }

      // Limit the TD symbol movement range in Y to +/-1,
      // but it may also adjust the range if it goes off the
      // top (increase range) or bottom (decrease range).
      if (mRadarTgtDesignatorY > 1.0)
      {
         mRadarTgtDesignatorY = 1.0;
         mRadarRng_nm *= 2;
         mRadarTgtDesignatorY = 0.0;
      }
      else if (mRadarTgtDesignatorY < -1.0)
      {
         mRadarTgtDesignatorY = -1.0;
         mRadarRng_nm /= 2;
         mRadarTgtDesignatorY = 0.0;

         if (mRadarRng_nm < 5)
         {
            mRadarRng_nm = 5;
         }
      }
   }

   // Set the main color
   osg::ref_ptr<osg::Vec4Array> gridColor(new osg::Vec4Array);

   // If the field is empty, set it to NO_SENSOR
   if (mPlatData.radarMode.empty())
   {
      mPlatData.radarMode = "NO_SENSOR";
   }

   if (mPlatData.radarMode == "NO_SENSOR")
   {
      // Set color
      osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
      textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));
      DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "RADAR DISPLAY", "UNAVAILABLE");

      gridColor->push_back(osg::Vec4(0.0, 0.5, 0.15, 0.45));
   }
   else if (mPlatData.radarMode == "OFF")
   {
      gridColor->push_back(osg::Vec4(0.0, 0.5, 0.15, 0.45));
   }
   else // ON and OPERATIONAL
   {
      gridColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));
   }

   // Draw the horizon line if pitch is within +/- 30 deg
   if (fabs(mPlatData.pitch_deg) < 30.0)
   {
      osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
      xform->postMult(osg::Matrix::translate(0.0, -mPlatData.pitch_deg * ppd * 0.2, 0.0));
      xform->postMult(osg::Matrix::rotate(osg::Quat(osg::DegreesToRadians(mPlatData.roll_deg), osg::Vec3(0, 0, 1))));

      osg::ref_ptr<osg::Vec2Array> horizonLftVertArray(new osg::Vec2Array);
      horizonLftVertArray->push_back(osg::Vec2(-5.0 * ppd, -0.6 * ppd));
      horizonLftVertArray->push_back(osg::Vec2(-5.0 * ppd, 0.0));
      horizonLftVertArray->push_back(osg::Vec2(-2.0 * ppd, 0.0));
      xform->addChild(CreateHudGeometry(horizonLftVertArray, GL_LINE_STRIP, gridColor).get());
      osg::ref_ptr<osg::Vec2Array> horizonRgtVertArray(new osg::Vec2Array);
      horizonRgtVertArray->push_back(osg::Vec2(2.0 * ppd, 0.0));
      horizonRgtVertArray->push_back(osg::Vec2(5.0 * ppd, 0.0));
      horizonRgtVertArray->push_back(osg::Vec2(5.0 * ppd, -0.6 * ppd));
      xform->addChild(CreateHudGeometry(horizonRgtVertArray, GL_LINE_STRIP, gridColor).get());

      // xform->setStateSet(GetStencilDrawWhereMatch().get());
      aVectorGeode->addChild(xform.get());
   }

   // Draw the main grid
   osg::ref_ptr<osg::Vec2Array> vertArray1(new osg::Vec2Array);
   vertArray1->push_back(osg::Vec2d(-8.0 * ppd, -8.0 * ppd));
   vertArray1->push_back(osg::Vec2d(8.0 * ppd, -8.0 * ppd));
   vertArray1->push_back(osg::Vec2d(8.0 * ppd, 8.0 * ppd));
   vertArray1->push_back(osg::Vec2d(-8.0 * ppd, 8.0 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(vertArray1, GL_LINE_LOOP, gridColor).get());

   osg::ref_ptr<osg::Vec2Array> vertArray2(new osg::Vec2Array);
   vertArray2->push_back(osg::Vec2d(-8.0 * ppd, 4.0 * ppd));
   vertArray2->push_back(osg::Vec2d(8.0 * ppd, 4.0 * ppd));
   vertArray2->push_back(osg::Vec2d(-8.0 * ppd, -4.0 * ppd));
   vertArray2->push_back(osg::Vec2d(8.0 * ppd, -4.0 * ppd));
   vertArray2->push_back(osg::Vec2d(-4.0 * ppd, -8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(-4.0 * ppd, 8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(4.0 * ppd, -8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(4.0 * ppd, 8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(-8.0 * ppd, 0.0 * ppd));
   vertArray2->push_back(osg::Vec2d(-2.0 * ppd, 0.0 * ppd));
   vertArray2->push_back(osg::Vec2d(2.0 * ppd, 0.0 * ppd));
   vertArray2->push_back(osg::Vec2d(8.0 * ppd, 0.0 * ppd));
   vertArray2->push_back(osg::Vec2d(0.0 * ppd, -8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(0.0 * ppd, -2.0 * ppd));
   vertArray2->push_back(osg::Vec2d(0.0 * ppd, 8.0 * ppd));
   vertArray2->push_back(osg::Vec2d(0.0 * ppd, 2.0 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(vertArray2, GL_LINES, gridColor).get());

   // Draw the side ticks
   osg::ref_ptr<osg::Vec2Array> sideTicksArray(new osg::Vec2Array);
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, 4.00 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, 4.00 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, 2.66 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, 2.66 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, 1.33 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, 1.33 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, 0.0));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, 0.0));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, -1.33 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, -1.33 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, -2.66 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, -2.66 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.4 * ppd, -4.00 * ppd));
   sideTicksArray->push_back(osg::Vec2d(-8.0 * ppd, -4.00 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(sideTicksArray, GL_LINES, gridColor).get());

   // Draw the bottom ticks
   osg::ref_ptr<osg::Vec2Array> bottomTicksArray(new osg::Vec2Array);
   bottomTicksArray->push_back(osg::Vec2d(-2.66 * ppd, -7.6 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(-2.66 * ppd, -8.0 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(-1.33 * ppd, -7.6 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(-1.33 * ppd, -8.0 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(1.33 * ppd, -7.6 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(1.33 * ppd, -8.0 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(2.66 * ppd, -7.6 * ppd));
   bottomTicksArray->push_back(osg::Vec2d(2.66 * ppd, -8.0 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(bottomTicksArray, GL_LINES, gridColor).get());

   // These are used for various circles below
   double cx           = 0.0;
   double cy           = 0.0;
   double circleRadius = 0.3 * ppd;

   // Scan (az,el) parameters
   double scanAzCenter_deg  = 0.5 * (mPlatData.radarMinAz_deg + mPlatData.radarMaxAz_deg);
   double scanElCenter_deg  = 0.5 * (mPlatData.radarMinEl_deg + mPlatData.radarMaxEl_deg);
   double scanAzHalfFOV_deg = 0.5 * (mPlatData.radarMaxAz_deg - mPlatData.radarMinAz_deg);
   double scanElHalfFOV_deg = 0.5 * (mPlatData.radarMaxEl_deg - mPlatData.radarMinEl_deg);

   // Az scan limit indicators
   double az_deg = scanAzCenter_deg - scanAzHalfFOV_deg;
   if (az_deg < -60.0)
   {
      az_deg = -60.0;
   }
   if (az_deg > 60.0)
   {
      az_deg = 60.0;
   }
   cx = (8.0 * ppd) * (az_deg / 60.0);
   cy = -7.7 * ppd;
   DrawCircle(cx, cy, circleRadius, gridColor, aVectorGeode, 12);
   az_deg = scanAzCenter_deg + scanAzHalfFOV_deg;
   if (az_deg < -60.0)
   {
      az_deg = -60.0;
   }
   if (az_deg > 60.0)
   {
      az_deg = 60.0;
   }
   cx = (8.0 * ppd) * (az_deg / 60.0);
   cy = -7.7 * ppd;
   DrawCircle(cx, cy, circleRadius, gridColor, aVectorGeode, 12);

   // El scan limit indicators
   double lowEl_rad  = (scanElCenter_deg - scanElHalfFOV_deg) * UtMath::cRAD_PER_DEG;
   double highEl_rad = (scanElCenter_deg + scanElHalfFOV_deg) * UtMath::cRAD_PER_DEG;
   double tdRange_ft = UtMath::cFT_PER_NM * 0.5 * (mRadarTgtDesignatorY + 1.0) * static_cast<double>(mRadarRng_nm);
   double elAltLo_ft = mPlatData.altitudeMSL_ft + (tdRange_ft * tan(lowEl_rad));
   double elAltHi_ft = mPlatData.altitudeMSL_ft + (tdRange_ft * tan(highEl_rad));
   if (elAltHi_ft > 60000.0)
   {
      elAltHi_ft = 60000.0;
   }
   if (elAltHi_ft < 0.0)
   {
      elAltHi_ft = 0.0;
   }
   if (elAltLo_ft > 60000.0)
   {
      elAltLo_ft = 60000.0;
   }
   if (elAltLo_ft < 0.0)
   {
      elAltLo_ft = 0.0;
   }

   double       elAlt_ft    = elAltHi_ft;
   const double elAltMax_ft = 60000.0;
   cx                       = -8.3 * ppd;
   cy                       = (8.0 * ppd) * (elAlt_ft / elAltMax_ft);
   DrawCircle(cx, cy, circleRadius, gridColor, aVectorGeode, 12);
   cx = -7.7 * ppd;
   std::stringstream elScanString1;
   elScanString1 << static_cast<int>(elAlt_ft / 1000.0);
   aVectorGeode->addChild(AddTextItem(elScanString1.str(), cx, cy, fontSize, gridColor, osgText::Text::LEFT_CENTER).get());

   elAlt_ft = elAltLo_ft;
   cx       = -8.3 * ppd;
   cy       = (8.0 * ppd) * (elAlt_ft / elAltMax_ft);
   DrawCircle(cx, cy, circleRadius, gridColor, aVectorGeode, 12);
   cx = -7.7 * ppd;
   std::stringstream elScanString2;
   elScanString2 << static_cast<int>(elAlt_ft / 1000.0);
   aVectorGeode->addChild(AddTextItem(elScanString2.str(), cx, cy, fontSize, gridColor, osgText::Text::LEFT_CENTER).get());

   // Draw El caret
   double                       elCaret_deg = mPlatData.radarCuedEl_deg;
   osg::ref_ptr<osg::Vec2Array> elCaretArray(new osg::Vec2Array);
   elCaretArray->push_back(osg::Vec2d(-7.4 * ppd, (elCaret_deg / 60.0) * 8.0 * ppd + 0.3 * ppd));
   elCaretArray->push_back(osg::Vec2d(-8.0 * ppd, (elCaret_deg / 60.0) * 8.0 * ppd));
   elCaretArray->push_back(osg::Vec2d(-7.4 * ppd, (elCaret_deg / 60.0) * 8.0 * ppd - 0.3 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(elCaretArray, GL_LINE_STRIP, gridColor).get());

   // Draw Az caret
   double                       azCaret_deg = mPlatData.radarCuedAz_deg;
   osg::ref_ptr<osg::Vec2Array> azCaretArray(new osg::Vec2Array);
   azCaretArray->push_back(osg::Vec2d((azCaret_deg / 60.0) * 8.0 * ppd - 0.3 * ppd, -7.4 * ppd));
   azCaretArray->push_back(osg::Vec2d((azCaret_deg / 60.0) * 8.0 * ppd, -8.0 * ppd));
   azCaretArray->push_back(osg::Vec2d((azCaret_deg / 60.0) * 8.0 * ppd + 0.3 * ppd, -7.4 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(azCaretArray, GL_LINE_STRIP, gridColor).get());

   // Draw radar mode
   std::stringstream modeString;
   modeString << mPlatData.radarMode;
   aVectorGeode->addChild(
      AddTextItem(modeString.str(), -8.0 * ppd, 8.5 * ppd, fontSize, gridColor, osgText::Text::LEFT_CENTER).get());

   // Draw heading range
   std::stringstream hdgString;
   int               hdg = static_cast<int>(mPlatData.heading_deg + 0.5);
   if (hdg <= 0)
   {
      hdg += 360;
   }
   if (hdg > 360)
   {
      hdg -= 360;
   }
   hdgString << std::setfill('0') << std::setw(3) << hdg;
   aVectorGeode->addChild(
      AddTextItem(hdgString.str(), 0.0, 8.5 * ppd, fontSize, gridColor, osgText::Text::CENTER_CENTER).get());

   // Draw max range
   std::stringstream rngString;
   rngString << std::setw(3) << mRadarRng_nm;
   aVectorGeode->addChild(
      AddTextItem(rngString.str(), 8.0 * ppd, 8.5 * ppd, fontSize, gridColor, osgText::Text::RIGHT_CENTER).get());

   // Draw ground speed
   std::stringstream gndSpdString;
   gndSpdString << "G" << static_cast<int>(mPlatData.ktas);
   aVectorGeode->addChild(
      AddTextItem(gndSpdString.str(), -8.0 * ppd, -8.5 * ppd, fontSize, gridColor, osgText::Text::LEFT_CENTER).get());

   // Draw true air speed
   std::stringstream trueAirSpdString;
   trueAirSpdString << "T" << static_cast<int>(mPlatData.ktas);
   aVectorGeode->addChild(
      AddTextItem(trueAirSpdString.str(), 8.0 * ppd, -8.5 * ppd, fontSize, gridColor, osgText::Text::RIGHT_CENTER).get());

   // Draw tracks

   // To draw tracks, we need to know the radar mode. For RWS, we draw multiple blocks.
   // For TWS, we draw multiple directional symbols. For STT, we draw a single directional
   // symbol. Note that auto-acquisition modes (WACQ, VACQ, and BORE) do not draw tracks,
   // since once they have a track, they will auto-switch to STT mode. We also check for
   // which modes should show the TD symbol.
   bool        trackDrawHandled = false;
   bool        drawTD_Symbol    = false;
   std::size_t found            = 0;
   found                        = mPlatData.radarMode.find("RWS");
   if ((found != std::string::npos) && !trackDrawHandled)
   {
      // RWS Mode
      trackDrawHandled = true;
      drawTD_Symbol    = true;

      for (auto& radarTrack : mPlatData.radarTracks)
      {
         if ((radarTrack.tgtBearingValid) && (radarTrack.tgtRngValid))
         {
            double az_deg = radarTrack.tgtBearing_rad * UtMath::cDEG_PER_RAD;
            double rng_nm = radarTrack.tgtRng_m * UtMath::cNM_PER_M;

            double hdg_deg = mPlatData.heading_deg;

            // Relative az
            az_deg = az_deg - hdg_deg;
            if (az_deg > 360.0)
            {
               az_deg -= 360.0;
            }
            if (az_deg < -360.0)
            {
               az_deg += 360.0;
            }

            // Now limit to +/-180
            if (az_deg > 180.0)
            {
               az_deg -= 360.0;
            }
            if (az_deg < -180.0)
            {
               az_deg += 360.0;
            }

            // Scale can change
            double blipX = (8.0 * ppd) * (az_deg / 60.0);
            double blipY = (-8.0 * ppd) + (16.0 * ppd) * (rng_nm / static_cast<double>(mRadarRng_nm));

            // Draw a blip
            osg::ref_ptr<osg::Vec2Array> blipVertArray(new osg::Vec2Array);
            blipVertArray->push_back(osg::Vec2d(blipX - 0.3 * ppd, blipY - 0.1 * ppd));
            blipVertArray->push_back(osg::Vec2d(blipX + 0.3 * ppd, blipY - 0.1 * ppd));
            blipVertArray->push_back(osg::Vec2d(blipX + 0.3 * ppd, blipY + 0.1 * ppd));
            blipVertArray->push_back(osg::Vec2d(blipX - 0.3 * ppd, blipY + 0.1 * ppd));
            aVectorGeode->addChild(CreateHudGeometry(blipVertArray, GL_POLYGON, gridColor).get());
         }
      }
   }

   found = mPlatData.radarMode.find("TWS");
   if ((found != std::string::npos) && !trackDrawHandled)
   {
      // TWS Mode
      trackDrawHandled = true;
      drawTD_Symbol    = true;

      for (auto& radarTrack : mPlatData.radarTracks)
      {
         if ((radarTrack.tgtBearingValid) && (radarTrack.tgtRngValid))
         {
            double az_deg = radarTrack.tgtBearing_rad * UtMath::cDEG_PER_RAD;
            double rng_nm = radarTrack.tgtRng_m * UtMath::cNM_PER_M;

            double hdg_deg = mPlatData.heading_deg;

            // Relative az
            az_deg = az_deg - hdg_deg;
            if (az_deg > 360.0)
            {
               az_deg -= 360.0;
            }
            if (az_deg < -360.0)
            {
               az_deg += 360.0;
            }

            // Now limit to +/-180
            if (az_deg > 180.0)
            {
               az_deg -= 360.0;
            }
            if (az_deg < -180.0)
            {
               az_deg += 360.0;
            }

            // Scale can change
            double blipX = (8.0 * ppd) * (az_deg / 60.0);
            double blipY = (-8.0 * ppd) + (16.0 * ppd) * (rng_nm / static_cast<double>(mRadarRng_nm));

            int  alt_kft      = static_cast<int>(radarTrack.tgtAlt_ft / 1000.0);
            bool primaryTgt   = false;
            bool secondaryTgt = false;

            DrawRadarTarget(aVectorGeode,
                            blipX,
                            blipY,
                            mPlatData.heading_deg,
                            radarTrack.tgtHdg_rad * UtMath::cDEG_PER_RAD,
                            alt_kft,
                            gridColor,
                            primaryTgt,
                            secondaryTgt);
         }
      }
   }

   found = mPlatData.radarMode.find("STT");
   if ((found != std::string::npos) && !trackDrawHandled)
   {
      //  STT Mode
      trackDrawHandled = true;

      for (auto& radarTrack : mPlatData.radarTracks)
      {
         // Only show the primary target
         if (mPlatData.currentTarget.tgtPlatformName == radarTrack.tgtPlatformName)
         {
            if (radarTrack.tgtRngValid)
            {
               double tgtRange_nm = radarTrack.tgtRng_m * UtMath::cNM_PER_M;

               // Auto-adjust range scale
               if (tgtRange_nm < 5.0)
               {
                  mRadarRng_nm = 5;
               }
               else if (tgtRange_nm < 10.0)
               {
                  mRadarRng_nm = 10;
               }
               else if (tgtRange_nm < 20.0)
               {
                  mRadarRng_nm = 20;
               }
               else if (tgtRange_nm < 40.0)
               {
                  mRadarRng_nm = 40;
               }
               else if (tgtRange_nm < 80.0)
               {
                  mRadarRng_nm = 80;
               }
               else
               {
                  mRadarRng_nm = 160;
               }
            }

            if ((radarTrack.tgtBearingValid) && (radarTrack.tgtRngValid))
            {
               double az_deg = radarTrack.tgtBearing_rad * UtMath::cDEG_PER_RAD;
               double rng_nm = radarTrack.tgtRng_m * UtMath::cNM_PER_M;

               double hdg_deg = mPlatData.heading_deg;

               // Relative az
               az_deg = az_deg - hdg_deg;
               if (az_deg > 360.0)
               {
                  az_deg -= 360.0;
               }
               if (az_deg < -360.0)
               {
                  az_deg += 360.0;
               }

               // Now limit to +/-180
               if (az_deg > 180.0)
               {
                  az_deg -= 360.0;
               }
               if (az_deg < -180.0)
               {
                  az_deg += 360.0;
               }

               // Scale can change
               double blipX = (8.0 * ppd) * (az_deg / 60.0);
               double blipY = (-8.0 * ppd) + (16.0 * ppd) * (rng_nm / static_cast<double>(mRadarRng_nm));

               int  alt_kft      = static_cast<int>(radarTrack.tgtAlt_ft / 1000.0);
               bool primaryTgt   = false;
               bool secondaryTgt = false;

               DrawRadarTarget(aVectorGeode,
                               blipX,
                               blipY,
                               mPlatData.heading_deg,
                               radarTrack.tgtHdg_rad * UtMath::cDEG_PER_RAD,
                               alt_kft,
                               gridColor,
                               primaryTgt,
                               secondaryTgt);
            }
         }
      }
   }

   // Draw the target designation symbol, if indicated by the modes above
   if (drawTD_Symbol)
   {
      double tdX    = mRadarTgtDesignatorX * 8.0 * ppd;
      double tdY    = mRadarTgtDesignatorY * 8.0 * ppd;
      double tdLftX = tdX - 0.35 * ppd;
      double tdLftY = tdY;
      double tdRgtX = tdX + 0.35 * ppd;
      double tdRgtY = tdY;

      osg::ref_ptr<osg::Vec2Array> tdLftVertArray(new osg::Vec2Array);
      tdLftVertArray->push_back(osg::Vec2d(tdLftX - 0.08 * ppd, tdLftY - 0.45 * ppd));
      tdLftVertArray->push_back(osg::Vec2d(tdLftX + 0.08 * ppd, tdLftY - 0.45 * ppd));
      tdLftVertArray->push_back(osg::Vec2d(tdLftX + 0.08 * ppd, tdLftY + 0.45 * ppd));
      tdLftVertArray->push_back(osg::Vec2d(tdLftX - 0.08 * ppd, tdLftY + 0.45 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(tdLftVertArray, GL_POLYGON, gridColor).get());

      osg::ref_ptr<osg::Vec2Array> tdRgtVertArray(new osg::Vec2Array);
      tdRgtVertArray->push_back(osg::Vec2d(tdRgtX - 0.08 * ppd, tdRgtY - 0.45 * ppd));
      tdRgtVertArray->push_back(osg::Vec2d(tdRgtX + 0.08 * ppd, tdRgtY - 0.45 * ppd));
      tdRgtVertArray->push_back(osg::Vec2d(tdRgtX + 0.08 * ppd, tdRgtY + 0.45 * ppd));
      tdRgtVertArray->push_back(osg::Vec2d(tdRgtX - 0.08 * ppd, tdRgtY + 0.45 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(tdRgtVertArray, GL_POLYGON, gridColor).get());
   }

   // Draw highlight box if active
   DrawMfdHighlightBox(aVectorGeode, aActive);
}

void WkP6DOF_Controller::HUD::DrawRadarTarget(osg::ref_ptr<osg::Geode>&          aVectorGeode,
                                              double                             aCx,
                                              double                             aCy,
                                              double                             aOwnshipHeading_deg,
                                              double                             aTargetHeading_deg,
                                              int                                aAlt_kft,
                                              const osg::ref_ptr<osg::Vec4Array> aColor,
                                              bool                               aPrimaryTgt,
                                              bool                               aSecondaryTgt)
{
   float ppd = mPixelsPerDegree;

   if (debugFlagHUD)
   {
      std::cout << "Ownship Heading = " << aOwnshipHeading_deg << "  Tgt Heading = " << aTargetHeading_deg << std::endl;
   }

   double relativeHeading_deg = aTargetHeading_deg - aOwnshipHeading_deg;
   if (relativeHeading_deg > 360.0)
   {
      relativeHeading_deg -= 360.0;
   }
   if (relativeHeading_deg < -180.0)
   {
      relativeHeading_deg += 360.0;
   }
   double                             relativeHeading_rad = relativeHeading_deg * UtMath::cRAD_PER_DEG;
   osg::ref_ptr<osg::MatrixTransform> xform(new osg::MatrixTransform());
   xform->postMult(osg::Matrix::rotate(osg::Quat(relativeHeading_rad, osg::Vec3(0, 0, -1))));
   xform->postMult(osg::Matrix::translate(aCx, aCy, 0.0));

   if (debugFlagHUD)
   {
      std::cout << "  relativeHeading_deg = " << relativeHeading_deg << std::endl;
   }

   if (aPrimaryTgt)
   {
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 1.0 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, -0.25 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, -0.25 * ppd));
      xform->addChild(CreateHudGeometry(vertArray, GL_LINES, aColor).get());
   }
   else if (aSecondaryTgt)
   {
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 1.0 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, -0.25 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, -0.25 * ppd));
      xform->addChild(CreateHudGeometry(vertArray, GL_LINES, aColor).get());
   }
   else
   {
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 1.0 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, 0.0));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(-0.25 * ppd, -0.25 * ppd));
      vertArray->push_back(osg::Vec2(0.0, 0.0));
      vertArray->push_back(osg::Vec2(0.25 * ppd, -0.25 * ppd));
      xform->addChild(CreateHudGeometry(vertArray, GL_LINES, aColor).get());
   }

   // xform->setStateSet(GetStencilDrawWhereMatch().get());
   aVectorGeode->addChild(xform.get());
}

void WkP6DOF_Controller::HUD::DrawMFD_RWR(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   int fontSizeBorderText = 8 * mFontSizeMult;
   int fontSizeSymbols    = 8 * mFontSizeMult;

   // Set color
   osg::ref_ptr<osg::Vec4Array> gridColor(new osg::Vec4Array);
   gridColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   // Rings
   const int                    numPts = 40;
   double                       dang   = 2.0 * 3.1415927 / (double)numPts;
   double                       radius = ppd * 2.0;
   osg::ref_ptr<osg::Vec2Array> vertArray1(new osg::Vec2Array);
   for (int i = 0; i < numPts; ++i)
   {
      double     ang = (double)i * dang;
      double     x   = radius * cos(ang);
      double     y   = radius * sin(ang);
      osg::Vec2d v(x, y);
      vertArray1->push_back(v);
   }
   aVectorGeode->addChild(CreateHudGeometry(vertArray1, GL_LINE_LOOP, gridColor).get());

   radius = ppd * 4.0;
   osg::ref_ptr<osg::Vec2Array> vertArray2(new osg::Vec2Array);
   for (int i = 0; i < numPts; ++i)
   {
      double     ang = (double)i * dang;
      double     x   = radius * cos(ang);
      double     y   = radius * sin(ang);
      osg::Vec2d v(x, y);
      vertArray2->push_back(v);
   }
   aVectorGeode->addChild(CreateHudGeometry(vertArray2, GL_LINE_LOOP, gridColor).get());

   radius = ppd * 6.0;
   osg::ref_ptr<osg::Vec2Array> vertArray3(new osg::Vec2Array);
   for (int i = 0; i < numPts; ++i)
   {
      double     ang = (double)i * dang;
      double     x   = radius * cos(ang);
      double     y   = radius * sin(ang);
      osg::Vec2d v(x, y);
      vertArray3->push_back(v);
   }
   aVectorGeode->addChild(CreateHudGeometry(vertArray3, GL_LINE_LOOP, gridColor).get());

   radius = ppd * 8.0;
   osg::ref_ptr<osg::Vec2Array> vertArray4(new osg::Vec2Array);
   for (int i = 0; i < numPts; ++i)
   {
      double     ang = (double)i * dang;
      double     x   = radius * cos(ang);
      double     y   = radius * sin(ang);
      osg::Vec2d v(x, y);
      vertArray4->push_back(v);
   }
   aVectorGeode->addChild(CreateHudGeometry(vertArray4, GL_LINE_LOOP, gridColor).get());

   // Center plus
   osg::ref_ptr<osg::Vec2Array> vertArray5(new osg::Vec2Array);
   vertArray5->push_back(osg::Vec2d(-0.3 * ppd, 0.0 * ppd));
   vertArray5->push_back(osg::Vec2d(0.3 * ppd, 0.0 * ppd));
   vertArray5->push_back(osg::Vec2d(0.0 * ppd, -0.3 * ppd));
   vertArray5->push_back(osg::Vec2d(0.0 * ppd, 0.3 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(vertArray5, GL_LINES, gridColor).get());

   // Jammer center blink
   if (mPlatData.ecmXmit)
   {
      if (mFastBlink)
      {
         osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
         vertArray->push_back(osg::Vec2d(-0.4 * ppd, 0.0 * ppd));
         vertArray->push_back(osg::Vec2d(-0.6 * ppd, 0.0 * ppd));
         vertArray->push_back(osg::Vec2d(0.4 * ppd, 0.0 * ppd));
         vertArray->push_back(osg::Vec2d(0.6 * ppd, 0.0 * ppd));
         vertArray->push_back(osg::Vec2d(0.0 * ppd, -0.4 * ppd));
         vertArray->push_back(osg::Vec2d(0.0 * ppd, -0.6 * ppd));
         vertArray->push_back(osg::Vec2d(0.0 * ppd, 0.4 * ppd));
         vertArray->push_back(osg::Vec2d(0.0 * ppd, 0.6 * ppd));
         vertArray->push_back(osg::Vec2d(-0.2 * ppd, 0.2 * ppd));
         vertArray->push_back(osg::Vec2d(-0.5 * ppd, 0.5 * ppd));
         vertArray->push_back(osg::Vec2d(-0.2 * ppd, -0.2 * ppd));
         vertArray->push_back(osg::Vec2d(-0.5 * ppd, -0.5 * ppd));
         vertArray->push_back(osg::Vec2d(0.2 * ppd, 0.2 * ppd));
         vertArray->push_back(osg::Vec2d(0.5 * ppd, 0.5 * ppd));
         vertArray->push_back(osg::Vec2d(0.2 * ppd, -0.2 * ppd));
         vertArray->push_back(osg::Vec2d(0.5 * ppd, -0.5 * ppd));
         aVectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINES, gridColor).get());
      }
   }

   const int lesserNumPts = 40;
   double    lesserDang   = 2.0 * 3.1415927 / (double)lesserNumPts;

   bool primaryThreatTypeAI  = false;
   bool primaryThreatTypeSAM = false;
   bool primaryThreatTypeAAA = false;
   bool primaryThreatTypeMSL = false;

   // Draw RWR tracks
   for (auto& rwrTrack : mPlatData.rwrTracks)
   {
      if (rwrTrack.tgtAzValid)
      {
         double az_rad = rwrTrack.tgtAz_rad;

         // Read bit-packed data

         bool primaryThreat = false;
         if ((rwrTrack.tgtPackedData & 0x00000001) == 0x00000001)
         {
            primaryThreat = true;
         }

         bool newThreat = false;
         if ((rwrTrack.tgtPackedData & 0x00000008) == 0x00000008)
         {
            newThreat = true;
         }

         bool highThreat = false;
         if ((rwrTrack.tgtPackedData & 0x00000010) == 0x00000010)
         {
            highThreat = true;
         }

         bool airThreat = false;
         if ((rwrTrack.tgtPackedData & 0x00000020) == 0x00000020)
         {
            airThreat = true;
         }

         // If this is the primary threat, set the primary threat type
         if (primaryThreat)
         {
            if ((rwrTrack.tgtPackedData & 0x000000C0) == 0x00000040)
            {
               primaryThreatTypeSAM = true;
            }
            else if ((rwrTrack.tgtPackedData & 0x000000C0) == 0x00000080)
            {
               primaryThreatTypeAAA = true;
            }
            else if ((rwrTrack.tgtPackedData & 0x000000C0) == 0x000000C0)
            {
               primaryThreatTypeMSL = true;
            }
            else
            {
               primaryThreatTypeAI = true;
            }
         }

         int ring = 0;
         if ((rwrTrack.tgtPackedData & 0x00000002) == 0x00000002)
         {
            ring = 1;
         }
         else if ((rwrTrack.tgtPackedData & 0x00000004) == 0x00000004)
         {
            ring = 2;
         }
         else if ((rwrTrack.tgtPackedData & 0x00000006) == 0x00000006)
         {
            ring = 3;
         }

         // Set blip location based on ring distance
         double blipX = 0.0;
         double blipY = 0.0;
         if (ring == 1) // Ring 1 is inner ring, for critical threats (such as launch or missile)
         {
            blipX = (3.0 * ppd) * sin(az_rad);
            blipY = (3.0 * ppd) * cos(az_rad);
         }
         else if (ring == 2) // Ring 2 is middle ring, for lethal threats (such as tracking)
         {
            blipX = (5.0 * ppd) * sin(az_rad);
            blipY = (5.0 * ppd) * cos(az_rad);
         }
         else if (ring == 3) // Ring 3 is outer ring, for non-lethal threats (such as searching)
         {
            blipX = (7.0 * ppd) * sin(az_rad);
            blipY = (7.0 * ppd) * cos(az_rad);
         }

         // If air threat, draw "wings" chevron
         if (airThreat)
         {
            osg::ref_ptr<osg::Vec2Array> wingVertArray(new osg::Vec2Array);
            wingVertArray->push_back(osg::Vec2d(blipX - 0.6 * ppd, blipY + 0.30 * ppd));
            wingVertArray->push_back(osg::Vec2d(blipX, blipY + 0.75 * ppd));
            wingVertArray->push_back(osg::Vec2d(blipX + 0.6 * ppd, blipY + 0.30 * ppd));
            aVectorGeode->addChild(CreateHudGeometry(wingVertArray, GL_LINE_STRIP, gridColor).get());
         }

         // If primary threat, draw diamond
         if (primaryThreat)
         {
            osg::ref_ptr<osg::Vec2Array> diamondVertArray(new osg::Vec2Array);
            diamondVertArray->push_back(osg::Vec2d(blipX - ppd, blipY));
            diamondVertArray->push_back(osg::Vec2d(blipX, blipY - ppd));
            diamondVertArray->push_back(osg::Vec2d(blipX + ppd, blipY));
            diamondVertArray->push_back(osg::Vec2d(blipX, blipY + ppd));
            aVectorGeode->addChild(CreateHudGeometry(diamondVertArray, GL_LINE_LOOP, gridColor).get());
         }

         // If new threat, draw a top half circle
         if (newThreat)
         {
            radius = ppd * 1.0;
            osg::ref_ptr<osg::Vec2Array> halfCircleArray(new osg::Vec2Array);
            for (int i = 0; i <= lesserNumPts / 2; ++i)
            {
               double     ang = (double)i * lesserDang;
               double     x   = blipX + radius * cos(ang);
               double     y   = blipY + radius * sin(ang);
               osg::Vec2d v(x, y);
               halfCircleArray->push_back(v);
            }
            aVectorGeode->addChild(CreateHudGeometry(halfCircleArray, GL_LINE_STRIP, gridColor).get());
         }

         // If high threat, draw a blinking circle
         if (highThreat)
         {
            if (mSuperFastBlink)
            {
               radius = ppd * 1.0;
               osg::ref_ptr<osg::Vec2Array> circleArray(new osg::Vec2Array);
               for (int i = 0; i < lesserNumPts; ++i)
               {
                  double     ang = (double)i * lesserDang;
                  double     x   = blipX + radius * cos(ang);
                  double     y   = blipY + radius * sin(ang);
                  osg::Vec2d v(x, y);
                  circleArray->push_back(v);
               }
               aVectorGeode->addChild(CreateHudGeometry(circleArray, GL_LINE_LOOP, gridColor).get());
            }
         }

         // Draw the threat label, or "U" is unknown
         std::stringstream blipName;
         if (rwrTrack.tgtIdValid)
         {
            blipName << rwrTrack.tgtIdString;
         }
         else
         {
            blipName << "U" << std::setw(4) << (int)mPlatData.ktas;
         }
         aVectorGeode->addChild(
            AddTextItem(blipName.str(), blipX, blipY, fontSizeSymbols, osgText::Text::CENTER_CENTER).get());
      }
   }

   // EW top label
   aVectorGeode->addChild(AddTextItem("EW", 0.0 * ppd, 9.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());

   // Jammer status text (top right)

   // Draw JAM text
   aVectorGeode->addChild(AddTextItem("JAM", 8.0 * ppd, 8.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());

   // If powered, draw additional items
   if (mPlatData.ecmPower)
   {
      // Draw box if powered on
      osg::ref_ptr<osg::Vec2Array> primaryThreatArray(new osg::Vec2Array);
      primaryThreatArray->push_back(osg::Vec2d(7.0 * ppd, 9.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(7.0 * ppd, 8.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(9.0 * ppd, 8.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(9.0 * ppd, 9.0 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(primaryThreatArray, GL_LINE_LOOP, gridColor).get());

      // Draw status - XMIT overrides other indications
      if (mPlatData.ecmXmit)
      {
         if (mFastBlink)
         {
            aVectorGeode->addChild(
               AddTextItem("XMIT", 8.0 * ppd, 7.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
         }
      }
      else if (mPlatData.ecmAuto)
      {
         aVectorGeode->addChild(
            AddTextItem("AUTO", 8.0 * ppd, 7.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
      }
      else
      {
         aVectorGeode->addChild(
            AddTextItem("STBY", 8.0 * ppd, 7.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
      }
   }

   // Priority threat indicator (top left)

   // Draw text
   bool drawPrimaryThreatBox = false;
   if (primaryThreatTypeAI)
   {
      drawPrimaryThreatBox = true;
      aVectorGeode->addChild(
         AddTextItem("AI", -8.0 * ppd, 8.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
   }
   else if (primaryThreatTypeSAM)
   {
      drawPrimaryThreatBox = true;
      aVectorGeode->addChild(
         AddTextItem("SAM", -8.0 * ppd, 8.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
   }
   else if (primaryThreatTypeAAA)
   {
      drawPrimaryThreatBox = true;
      aVectorGeode->addChild(
         AddTextItem("AAA", -8.0 * ppd, 8.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
   }
   else if (primaryThreatTypeMSL)
   {
      drawPrimaryThreatBox = true;
      aVectorGeode->addChild(
         AddTextItem("MSL", -8.0 * ppd, 8.5 * ppd, fontSizeBorderText, osgText::Text::CENTER_CENTER).get());
   }

   // Draw the box, if appropriate
   if (drawPrimaryThreatBox)
   {
      osg::ref_ptr<osg::Vec2Array> primaryThreatArray(new osg::Vec2Array);
      primaryThreatArray->push_back(osg::Vec2d(-9.0 * ppd, 9.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(-9.0 * ppd, 8.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(-7.0 * ppd, 8.0 * ppd));
      primaryThreatArray->push_back(osg::Vec2d(-7.0 * ppd, 9.0 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(primaryThreatArray, GL_LINE_LOOP, gridColor).get());
   }

   // Chaff/flares status text (bottom left)

   // Draw chaff count
   std::stringstream chaffStr;
   chaffStr << "C " << std::setw(3) << mPlatData.numChaff;
   aVectorGeode->addChild(
      AddTextItem(chaffStr.str(), -9.0 * ppd, -7.5 * ppd, fontSizeBorderText, osgText::Text::LEFT_CENTER).get());

   // Draw box if dispensing chaff
   if (mPlatData.dispenseChaffPressed || mPlatData.dispenseCountermeasuresPressed)
   {
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2d(-9.2 * ppd, -7.0 * ppd));
      vertArray->push_back(osg::Vec2d(-9.2 * ppd, -8.0 * ppd));
      vertArray->push_back(osg::Vec2d(-6.0 * ppd, -8.0 * ppd));
      vertArray->push_back(osg::Vec2d(-6.0 * ppd, -7.0 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, gridColor).get());
   }

   // Draw flares count
   std::stringstream flareStr;
   flareStr << "F " << std::setw(3) << mPlatData.numFlares;
   aVectorGeode->addChild(
      AddTextItem(flareStr.str(), -9.0 * ppd, -8.5 * ppd, fontSizeBorderText, osgText::Text::LEFT_CENTER).get());

   // Draw box if dispensing flares
   if (mPlatData.dispenseFlaresPressed || mPlatData.dispenseCountermeasuresPressed)
   {
      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2d(-9.2 * ppd, -8.0 * ppd));
      vertArray->push_back(osg::Vec2d(-9.2 * ppd, -9.0 * ppd));
      vertArray->push_back(osg::Vec2d(-6.0 * ppd, -9.0 * ppd));
      vertArray->push_back(osg::Vec2d(-6.0 * ppd, -8.0 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, gridColor).get());
   }

   // Towed decoys status text (bottom right)
   // *** This will be a future improvement ***

   // Draw highlight box if active
   DrawMfdHighlightBox(aVectorGeode, aActive);
}

void WkP6DOF_Controller::HUD::DrawMFD_IRST(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   // Set color
   osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
   textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "IRST DISPLAY", "UNAVAILABLE");
}

void WkP6DOF_Controller::HUD::DrawMFD_FLIR(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   // Set color
   osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
   textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "FLIR DISPLAY", "UNAVAILABLE");
}

void WkP6DOF_Controller::HUD::DrawMFD_Datalink(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   int fontSize = 8 * mFontSizeMult;

   // Set the main color
   osg::ref_ptr<osg::Vec4Array> gridColor(new osg::Vec4Array);
   gridColor->push_back(osg::Vec4(1.0, 1.0, 1.0, 0.7));

   // Set the red forces color
   osg::ref_ptr<osg::Vec4Array> redColor(new osg::Vec4Array);
   redColor->push_back(osg::Vec4(1.0, 0.2, 0.1, 0.7));

   // Set the blue forces color
   osg::ref_ptr<osg::Vec4Array> blueColor(new osg::Vec4Array);
   blueColor->push_back(osg::Vec4(0.0, 0.5, 1.0, 0.7));

   // Set the green forces color
   osg::ref_ptr<osg::Vec4Array> greenColor(new osg::Vec4Array);
   greenColor->push_back(osg::Vec4(0.1, 1.0, 0.3, 0.7));

   // Set the yellow forces color
   osg::ref_ptr<osg::Vec4Array> yellowColor(new osg::Vec4Array);
   yellowColor->push_back(osg::Vec4(0.8, 0.8, 0.2, 0.7));

   // Draw center symbol
   osg::ref_ptr<osg::Vec2Array> centerSymbol(new osg::Vec2Array);
   centerSymbol->push_back(osg::Vec2d(0.0, -4.0 * ppd));
   centerSymbol->push_back(osg::Vec2d(-0.3 * ppd, -4.7 * ppd));
   centerSymbol->push_back(osg::Vec2d(0.3 * ppd, -4.7 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(centerSymbol, GL_LINE_LOOP, blueColor).get());

   // Draw the rings
   DrawCircle(0.0, -4.0 * ppd, 3.5 * ppd, gridColor, aVectorGeode, 60.0);
   DrawCircle(0.0, -4.0 * ppd, 7.0 * ppd, gridColor, aVectorGeode, 60.0);
   DrawCircle(0.0, -4.0 * ppd, 10.5 * ppd, gridColor, aVectorGeode, 60.0);

   DrawCircle(4.0 * ppd, 4.0 * ppd, 2.0 * ppd, yellowColor, aVectorGeode, 60.0);
   DrawCircle(-5.0 * ppd, 2.0 * ppd, 1.0 * ppd, greenColor, aVectorGeode, 60.0);

   // Draw heading range
   std::stringstream hdgString;
   int               hdg = static_cast<int>(mPlatData.heading_deg + 0.5);
   if (hdg <= 0)
   {
      hdg += 360;
   }
   if (hdg > 360)
   {
      hdg -= 360;
   }
   hdgString << std::setfill('0') << std::setw(3) << hdg;
   aVectorGeode->addChild(
      AddTextItem(hdgString.str(), 0.0, 8.5 * ppd, fontSize, gridColor, osgText::Text::CENTER_CENTER).get());

   // Draw max range
   std::stringstream rngString;
   rngString << std::setw(3) << mRadarRng_nm;
   aVectorGeode->addChild(
      AddTextItem(rngString.str(), 8.0 * ppd, 8.5 * ppd, fontSize, gridColor, osgText::Text::RIGHT_CENTER).get());

   // Draw ground speed
   std::stringstream gndSpdString;
   gndSpdString << "G" << static_cast<int>(mPlatData.ktas);
   aVectorGeode->addChild(
      AddTextItem(gndSpdString.str(), -8.0 * ppd, -8.5 * ppd, fontSize, gridColor, osgText::Text::LEFT_CENTER).get());

   // Draw true air speed
   std::stringstream trueAirSpdString;
   trueAirSpdString << "T" << static_cast<int>(mPlatData.ktas);
   aVectorGeode->addChild(
      AddTextItem(trueAirSpdString.str(), 8.0 * ppd, -8.5 * ppd, fontSize, gridColor, osgText::Text::RIGHT_CENTER).get());


   // Draw tracks

   // To draw tracks, we need to know the radar mode. For RWS, we draw multiple blocks.
   // For TWS, we draw multiple directional symbols. For STT, we draw a single directional
   // symbol. Note that auto-acquisition modes (WACQ, VACQ, and BORE) do not draw tracks,
   // since once they have a track, they will auto-switch to STT mode. We also check for
   // which modes should show the TD symbol.
   int trackCount = 0;
   for (auto& masterTrack : mPlatData.masterTracks)
   {
      ++trackCount;
      if ((masterTrack.tgtBearingValid) && (masterTrack.tgtRngValid))
      {
         double az_deg  = masterTrack.tgtBearing_rad * UtMath::cDEG_PER_RAD;
         double rng_nm  = masterTrack.tgtRng_m * UtMath::cNM_PER_M;
         double hdg_deg = mPlatData.heading_deg;

         // Relative az
         az_deg = az_deg - hdg_deg;
         if (az_deg > 360.0)
         {
            az_deg -= 360.0;
         }
         if (az_deg < -360.0)
         {
            az_deg += 360.0;
         }

         // Now limit to +/-180
         if (az_deg > 180.0)
         {
            az_deg -= 360.0;
         }
         if (az_deg < -180.0)
         {
            az_deg += 360.0;
         }

         // Scale can change
         double radius       = 10.5 * ppd * (rng_nm / static_cast<double>(mRadarRng_nm));
         double az_rad       = az_deg * UtMath::cRAD_PER_DEG;
         double blipX        = radius * sin(az_rad);
         double blipY        = (-4.0 * ppd) + radius * cos(az_rad);
         int    alt_kft      = static_cast<int>(masterTrack.tgtAlt_ft / 1000.0);
         bool   primaryTgt   = false;
         bool   secondaryTgt = false;

         DrawRadarTarget(aVectorGeode,
                         blipX,
                         blipY,
                         mPlatData.heading_deg,
                         masterTrack.tgtHdg_rad * UtMath::cDEG_PER_RAD,
                         alt_kft,
                         redColor,
                         primaryTgt,
                         secondaryTgt);
      }
      else if (masterTrack.tgtLLA_Valid)
      {
         double tgtLat    = masterTrack.tgtLat;
         double tgtLon    = masterTrack.tgtLon;
         double tgtAlt_ft = masterTrack.tgtAlt_m * UtMath::cFT_PER_M;
         double platLat   = mPlatData.lat_deg;
         double platLon   = mPlatData.lon_deg;

         double heading_rad = 0.0;
         double distance_m  = 0.0;
         UtSphericalEarth::GreatCircleHeadingAndDistance(platLat, platLon, tgtLat, tgtLon, heading_rad, distance_m);

         double heading_deg = heading_rad * UtMath::cDEG_PER_RAD;
         double platHdg_deg = mPlatData.heading_deg;
         heading_deg        = heading_rad - platHdg_deg;
         if (heading_deg > 360.0)
         {
            heading_deg -= 360.0;
         }
         if (heading_deg < 0.0)
         {
            heading_deg += 360.0;
         }

         if (heading_deg > 180.0)
         {
            heading_deg -= 360.0;
         }
         if (heading_deg < -180.0)
         {
            heading_deg += 360.0;
         }

         double az_rad = heading_deg * UtMath::cRAD_PER_DEG;
         double rng_nm = distance_m * UtMath::cNM_PER_M;
         double radius = 10.5 * ppd * (rng_nm / static_cast<double>(mRadarRng_nm));

         double blipX = radius * sin(az_rad);
         double blipY = (-4.0 * ppd) + radius * cos(az_rad);

         int  alt_kft      = static_cast<int>(tgtAlt_ft / 1000.0);
         bool primaryTgt   = false;
         bool secondaryTgt = false;

         DrawRadarTarget(aVectorGeode,
                         blipX,
                         blipY,
                         mPlatData.heading_deg,
                         masterTrack.tgtHdg_rad * UtMath::cDEG_PER_RAD,
                         alt_kft,
                         redColor,
                         primaryTgt,
                         secondaryTgt);
      }
      else
      {
         if (debugFlagHUD)
         {
            if ((!masterTrack.tgtBearingValid) && (!masterTrack.tgtRngValid))
            {
               std::cout << "  TRACK (" << trackCount - 1 << ") lack bearing and range info" << std::endl;
            }
            else if (!masterTrack.tgtBearingValid)
            {
               std::cout << "  TRACK (" << trackCount - 1 << ") lack bearing info" << std::endl;
            }
            else if (!masterTrack.tgtRngValid)
            {
               std::cout << "  TRACK (" << trackCount - 1 << ") lack range info" << std::endl;
            }
         }
      }
   }
   // Draw highlight box if active
   DrawMfdHighlightBox(aVectorGeode, aActive);
}

void WkP6DOF_Controller::HUD::DrawMFD_Weapons(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   // Set color
   osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
   textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "WEAPONS DISPLAY", "UNAVAILABLE");
}

void WkP6DOF_Controller::HUD::DrawMFD_Fuel(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   // Set color
   osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
   textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "FUEL DISPLAY", "UNAVAILABLE");
}

void WkP6DOF_Controller::HUD::DrawMFD_Engines(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   // Set color
   osg::ref_ptr<osg::Vec4Array> textColor(new osg::Vec4Array);
   textColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   DrawCenterTextForUnsupportedDisplay(aVectorGeode, aActive, textColor, "ENGINES DISPLAY", "UNAVAILABLE");
}

void WkP6DOF_Controller::HUD::DrawCenterTextForUnsupportedDisplay(osg::ref_ptr<osg::Geode>&     aVectorGeode,
                                                                  bool                          aActive,
                                                                  osg::ref_ptr<osg::Vec4Array>& aGridColor,
                                                                  const std::string&            aString1,
                                                                  const std::string&            aString2)
{
   float ppd = mPixelsPerDegree;

   int fontSize = 12 * mFontSizeMult;

   // Display Type
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-6.0 * ppd, -2.0 * ppd));
   vertArray->push_back(osg::Vec2d(6.0 * ppd, -2.0 * ppd));
   vertArray->push_back(osg::Vec2d(6.0 * ppd, 2.0 * ppd));
   vertArray->push_back(osg::Vec2d(-6.0 * ppd, 2.0 * ppd));
   aVectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, aGridColor).get());
   std::stringstream upperStr;
   upperStr << aString1;
   aVectorGeode->addChild(AddTextItem(upperStr.str(), 0.0, 0.8 * ppd, fontSize, osgText::Text::CENTER_CENTER).get());
   std::stringstream lowerStr;
   lowerStr << aString2;
   aVectorGeode->addChild(AddTextItem(lowerStr.str(), 0.0, -0.8 * ppd, fontSize, osgText::Text::CENTER_CENTER).get());

   // Draw highlight box if active
   DrawMfdHighlightBox(aVectorGeode, aActive);
}

void WkP6DOF_Controller::HUD::DrawMfdHighlightBox(osg::ref_ptr<osg::Geode>& aVectorGeode, bool aActive)
{
   float ppd = mPixelsPerDegree;

   if (aActive)
   {
      // Set color
      osg::ref_ptr<osg::Vec4Array> highlightColor(new osg::Vec4Array);
      highlightColor->push_back(osg::Vec4(0.9, 0.9, 0.9, 0.7));

      osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
      vertArray->push_back(osg::Vec2d(-9.9 * ppd, -9.9 * ppd));
      vertArray->push_back(osg::Vec2d(9.9 * ppd, -9.9 * ppd));
      vertArray->push_back(osg::Vec2d(9.9 * ppd, 9.9 * ppd));
      vertArray->push_back(osg::Vec2d(-9.9 * ppd, 9.9 * ppd));
      aVectorGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, highlightColor).get());
   }
}

void WkP6DOF_Controller::HUD::DrawUFC()
{
   float ppd = mPixelsPerDegree;

   double cx = 0.0 * ppd;
   double cy = -18.0 * ppd;

   osg::ref_ptr<osg::Geode>     vectorGeode(new osg::Geode);
   osg::ref_ptr<osg::Group>     vectorGroup(new osg::Group);
   osg::ref_ptr<osg::Vec2Array> verts(new osg::Vec2Array);

   // Transformation
   osg::ref_ptr<osg::PositionAttitudeTransform> vectorXform(new osg::PositionAttitudeTransform());
   vectorXform->setPosition(osg::Vec3d(cx, cy, 0.0));

   // Set background color
   osg::ref_ptr<osg::Vec4Array> backgroundColor(new osg::Vec4Array);
   backgroundColor->push_back(osg::Vec4(0.0, 0.0, 0.0, 0.3));

   // UFC background
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);
   vertArray->push_back(osg::Vec2d(-14.0 * ppd, -3.0 * ppd));
   vertArray->push_back(osg::Vec2d(14.0 * ppd, -3.0 * ppd));
   vertArray->push_back(osg::Vec2d(14.0 * ppd, 1.0 * ppd));
   vertArray->push_back(osg::Vec2d(12.0 * ppd, 3.0 * ppd));
   vertArray->push_back(osg::Vec2d(-12.0 * ppd, 3.0 * ppd));
   vertArray->push_back(osg::Vec2d(-14.0 * ppd, 1.0 * ppd));
   vectorGeode->addChild(CreateHudGeometry(vertArray, GL_POLYGON, backgroundColor).get());

   // Green color
   osg::ref_ptr<osg::Vec4Array> greenColor(new osg::Vec4Array);
   greenColor->push_back(osg::Vec4(0.0, 0.9, 0.3, 0.7));

   // Yellow color
   osg::ref_ptr<osg::Vec4Array> yellowColor(new osg::Vec4Array);
   yellowColor->push_back(osg::Vec4(0.9, 0.8, 0.0, 0.7));

   // Red color
   osg::ref_ptr<osg::Vec4Array> redColor(new osg::Vec4Array);
   redColor->push_back(osg::Vec4(0.9, 0.2, 0.0, 0.7));

   // Engine MIL ring
   osg::ref_ptr<osg::Vec2Array> ringArray(new osg::Vec2Array);
   for (int ang_deg = 90; ang_deg >= -180; ang_deg -= 10)
   {
      double ang_rad = (double)ang_deg * UtMath::cRAD_PER_DEG;
      double x       = (-10.0 * ppd) + (2.0 * ppd) * cos(ang_rad);
      double y       = (2.0 * ppd) * sin(ang_rad);
      ringArray->push_back(osg::Vec2d(x, y));
   }
   vectorGeode->addChild(CreateHudGeometry(ringArray, GL_LINE_STRIP, greenColor).get());

   // Engine AB ring
   osg::ref_ptr<osg::Vec2Array> abRingArray(new osg::Vec2Array);
   for (int ang_deg = -180; ang_deg >= -240; ang_deg -= 10)
   {
      double ang_rad = (double)ang_deg * UtMath::cRAD_PER_DEG;
      double x       = (-10.0 * ppd) + (2.0 * ppd) * cos(ang_rad);
      double y       = (2.0 * ppd) * sin(ang_rad);
      abRingArray->push_back(osg::Vec2d(x, y));
   }
   vectorGeode->addChild(CreateHudGeometry(abRingArray, GL_LINE_STRIP, redColor).get());


   double tickAng_deg = 0.0;
   if (mPlatData.throttleMilitaryControllerPosition >= 1.0)
   {
      if (mPlatData.throttleAfterburnerControllerPosition > 0.0)
      {
         // Using afterburner
         tickAng_deg = 270.0 + 60.0 * mPlatData.throttleAfterburnerControllerPosition;
      }
      else
      {
         // No afterburner but full MIL
         tickAng_deg = 270.0;
      }
   }
   else
   {
      tickAng_deg = 270.0 * mPlatData.throttleMilitaryControllerPosition;
   }
   double                             tickAng_rad = tickAng_deg * UtMath::cRAD_PER_DEG;
   osg::ref_ptr<osg::MatrixTransform> tickXform(new osg::MatrixTransform());
   tickXform->postMult(osg::Matrix::rotate(osg::Quat(tickAng_rad, osg::Vec3(0, 0, -1))));
   tickXform->postMult(osg::Matrix::translate(-10.0 * ppd, 0.0, 0.0));
   osg::ref_ptr<osg::Vec2Array> tickArray(new osg::Vec2Array);
   tickArray->push_back(osg::Vec2d(0.0, 2.0 * ppd));
   tickArray->push_back(osg::Vec2d(0.0, 2.4 * ppd));

   if (tickAng_deg > 270.0)
   {
      tickXform->addChild(CreateHudGeometry(tickArray, GL_LINES, redColor).get());
   }
   else
   {
      tickXform->addChild(CreateHudGeometry(tickArray, GL_LINES, greenColor).get());
   }

   vectorGeode->addChild(tickXform.get());

   double needleAng_deg = 0.0;
   if (mPlatData.engineThrottleLeverMilitary >= 1.0)
   {
      if (mPlatData.engineThrottleLeverAfterburner > 0.0)
      {
         // Using afterburner
         needleAng_deg = 270.0 + 60.0 * mPlatData.engineThrottleLeverAfterburner;
      }
      else
      {
         // No afterburner but full MIL
         needleAng_deg = 270.0;
      }
   }
   else
   {
      needleAng_deg = 270.0 * mPlatData.engineThrottleLeverMilitary;
   }
   double                             needleAng_rad = needleAng_deg * UtMath::cRAD_PER_DEG;
   osg::ref_ptr<osg::MatrixTransform> needleXform(new osg::MatrixTransform());
   needleXform->postMult(osg::Matrix::rotate(osg::Quat(needleAng_rad, osg::Vec3(0, 0, -1))));
   needleXform->postMult(osg::Matrix::translate(-10.0 * ppd, 0.0, 0.0));
   osg::ref_ptr<osg::Vec2Array> needleArray(new osg::Vec2Array);
   needleArray->push_back(osg::Vec2d(0.0, 0.0));
   needleArray->push_back(osg::Vec2d(0.0, 2.0 * ppd));
   needleArray->push_back(osg::Vec2d(0.0, 2.0 * ppd));
   needleArray->push_back(osg::Vec2d(0.2 * ppd, 1.6 * ppd));
   needleArray->push_back(osg::Vec2d(0.0, 2.0 * ppd));
   needleArray->push_back(osg::Vec2d(-0.2 * ppd, 1.6 * ppd));
   if (needleAng_deg > 270.0)
   {
      needleXform->addChild(CreateHudGeometry(needleArray, GL_LINES, yellowColor).get());
   }
   else
   {
      needleXform->addChild(CreateHudGeometry(needleArray, GL_LINES, greenColor).get());
   }
   vectorGeode->addChild(needleXform.get());


   int fontSize = 8 * mFontSizeMult;

   double            y = 1.5 * ppd;
   std::stringstream totWgt;
   totWgt << "WGT " << std::fixed << std::setprecision(1) << std::setw(4) << mPlatData.currentWeight_lbs * 0.001;
   vectorGeode->addChild(AddTextItem(totWgt.str(), 0.0 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream totFuel;
   totFuel << "TOT " << std::fixed << std::setprecision(1) << std::setw(4)
           << mPlatData.currentTotalFuelTankQuantity_lbs * 0.001;
   vectorGeode->addChild(AddTextItem(totFuel.str(), 0.0 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream intFuel;
   intFuel << "INT " << std::fixed << std::setprecision(1) << std::setw(4)
           << mPlatData.currentInternalTotalFuelTankQuantity_lbs * 0.001;
   vectorGeode->addChild(AddTextItem(intFuel.str(), 0.0 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());

   y -= ppd;
   std::stringstream extFuel;
   double extFuel_lbs = mPlatData.currentTotalFuelTankQuantity_lbs - mPlatData.currentInternalTotalFuelTankQuantity_lbs;
   extFuel << "EXT " << std::fixed << std::setprecision(1) << std::setw(4) << extFuel_lbs * 0.001;
   vectorGeode->addChild(AddTextItem(extFuel.str(), 0.0 * ppd, y, fontSize, osgText::Text::RIGHT_CENTER).get());


   vectorXform->addChild(vectorGeode.get());
   vectorGroup->addChild(vectorXform.get());
   vectorGroup->setStateSet(GetStencilDrawWhereMatch().get());
   mHudGeode->addChild(vectorGroup.get());
}

void WkP6DOF_Controller::HUD::DrawCircle(double                        aCx,
                                         double                        aCy,
                                         double                        aRadius,
                                         osg::ref_ptr<osg::Vec4Array>& aColor,
                                         osg::ref_ptr<osg::Geode>&     aGeode,
                                         int                           aNumPts,
                                         bool                          aFilled)
{
   osg::ref_ptr<osg::Vec2Array> vertArray(new osg::Vec2Array);

   double deltaAng_rad = UtMath::cTWO_PI / static_cast<double>(aNumPts);
   for (double ang = 0; ang < UtMath::cTWO_PI; ang += deltaAng_rad)
   {
      vertArray->push_back(osg::Vec2d(aCx + aRadius * cos(ang), aCy + aRadius * sin(ang)));
   }

   if (aFilled)
   {
      aGeode->addChild(CreateHudGeometry(vertArray, GL_POLYGON, aColor).get());
   }
   else
   {
      aGeode->addChild(CreateHudGeometry(vertArray, GL_LINE_LOOP, aColor).get());
   }
}

bool WkP6DOF_Controller::HUD::LockRadarTarget(std::string& aLockedTargetName)
{
   // mRadarTgtDesignatorX ... Y ranges from +/-1
   // mPlatData.radarMinAz_deg + mPlatData.radarMaxAz_deg
   // mPlatData.radarMinEl_deg + mPlatData.radarMaxEl_deg

   // Scan (az,el) parameters
   // double scanAzCenter_deg = 0.5 * (mPlatData.radarMinAz_deg + mPlatData.radarMaxAz_deg);
   // double scanElCenter_deg = 0.5 * (mPlatData.radarMinEl_deg + mPlatData.radarMaxEl_deg);
   // double scanAzHalfFOV_deg = 0.5 * (mPlatData.radarMaxAz_deg - mPlatData.radarMinAz_deg);
   // double scanElHalfFOV_deg = 0.5 * (mPlatData.radarMaxEl_deg - mPlatData.radarMinEl_deg);

   // Determine the track x/y
   std::vector<double> trackX;
   std::vector<double> trackY;

   for (auto& radarTrack : mPlatData.radarTracks)
   {
      if ((radarTrack.tgtBearingValid) && (radarTrack.tgtRngValid))
      {
         double az_deg = radarTrack.tgtBearing_rad * UtMath::cDEG_PER_RAD;
         double rng_nm = radarTrack.tgtRng_m * UtMath::cNM_PER_M;

         double hdg_deg = mPlatData.heading_deg;

         // Relative az
         az_deg = az_deg - hdg_deg;
         if (az_deg > 360.0)
         {
            az_deg -= 360.0;
         }
         if (az_deg < -360.0)
         {
            az_deg += 360.0;
         }

         // Now limit to +/-180
         if (az_deg > 180.0)
         {
            az_deg -= 360.0;
         }
         if (az_deg < -180.0)
         {
            az_deg += 360.0;
         }

         // Scale can change
         double blipX = 8.0 * (az_deg / 60.0);
         double blipY = -8.0 + 16.0 * (rng_nm / static_cast<double>(mRadarRng_nm));

         trackX.push_back(blipX);
         trackY.push_back(blipY);
      }
   }

   // Determine the x/y for the target designation symbol
   double tdX = mRadarTgtDesignatorX * 8.0;
   double tdY = mRadarTgtDesignatorY * 8.0;

   // Now find the closest track
   size_t numTracks = mPlatData.radarTracks.size();
   double bestDist  = 1000000000000.0; // Large number
   int    bestIndex = -1;
   for (size_t i = 0; i < numTracks; ++i)
   {
      double dx   = trackX.at(i) - tdX;
      double dy   = trackY.at(i) - tdY;
      double dist = sqrt(dx * dx + dy * dy);
      if (dist < bestDist)
      {
         bestIndex = static_cast<int>(i);
         bestDist  = dist;
      }
   }

   if (bestIndex >= 0)
   {
      // We have a valid lock, so call script function
      aLockedTargetName = mPlatData.radarTracks.at(bestIndex).tgtPlatformName;
      return true;
   }

   return false;
}
