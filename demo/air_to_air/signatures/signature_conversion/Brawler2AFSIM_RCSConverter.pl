# ****************************************************************************
# CUI//REL TO USA ONLY
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
#!perl

use POSIX;
#Classification: UNCLASSIFIED//FOUO

# Input file is Brawler *.FXW file containing rcs for various frequencies (in meters squared)
# This script will create a separate *.rcs file for each frequency specified in the *.FXW file

# Brawler signature may be defined as symmetric in azimuth and thus only azimuth angles 0-180 are specified
# If <az_symmetry> != 0, it is assumed that azimuth angles for one side are specified in the input file and reflected azimuths will be generated
# If <az_symmetry>  = 0, it is assumed that azimuth angles for both sides are specified in the input file and only input values will be utilized

if (scalar (@ARGV) != 2)
{
   die "\nUsage: <input file> <az_symmetry flag>";
}

$inputFile = shift(@ARGV);
$az_symmetry = shift(@ARGV);

###################################################################################################

open(INPUT_FILE, $inputFile) || die " *** Error Opening File: $inputFile ***\n";
@lines = <INPUT_FILE>;
close INPUT_FILE;

###################################################################################################
$numAZ = 0;
$numEL = 0;
$numRCS = 0;
@azValues = ();
@elValues = ();
@RCSValues = ();
@outfile_name = split(/\./, $inputFile);

for($i=0; $i<=$#lines; $i++)
{
   $currentLine = Trim($lines[$i]);
   
   if($currentLine =~ /RCSFREQ\s+(\d+\.\d+)/)
   {
      $freq_mhz = $1*1000;
      $outputFile = $outfile_name[0] . "_" . $freq_mhz . "mhz.rcs";
      print "generating output file: $outputFile\n";
   }
   if($currentLine =~ /NAZPTS\s+(\d+)/)
   {
      $numAZ = $1;
   }
   if($currentLine =~ /NELPTS\s+(\d+)/)
   {
      $numEL = $1;
   }
   if($currentLine =~ /AZVAL/)
   {
      @azValues = ReadInput($i, $numAZ);
   }
   if($currentLine =~ /ELVAL/)
   {
      @elValues = ReadInput($i, $numEL);
   }
   if($currentLine =~ /RDRXS/)
   {
      $numRCS = $numAZ * $numEL;
      @RCSValues = ReadInput($i, $numRCS);
      $rcsData = ConvertRCSData(\@elValues, \@azValues, \@RCSValues);
      PrintOutput($rcsData, $outputFile);
   }
}

###################################################################################################
sub PrintOutput
{
   my %rcsData = %{$_[0]};
   my $outputFile = $_[1];

   open(OUTPUT_FILE,'>',$outputFile) || die " *** Error Opening File: $outputFile ***\n";

   @azValues = sort NumericalSort keys %rcsData;
   @elValues = sort NumericalSort keys %{ $rcsData{$azValues[0]} };
   $numAZ = scalar(@azValues);
   $numEL = scalar(@elValues);

   # Write header information
   # Replace "line 1-3" in print statements below with header information if desired...
   print OUTPUT_FILE "line 1\n";
   print OUTPUT_FILE "line 2\n";
   print OUTPUT_FILE "line 3\n";
   print OUTPUT_FILE " $numAZ $numEL\n";
   # Write elevation angle values...
   print OUTPUT_FILE ("          ", (map{ sprintf('%10s', $_)} @elValues), "\n");

   # Write azimuth angle and rcs values...
   foreach $azValue (@azValues)
   {
      print OUTPUT_FILE sprintf('%10s', $azValue);
      foreach $elValue (@elValues)
      {
         print OUTPUT_FILE sprintf('%10s', $rcsData{$azValue}{$elValue});
      }
      print OUTPUT_FILE "\n";
  }

   close OUTPUT_FILE;
}

###################################################################################################
sub ConvertRCSData
{
   my @elValues = @{$_[0]};
   my @azValues = @{$_[1]};
   my @rcsValues = @{$_[2]};
   
   foreach $elValue (@elValues)
   {
      $elValue = sprintf("%.2f",$elValue);  #Convert from scientific notation to floating point
   
      foreach $azValue (@azValues)
      {
         $azValue = sprintf("%.2f",$azValue);  #Convert from scientific notation to floating point
      
         $rcsData_m2 = shift(@RCSValues);
      
         #Convert from m2 to dB and convert to floating point
         $rcsData{$azValue}{$elValue} = sprintf("%.2f", 10.0 * log10($rcsData_m2));
         if($az_symmetry && ($azValue != 0.0))
         {
            $negAzValue = sprintf("%.2f",-$azValue);
            $rcsData{$negAzValue}{$elValue} = $rcsData{$azValue}{$elValue};
         }
      }
   }
   return \%rcsData;
}

####################################################################################################
sub NumericalSort
{
   $a <=> $b;
}

###################################################################################################
sub Trim
{
   my $string = $_[0];
   $string =~ s/^\s+|\s+$//g;
   return $string;
}

###################################################################################################
sub ReadInput
{
   my $lineCounter = $_[0];
   my $numOfValueToRead = $_[1];
   my @array = ();
   $currentLine = Trim($lines[$lineCounter]);
   
   @array = split(/\s+/, $currentLine);
   shift (@array);  # Remove label
   while ($#array+1 < $numOfValueToRead)
   {
      $lineCounter++;
      $currentLine = Trim($lines[$lineCounter]);
      push @array, split(/\s+/, $currentLine);
   }
   return @array;
}
