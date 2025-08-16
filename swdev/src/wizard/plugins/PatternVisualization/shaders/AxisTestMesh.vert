#version 330 core
// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
in vec4 vertex;
in vec3 normal;
in vec3 color;

out vec3 vert;
out vec3 vertNormal;
out vec4 vertColor;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main()
{
   vert = vertex.xyz;
   vertNormal = normalMatrix * normal;
   vertColor = vec4(color, 1.0);
   gl_Position = projMatrix * mvMatrix * vertex;
}
