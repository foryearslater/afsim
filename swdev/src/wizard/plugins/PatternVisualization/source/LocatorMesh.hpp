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

#ifndef LOCATORMESH_HPP
#define LOCATORMESH_HPP

#include "GraphicsEngine.hpp"
#include "Mesh.hpp"
#include "PlaneType.hpp"

namespace PatternVisualizer
{
enum class LocatorComponent
{
   ARC,
   RADIALS,
   SECTORS
};

class LocatorMesh : public Mesh
{
public:
   LocatorMesh(Session*         aSessionPtr,
               GraphicsEngine&  aGraphicsEngine,
               PlaneType        aPlaneType,
               LocatorComponent aCompType,
               float            aDbMin,
               float            aDbMax,
               float            aDbInc = 0);
   ~LocatorMesh() override = default;

private:
   std::unique_ptr<MeshBuilder> CreateMeshBuilder() override;
   static GLenum                GetMeshType(LocatorComponent aType);

   static constexpr float cMIN_RADIUS_LOCATOR = 0.0f;

   PlaneType        mPlaneType;
   LocatorComponent mCompType;
};

class LocatorMeshBuilder : public MeshBuilder
{
public:
   explicit LocatorMeshBuilder(PlaneType aPlaneType, LocatorComponent aCompType, float aDbMin, float aDbMax, float aDbInc);
   ~LocatorMeshBuilder() = default;

private:
   int  CreateVertices(PlaneType aPlaneType, LocatorComponent aCompType, float aDbMin, float aDbMax, float aDbInc);
   void CreateIndices(LocatorComponent aType, int aNsteps);

   constexpr static int cANGLE_MIN = -90;
   constexpr static int cANGLE_MAX = 90;
};
} // namespace PatternVisualizer

#endif // LOCATORMESH_HPP
