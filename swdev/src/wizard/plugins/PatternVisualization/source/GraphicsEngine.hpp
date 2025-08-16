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

#ifndef GRAPHICSENGINE_HPP
#define GRAPHICSENGINE_HPP

#include <limits>
#include <memory>

#include <QColor>
#include <QOpenGLFunctions_3_3_Core>
#include <QString>

#include "ShaderLocator.hpp"

namespace PatternVisualizer
{

class ShaderProgram;
struct ShaderProgramDefinition;

class GraphicsEngine
{
public:
   static const uint32_t RestartIndex = std::numeric_limits<uint32_t>::max();

   explicit GraphicsEngine(QOpenGLFunctions_3_3_Core& aGlFuncs, const QColor& aClearColor);
   ~GraphicsEngine() = default;

   std::unique_ptr<ShaderProgram> CreateShader(const QString& aShaderName, const ShaderProgramDefinition& aDefinition);

   void ActivateTexture(int aNum);
   void DrawElements(GLenum aMode, GLsizei aCount, GLenum aType, const GLvoid* aIndicesPtr);

   void ClearCanvas();

   void StartGraphicsFrame();
   void EndGraphicsFrame();
   void SetTranslucent();

   void EnableVertexAttribArray(GLuint aIndex);
   void VertexAttribPointer(GLuint aIndex, GLint aSize, GLenum aType, GLboolean aNormalized, GLsizei aStride, const GLvoid* aPointer);

private:
   QOpenGLFunctions_3_3_Core& mGLFuncs;
   ShaderLocator              mShaderLocator;
   QColor                     mClearColor;
};

} // namespace PatternVisualizer

#endif
