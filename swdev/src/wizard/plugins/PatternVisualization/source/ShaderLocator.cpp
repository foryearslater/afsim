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

#include "ShaderLocator.hpp"

#include <QCoreApplication>
#include <QDebug>

#include "Debug.hpp"
#include "UtPath.hpp"

namespace PatternVisualizer
{
ShaderLocator::ShaderLocator()
{
   UtPath path = UtPath::GetExePath();
   path.Up();

   const char* shaderPath = getenv("SOURCE_ROOT");

   if (shaderPath != nullptr)
   {
      path = std::string(shaderPath) + "/wizard/plugins/PatternVisualization/shaders";
   }
   else
   {
      path = path.GetSystemPath() + "/../resources/shaders";
   }

   // QByteArray is implicitly convertible to QString, so this is functionally equivalent to
   // QString::fromStdString. However, using QByteArray as an intermediate masks a string
   // termination vulnerability identified by static code analysis, with sink in QString's
   // use of 'strlen'. The source is string input from 'getenv', which should be null-terminated,
   // so this is apparently a false positive, but this workaround might be unnecessary in a
   // future version of Qt.
   mShaderPath = QDir::cleanPath(QByteArray::fromStdString(path.GetSystemPath()));

   if (!mShaderPath.exists())
   {
      QString nativeShaderPath = QDir::toNativeSeparators(mShaderPath.path());
      QString errorMessage =
         QString{
            R"(
             Could not locate the OpenGL shaders in the directory:
             <pre>%1</pre>
           )"}
            .arg(nativeShaderPath);

      Debug::ShowFatalError(errorMessage);
   }
}

QString ShaderLocator::GetShaderPath(const QString& aShaderName) const
{
   return mShaderPath.absoluteFilePath(aShaderName);
}

QByteArray ShaderLocator::GetShaderSource(const QString& aShaderName) const
{
   QString shaderPath = GetShaderPath(aShaderName);

   QFile file{shaderPath};
   if (!file.open(QIODevice::ReadOnly))
   {
      QString errorMessage = QString("Could not open OpenGL shader <pre>%1</pre>").arg(shaderPath);
      Debug::ShowFatalError(errorMessage);
   }

   return file.readAll();
}
} // namespace PatternVisualizer
