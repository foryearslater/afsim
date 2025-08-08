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

function[f,drag,gravity,rho] = Rocket(t,y,m,thrust,Cd,A,total_burntime)

% global EARTH

% %%%%%%%%%%%% DRAG FORCE %%%%%%%%%%%%
r = (y(1)^2 + y(3)^2 + y(5)^2)^.5;
altitude = r - 6378000;

% http://spiff.rit.edu/classes/phys317/lectures/multiple_funcs/temp_profile.html
rho =  (1.21) * exp(-altitude/8000);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if t > total_burntime
    Cd = 0;
end

drag(1) = 0.5*rho*Cd*A*y(2)^2;
drag(2) = 0.5*rho*Cd*A*y(4)^2;
drag(3) = 0.5*rho*Cd*A*y(6)^2;
drag = drag';

% Determine sign of Drag force --> If the value is less than zero the drag will stay positive
if y(2) > 0
    drag(1) = -drag(1);    
end
if y(4) > 0
    drag(2) = -drag(2);    
end
if y(6) > 0
    drag(3) = -drag(3);    
end
%%%%%%%%%%%% END DRAG FORCE %%%%%%%%%%%%

% %%%%%%%%%%%% GRAVITY FORCE %%%%%%%%%%%%
grav_radius = [y(1) y(3) y(5)];
gravity = m * Gravity(grav_radius)';
% %%%%%%%%%%%% END GRAVITY FORCE %%%%%%%%%%%%

% Sum Forces
force = thrust + drag + gravity;

% Return evaluated function for the Runge-Kutta integration
f(1) = y(2);
f(2) = force(1)/m;
f(3) = y(4);
f(4) = force(2)/m;
f(5) = y(6);
f(6) = force(3)/m;
