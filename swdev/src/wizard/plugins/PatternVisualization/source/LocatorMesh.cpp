// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "LocatorMesh.hpp"

#include "Angle.hpp"
#include "PlaneType.hpp"

namespace PatternVisualizer
{
LocatorMesh::LocatorMesh(Session*         aSessionPtr,
                         GraphicsEngine&  aGraphicsEngine,
                         PlaneType        aPlaneType,
                         LocatorComponent aCompType,
                         float            aDbMin,
                         float            aDbMax,
                         float            aDbInc)
   : Mesh(aSessionPtr, aGraphicsEngine, GetMeshType(aCompType), aDbMin, aDbMax, aDbInc)
   , mPlaneType(aPlaneType)
   , mCompType(aCompType)
{
   mRadiusMin = (aCompType != LocatorComponent::ARC ? cMIN_RADIUS_LOCATOR : cMIN_RADIUS_PATTERN);
   mRadiusMax = cMAX_RADIUS_PATTERN;

   CreateShader();
   CreateVertices();

   QColor color;
   switch (mCompType)
   {
   case LocatorComponent::RADIALS:
      color = QColor(150, 150, 150, 255);
      CreateColorTable(&color);
      break;
   case LocatorComponent::SECTORS:
      color = QColor(0, 0, 0, 20);
      CreateColorTable(&color);
      break;
   case LocatorComponent::ARC:
      CreateColorTable();
      break;
   }
}

std::unique_ptr<MeshBuilder> LocatorMesh::CreateMeshBuilder()
{
   return ut::make_unique<LocatorMeshBuilder>(mPlaneType, mCompType, mDBMin, mDBMax, mDBInc);
}

GLenum LocatorMesh::GetMeshType(LocatorComponent aType)
{
   GLenum retval{};
   switch (aType)
   {
   case LocatorComponent::ARC:
      retval = GL_LINE_STRIP;
      break;
   case LocatorComponent::RADIALS:
      retval = GL_LINES;
      break;
   case LocatorComponent::SECTORS:
      retval = GL_TRIANGLE_FAN;
      break;
   }
   return retval;
}

LocatorMeshBuilder::LocatorMeshBuilder(PlaneType aPlaneType, LocatorComponent aCompType, float aDbMin, float aDbMax, float aDbInc)
{
   if (aDbMin < aDbMax)
   {
      int nArcs = CreateVertices(aPlaneType, aCompType, aDbMin, aDbMax, aDbInc);
      CreateIndices(aCompType, nArcs);
   }
}

void LocatorMeshBuilder::CreateIndices(LocatorComponent aType, int aNsteps)
{
   size_t n;
   switch (aType)
   {
   case LocatorComponent::ARC:
      // Using GL_LINE_STRIP
      n = mVertices.size() / aNsteps; // # of points in one arc
      mIndices.reserve((n + 1) * aNsteps);
      for (int iArc = 0; iArc < aNsteps; ++iArc)
      {
         // Store indices for one arc
         size_t offset = n * iArc;
         for (size_t i = 0u; i < n; ++i)
         {
            mIndices.push_back(static_cast<unsigned int>(i + offset));
         }

         // Store index for break between arcs
         mIndices.push_back(GraphicsEngine::RestartIndex);
      }
      break;

   case LocatorComponent::RADIALS:
      // Using GL_LINES
      n = mVertices.size() - 1; // # of radials
      mIndices.reserve(2 * n);
      for (uint32_t i = 1u; i <= n; i++)
      {
         mIndices.push_back(0);
         mIndices.push_back(i);
      }
      break;

   case LocatorComponent::SECTORS:
      // Using GL_TRIANGLE_FAN
      n = mVertices.size() - 1; // # points at max dB that define outer limits of sectors
      mIndices.reserve(2 * n + 3);
      for (size_t i = 0; i <= n; i++)
      {
         mIndices.push_back(static_cast<unsigned int>(i));
      }
      mIndices.push_back(GraphicsEngine::RestartIndex);
      mIndices.push_back(0);
      for (size_t i = n; i > 0; --i)
      {
         mIndices.push_back(static_cast<unsigned int>(i));
      }
      break;
   }
}

int LocatorMeshBuilder::CreateVertices(PlaneType aPlaneType, LocatorComponent aCompType, float aDbMin, float aDbMax, float aDbInc)
{
   // Radials are drawn every 10 degrees. Arcs and sectors require points at the max DB value
   // every 5 degrees to make the curves smooth. Radials and sectors require the first vertex
   // to be for az/el = 0 and the dB value needed to achieve a radius of zero.

   int   angInc = (aCompType == LocatorComponent::RADIALS ? 10 : 5);
   float dbCenter;
   float db = aDbMin;

   QString valueStr;
   int     nArcs = 0;
   switch (aCompType)
   {
   case LocatorComponent::ARC:
      // For each dB arc ...
      while (db <= aDbMax)
      {
         valueStr += QString("  %1").arg(db);
         nArcs++;

         // Add vertices for current arc in azimuth or elevation plane
         switch (aPlaneType)
         {
         case PlaneType::AZ:
            for (int angDeg = cANGLE_MIN; angDeg <= cANGLE_MAX; angDeg += angInc)
            {
               float angRad = static_cast<float>(Angle::FromDegrees(angDeg).GetRadians());
               mVertices.emplace_back(angRad, 0.0f, db);
            }
            break;
         case PlaneType::EL:
            for (int angDeg = cANGLE_MIN; angDeg <= cANGLE_MAX; angDeg += angInc)
            {
               float angRad = static_cast<float>(Angle::FromDegrees(angDeg).GetRadians());
               mVertices.emplace_back(0.0f, angRad, db);
            }
            break;
         default:
            break;
         }
         db += aDbInc;
      }
      break;

   case LocatorComponent::RADIALS:
   case LocatorComponent::SECTORS:
      dbCenter = aDbMin;
      mVertices.emplace_back(0.0f, 0.0f, dbCenter);

      // Add vertices for max dB points in azimuth or elevation plane
      switch (aPlaneType)
      {
      case PlaneType::AZ:
         for (int angDeg = cANGLE_MIN; angDeg <= cANGLE_MAX; angDeg += angInc)
         {
            float angRad = static_cast<float>(Angle::FromDegrees(angDeg).GetRadians());
            mVertices.emplace_back(angRad, 0.0f, aDbMax);
         }
         break;
      case PlaneType::EL:
         for (int angDeg = cANGLE_MIN; angDeg <= cANGLE_MAX; angDeg += angInc)
         {
            float angRad = static_cast<float>(Angle::FromDegrees(angDeg).GetRadians());
            mVertices.emplace_back(0.0f, angRad, aDbMax);
         }
         break;
      default:
         break;
      }
      break;
   }

   return nArcs;
}
} // namespace PatternVisualizer
