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

#ifndef SHADERLOCATOR_HPP
#define SHADERLOCATOR_HPP

#include <QByteArray>
#include <QDir>

namespace PatternVisualizer
{

class ShaderLocator
{
public:
   ShaderLocator();

   QByteArray GetShaderSource(const QString& aShaderName) const;

private:
   QString GetShaderPath(const QString& aShaderName) const;

   QDir mShaderPath;
};

} // namespace PatternVisualizer

#endif
