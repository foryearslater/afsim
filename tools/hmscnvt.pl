#!/usr/bin/perl
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Take an input time in one specification and display it
# various formats.
#
# The input can be:
#   - h.f        Hours and fractions of an hour
#   - h:m.f      Hours, minutes and fractions of a minute
#   - h:m:s.f    Hours, minutes, seconds and fractions of a second.

if ($#ARGV < 0)
{
   print "Usage: hmscnvt.pl <value>\n";
   exit 1;
}

@hms = split(':', $ARGV[0]);

if ($#hms == 0)
{
   $hhmmssf = $hms[0];
}
elsif ($#dms == 1)
{
   $hhmmssf = ($hms[0] * 60.0) + $hms[1] * 60.0;
}
elsif ($#hms == 2)
{
   $hhmmssf = ($hms[0] * 3600.0) + ($hms[1] * 60.0) + $hms[2];
}
else
{
   print "Invalid format: ", $ARGV[0], "\n";
   exit 1;
}

$hh    = int($hhmmssf / 3600.0);
$mmssf = $hhmmssf - ($hh * 3600.0);
$mm    = int($mmssf / 60.0);
$ssf   = $mmssf - ($mm * 60);
$ss    = int($ssf);
$f     = $ssf - $ss;

printf "%s = %d:%02d:%02d.%02d = %d:%02d.%04d = %12.9f = %12.3f\n",
  $ARGV[0],
  $hh, $mm, $ss, 100.0 * $f + 0.5,
  $hh, $mm, 10000.0 * $ssf / 60.0 + 0.5,
  $hhmmssf / 3600.0,
  $hhmmssf;
