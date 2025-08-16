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
const float AnglePi = 3.14159265358979323846;
const float AnglePiOverTwo = 1.57079632679489661923;

in float vertexAzimuthRad;
in float vertexElevationRad;
in float vertexDB;

out float db;

uniform mat4 transform;
uniform float minDB;
uniform float maxDB;
uniform float minRadius;
uniform float maxRadius;

float lerp(float value, float valueMin, float valueMax, float rangeMin, float rangeMax)
{
   return rangeMin + ((value - valueMin) * (rangeMax - rangeMin)) / (valueMax - valueMin);
}

void main()
{
   float radius = clamp(lerp(vertexDB, minDB, maxDB, minRadius, maxRadius), minRadius, maxRadius);

   // The given elevation is in the range of [+pi/2,-pi/2], but we need it to be in the
   // range of [pi,0] for this projection.
   float elevationRad = lerp(vertexElevationRad, AnglePiOverTwo, -AnglePiOverTwo, AnglePi, 0.0);

   vec4 position = vec4(
     radius * cos(vertexAzimuthRad) * sin(elevationRad),
     radius * sin(vertexAzimuthRad) * sin(elevationRad),
     radius * cos(elevationRad),
     1.0);

   db = vertexDB;
   gl_Position = transform * position;
}
