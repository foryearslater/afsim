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

in highp vec3 vert;
in highp vec3 vertNormal;
in highp vec4 vertColor;

out highp vec4 fragColor;

uniform highp vec3 lightPos;

void main()
{
   highp vec3 L = normalize(lightPos - vert);
   highp float NL = max(dot(normalize(vertNormal), L), 0.0);
   highp vec3 color = vertColor.xyz;
   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);
   fragColor = vec4(col, 1.0);
}
