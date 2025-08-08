% ****************************************************************************
% CUI//REL TO USA ONLY
%
% The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
%
% The use, dissemination or disclosure of data in this file is subject to
% limitation or restriction. See accompanying README and LICENSE for details.
% ****************************************************************************

Snyder
% attempting to convert brawler IR signature (.FXW format, IR DV3 W/SR table)
% to SOSM, this does not work for Temperature table format
%
% NOTES:
% Brawler's IR deck defines plume temperature to be used to calculate a
% separate plume transmission, SOSM does not do this, just uses modtran
% foreground transmission for both plume and AFHP, this will result in
% different performances between Brawler and SOSM especially when looking
% at midwave (calibrated nei values on sensor will vary between brawler and
% SOSM)
%
% Brawler defines 3 throttle settings - idle, mil, and full AB (1,2,3
% respectively), SOSM is expecting thrust from 0-1, this file attempts to
% match the convention of FIRE where: 
%     0        engine off
%     5  - 15  sweep of thrust reversing
%     20 - 50  sweep of unaugmented thrust; flight idle to IRT (MIL)
%     60 - 100 sweep of augmented thrust; min to max AB
% This script will do a polyfit to map sosm throttle to brawler:
%   Brawler  1    2    3
%   sosm    20   50   100
%   polyfit(Brawler,sosm,2)
%   sosm_throttle = 10*x^2 + 0*x + 10
% out of the box, sosm plugin does not check for a throttle setting due to
% limitations of the air_mover, very easy code addition can be made to 
% check if mover type equals Brawler or P6DOF then check throttle setting
%
% Brawler has 4 IR bands:
%   Brawler band 1: 1.8 - 2.7     
%   Brawler band 2: 4.0 - 5.0    
%   Brawler band 3: 3.0 - 4.0    
%   Brawler band 4: 8.0 - 12.0  
% Notice that band 2 & 3 are in reverse order, this is a standard brawler
% convention. The signature bands as defined in the FXW however are defined
% in the order - Band 1,3,2,4. 
% The indexing of bands in this script are:
%   Band1 = Brawler band 1
%   Band2 = Brawler band 3
%   Band3 = Brawler band 2
%   Band4 = Brawler band 4
%
% Default Brawler uses a "shoebox" method to look at presented visual area
% which is also used in the IR code. The area is an array of size(3) w/
% (front area, side area, top area). Brawler also does not distinguish
% between plume area and body area. 
% The Brawler S/N ratio is calculated as follows:
%
% S/N = ( signal*gain - bckgnd*gain*targ_solid_angle ) / nei
%   targ_solid_ang = (shoebox visual area)/rng^2
%   nei - defined on the sensor
%   gains - added multipliers defined on the sensor
%
% This will script will build a .bda and a .pla file with the .bda file
% displaying the shoebox area and the .pla file zero'd out. sosm plugin
% combines bda+pla=tgt_area 
%
% this script will not work if there are random comments within IR
% sig file (in between lines of data points). This file is checking for
% strings like "N_THROT_VAL" and "END ELEVATION_POINTS" so having any
% comments with matching strings in the IR signature data can mess things
% up
%
% It is worth noting that this file takes roughly 30 minutes to run on a
% standard Brawler IR deck dimensioned (band,throt,mach,alt,el,az)
%                                      (4,     5,    8,  5, 19,7 ) = 106,400 data points
% I understand matlab is probably not the best tool for this job as far as
% runtime goes but too bad this is what I decided to use
clear
% clc
fclose('all');
close('all');

% define input .FXW file to be read in 
input = 'H:\LTE_FIGHTER.FXW';
% define the output target_model table file
output = 'H:\SOSM_SIGS\LTE_FIGHTER\LTE_FIGHTER_IR_DV3';
% define the path that will hold all of the .bdi .pli .bda .pla files
sig_path = 'H:\SOSM_SIGS\LTE_FIGHTER\SIGS\';
% define the classification of the input data
classification = "UNCLASSIFIED";

bands = 0; % initialize
data = import_fxw(input);

n_col = length(data(:,1));

