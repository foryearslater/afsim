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

% Rotation matrix - Ang in radians
function Rot3 = Rotate3(Ang)

CA = cos(Ang);
SA = sin(Ang);
Rot3 = [CA SA 0;-SA CA 0;0 0 1];
