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

#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include <QOpenGLShader>

#include "Debug.hpp"
#include "ShaderLocator.hpp"

namespace PatternVisualizer
{

struct ShaderAttributeDefinition
{
   QString mName;
   int     mLocation;
};

struct ShaderDefinition
{
   QOpenGLShader::ShaderType mType;
   QString                   mName;
};

struct ShaderProgramDefinition
{
   std::vector<ShaderDefinition>          mShaders;
   std::vector<ShaderAttributeDefinition> mAttributes;
};

template<typename T>
using ShaderUniformSetter = std::function<void(T)>;

class ShaderProgram
{
public:
   ShaderProgram(const QString& aShaderName, const ShaderLocator& aShaderLocator, const ShaderProgramDefinition& aDefinition);
   ~ShaderProgram() = default;

   void bind() { mProgram.bind(); }
   void release() { mProgram.release(); }

   template<typename T>
   ShaderUniformSetter<T> MakeUniformSetter(const char* aNamePtr)
   {
      int location = mProgram.uniformLocation(aNamePtr);
      if (location < 0)
      {
         Debug::ShowFatalError(QString{"Could not find uniform '%1'"}.arg(aNamePtr));
      }

      auto setter = [this, location](T aValue) { mProgram.setUniformValue(location, aValue); };

      return setter;
   }

   template<typename T>
   void SetUniform(const char* aNamePtr, T aValue)
   {
      ShaderUniformSetter<T> setter = MakeUniformSetter<T>(aNamePtr);
      setter(aValue);
   }

private:
   void AddShader(const ShaderLocator& aShaderLocator, const ShaderDefinition& aDefinition);
   void AddAttribute(const ShaderAttributeDefinition& aDefinition);
   void CheckErrors(const QString& aContext);

   QOpenGLShaderProgram mProgram;
};

} // namespace PatternVisualizer

#endif
