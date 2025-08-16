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

#include "ShaderProgram.hpp"

namespace PatternVisualizer
{
ShaderProgram::ShaderProgram(const QString&                 shaderName,
                             const ShaderLocator&           shaderLocator,
                             const ShaderProgramDefinition& definition)
{
   for (const ShaderDefinition& shader : definition.mShaders)
   {
      AddShader(shaderLocator, shader);
   }

   for (const ShaderAttributeDefinition& attribute : definition.mAttributes)
   {
      AddAttribute(attribute);
   }

   mProgram.link();
   CheckErrors(QString("linking shader %1").arg(shaderName));
}

void ShaderProgram::AddShader(const ShaderLocator& shaderLocator, const ShaderDefinition& definition)
{
   QByteArray shaderSource = shaderLocator.GetShaderSource(definition.mName);
   mProgram.addShaderFromSourceCode(definition.mType, shaderSource);
   CheckErrors(QString("compiling shader %1").arg(definition.mName));
}

void ShaderProgram::AddAttribute(const ShaderAttributeDefinition& definition)
{
   mProgram.bindAttributeLocation(definition.mName, definition.mLocation);
}

void ShaderProgram::CheckErrors(const QString& context)
{
   QString errors = mProgram.log();
   if (errors.isEmpty())
   {
      return;
   }

   QString message =
      QString{
         R"(
            Error %1:
            <pre>%2</pre>
         )"}
         .arg(context)
         .arg(errors);

   Debug::ShowFatalError(message);
}
} // namespace PatternVisualizer
