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
function Rot2 = Rotate2(Ang)

CA = cos(Ang);
SA = sin(Ang);
Rot2 = [CA 0 -SA;0 1 0;SA 0 CA];
