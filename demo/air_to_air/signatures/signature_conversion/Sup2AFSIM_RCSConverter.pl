# ****************************************************************************
# CUI//REL TO USA ONLY
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
#!perl -w
use strict;
#Notes...
#This perl script is to reformat suppressor res data in salram format
#   which can be read by AFSIM and viewed with sigview.
#This might not work in general because I only have one suppressor sig
#   file to work from.
#I know that the dimensions can be swapped and this script works for the
#   following dimension order...
#   Dimension 1 Frequency
#   Dimension 2 Polarization
#   Dimension 3 Elevation
#   Dimension 4 Azimuth
#The perl script writes all the polarization/frequency combinations to a
#   single file. The user can then copy and paste these into individual
#   files.

#AFSIM RCS convension:
#Body coordinates are x-nose, y-rightwing, z-down
#   los vector points from aircraft to emitter
#   +az is on the right, and +el is up from pilot's perspective
#   AFSIM interpolates between RCS values

#Suppressor RCS convension (according to Brett Meador):
#   It does not interpolate, thus is puts values in bins.
#   +az is on the left, and +el is up from pilot's perspective

#Assumptions...
#Aspect angles cover entire space ie az is [-180,180] and el is
#   [-90,90]. Aspect angle increments do not have to be constant,
#   but they do have to be consistent within an frequency/polarization
#   table.

my $inputFileName   = "";
my $outputFileName  = "";
my $classification  = "";
my @Lines           = ();
my $line            = "";
my @Data            = ();
my @Polarizations   = ();
my $polarization    = "";
my $dimension2Count = 1;
my $dimension3Count = 0;
my $dimension4Count = 0;
my @Frequencies     = ();
my $frequency       = "";
my @SuppAzBins      = ();
my @Azimuths        = ();
my $azimuth         = "";
my $numAzPts        = "";
my $azPt            = "";
my @SuppElBins      = ();
my @Elevations      = ();
my $elevation       = "";
my $numElPts        = "";
my $elPt            = "";
my @Rcs             = 0;
my $rcs             = "";
my %RcsByAzEl       = ();
my $numRcsPts       = "";
my $insideRcsTable  = 0;