m22ft2 = 1/10.763911; % square feet to square meter
%% band independent data
b_index = zeros(1,4);
for i=1:n_col
    if (data(i,1) == "N_THROT_VAL")
        N_THROT_VAL = str2double(data(i,2));
        for j=1:N_THROT_VAL
            L_THROT_VAL(j) = str2double( data(i+1,j+1) );
        end
    end
    if (data(i,1) == "N_MACH_VAL")
        N_MACH_VAL = str2double(data(i,2));
        for j=1:N_MACH_VAL
            L_MACH_VAL(j) = str2double( data(i+1,j+1) );
        end
    end   
    if (data(i,1) == "N_ALT_VAL")
        N_ALT_VAL = str2double(data(i,2));
        for j=1:N_ALT_VAL
            L_ALT_VAL(j) = str2double( data(i+1,j+1) );
        end
    end
    if (data(i,1) == "BAND")
        bands = bands+1;
        b_index(bands) = i;
    end 
    if (data(i,1) == "PAREA")
        PAREA = [str2double(data(i,2))*m22ft2 str2double(data(i,3))*m22ft2 str2double(data(i,4))*m22ft2];
    end
end
% sosm is very picky about defining multiple bands, which is why i changed
% 2.7 to 2.9944 and 12 to 11.9048. Apparently each band has to have the
% same increment and the origins of each band must line up by some integer
% multiplied by that increment within a .01 tolerance
% sosm plugin source code below
% sosm code 10000/microns = origin (1/cm)
% 1.0E+4F / (mOrigin + (mCount - 1) * mIncrement);
% // First object has the 'left-most' origin.
% // Determine the number of samples the origin of second object is offset from the origin of the first object.
% int offset = static_cast<int>((origin2 - origin1 + 0.01 * mIncrement) / mIncrement);
% // Ensure the distance between the origins is a multiple of the sampling increment.
% if (fabs((origin1 + offset * mIncrement) - origin2) < 0.01)
%     {
%         status  = cOK;
Band.band1.wavelength=[1.8 2.9944];
Band.band2.wavelength=[3.0 4.0];
Band.band3.wavelength=[4.0 5.0];
Band.band4.wavelength=[8.0 11.9048]; 

%% Band Dependent Data

% Band is a 4x1 struct (band 1-4)

for i=1:length(b_index)
    if (i==1)
        B = 'band1';
    elseif (i==2)
        B = 'band2';
    elseif (i==3)
        B = 'band3';
    elseif (i==4)
        B = 'band4';
    end
    Band.(B).start = b_index(i);
    for j=Band.(B).start:length(data)
        if (data(j,2) == "THROTTLE_POINTS")
            Band.(B).end = j;
            break;
        end
    end 
    
    % for each band, find number of az and el points (angle data)
    for j=Band.(B).start:length(data)
        if (data(j,2) == "ANGLE_DATA")
            Band.(B).ang_end = j;
            break;
        end
    end
    Band.(B).n_el = 0;
    for j=Band.(B).start:Band.(B).ang_end
        if (data(j,1) == "EL_VAL")
            Band.(B).n_el = Band.(B).n_el + 1;
            Band.(B).el{Band.(B).n_el,1} = str2double(data(j,2)); % cell array
            Band.(B).el{Band.(B).n_el,2} = str2double(data(j+1,2));
            Band.(B).el{Band.(B).n_el,3} = str2double(data(j+2,2:str2double(data(j+1,2))+1));
        end
    end
    % end angle data
    
%% loop through band independent data
    % loop through throttles
        % loop through machs
            % loop through altitudes
            
     % line to start looking at        
     start = Band.(B).ang_end;
     found_start = false;
%      Band.(B).SIG_AFHP = zeros(N_THROT_VAL,N_MACH_VAL,N_ALT_VAL);
     
     for t=1:N_THROT_VAL
         for m=1:N_MACH_VAL
             for a=1:N_ALT_VAL
                 %start a block of altitude
        
                 % find starting and stopping point for this altitude bin
%                  start = Band.(B).ang_end;
                 for eap=start:n_col
                     if (found_start == false && data(eap,1) == "ELEV")
                         found_start = true;
                         start = eap;
                     end                     
                     if (data(eap,2) == "ELEVATION_POINTS")
                         stop = eap;
                         break;
                     end
                 end
                 el_ind = 1;
                 for row=start:stop
                     if (data(row,1) == "ELEV")
                         Band.(B).AFHP{el_ind,1} = str2double(data(row+1,2:Band.(B).el{el_ind,2}+1));
                         Band.(B).PL{el_ind,1} = str2double(data(row+2,2:Band.(B).el{el_ind,2}+1));                         
                         for az_ind = 1:Band.(B).el{el_ind,2}
                             Band.(B).SIG_AFHP(t,m,a,el_ind,az_ind) = Band.(B).AFHP{el_ind,1}(az_ind); 
                             Band.(B).SIG_PL(t,m,a,el_ind,az_ind) = Band.(B).PL{el_ind,1}(az_ind);                              
                             Band.(B).AZ(t,m,a,el_ind,az_ind) = Band.(B).el{el_ind,3}(az_ind);                             
                         end
                         el_ind = el_ind + 1;
                     end
                 end
                 
                 % get new starting point for the next elevation block
                 found_start = false;
                 start = stop + 1;
             end
         end
     end  
end
% end reading in signature data for each band
%% map brawler throttle settings to sosm power value & create mach, throttle, altitude bins
% Brawler throttle settings:
%     1 - idle
%     2 - mil
%     3 - Full AB
% SOSM power settings
%     0        engine off
%     5  - 15  sweep of thrust reversing
%     20 - 50  sweep of unaugmented thrust; flight idle to IRT (MIL)
%     60 - 100 sweep of augmented thrust; min to max AB
% 
%     Polyfit this
% Brawler  1    2    3
% sosm    20   50   100
b_throt = [1 2 3];
sosm_pc = [20 50 100];
p = polyfit(b_throt,sosm_pc,2);
L_PC_VAL = zeros(1,length(L_THROT_VAL));
for i = 1:length(L_THROT_VAL)
    L_PC_VAL(i) = p(1)*L_THROT_VAL(i)^2 + p(3);
end

% create throttle bins (PC)
PC_Bin = zeros(1,N_THROT_VAL);
PC_Bin(N_THROT_VAL) = 100; % set max to 100
for i = 1:N_THROT_VAL-1
    PC_Bin(i) = (L_PC_VAL(i)+L_PC_VAL(i+1)) / 2;
end

% create altitude bins (kft)
ALT_Bin = zeros(1,N_ALT_VAL);
ALT_Bin(N_ALT_VAL) = 100; % set max to 100kft
for i = 1:N_ALT_VAL-1
    ALT_Bin(i) = (L_ALT_VAL(i)+L_ALT_VAL(i+1)) / 2000;
end

% create mach bins
MACH_Bin = zeros(1,N_MACH_VAL);
MACH_Bin(N_MACH_VAL) = 3; % set max to 100kft
for i = 1:N_MACH_VAL-1
    MACH_Bin(i) = (L_MACH_VAL(i)+L_MACH_VAL(i+1)) / 2;
end

%% create each individual .bdi and .pli
B = 'band4';
sigID = fopen(output,'w');
fprintf(sigID,'%s\n','target_model table');
for a=1:N_ALT_VAL
    for m = 1:N_MACH_VAL
        for t = 1:N_THROT_VAL
            state_string = {num2str(L_ALT_VAL(a)/1000),'kft_M',num2str(L_MACH_VAL(m)),'_',num2str(L_PC_VAL(t))};
            state_string = strjoin(state_string,'');
            fprintf(sigID,'\t %s %s\n','state',state_string);
            % print band independent data bins
            if (a==1)
                fprintf(sigID,'\t\t %s\t %3.2f %s  %3.2f %s\n','altitude_range',0.00,'kft',ALT_Bin(a),'kft');
            else
                fprintf(sigID,'\t\t %s\t %3.2f %s  %3.2f %s\n','altitude_range',ALT_Bin(a-1),'kft',ALT_Bin(a),'kft');                
            end
            if (m==1)
                fprintf(sigID,'\t\t %s\t %3.2f       %3.2f\n','mach_range',0.00,MACH_Bin(m));
            else
                fprintf(sigID,'\t\t %s\t %3.2f       %3.2f\n','mach_range',MACH_Bin(m-1),MACH_Bin(m));                
            end 
            if (t==1)
                fprintf(sigID,'\t\t %s\t %3.2f       %3.2f\n','throttle_range',0.00,PC_Bin(t)/100);
            else
                fprintf(sigID,'\t\t %s\t %3.2f       %3.2f\n','throttle_range',PC_Bin(t-1)/100,PC_Bin(t)/100);                
            end
            for b=1:bands
                if (b==1)
                    B = 'band1';
                elseif (b==2)
                    B = 'band2';
                elseif (b==3)
                    B = 'band3';
                elseif (b==4)
                    B = 'band4';
                end
                n_az = max(cell2mat(Band.(B).el(:,2)));
                % .bdi
                bd_filename = [sig_path,B,'_Alt_',num2str(L_ALT_VAL(a)),'_MACH_',num2str(L_MACH_VAL(m)),'_THROT_',num2str(L_PC_VAL(t)),'.bdi'];
                afhpID = fopen(bd_filename,'w');
                fprintf(afhpID,'%s\n%s %s %2.2f %s %4.1f %s %2.0f %s %2.2f %s %2.2f\n',bd_filename,'AF+HP','MACH',L_MACH_VAL(m),'Alt',L_ALT_VAL(a),'Throttle',L_PC_VAL(t),'Bandwidth',Band.(B).wavelength(1),'to',Band.(B).wavelength(2));
                fprintf(afhpID,'%s\n',classification);
                % sosm code 10000/x = origin
                % 1.0E+4F / (mOrigin + (mCount - 1) * mIncrement);
                inc = 20;
                Band.(B).n = floor((10000/Band.(B).wavelength(1) - (10000/Band.(B).wavelength(2)))/inc + 1);
                fprintf(afhpID,'%4.0f %4.0f %1.0f\n',10000/Band.(B).wavelength(2), inc, Band.(B).n);
                fprintf(afhpID,"%2.0f %2.0f\n",n_az,Band.(B).n_el);
                % .pli
                pl_filename = [sig_path,B,'_Alt_',num2str(L_ALT_VAL(a)),'_MACH_',num2str(L_MACH_VAL(m)),'_THROT_',num2str(L_PC_VAL(t)),'.pli'];
                plID = fopen(pl_filename,'w');
                fprintf(plID,'%s\n%s %s %2.2f %s %4.1f %s %2.0f %s %2.2f %s %2.2f\n',pl_filename,'PL','MACH',L_MACH_VAL(m),'Alt',L_ALT_VAL(a),'Throttle',L_PC_VAL(t),'Bandwidth',Band.(B).wavelength(1),'to',Band.(B).wavelength(2));
                fprintf(plID,'%s\n',classification);
                fprintf(plID,'%4.0f %4.0f %3.0f\n',10000/Band.(B).wavelength(2), inc, Band.(B).n);
                fprintf(plID,"%2.0f %2.0f\n",n_az,Band.(B).n_el);
                for az=1:n_az
                    for el=1:Band.(B).n_el
                        % correct for el=-90,90 only having one az point
                        if (el == 1)
                            Band.(B).el{el,2} = n_az;
                            Band.(B).AZ(t,m,a,el,:) = Band.(B).AZ(t,m,a,el+1,:);
                            Band.(B).SIG_AFHP(t,m,a,el,:) = Band.(B).SIG_AFHP(t,m,a,el,1);
                            Band.(B).SIG_PL(t,m,a,el,:) = Band.(B).SIG_PL(t,m,a,el,1);
                        elseif (el == Band.(B).n_el)
                            Band.(B).el{el,2} = n_az;
                            Band.(B).AZ(t,m,a,el,:) = Band.(B).AZ(t,m,a,el-1,:);
                            Band.(B).SIG_AFHP(t,m,a,el,:) = Band.(B).SIG_AFHP(t,m,a,el,1);
                        end
                        fprintf(afhpID,'%4.0f %4.0f\n',Band.(B).AZ(t,m,a,el,az),Band.(B).el{el,1});  
                        fprintf(plID,'%4.0f %4.0f\n',Band.(B).AZ(t,m,a,el,az),Band.(B).el{el,1});
                        for n_inc = 1:Band.(B).n
                            fprintf(afhpID,'%6.6f ',Band.(B).SIG_AFHP(t,m,a,el,az));
                            fprintf(plID,'%6.6f ',Band.(B).SIG_PL(t,m,a,el,az));
                        end
                        fprintf(afhpID,' \n');
                        fprintf(plID,' \n');                                                               
                    end
                end
                % generate .bda data
                % bda_file
                bda_filename = [sig_path B '_body_area.bda'];
                pla_filename = [sig_path B '_plume_area.pla'];
                bdaID = fopen(bda_filename,'w');
                plaID = fopen(pla_filename,'w');
                fprintf(bdaID,'%s\n%s\n%s\n   %3.0f   %3.0f\n\t','Brawler ShoeBox Area (m^2)','All Machs, Altitudes, Throttles',classification,n_az,Band.(B).n_el);
                fprintf(plaID,'%s\n%s\n%s\n   %3.0f   %3.0f\n\t','Brawler ShoeBox Area (m^2)','All Machs, Altitudes, Throttles',classification,n_az,Band.(B).n_el);
                for el = 1:Band.(B).n_el
                    fprintf(bdaID,' %6.1f ',Band.(B).el{el,1}(1));
                    fprintf(plaID,' %6.1f ',Band.(B).el{el,1}(1)); 
                    if (el == Band.(B).n_el)
                        fprintf(bdaID,' \n');
                        fprintf(plaID,' \n');
                    end
                end
%                 fprintf(plaID,'%s\n','      -90    -80    -70    -60    -50    -40    -30    -20    -10      0     10     20     30     40     50     60     70     80     90');
                prjarea = zeros(19);
                az_i = 0;
                for az_i=1:n_az
                    az = Band.(B).AZ(t,m,a,1,az_i);
                    fprintf(bdaID,' %3.0f\t',az);
                    fprintf(plaID,' %3.0f\t',az);
                    for el_i=1:Band.(B).n_el
                        el = Band.(B).el{el_i,1}(1);
                        prjarea(az_i,el_i) = sqrt( (PAREA(1)*cosd(az))^2 + (PAREA(2)*sind(az))^2 + (PAREA(3)*sind(el))^2 );
                        fprintf(bdaID,' %6.1f ',prjarea(az_i,el_i));
                        fprintf(plaID,' %6.1f ',0.0);
                    end
                    fprintf(bdaID,' \n');
                    fprintf(plaID,' \n');
                end
                fclose(bdaID);
                fclose(plaID);
                fclose(afhpID);
                fclose(plID);
                fprintf(sigID,' \t\t %s %s\n\t\t\t %s \t\t %s\n\t\t\t %s \t\t %s\n','band',B,'body_area',bda_filename,'plume_area',pla_filename);               
                fprintf(sigID,' \t\t\t %s \t %s\n\t\t\t %s \t %s\n','body_intensity',bd_filename,'plume_intensity',pl_filename);
                fprintf(sigID,' \t\t %s\n','end_band');
            end
            fprintf(sigID,'\t %s\n','end_state');
        end
    end
end
fprintf(sigID,'%s','end_target_model');
fclose(sigID);

% Apparently, mIncrement has to be the same for every band...that's dumb
% sosm code 10000/x = origin
% 1.0E+4F / (mOrigin + (mCount - 1) * mIncrement);
% band 1: 1.8 - 2.7    3703 1853 2  
% band 2: 3.0 - 4.0    2500 833 2
% band 3: 4.0 - 5.0    2000 500 2
% band 4: 8.0 - 12.0   833 417 2
% 





