% ****************************************************************************
% CUI
%
% The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
%
% Copyright 2003-2013 The Boeing Company
%
% The use, dissemination or disclosure of data in this file is subject to
% limitation or restriction. See accompanying README and LICENSE for details.
% ****************************************************************************

function [GCR] = GreatCircleRange(lat,long,LAT,LONG)

d2r = pi/180;
delta_long = LONG - long;
cosPhi = sin(lat*d2r)*sin(LAT*d2r) + cos(lat*d2r)*cos(LAT*d2r)*cos(delta_long*d2r);
Phi = acos(cosPhi);
GCR = Phi*6378.137;
