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

function [latitude, longitude, altitude] = ECEF2LLA(ecef)

earth.a = 6378137.0; % Semi-major axis of earth (m)
earth.f = 0.00335281066474; % Flattening of earth(-)
earth.b = earth.a*(1-earth.f);

e2 = 2*earth.f-earth.f^2;
ep2 = (earth.a^2-earth.b^2)/(earth.b^2);
p = (ecef(1).^2+ ecef(2).^2).^.5;

theta = atan(ecef(3)*earth.a./(p*earth.b));

lat = atan((ecef(3)+earth.b*ep2*sin(theta).^3)./(p-e2*earth.a*cos(theta).^3));
lon = atan2(ecef(2),ecef(1));

latitude = lat*180/pi;
longitude = lon*180/pi;

% Compute radius of oblate earth
temp = (1-e2*sin(lat)^2)^0.5;
rp = (earth.a/temp);

% Double scripting forces vectors to be 3x1 and not 1x3
clat = cos(lat);
vector(1,1) = (rp)*clat*cos(lon);
vector(2,1) = (rp)*clat*sin(lon);
vector(3,1) = (rp*(1-e2))*sin(lat);

earth_radius = (vector(1,1)^2 + vector(2,1)^2 + vector(3,1)^2)^0.5;
altitude = norm(ecef)-earth_radius;
