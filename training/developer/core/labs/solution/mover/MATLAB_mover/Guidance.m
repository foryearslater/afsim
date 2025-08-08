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

function [alpha] = Guidance(t,dt,ECI,M,dm,azimuth,alpha,ECEF2LaunchSite,inOrientation,inBoosterParams)

azimuth = inOrientation(1);
fpa     = inOrientation(2);

burn(1)        = inBoosterParams(1);
burn(2)        = inBoosterParams(2);
burn(3)        = inBoosterParams(3);  
Cd             = inBoosterParams(4);
A              = inBoosterParams(5);
maxq           = inBoosterParams(13);
pitch_interval = inBoosterParams(14);
thrust(1)      = inBoosterParams(15);
thrust(2)      = inBoosterParams(16);
thrust(3)      = inBoosterParams(17);
total_burntime = sum(burn,2);

% Earth rotation
omega = 7292115e-11; % Earth rate (WGS-84, rad/sec)
wt = omega*t;
% ECI to ECEF
ECI2ECEF = Rotate3(wt);
% ECEF to ECI
ECEF2ECI = ECI2ECEF';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 delta_aoa = (alpha - fpa) * dt/pitch_interval;
 
 alpha = alpha - delta_aoa;
 
 drive_vector = NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,azimuth);
 if t < burn(1)
     F_thrust = thrust(1) * drive_vector;
 elseif (t >= burn(1) && t < (burn(1) + burn(2)))
     F_thrust = thrust(2) * drive_vector;
 else
     F_thrust = thrust(3) * drive_vector;
 end
 
 vel_past = [ECI(2),ECI(4),ECI(6)];
 
 % Run the engine to get the variables at this state before iterating ...
 [ECI,T,M,rho] = RK4(t,dt,ECI,M,F_thrust,dm,Cd,A,total_burntime);
 
 % Find the magnitude of the velocity
 Velocity_mag_temp = (ECI(2)^2 + ECI(4)^2 + ECI(6)^2)^0.5;
 
 vel_current = [ECI(2),ECI(4),ECI(6)];
 
 gamma_temp = (acos((dot(vel_current,vel_past))/(norm(vel_current)*norm(vel_past))))*180/pi;
 
% Check dynamic pressure constraint at this AOA ...
 dpress_temp = (.5*rho*(Velocity_mag_temp)^2)*sin(gamma_temp*pi/180)/47.8802589;
 
% Iteration on AOA to get below max q (this could possibly be done faster
% with something like newton-raphson instead of delta increment ...
 
while dpress_temp >= maxq  % If we exceeded max q, pitch back up a delta increment
    alpha = alpha + delta_aoa/5;
    drive_vector = NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,azimuth);
    if t <= burn(1)
        F_thrust = thrust(1) * drive_vector;
    elseif (t > burn(1) && t <= (burn(1) + burn(2)))
        F_thrust = thrust(2) * drive_vector;
    else
        F_thrust = thrust(3) * drive_vector;
    end

    vel_past = [ECI(2),ECI(4),ECI(6)];
    
    % Run the engine to get the variables at this state before iterating ...
    [ECI,T,M,rho] = RK4(t,dt,ECI,M,F_thrust,dm,Cd,A,total_burntime);
    
    % Find the magnitude of the velocity
    Velocity_mag_temp = (ECI(2)^2 + ECI(4)^2 + ECI(6)^2)^0.5;

    vel_current = [ECI(2),ECI(4),ECI(6)];
    
    gamma_temp = (acos((dot(vel_current,vel_past))/(norm(vel_current)*norm(vel_past))))*180/pi;

    % Check dynamic pressure constraint at this AOA ...
    dpress_temp = (.5*rho*(Velocity_mag_temp)^2)*sin(gamma_temp*pi/180)/47.8802589;
end
