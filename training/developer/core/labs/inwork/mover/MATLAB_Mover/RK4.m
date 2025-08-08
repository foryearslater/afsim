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

function[y,x,m,rho] = RK4(x0,h,y0,m0,thrust,dm,Cd,A,total_burntime)

[f0,drag,gravity,rho] = Rocket(x0,y0,m0,thrust,Cd,A,total_burntime);

y = zeros(1,6);
x = x0 + h/2;
for i = 1:6
    y(i) = y0(i) + (h/2)*f0(i);
end
m    = m0 - dm*(h/2);
[f1,drag,gravity] = Rocket(x,y,m,thrust,Cd,A,total_burntime);

x = x0 + h/2;
for i = 1:6
    y(i) = y0(i) + (h/2)*f1(i);
end
m = m0 - dm*(h/2);
[f2,drag,gravity] = Rocket(x,y,m,thrust,Cd,A,total_burntime);

x = x0 + h;
for i = 1:6
    y(i) = y0(i) + h*f2(i);
end
m = m0 - dm*h;
[f3,drag,gravity] = Rocket(x,y,m,thrust,Cd,A,total_burntime);

for i = 1:6
    y(i) = y0(i) + (h/6)*(f0(i) + 2*f1(i) + 2*f2(i) + f3(i));
end
x = x0 + h;
m = m0 - dm*h;
