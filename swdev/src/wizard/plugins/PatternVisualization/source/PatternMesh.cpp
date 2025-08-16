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

#include "PatternMesh.hpp"

#include "Pattern.hpp"
#include "PatternDataAllIterator.hpp"
#include "Session.hpp"

namespace PatternVisualizer
{
PatternMesh::PatternMesh(Session* aSessionPtr, const Pattern& aPattern, GraphicsEngine& aGraphicsEngine, float aDbMin, float aDbMax)
   : Mesh(aSessionPtr, aGraphicsEngine, GL_TRIANGLE_STRIP, aDbMin, aDbMax)
   , mPattern(aPattern)
{
   mRadiusMin = cMIN_RADIUS_PATTERN;
   mRadiusMax = cMAX_RADIUS_PATTERN;

   CreateShader();
   CreateVertices();
   CreateColorTable();
}

std::unique_ptr<MeshBuilder> PatternMesh::CreateMeshBuilder()
{
   return ut::make_unique<PatternMeshBuilder>(mPattern, mDBMin, mDBMax);
}

//###############################################################################################

PatternMeshBuilder::PatternMeshBuilder(const Pattern& aPattern, float aDbMin, float aDbMax)
{
   const PatternData* patternDataPtr = aPattern.GetPatternData();
   if (patternDataPtr != nullptr && aDbMin < aDbMax)
   {
      const uint32_t numRows    = static_cast<uint32_t>(patternDataPtr->GetNumRows());
      const uint32_t numColumns = static_cast<uint32_t>(patternDataPtr->GetNumColumns());
      CreateVertices(*patternDataPtr, aDbMin, aDbMax);
      CreateIndices(numRows, numColumns);
   }
}

void PatternMeshBuilder::CreateIndices(uint32_t aNumRows, uint32_t aNumColumns)
{
   mIndices.reserve((2 * (aNumColumns + 1) + 1) * (aNumRows - 1));

   for (uint32_t row = 1u; row < aNumRows; ++row)
   {
      for (uint32_t col = 0u; col <= aNumColumns; ++col)
      {
         uint32_t index = row * aNumColumns + col % aNumColumns;
         mIndices.push_back(index);
         mIndices.push_back(index - aNumColumns);
      }

      mIndices.push_back(GraphicsEngine::RestartIndex);
   }
}

void PatternMeshBuilder::CreateVertices(const PatternData& aPatternData, float aDbMin, float aDbMax)
{
   mVertices.reserve(aPatternData.GetNumElements());

   PatternDataAllIterator       i   = aPatternData.EnumerateAllBegin();
   const PatternDataAllIterator end = aPatternData.EnumerateAllEnd();
   for (; i != end; ++i)
   {
      const PatternDataDatum& datum = *i;

      mVertices.emplace_back(static_cast<float>(datum.Azimuth.GetRadians()),
                             static_cast<float>(datum.Elevation.GetRadians()),
                             std::max(aDbMin, datum.DB));
   }

   assert(mVertices.size() == aPatternData.GetNumElements());
}
} // namespace PatternVisualizer
