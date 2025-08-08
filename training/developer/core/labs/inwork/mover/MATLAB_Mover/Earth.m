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

%  Gives a vector and earth radius that takes the oblate earth
%  into account plus local variations from sea level (i.e. alt)

function [earth_radius_at_lat_long,initial_vector,normal] = Earth(lat,lon,alt)

D2R = pi/180;

earth.a = 6378137.0; % Semi-major axis of earth (m)
earth.f = 0.00335281066475; % Flattening of Earth(-)
e2 = 2 * earth.f - earth.f^2;
temp = (1 - e2 * (sin(lat * D2R))^2)^0.5;
rp = (earth.a / temp);

% Double scripting forces vectors to be 3x1 and not 1x3
clat = cos(lat * D2R);
initial_vector(1) = (rp + alt) * clat * cos(lon * D2R);
initial_vector(2) = (rp + alt) * clat * sin(lon * D2R);
initial_vector(3) = (rp * (1 - e2) + alt) * sin(lat * D2R);

earth_radius_at_lat_long = (initial_vector(1)^2 + initial_vector(2)^2 + initial_vector(3)^2)^0.5;
normal(1) = initial_vector(1)/earth_radius_at_lat_long;
normal(2) = initial_vector(2)/earth_radius_at_lat_long;
normal(3) = initial_vector(3)/earth_radius_at_lat_long;

normal = normal';
initial_vector = initial_vector';
