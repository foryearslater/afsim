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

function[drive_vector] = NewDriveVector(alpha,ECEF2LaunchSite,ECEF2ECI,flyout_az)

% Other useful constants
D2R = pi/180;

% Calculate the flyout vector from data entered by user (flyout_angle, flyout_az)
% lsf = "launch site frame"
x_lsf = cos((alpha)*D2R)  * cos(flyout_az*D2R);
y_lsf = -cos((alpha)*D2R) * sin(flyout_az*D2R);
z_lsf = sin((alpha)*D2R);

flyout_vector_lsf  = [x_lsf; y_lsf; z_lsf];
flyout_vector_ecef = ECEF2LaunchSite' * flyout_vector_lsf;
flyout_vector_eci  = ECEF2ECI * flyout_vector_ecef;

% Normalize
flyout_vector_eci = flyout_vector_eci./ norm(flyout_vector_eci);
drive_vector = flyout_vector_eci;
