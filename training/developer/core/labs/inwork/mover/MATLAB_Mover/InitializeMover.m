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

% Initializes the state of the mover
function [state] = InitializeMover(inLLA, inBoosterParams)

% Zero the return values
state = zeros(1,24);

% Read input
% LLA
latitude  = inLLA(1);     % degrees
longitude = inLLA(2);     % degrees
altitude  = inLLA(3);     % km

% Booster parameters
mass_1st       = inBoosterParams(6);
mass_2nd       = inBoosterParams(7);
mass_3rd       = inBoosterParams(8);
mass_payload   = inBoosterParams(12);
thrust_1st     = inBoosterParams(15);

% Other useful constants
D2R = pi/180;
omega = 7292115e-11; % Earth rate (WGS-84, rad/sec)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Find total mass
total_mass = mass_1st + mass_2nd + mass_3rd + mass_payload;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialize position and velocity vectors

% Calculate earth radius at launch site and get initial ecef position:
[earth_radius_at_lat_long, x_ecef, normal] = Earth(latitude, longitude, altitude);

% ECEF_2_LaunchSite frame
ECEF2LaunchSite = Rotate3(pi) * Rotate2(pi/2 - latitude*D2R) * Rotate3(longitude*D2R);
% ECI to ECEF
ECI2ECEF = Rotate3(0);
% ECEF to ECI
ECEF2ECI = ECI2ECEF';

% Find position in ECI
x_eci = ECEF2ECI * x_ecef;

% Now calculate initial velocity due to earth's rotation
% r is the projection onto the semi-major axis of the earth
r = earth_radius_at_lat_long * cos(latitude*D2R);
dy_lsf = -r * omega;
% Convert from lsf to ecef
initial_velocity_lsf = [0;dy_lsf;0];
% There is no velocity in the ECEF frame - only ECI, ECI and ECEF are same at launch
v_eci = ECEF2ECI * ECEF2LaunchSite' * initial_velocity_lsf;

% Initial Thrust
normal_eci = ECEF2ECI * normal;
F_thrust = thrust_1st * normal_eci; % Thrust of first stage
Thrust_mag  = (F_thrust(1)^2 + F_thrust(2)^2 + F_thrust(3)^2)^0.5;

% Initial Gravity
gravity = total_mass * Gravity(x_ecef);
F_gravity = gravity';
Gravity_mag = (F_gravity(1)^2 + F_gravity(2)^2 + F_gravity(3)^2)^0.5/total_mass;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fill in initial state values
state(1)  = x_eci(1);
state(2)  = v_eci(1);
state(3)  = x_eci(2);
state(4)  = v_eci(2);
state(5)  = x_eci(3);
state(6)  = v_eci(3);
state(7)  = total_mass;       % kg
state(8)  = latitude;         % degrees
state(9)  = longitude;        % degrees
state(10) = altitude;         % km
state(11) = 0;                % Great circle range  0 km
state(12) = Thrust_mag;       % Thrust magnitude
state(13) = Gravity_mag;      % Gravity magnitude
state(14) = 0;                % Drag magnitude      
state(15) = 90;               % alpha               90 degrees
state(16) = 90;               % aoa                 90 degrees
state(17) = x_ecef(1);        % posiiton in ECEF frame
state(18) = x_ecef(2);        %
state(19) = x_ecef(3);        %
state(20) = 0;                % velocity in ECEF frame
state(21) = 0;                %
state(22) = 0;                %
state(23) = 0;                % time                0 sec
state(24) = 0;                % stage






