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

function [grav] = Gravity(pos_in)

a = 6378137.0;  % Semi-major axis of earth (m) from WGS 84
GM = 3986012.418e8;  % Gravitational constant
j2 = 0.00108263;

radius_mag_sq = sum(pos_in.* pos_in);
radius_mag = sqrt(radius_mag_sq);
radius_mag_qbd = radius_mag_sq * radius_mag;
radius_z_sq = pos_in(3) * pos_in(3);

g_over_r = -GM / radius_mag_qbd;

j2_re = 1.5 * j2 * (a^2) / radius_mag_sq;

five_rz = 5.0 * radius_z_sq / radius_mag_sq;

elp_corr_xy = 1.0 + j2_re * (1.0 - five_rz);
elp_corr_z  = 1.0 + j2_re * (3.0 - five_rz);

grav(1) = g_over_r * elp_corr_xy * pos_in(1);
grav(2) = g_over_r * elp_corr_xy * pos_in(2);
grav(3) = g_over_r * elp_corr_z  * pos_in(3);
