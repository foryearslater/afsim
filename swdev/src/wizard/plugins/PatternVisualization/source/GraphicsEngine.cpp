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

#include "GraphicsEngine.hpp"

#include "ShaderProgram.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{

const std::uint32_t GraphicsEngine::RestartIndex;

GraphicsEngine::GraphicsEngine(QOpenGLFunctions_3_3_Core& aGlFuncs, const QColor& aClearColor)
   : mGLFuncs(aGlFuncs)
   , mClearColor(aClearColor)
{
}

std::unique_ptr<ShaderProgram> GraphicsEngine::CreateShader(const QString&                 aShaderName,
                                                            const ShaderProgramDefinition& aDefinition)
{
   return ut::make_unique<ShaderProgram>(aShaderName, mShaderLocator, aDefinition);
}

void GraphicsEngine::ActivateTexture(int aNum)
{
   mGLFuncs.glActiveTexture(GL_TEXTURE0 + aNum);
}

void GraphicsEngine::ClearCanvas()
{
   mGLFuncs.glClearColor(static_cast<GLfloat>(mClearColor.redF()),
                         static_cast<GLfloat>(mClearColor.greenF()),
                         static_cast<GLfloat>(mClearColor.blueF()),
                         static_cast<GLfloat>(mClearColor.alphaF()));
   mGLFuncs.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsEngine::DrawElements(GLenum aMode, GLsizei aCount, GLenum aType, const GLvoid* aIndicesPtr)
{
   mGLFuncs.glDrawElements(aMode, aCount, aType, aIndicesPtr);
}

void GraphicsEngine::EnableVertexAttribArray(GLuint aIndex)
{
   mGLFuncs.glEnableVertexAttribArray(aIndex);
}

void GraphicsEngine::EndGraphicsFrame()
{
   mGLFuncs.glPrimitiveRestartIndex(RestartIndex);

   mGLFuncs.glDisable(GL_DEPTH_TEST);
   mGLFuncs.glDisable(GL_PRIMITIVE_RESTART);
   mGLFuncs.glDisable(GL_BLEND);
   mGLFuncs.glDepthMask(false);
}

void GraphicsEngine::VertexAttribPointer(GLuint        aIndex,
                                         GLint         aSize,
                                         GLenum        aType,
                                         GLboolean     aNormalized,
                                         GLsizei       aStride,
                                         const GLvoid* aPointer)
{
   mGLFuncs.glVertexAttribPointer(aIndex, aSize, aType, aNormalized, aStride, aPointer);
}

void GraphicsEngine::StartGraphicsFrame()
{
   mGLFuncs.glEnable(GL_DEPTH_TEST);
   mGLFuncs.glEnable(GL_PRIMITIVE_RESTART);
   mGLFuncs.glEnable(GL_BLEND);
   mGLFuncs.glDepthMask(true);
   mGLFuncs.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   mGLFuncs.glPrimitiveRestartIndex(RestartIndex);
}

void GraphicsEngine::SetTranslucent()
{
   mGLFuncs.glDepthMask(false);
}
} // namespace PatternVisualizer
