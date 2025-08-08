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

%  fprintf('total burntime\n')
%  fprintf('%0.5g\n', total_burntime)

function [state, hit_ground_time] = UpdateMover(t,x,inLLA,inOrientation,inBoosterParams)

% Zero the return values
state = zeros(1,24);
hit_ground_time = 0.0;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Set state variables
ECI(1) = x(1);
ECI(2) = x(2);
ECI(3) = x(3);
ECI(4) = x(4);
ECI(5) = x(5);
ECI(6) = x(6);
M      = x(7);
alpha  = x(15);
aoa    = x(16);
dt     = t - x(23);
stage  = round(x(24));
hit_ground_time = 0.0;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
latitude  = inLLA(1);
longitude = inLLA(2);
altitude  = inLLA(3);

% Orientation
azimuth = inOrientation(1);
fpa     = inOrientation(2);

% Booster parameters
burn(1)       = inBoosterParams(1);
burn(2)       = inBoosterParams(2);
burn(3)       = inBoosterParams(3);  
Cd            = inBoosterParams(4);
A             = inBoosterParams(5);
mass(1)       = inBoosterParams(6);
mass(2)       = inBoosterParams(7);
mass(3)       = inBoosterParams(8);
mass_fuel(1)  = inBoosterParams(9);
mass_fuel(2)  = inBoosterParams(10);
mass_fuel(3)  = inBoosterParams(11);
thrust(1)     = inBoosterParams(15);
thrust(2)     = inBoosterParams(16);
thrust(3)     = inBoosterParams(17);
vert_time     = inBoosterParams(18);
 
% Other useful constants
D2R = pi/180;
omega = 7292115e-11; % Earth rate (WGS-84, rad/sec)

% Calculate some parameters
total_burntime = sum(burn,2);
mass_stage_wo_fuel = mass - mass_fuel;

% Calculate earth radius at launch site and get initial ecef position:
[earth_radius_at_lat_long, x_ecef, normal] = Earth(latitude, longitude, altitude);

% Earth rotation
wt = omega*t;
% ECEF_2_LaunchSite frame
ECEF2LaunchSite = Rotate3(pi) * Rotate2(pi/2 - latitude*D2R) * Rotate3(longitude*D2R);
% ECI to ECEF
ECI2ECEF = Rotate3(wt);
% ECEF to ECI
ECEF2ECI = ECI2ECEF';
normal_eci = ECEF2ECI * normal;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Stage 1
if (t < burn(1))
   if (stage == 0)
      stage = stage + 1;  % start with stage 1
   end
   dm = mass_fuel(stage) / burn(stage);

    if t > vert_time  % vert_time
       if alpha >= fpa % Pitch delta AOA to (z) degrees each time step ...             
          [alpha] = Guidance(t,dt,ECI,M,dm,azimuth,alpha,ECEF2LaunchSite,inOrientation,inBoosterParams);  
          % Start Pitch Program to determine drive_vector before iteration
       end
       drive_vector = NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,azimuth);
       F_thrust = thrust(stage) * drive_vector;
    else
       % Drive rocket straight up for X steps
       F_thrust = thrust(stage) * normal_eci;
    end
   
elseif ((t >= burn(1)) && (t < (burn(1) + burn(2))))
    if (stage == 1)  % Drop stage 1
       M = M - mass_stage_wo_fuel(1);
       stage = stage + 1;
    end
   dm = mass_fuel(stage) / burn(stage);
  
   if alpha >= fpa % Pitch delta AOA to (z) degrees each time step ...
      [alpha] = Guidance(t,dt,ECI,M,dm,azimuth,alpha,ECEF2LaunchSite,inOrientation,inBoosterParams);  
      % Start Pitch Program to determine drive_vector before iteration
   end
   drive_vector = NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,azimuth);
   F_thrust = thrust(stage) * drive_vector;
         
 elseif ((t >= (burn(1) + burn(2))) && (t < (burn(1) + burn(2) + burn(3))))     
   if (stage == 2)  % Drop stage 2
      M = M - mass_stage_wo_fuel(2);
      stage = stage + 1;
   end  
   dm = mass_fuel(stage) / burn(stage);
     
   if alpha >= fpa % Pitch delta AOA to (z) degrees each time step ...
      [alpha] = Guidance(t,dt,ECI,M,dm,azimuth,alpha,ECEF2LaunchSite,inOrientation,inBoosterParams);  
      % Start Pitch Program to determine drive_vector before iteration
   end
   F_thrust = thrust(stage) * NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,azimuth);
              
  else    
     if (stage == 3)   % Drop stage 3
         M = M - mass_stage_wo_fuel(3);
         stage = stage + 1;
     end   
     dm = 0;   
     F_thrust = [0;0;0];
end

% Save the ECI velocity
vel_past = [ECI(2),ECI(4),ECI(6)];
     
% Now iterate ...
[ECI,T,M,rho] = RK4(t,dt,ECI,M,F_thrust,dm,Cd,A,total_burntime);
[Velocity_mag,Thrust_mag,Gravity_mag,Drag_mag,LAT,LONG,ALT,GCR,x_ecef,v_ecef]= ...
   SaveVariables(t,ECI,M,F_thrust,latitude,longitude,Cd,A,total_burntime); 
  
% Set the current ECI velocity 
vel_current = [ECI(2),ECI(4),ECI(6)];
     
if norm(vel_past) > 0
   gamma = (acos((dot(vel_current,vel_past))/(norm(vel_current)*norm(vel_past))))*180/pi;
   aoa = aoa - gamma;
else
   gamma = 0;
   aoa = 90;
end
          
% Check for earth impact
if ALT <= 0
   hit_ground_time = t;
   state(1) = x(1);
   state(2) = x(2);
   state(3) = x(3);
   state(4) = x(4);
   state(5) = x(5);
   state(6) = x(6);
   state(7) = x(7);
   state(8) = x(8);
   state(9) = x(9);
   state(10) = x(10);
   state(11) = x(11);
   state(12) = x(12);
   state(13) = x(13);
   state(14) = x(14);
   state(15) = alpha;
   state(16) = aoa;
   state(17) = x_ecef(1);
   state(18) = x_ecef(2);
   state(19) = x_ecef(3);
   state(20) = v_ecef(1);
   state(21) = v_ecef(2);
   state(22) = v_ecef(3);
   state(23) = t;
   state(24) = stage; 
   return;
end

% Save the state
state(1)  = ECI(1);
state(2)  = ECI(2);
state(3)  = ECI(3);
state(4)  = ECI(4);
state(5)  = ECI(5);
state(6)  = ECI(6);
state(7)  = M;
state(8)  = LAT;
state(9)  = LONG;
state(10) = ALT;
state(11) = GCR;
state(12) = Thrust_mag;
state(13) = Gravity_mag;
state(14) = Drag_mag;
state(15) = alpha;
state(16) = aoa;
state(17) = x_ecef(1);
state(18) = x_ecef(2);
state(19) = x_ecef(3);
state(20) = v_ecef(1);
state(21) = v_ecef(2);
state(22) = v_ecef(3);
state(23) = t;   
state(24) = stage;

 



