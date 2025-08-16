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

#ifndef MESH_HPP
#define MESH_HPP

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include "GraphicsEngine.hpp"
#include "ShaderProgram.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{
class MeshBuilder;
class Session;
class ShaderProgram;

template<typename T>
void FillBuffer(QOpenGLBuffer& aBuffer, T& aData)
{
   uint32_t byteSize = static_cast<uint32_t>(aData.size() * sizeof(typename T::value_type));
   aBuffer.bind();
   aBuffer.allocate(aData.data(), byteSize);
}

class Mesh
{
public:
   Mesh(Session* aSessionPtr, GraphicsEngine& aGraphicsEngine, GLenum aMode, float aDbMin, float aDbMax, float aDbInc = 0);
   virtual ~Mesh();

   void       FillBuffers();
   QMatrix4x4 GetTransform() const { return mTransform; }
   void       Render();
   void       SetTransform(QMatrix4x4 aTransform) { mTransform = aTransform; }
   void       UpdateScale(float aDbMin, float aDbMax, float aDbInc = 0);

protected:
   GraphicsEngine&                 mGraphicsEngine;
   QOpenGLVertexArrayObject        mVao;
   QOpenGLBuffer                   mVerticesBuffer;
   QOpenGLBuffer                   mIndicesBuffer;
   QOpenGLTexture                  mColorTable;
   int                             mIndicesCount;
   std::unique_ptr<ShaderProgram>  mProgram;
   ShaderUniformSetter<QMatrix4x4> mSetTransform;
   GLenum                          mMode;
   float                           mDBMin;
   float                           mDBMax;
   float                           mDBInc;
   float                           mRadiusMin;
   float                           mRadiusMax;

   void                                 CreateColorTable(QColor* aColor = nullptr);
   void                                 CreateColorTable(const QColor& aBorderColor, std::vector<QRgb> aData);
   virtual std::unique_ptr<MeshBuilder> CreateMeshBuilder() = 0;
   void                                 CreateShader();
   void                                 CreateVertices();
   void SetupVertexAttribute(int aAttributeIndex, int aNumChannels, size_t aFieldOffset);

   // These two provide the range of radii at which any pattern data are plotted.
   // This includes anything that is not pattern data, but which lives on the same
   // scale as pattern data.
   static constexpr float cMIN_RADIUS_PATTERN = 0.05f;
   static constexpr float cMAX_RADIUS_PATTERN = 0.9f;

private:
   Session*   mSessionPtr;
   QMatrix4x4 mTransform;
};

class MeshBuilder
{
public:
   explicit MeshBuilder() {}
   virtual ~MeshBuilder() = default;

   struct VertexData
   {
      VertexData(float aAzimuthRad, float aElevationRad, float aDb)
         : mAzimuthRad(aAzimuthRad)
         , mElevationRad(aElevationRad)
         , mDb(aDb)
      {
      }

      float mAzimuthRad;
      float mElevationRad;
      float mDb;
   };

   static_assert(sizeof(VertexData) == 3 * sizeof(float), "Wrong size VertexData");

   const std::vector<VertexData>& GetVertices() const { return mVertices; }
   const std::vector<uint32_t>&   GetIndices() const { return mIndices; }

protected:
   std::vector<VertexData> mVertices;
   std::vector<uint32_t>   mIndices;
};
} // namespace PatternVisualizer

#endif
