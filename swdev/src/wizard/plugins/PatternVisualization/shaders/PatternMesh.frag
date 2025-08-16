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
in highp float db;
out highp vec4 fragColor;

uniform sampler1D colorTable;
uniform float minDB;
uniform float maxDB;

void main()
{
   float t = (db - minDB) / (maxDB - minDB);
   fragColor = texture(colorTable, t);
}
