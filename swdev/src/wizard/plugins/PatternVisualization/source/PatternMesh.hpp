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

#ifndef PATTERNMESH_HPP
#define PATTERNMESH_HPP

#include <memory>

#include "Mesh.hpp"
#include "Pattern.hpp"

namespace PatternVisualizer
{
class PatternMesh : public Mesh
{
public:
   PatternMesh(Session* aSessionPtr, const Pattern& aPattern, GraphicsEngine& aGraphicsEngine, float aDbMin, float aDbMax);
   ~PatternMesh() override = default;

private:
   std::unique_ptr<MeshBuilder> CreateMeshBuilder() override;

   const Pattern& mPattern;
};

class PatternMeshBuilder : public MeshBuilder
{
public:
   explicit PatternMeshBuilder(const Pattern& aPattern, float aDbMin, float aDbMax);
   ~PatternMeshBuilder() = default;

private:
   void CreateVertices(const PatternData& aPatternData, float aDbMin, float aDbMax);
   void CreateIndices(uint32_t aNumRows, uint32_t aNumColumns);
};
} // namespace PatternVisualizer

#endif