open IN,  "<$inputFileName";
open OUT, ">$outputFileName";
while ($line = <IN>) {

#   if ($line =~ /^\s+RCS-TABLE]^RCS-TABLE/) {
#      $insideRcsTable = 1;
#   }
   
#   if ($line =~ /^\s+END\s+RCS-TABLE|^END\s+RCS-TABLE/) {
#      $insideRcsTable =0;
#   }
   
#   if (!$insideRcsTable) { next; }

	if ($line =~ /^\s*\$/) { next; }
   
  if ($line =~ /^\s+DIMENSION 1|^DIMENSION l/) {
      #Reset Variables and Counters
      @Frequencies     = ();
      $dimension2Count = 1;
      until (($line = <IN>) =~ /^\s*\$/ || ($line = <IN>) =~ /\s+DIMENSION 2|^DIMENSION 2/) {
         #print "$line\n";
         @Data = split ' ', $line;
         push @Frequencies, @Data;
      }
      #print "@Frequencies\n";
      #exit;
   }
   
   if ($line =~ /\s+DIMENSION 2|^DIMENSION 2/) {
      #Reset Variables and Counters
      @Polarizations   = ();
      $dimension3Count = 0;
      #Increment Variables and Counters
      $frequency = $Frequencies[$dimension2Count];
      $dimension2Count++;
      until (($line = <IN>) =~ /^\s*\$/ || ($line = <IN>) =~ /\s+DIMENSION 3|^DIMENSION 3/) {
         @Data = split ' ', $line;
         push @Polarizations, @Data;
      }
      #debug#print "@Polarizations\n";
      #debug#exit;
   }
   
   if ($line =~ /\s+DIMENSION 3|^DIMENSION 3/) {
      #Reset Variables and Counters
      @SuppElBins = ();
      @Elevations = ();
      %RcsByAzEl  = ();
      $dimension4Count = 0;
      
      #Increment Variables and Counters
      $polarization = $Polarizations[$dimension3Count];
      $dimension3Count++;
      
      until (($line = <IN>) =~ /^\s*\$/ || ($line = <IN>) =~ /\s+DIMENSION 4|DIMENSION 4/) {
         #debug#print "$line\n"; 
         @Data = split ' ', $line;
         push @SuppElBins, @Data;
      }
      $numElPts = @SuppElBins;
      #debug#print "$polarization $frequency $numElPts\n";
      #debug#exit;
      push @Elevations,-90.;
      for ($elPt = 1; $elPt < $numElPts-2; $elPt++) { 
         push @Elevations, ($SuppElBins[$elPt]+$SuppElBins[$elPt+1])/2;
      }
      push @Elevations,90.;
      $numElPts = @Elevations;
      #debug#print "@Elevations\n"; 
      #debug#exit;   
   }
      
   if ($line =~ /\s+DIMENSION 4|DIMENSION 4/) {
      #Reset Variables and Counters
      @SuppAzBins = 0;
      @Azimuths   = ();
      
      #Increment Variables and Counters
      $elevation = $Elevations[$dimension4Count];
      $dimension4Count++;
      
      until (($line = <IN>) =~ /^\s*\$/ || ($line = <IN>) =~ /^\s+RCS \(DBSM\)/) {
         #debug#print "$line\n";
         @Data = split ' ', $line;
         push @SuppAzBins, @Data;
      }
      $numAzPts = @SuppAzBins;
      #debug#print "$numAzPts\n";
      #debug#exit;
      
      push @Azimuths, -180.;
      
      for ($azPt = 1; $azPt < $numAzPts-2; $azPt++) { 
         push @Azimuths,($SuppAzBins[$azPt]+$SuppAzBins[$azPt+1])/2;
      }
      
      push @Azimuths,180.;
      $numAzPts = @Azimuths;
      #debug#print "@Azimuths\n";
      #debug#exit;
      #debug#print "$polarization $frequency $numAzPts $numElPts\n";
      #debug#exit;
   }
   
   if ($line =~ /^\s+RCS\s+\(DBSM\)|^RCS\s+\(DBSM\)/) {
      #Reset Variables and Counters
      @Rcs = ();
      
      #until (($line = <IN>) =~ /^\s+\$ AZ\:|^\s+DIMENSION 2|^\s+DIMENSION 3|^\s+END DATA-TABLE/) {
      until ( ($line = <IN>) !~ /^\s+-?\d+/) {
         #debug#print "$line\n";
         @Data = split ' ', $line;
         push @Rcs, @Data;
      }
      
      $numRcsPts = @Rcs;
      if ($numRcsPts != $numAzPts) {
         die("ERROR! Dimension mismatch between number of az pts, $numAzPts, and rcs pts, $numRcsPts!\n");
      }
      #debug#print "$polarization $frequency $numAzPts $numElPts $numRcsPts\n";
      #debug#exit;
      
      #Note: Azimuths are swapped here!!!
      $azPt = $numAzPts - 1;
      foreach $rcs (@Rcs) {
         if (!defined($Azimuths[$azPt])) {
            print "$polarization $frequency $elevation\n";;
            print "$azPt $numAzPts\n";
            print "@Azimuths\n";
            die("Undefined azimuth point.\n");
         }
         if (!defined($elevation)) {
            print "$polarization $frequency\n";
            print "$numElPts $numAzPts\n";
            print "$dimension4Count\n";
            print "@Elevations\n"; 
            die("Undefined elevation point.\n");
         }
         $azimuth = $Azimuths[$azPt];
         $RcsByAzEl{$azimuth}{$elevation} = $rcs;
         $azPt--; 
         #debug#print "$azimuth $elevation $rcs\n";
      }
      #debug#exit;
      
      #debug#print "$dimension4Count $numElPts\n";
      if ($dimension4Count == $numElPts) {
        #At end of a polarization/frequency table, write out data.
        #debug#exit;
        print OUT "\# Classification: $classification\n";
        print OUT "\# Source File: $inputFileName\n";
        print OUT "\# Polarization: $polarization; Upperbound Frequency: $frequency\n";
        print OUT " $numAzPts $numElPts\n";
        print OUT "\n";
        foreach $elevation (@Elevations) {
					printf OUT ("%8.2f",$elevation);
				}
        print OUT "\n";
         
        foreach $azimuth (@Azimuths) {
          printf OUT ("%8.2f",$azimuth);
          foreach $elevation (@Elevations) {
            #debug#print "$azimuth $elevation $RcsByAzEl{$azimuth}{$elevation}\n";
            printf OUT ("%8.2f",$RcsByAzEl{$azimuth}{$elevation});
            #debug#exit;
          }
          print OUT "\n";
          #debug#exit;
        }
        #debug#exit;
      }
      
      if ($line =~
      /^\s+DIMENSI0N\s+2|^DIMENSION\s+2|^\s+DIMENSION\s+3|^DIMENSI0N\s+3|^\s+DIMENSION\s+4|^DIMENSION\s+4|^\s+RCS\s+\(DBSM\)|^RCS\s+\(DBSM\)/) {
         #If $line ends on the above, don't want the while loop to
         #increment to the next line so use redo to go back to the top
         #of the block without incrementing.
         redo;
      }
   }
}
close IN;
close OUT;
