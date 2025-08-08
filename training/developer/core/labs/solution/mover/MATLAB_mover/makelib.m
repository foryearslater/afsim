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

 cmdFlags = {...
%     '-v'; % verbose
     '-N'; % clear MATLAB path
     '-W';'cpplib:libAFSIM_Mover'; % create C++ library wrapper named libAFSIM_Mover
     '-T';'link:lib'; % link library
     '-d';'./lib'; % output directory
     };

mFiles = {...
    'InitializeMover.m';
    'UpdateMover.m';
    };

mcc(cmdFlags{:}, mFiles{:});
