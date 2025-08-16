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

#include "Mesh.hpp"

#include <assert.h>

#include "Binders.hpp"
#include "MaterialColors.hpp"
#include "Session.hpp"

namespace PatternVisualizer
{
Mesh::Mesh(Session* aSessionPtr, GraphicsEngine& aGraphicsEngine, GLenum aMode, float aDbMin, float aDbMax, float aDbInc)
   : mGraphicsEngine(aGraphicsEngine)
   , mIndicesBuffer(QOpenGLBuffer::IndexBuffer)
   , mColorTable(QOpenGLTexture::Target::Target1D)
   , mIndicesCount{}
   , mMode(aMode)
   , mDBMin(aDbMin)
   , mDBMax(aDbMax)
   , mDBInc(aDbInc)
   , mRadiusMin{cMIN_RADIUS_PATTERN}
   , mRadiusMax{cMAX_RADIUS_PATTERN}
   , mSessionPtr{aSessionPtr}
   , mTransform{}
{
}

Mesh::~Mesh()
{
   mVerticesBuffer.destroy();
}

void Mesh::Render()
{
   if (mDBMin >= mDBMax)
   {
      return;
   }

   VaoBinder    vaoBinder(mVao);
   ShaderBinder shaderBinder(*mProgram);

   mProgram->SetUniform<float>("minDB", mDBMin);
   mProgram->SetUniform<float>("maxDB", mDBMax);
   mProgram->SetUniform<float>("minRadius", mRadiusMin);
   mProgram->SetUniform<float>("maxRadius", mRadiusMax);

   mGraphicsEngine.ActivateTexture(0);
   TextureBinder textureBinder(mColorTable);

   mSetTransform(GetTransform());

   mGraphicsEngine.DrawElements(mMode, mIndicesCount, GL_UNSIGNED_INT, nullptr);
}

void Mesh::CreateShader()
{
   assert(!mProgram);

   ShaderProgramDefinition definition = ShaderProgramDefinition();
   definition.mShaders = {{QOpenGLShader::Vertex, "PatternMesh.vert"}, {QOpenGLShader::Fragment, "PatternMesh.frag"}};

   definition.mAttributes = {{"vertexAzimuthRad", 0}, {"vertexElevationRad", 1}, {"vertexDB", 2}};

   mProgram = mGraphicsEngine.CreateShader("PatternMesh", definition);

   ShaderBinder shaderBinder(*mProgram);
   mSetTransform = mProgram->MakeUniformSetter<QMatrix4x4>("transform");
}

void Mesh::CreateColorTable(QColor* aColor)
{
   static_assert(sizeof(QRgb) == 4, "QRgb not expected size (32-bits)");

   QColor            borderColor(0, 0, 0, 0);
   std::vector<QRgb> data;

   if (aColor == nullptr)
   {
      const DataScale& dataScale = mSessionPtr->GetScale();
      for (const auto& step : dataScale)
      {
         data.push_back(step.mColor.rgb());
      }
      borderColor = MaterialColors::Cyan900;
   }
   else
   {
      // Single color
      data.push_back(aColor->rgba());
   }

   CreateColorTable(borderColor, data);
}

void Mesh::CreateColorTable(const QColor& aBorderColor, std::vector<QRgb> aData)
{
   mColorTable.create();
   mColorTable.setSize(static_cast<int>(aData.size()));
   mColorTable.setBorderColor(aBorderColor);
   mColorTable.setAutoMipMapGenerationEnabled(false);
   mColorTable.setFormat(QOpenGLTexture::TextureFormat::RGBAFormat);
   mColorTable.setMinMagFilters(QOpenGLTexture::Filter::Linear, QOpenGLTexture::Linear);
   mColorTable.setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);
   mColorTable.allocateStorage();
   mColorTable.setData(QOpenGLTexture::PixelFormat::BGRA, QOpenGLTexture::PixelType::UInt8, aData.data());
}

void Mesh::CreateVertices()
{
   mVao.create();
   mVao.bind();

   // Setup our vertex & index buffers.
   mIndicesBuffer.create();
   mVerticesBuffer.create();

   FillBuffers();

   // Store the vertex attribute bindings for the program.
   SetupVertexAttribute(0, 1, offsetof(MeshBuilder::VertexData, mAzimuthRad));
   SetupVertexAttribute(1, 1, offsetof(MeshBuilder::VertexData, mElevationRad));
   SetupVertexAttribute(2, 1, offsetof(MeshBuilder::VertexData, mDb));

   mVao.release();
   mIndicesBuffer.release();
   mVerticesBuffer.release();
}

void Mesh::FillBuffers()
{
   std::unique_ptr<MeshBuilder> builder = CreateMeshBuilder();

   FillBuffer(mVerticesBuffer, builder->GetVertices());

   const std::vector<uint32_t>& indices = builder->GetIndices();
   FillBuffer(mIndicesBuffer, indices);
   mIndicesCount = static_cast<int32_t>(indices.size());
}

void Mesh::SetupVertexAttribute(int aAttributeIndex, int aNumChannels, size_t aFieldOffset)
{
   mGraphicsEngine.EnableVertexAttribArray(aAttributeIndex);
   mGraphicsEngine.VertexAttribPointer(aAttributeIndex,
                                       aNumChannels,
                                       GL_FLOAT,
                                       GL_FALSE,
                                       sizeof(MeshBuilder::VertexData),
                                       reinterpret_cast<void*>(aFieldOffset));
}

void Mesh::UpdateScale(float aDbMin, float aDbMax, float aDbInc)
{
   if (aDbMin == mDBMin && aDbMax == mDBMax && aDbInc == mDBInc)
   {
      return;
   }

   mDBMin = aDbMin;
   mDBMax = aDbMax;
   mDBInc = aDbInc;
   FillBuffers();
}
} // namespace PatternVisualizer
