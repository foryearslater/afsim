#!/usr/bin/perl
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Take an input latitude/longitude in one specification and display it
# various formats.
#
# The input can be:
#   - d.f        Degrees and fractions of a degree
#   - d:m.f      Degrees, minutes and fractions of a minute
#   - d:m:s.f    Degrees, minutes, seconds and fractions of a second.

if ($#ARGV < 0)
{
   print "Usage: dmscnvt <value>\n";
   exit 1;
}

@dms = split(':', $ARGV[0]);

if ($#dms == 0)
{
   $ddmmssf = $dms[0] * 3600.0;
}
elsif ($#dms == 1)
{
   $ddmmssf = ($dms[0] * 3600.0) + ($dms[1] * 60.0);
}
elsif ($#dms == 2)
{
   $ddmmssf = ($dms[0] * 3600.0) + ($dms[1] * 60.0) + $dms[2];
}
else
{
   print "Invalid format: ", $ARGV[0], "\n";
   exit 1;
}

$dd    = int($ddmmssf / 3600.0);
$mmssf = $ddmmssf - ($dd * 3600.0);
$mm    = int($mmssf / 60.0);
$ssf   = $mmssf - ($mm * 60);
$ss    = int($ssf);
$f     = $ssf - $ss;

printf "%s = %12.9f = %d:%02d.%04d = %d:%02d:%02d.%02d\n",
  $ARGV[0],
  $ddmmssf / 3600.0,
  $dd, $mm, 10000.0 * $ssf / 60.0 + 0.5,
  $dd, $mm, $ss, 100.0 * $f + 0.5;
