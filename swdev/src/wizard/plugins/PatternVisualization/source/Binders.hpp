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

#ifndef BINDERS_HPP
#define BINDERS_HPP

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

namespace PatternVisualizer
{
// Simple stack based binder that calls bind() and release() on the given object
// based on the current scope. Useful for automatically binding and releasing
// various Qt OpenGL utility classes.
template<typename T>
class GenericBinder
{
public:
   explicit GenericBinder(T& aObject)
      : mObject(aObject)
   {
      mObject.bind();
   }

   ~GenericBinder() { mObject.release(); }

private:
   T& mObject;
};

// Type aliases for commonly used Qt OpenGL objects.
using BufferBinder  = GenericBinder<QOpenGLBuffer>;
using ShaderBinder  = GenericBinder<class ShaderProgram>;
using TextureBinder = GenericBinder<QOpenGLTexture>;
using VaoBinder     = GenericBinder<QOpenGLVertexArrayObject>;
} // namespace PatternVisualizer

#endif
