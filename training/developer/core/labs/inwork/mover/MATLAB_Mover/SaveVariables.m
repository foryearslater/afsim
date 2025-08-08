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

function [Velocity_mag,Thrust_mag,Gravity_mag,Drag_mag,LAT,LONG,ALT,GCR,x_ecef,v_ecef] = SaveVariables(t,ECI,M,F_thrust,lat,long,Cd,A,total_burntime)

% Save position and velocity variables
x_eci(1) = ECI(1);
v_eci(1) = ECI(2);
x_eci(2) = ECI(3);
v_eci(2) = ECI(4);
x_eci(3) = ECI(5);
v_eci(3) = ECI(6);

% Find gravity and drag variables for the position and velocity
[f,drag,gravity] = Rocket(t,ECI,M,F_thrust,Cd,A,total_burntime);
F_drag = drag;
F_gravity = gravity;

% Find the magnitude of the variables
Velocity_mag = (ECI(2)^2 + ECI(4)^2 + ECI(6)^2)^0.5;
Thrust_mag   = (F_thrust(1)^2 + F_thrust(2)^2 + F_thrust(3)^2)^0.5;
Gravity_mag  = (F_gravity(1)^2 + F_gravity(2)^2 + F_gravity(3)^2)^0.5/M;
Drag_mag     = (F_drag(1)^2 + F_drag(2)^2 + F_drag(3)^2)^0.5;

% Earth rotation
omega = 7292115e-11; % Earth rate (WGS-84, rad/sec)
wt = omega * t;
% ECI to ECEF
ECI2ECEF = Rotate3(wt);
 
% Get ECEF coordinates using updated transformation matrix
x_ecef = ECI2ECEF * x_eci';
v_ecef = ECI2ECEF * v_eci';
 
% Get the LAT/LONG/ALT from ECEF coordinates
[LAT, LONG, ALT] = ECEF2LLA(x_ecef);
ALT = ALT/1000;  %Covert to km
GCR = GreatCircleRange(lat,long,LAT,LONG);
