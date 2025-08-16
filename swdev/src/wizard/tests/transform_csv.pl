# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
#!/usr/bin/perl

use strict;
use feature qw(switch say);
use Text::ParseWords;
use List::Util qw(first);
use Data::Dumper;

my $d = $ARGV[0];

my $true = 1;
my $separator = ",";
my @table;
my $dir;

foreach $dir (glob("auto_*"))
{
   if (-d $dir)
   {
      chdir $dir;
      my %colNumToNames;
      my %outputTables;
      my $nameCol;
      my $file;
      foreach $file (glob("test_*.csv"))
      {
         open (FILE_CSV, "<", $file) or die $!;
         $_ = <FILE_CSV>;
         chomp;
         my @row = parse_line($separator, $true, $_);

         my $header;
         my $testNameCol;
         foreach $header (@row)
         {
            # remove white space
            $header=~s/^\s+//;
            $header=~s/\s+$//;
            my $col = first { $row[$_] eq $header } 0 .. $#row;
            if ($header ne "Test Name")
            {
               $colNumToNames{$col} = $header;
            }
            else
            {
               $testNameCol = $col;
            }
         }
         while (<FILE_CSV>)
         {
            chomp;
            @row = parse_line($separator, $true, $_);
            my $value;
            foreach $value (@row)
            {
               my $col = first { $row[$_] eq $value } 0 .. $#row;
               my $header = $colNumToNames{$col};
               if (exists $colNumToNames{$col})
               {
                  my $colName = $colNumToNames{$col};

                  push @{$outputTables{$header}[0]}, $row[$testNameCol];
                  push @{$outputTables{$header}[1]}, $value;
               }
            }
         }
         close FILE_CSV;
      }
      for my $file (keys %outputTables)
      {
         open (OUTPUTFILE, ">$file.csv");
         local($\, $,) = ("\n", ", ");
         print OUTPUTFILE map {
            my $s = $_;
            $s =~ s/([\n;])|"/ $1 || '""' /ge and $s = qq["$s"];
            $s
         } @$_ foreach \@{$outputTables{$file}[0]}, \@{$outputTables{$file}[1]};
         close (OUTPUTFILE);
      }
   }
}
