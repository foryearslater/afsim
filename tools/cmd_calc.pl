#!/usr/bin/perl -X
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# NOTE: Most Perl scripts should always use the "-w" option on
#       the top line of the program to provide the user with
#       warning messages that will help identify coding errors.
#       However, this script is different because it is providing
#       all of the power of the Perl interpreter on each entered
#       line. Entering the "-w" option as follows:

#       #!/usr/bin/perl -w

#       will produce a correct, but irrelevant, error messages
#       that, after entering an "h" command for example, will
#       display the following error message: 'Unquoted string
#       "h" may clash with future reserved word at .....'.
#       This is because the Perl interpreter parses the entry
#       before it can be evaluated as a command. The use of
#       the "-X" option will disable all warnings, and, therefore,
#       its use is appropriate in this script.

$cx3_program = "cmd_calc.pl - An interactive command-line calculator";
$cx3_version = "3.2, 07/29/09";

# NOTE: To prevent needless warning messages,
#       do not run with perl -w option

$cx3_DEBUG = 0;  # Set DEBUG level: 0 = OFF, 1 = ON

# Use real constants for PI and K:
use constant PI => 4*atan2(1,1);  # Math CONSTANT PI
use constant K => PI/180.0;       # Math CONSTANT Degress to Radians

$cx3_max_cmds = 60;

$cx3_cmd_nr = 0;  # Init command number

$cx3_incmd[$cx3_cmd_nr] = "q"; # Init first input to "Quit"
$cx3_cmd[$cx3_cmd_nr] = "q";   # Init first element command list to "Quit"
$cx3_result[$cx3_cmd_nr] = 0;  # Init first result from command;

$cx3_last_entry = "q";

&help();

while (1) {
    $cx3_cmd_nr++;  # Increment command number
    print "\n[$cx3_cmd_nr]> ";
    chomp($cx3_entry = <STDIN>);

    # Prevent "help" command from executing twice:
    if ($cx3_entry =~ /^help/i) {
	$cx3_entry = '?';
    }

    # Save input for later display:
    $cx3_incmd[$cx3_cmd_nr] = $cx3_entry;

    # Ver 1.7 - Add feature to re-calculate previous formulae
    while ($cx3_entry =~ /\#(\d+)/) {
	$nr = $1;                        # Save command number

	# Ver 2.5
	if ($nr >= $cx3_cmd_nr) {
	    print "\n* ERROR: Invalid Command Number \"$nr\"";
	    $cx3_entry = 'ERROR';
	}
	else {
	    $cx3_entry =~ s/\#\d+/eval($cx3_incmd[$nr])/; # Substitute the command
	}

	if ($cx3_DEBUG >= 1) {
	    print "\n\n** DEBUG: \$nr    = $nr";
	    print "\n** DEBUG: \$cx3_entry = $cx3_entry\n";
	}
    }

    # Substitute result from last entry (Ver 2.4):
    while ($cx3_entry =~ /\!\!/) {
	$cx3_entry =~ s/\!\!/$cx3_last_entry/; # Substitute last entry

	if ($cx3_DEBUG >= 1) {
	    print "\n\n** DEBUG: \$nr    = $nr";
	    print "\n** DEBUG: \$cx3_entry = $cx3_entry\n";
	}
    }

    while ($cx3_entry =~ /\!(\d+)/) {
	$nr = $1;                         # Extract command number

	# Ver 2.5 - Invalid command number
	if ($nr >= $cx3_cmd_nr) {
	    print "\n* ERROR: Invalid Command Number \"$nr\"";
	    $cx3_entry = 'ERROR';
	}
	else {
	    # Ver 2.5 - Use saved result instead of re-calculating formulae
	    $cx3_entry =~ s/\!\d+/\$cx3_result[$nr]/; # Substitute the result
	}

	if ($cx3_DEBUG >= 1) {
	    print "\n\n** DEBUG: \$nr    = $nr";
	    print "\n** DEBUG: \$cx3_entry = $cx3_entry\n";
	}
    }

    if ($cx3_entry =~ /^eval\(/) {
	$cx3_entry =~ s/^eval\(//;
	$cx3_entry =~ s/\)$//;
    }

    if ($cx3_entry =~ /^\?$/) {
	# Execute "help" command
	$cx3_cmd[$cx3_cmd_nr] = $cx3_entry;  # Save for later use
	&help(); # Display help
    }
    elsif ($cx3_entry =~ /^h$/i) {
        # Execute "history" command
	$cx3_cmd[$cx3_cmd_nr] = $cx3_entry;  # Save for later use
	&history(); # Display last 30 commands
    }
    elsif ($cx3_entry =~ /^q$/i) {
	# QUIT program
	print "\nDONE\n\n";
	last; # Quit
    }

    if ($cx3_DEBUG >= 1) {
	print "\n** DEBUG: Final \$cx3_entry = $cx3_entry\n";
    }

    $cx3_cmd[$cx3_cmd_nr] = $cx3_entry; # Save entered command for later
    $cx3_result[$cx3_cmd_nr] = eval($cx3_entry); # Evaluate result

    print "Entry: $cx3_incmd[$cx3_cmd_nr] -> $cx3_entry -> $cx3_result[$cx3_cmd_nr]\n\t";

    $cx3_last_entry = $cx3_entry;
}

# Tangent(radians):
sub tan {
    if (!$_[0]) {
	sin($_)/cos($_); # Use default value
    }
    else {
	sin($_[0])/cos($_[0]);
    }
}

# Ver 2.4: Trig functions with degree arguments

# Sine(degrees):
sub sind {
    if (!$_[0]) {
	sin(K*$_); # Use default value
    }
    else {
	sin(K*$_[0]);
    }
}

# Cosine(degrees):
sub cosd {
    if (!$_[0]) {
	cos(K*$_); # Use default value
    }
    else {
	cos(K*$_[0]);
    }
}

# Tangent(degrees):
sub tand {
    if (!$_[0]) {
	sind($_)/cosd($_); # Use default value
    }
    else {
	sind($_[0])/cosd($_[0]);
    }
}

# Common logarithm (base 10):
sub log10 {
    if (!$_[0]) {
	log($_)/log(10); # Use default value
    }
    else {
	log($_[0])/log(10);
    }
}

# Radians - Arc Sine:
sub asin {
    if (!$_[0]) {
	atan2($_, sqrt(1-$_*$_));
    }
    else {
	atan2($_[0], sqrt(1-$_[0]*$_[0]));
    }
}

# Radians - Arc Cosine:
sub acos {
    if (!$_[0]) {
	atan2(sqrt(1-$_*$_), $_);
    }
    else {
	atan2(sqrt(1-$_[0]*$_[0]), $_[0]);
    }
}

# Ver 2.4 - Inverse trig function output in degrees

# Degrees - Arc Sine:
sub dasin {
    if (!$_[0]) {
	atan2($_, sqrt(1-$_*$_))/K;
    }
    else {
	atan2($_[0], sqrt(1-$_[0]*$_[0]))/K;
    }
}

# Degrees - Arc Cosine:
sub dacos {
    if (!$_[0]) {
	atan2(sqrt(1-$_*$_), $_)/K;
    }
    else {
	atan2(sqrt(1-$_[0]*$_[0]), $_[0])/K;
    }
}

# Degrees - Arc Tangent:
sub datan2 {
    if (!$_[0]) {
	print "\n** ERROR: datan2(Y,X) - Function needs 2 arguments";
    }
    else {
	atan2($_[0], $_[1])/K;
    }
}

# Power(A,X) - A to the X Power:
sub pow {
    if (!$_[0]) {
	print "\n** ERROR: pow(A,X) - Function needs 2 arguments";
    }
    else {
	$_[0]**$_[1];
    }
}

# Decimal degress to String DD:MM:SS.ss:
sub dms {
    # Ver 1.8 --
    my ($dd);       # Input value in decimal degrees

    if (!$_[0]) {   # If first argument does not exist:
	$dd = $_;   # Use default value
    }
    else {
	$dd = $_[0]; # Get first argument
    }

    my $sign = 1;   # Sign of input (default to positive)

    if ($dd < 0) {
	$sign = -1; # Save sign of input argument
	$dd = -$dd; # Work with positive values
    }

    my $dg = int($dd);         # Break out integer degrees
    my $fmn = 60*($dd - $dg);  # Break out float minutes
    my $mn = int($fmn);        # Break out integer minutes
    my $fsc = 60*($fmn - $mn); # Break out float seconds

    $fsc = sprintf("%.2f", $fsc);  # Round seconds to SS.ss

    # Check if seconds round-up to >= 60 and propagate round-up
    #  to minutes and degrees
    if ($fsc >=60.0) {
	$fsc -= 60.0;
	$mn++; # Increment number of minutes
    }
    if ($mn >= 60) {
	$mn -= 60;
	$dg++; # Increment number of degrees
    }

    # Ver 3.1 -- Properly handle sign if Degrees value = zero
    if ($sign > 0) {
	# Ver 1.8 -- Return String DD:MM:SS.ss:
	return sprintf("%d:%02d:%05.2f", $dg, $mn, $fsc);
    }
    else {
	return sprintf("-%d:%02d:%05.2f", $dg, $mn, $fsc);
    }
}

# Decimal degress to String DD:MM.mm:
sub dm {
    my ($dd);       # Input value in decimal degrees

    if (!$_[0]) {   # If first argument does not exist:
	$dd = $_;   # Use default value
    }
    else {
	$dd = $_[0]; # Get first argument
    }

    my $sign = 1;   # Sign of input (default to positive)

    if ($dd < 0) {
	$sign = -1; # Save sign of input argument
	$dd = -$dd; # Work with positive values
    }

    my $dg = int($dd);         # Break out integer degrees
    my $fmn = 60*($dd - $dg);  # Break out float minutes

    # Check if minutes round-up to >= 60 and propagate round-up
    #  to minutes and degrees (insuring round-up to 2-places after
    #  decimal:
    $fmn += 0.00004; # Ver 2.7
    if ($fmn >= 60.0) {
	$fmn -= 60.0;
	$dg++; # Increment number of degrees
    }

    # Ver 3.1 -- Properly handle sign if Degrees value = zero
    if ($sign > 0) {
	# Ver 1.8 -- Return String DD:MM.mmmm
	return sprintf("%d:%07.4f", $dg, $fmn);
    }
    else {
	return sprintf("-%d:%07.4f", $dg, $fmn);
    }
}

# String DD:MM:SS.ss or DD:MM.mm to decimal degrees
sub deg {
    my ($dstr);       # Input string in DMS form

    if (!$_[0]) {     # If first argument does not exist:
	$dstr = $_;   # Use default value
    }
    else {
	$dstr = $_[0]; # Get first argument
    }

    my ($dg, $mn, $sc); # Local variables: degrees, minutes, seconds
    my $sign = 1;     # Sign of input value

    if ($dstr =~ /^\-/) {
	$sign = -1;        # Set sign to negative
	$dstr =~ s/^\-//;  # Remove leading minus sign
    }

    if ($dstr =~ /^(\d+):(\d\d):(\d+\.?\d*)$/) {
	# Format: DD:MM:SS[.sss]
	$dg = $1;
	$mn = $2;
	$sc = $3;
    }
    elsif ($dstr =~ /^(\d+):(\d+\.?\d*)$/) {
	# Format: DD:MM[.mmm]
	$dg = $1;
	$mn = $2;
	$sc = 0.0;
    }
    else {
	print "\n** ERROR: Input string format not recognized";
	print "\n**        Returning -999999.99";
	$sign = -1;
	$dg = 999999.99;
	$mn = 0.0;
	$sc = 0.0;
    }

    if ($mn >= 60) {
	print "\n** WARNING: Minutes value greater than or equal to 60";
	print "\n**          Minutes = $mn\n";
    }

    if ($sc >= 60) {
	print "\n** WARNING: Seconds value greater than or equal to 60";
	print "\n**          Seconds = $sc\n";
    }

    # Ver 1.8 -- Output value in decimal degrees with restored sign:
    return $sign*($dg + $mn/60 + $sc/3600);
}

# SUBROUTINE sec2hms(<TIME in Seconds) - Convert Time in Seconds to
#                                        HH:MM:SS.ss format
sub sec2hms {
    my ($t_sec);

    if (!$_[0]) {    # If first argument does not exist:
	$t_sec = $_; # Use default value
    }
    else {
	$t_sec = $_[0]; # Get first argument
    }

    if ($t_sec < 0) {
	print "\n*ERROR - hms() - Input time less than zero = $t_sec";
	print "\nPress ENTER key to Quit ...";
	<STDIN>;
	die("\nTerminating execution ...\n");
    }

    my $hr = int($t_sec/3600);        # Break out integer hours
    my $fmn = ($t_sec - 3600*$hr)/60; # Break out float minutes
    my $mn = int($fmn);               # Break out integer minutes
    my $fsc = ($t_sec - 3600*$hr - 60*$mn); # Break out float seconds

    ##my $sc = int(0.5 + $fsc); # Round seconds to nearest second

    # Check if seconds round-up to >= 60 and propagate round-up
    #  to minutes and hours
    if ($sc >=60) {
	$sc -= 60;
	$mn++; # Increment number of minutes
    }
    if ($mn >= 60) {
	$mn -= 60;
	$hr++; # Increment number of hours
    }

    # Return String HH:MM:SS
    ##return sprintf("%d:%02d:%02d", $hr, $mn, $sc);

    # Return String HH:MM:SS.ss
    return sprintf("%d:%02d:%05.2f", $hr, $mn, $fsc);
}

# String HH:MM:SS.ss or HH:MM.mm to seconds
sub hms2sec {
    my ($hms_str);       # Input string in HMS form

    if (!$_[0]) {     # If first argument does not exist:
	$hms_str = $_;   # Use default value
    }
    else {
	$hms_str = $_[0]; # Get first argument
    }

    my ($hr, $mn, $sc); # Local variables: hours, minutes, seconds

    if ($hms_str =~ /^(\d+):(\d\d):(\d+\.?\d*)$/) {
	# Format: HH:MM:SS[.sss]
	$hr = $1;
	$mn = $2;
	$sc = $3;
    }
    elsif ($hms_str =~ /^(\d+):(\d+\.?\d*)$/) {
	# Format: HH:MM[.mmm]
	$hr = $1;
	$mn = $2;
	$sc = 0.0;
    }
    else {
	print "\n** ERROR: Input H:M:S.s string format not recognized";
	print "\n**        Returning -999999.99";
	$hr = -999999.99;
	$mn = 0.0;
	$sc = 0.0;
    }

    if ($mn >= 60) {
	print "\n** WARNING: Minutes value greater than or equal to 60";
	print "\n**          Minutes = $mn\n";
    }

    if ($sc >= 60) {
	print "\n** WARNING: Seconds value greater than or equal to 60";
	print "\n**          Seconds = $sc\n";
    }

    return (3600*$hr + 60*$mn + $sc);
}

# Convert degrees Fahrenheit to degrees Centigrade:
sub f2c {
    if (!$_[0]) {
	5.0/9.0*($_ - 32.0);    # Use default value
    }
    else {
	5.0/9.0*($_[0] - 32.0); # Use first argument
    }
}

# Convert degrees Centigrade to degrees Fahrenheit:
sub c2f {
    if (!$_[0]) {
	9.0/5.0*$_ + 32.0;    # Use default value
    }
    else {
	9.0/5.0*$_[0] + 32.0; # Use first argument
    }
}

# Convert nautical miles to kilometers:
sub nmi2km {
    if (!$_[0]) {
	$_*1.85200;    # Use default value
    }
    else {
	$_[0]*1.85200; # Use first argument
    }
}

# Convert kilometers to nautical miles:
sub km2nmi {
    if (!$_[0]) {
	$_/1.85200;    # Use default value
    }
    else {
	$_[0]/1.85200; # Use first argument
    }
}

# Convert statute miles to nautical miles
sub mi2nmi {
    if (!$_[0]) {
	$_/1.150779448;    # Use default value
    }
    else {
	$_[0]/1.150779448; # Use first argument
    }
}

# Convert nautical miles to statute miles
sub nmi2mi {
    if (!$_[0]) {
	$_*1.150779448;    # Use default value
    }
    else {
	$_[0]*1.150779448; # Use first argument
    }
}

# Convert feet to meters:
sub ft2m {
    if (!$_[0]) {
	$_/3.2808399;    # Use default value
    }
    else {
	$_[0]/3.2808399; # Use first argument
    }
}

# Convert meter to feet:
sub m2ft {
    if (!$_[0]) {
	$_*3.2808399;    # Use default value
    }
    else {
	$_[0]*3.2808399; # Use first argument
    }
}

# Convert meters to nautical miles:
sub m2nmi {
    if (!$_[0]) {
	$_/1852;    # Use default value
    }
    else {
	$_[0]/1852; # Use first argument
    }
}

# Convert nautical miles to meters:
sub nmi2m {
    if (!$_[0]) {
	$_*1852;    # Use default value
    }
    else {
	$_[0]*1852; # Use first argument
    }
}

# Convert feet to nautical miles (Ver 2.4):
sub ft2nmi {
    if (!$_[0]) {
	m2nmi(ft2m($_));    # Use default value
    }
    else {
	m2nmi(ft2m($_[0])); # Use first argument
    }
}

# Convert nautical miles to feet (Ver 2.4):
sub nmi2ft {
    if (!$_[0]) {
	m2ft(nmi2m($_));    # Use default value
    }
    else {
	m2ft(nmi2m($_[0])); # Use first argument
    }
}

# Convert knots to meters/second:
sub kts2mps {
    if (!$_[0]) {
	$_/1.9438440;    # Use default value
    }
    else {
	$_[0]/1.9438440; # Use first argument
    }
}

# Convert meters/second to knots:
sub mps2kts {
    if (!$_[0]) {
	$_*1.9438440;    # Use default value
    }
    else {
	$_[0]*1.9438440; # Use first argument
    }
}

# Convert knots to feet/second (Ver 2.4):
sub kts2fps {
    if (!$_[0]) {
	m2ft(kts2mps($_));    # Use default value
    }
    else {
	m2ft(kts2mps($_[0])); # Use first argument
    }
}

# Convert feet/second to knots (Ver 2.4):
sub fps2kts {
    if (!$_[0]) {
	mps2kts(ft2m($_));    # Use default value
    }
    else {
	mps2kts(ft2m($_[0])); # Use first argument
    }
}

# Convert heading in degrees to radians
#   NOTE: Quadrant uses AFSIM model standard: 0->180 = +, 360->180 = -
#                                             0->PI     , 0->-PI
sub hdg2rad {
    my $x = 0; # Declare local variable
    if (!$_[0]) {
	$x = $_; # Use default value
    }
    else {
	$x = $_[0];
    }
    if ($x > 360 || $x < 0) {
	print "\n** ERROR: Heading not in range 0 -> 360";
	print "\n**        Returning -999999.99";
	return -999999.99;
    }
    if ($x <= 180) {
	return $x*PI/180;
    }
    else {
	return ($x - 360)*PI/180;
    }
}

# Convert heading in radians to degrees
#   NOTE: Quadrant uses AFSIM model standard: 0->180 = +, 360->180 = -
#                                             0->PI     , 0->-PI
sub rad2hdg {
    my $x = 0; # Declare local variable
    if (!$_[0]) {
	$x = $_; # Use default value
    }
    else {
	$x = $_[0];
    }
    if ($x > PI || $x < -1*PI) {
	print "\n** ERROR: Heading not in range -PI -> +PI";
	print "\n**        Returning -999999.99";
	return -999999.99;
    }
    if ($x >= 0) {
	return $x*180/PI;
    }
    else {
	return (360 + $x*180/PI);
    }
}

# Convert a ratio to equivalent value in decibels (dB)
sub ratio2db {
    my $x = 0; # Declare local variable
    if (!$_[0]) {
	$x = $_; # Use default value
    }
    else {
	$x = $_[0];
    }
    $x = abs($x);  # All ratios must be positive

    return 10*log10($x);
}

# Convert a value in decibels (dB) to an equivalent ratio
sub db2ratio {
    my $x = 0; # Declare local variable
    if (!$_[0]) {
	$x = $_; # Use default value
    }
    else {
	$x = $_[0];
    }
    return pow(10, $x/10);
}

# Convert hours to days:
sub hrs2days {
    if (!$_[0]) {
	$_/24.0;    # Use default value
    }
    else {
	$_[0]/24.0; # Use first argument
    }
}

# Convert days to hours:
sub days2hrs {
    if (!$_[0]) {
	$_*24.0;    # Use default value
    }
    else {
	$_[0]*24.0; # Use first argument
    }
}

# Calculate the day-of-the-week (Ver 2.6)
sub dow {
    use Time::Local;
    my ($time, $time_str, $day_of_week);
    my $mon  = $_[0] - 1; # January Month-number equals zero
    my $mday = $_[1];
    my $yr   = $_[2];
    my $date_str = "";

    if ($mon < 0 || $mon > 11) {
	print "\n* ERROR: Month-number not in range 1-12\n";
    }
    elsif ($mday < 1 || $mday > 31) {
	print "\n* ERROR: Day-number not in range 1-31\n";
    }
    elsif ($yr < 1970 || $yr > 2037) {
	print "\n* ERROR: Year-number not in range 1970-2037\n";
    }
    else {
	$time = timelocal(0, 0, 0, $mday, $mon, $yr);
	$time_str = localtime($time);

	($day_of_week, $month, $day, $hms, $year) = split(/\s+/, $time_str);
	$date_str = sprintf("\"%s %d, %d = %s\"",
			    $month, $day, $year, $day_of_week);
    }
    return $date_str;
}

sub help {
    print "\n$cx3_program";
    print "\nVersion: $cx3_version";
    print "\n\nAvailable CONSTANTS:";
    print   "\n--------------------";
    print "\nPI = ", PI, "  -> PI";
    print "\nK = ", K, " -> Degrees to Radians";
    print "\n\nSome useful FUNCTIONS:";
    print "\n----------------------";
    print "\nsin(rad),  cos(rad),  tan(rad)";
    print "\nsind(deg), cosd(deg), tand(deg)";
    print "\nRadians: asin(x),  acos(x),  atan2(y,x)";
    print "\nDegrees: dasin(x), dacos(x), datan2(y,x)";
    print "\nsqrt(x), log(x),  exp(x), log10(x), pow(a,x)";
    my $dms_str = "\"D:M:S\"";
    print "\ndms(deg), dm(deg), deg($dms_str)";
    my $hms_str = "\"H:M:S\"";
    print "\nsec2hms(sec), hms2sec($hms_str)";
    print "\nf2c(deg F), c2f(deg C)";
    print "\nnmi2km(nmi), km2nmi(km), mi2nmi(mi), nmi2mi(nmi)";
    print "\nft2m(ft), m2ft(m), m2nmi(m), nmi2m(nmi)";
    print "\nft2nmi(ft), nmi2ft(nmi)";
    print "\nkts2mps(kts), mps2kts(mps)";
    print "\nkts2fps(kts), fps2kts(fps)";
    print "\nhdg2rad(hdg), rad2hdg(rad)";
    print "\nratio2db(ratio), db2ratio(db)";
    print "\nhrs2days(hrs), days2hrs(days)";
    print "\ndow(month,day,year)";
    print "\n\n!!                - Recall result from last entry";
    print "\n!<COMMAND NUMBER> - Recall result from a previous command number";
    print "\n#<COMMAND NUMBER> - Recall formula from a previous command number";
    print "\n\? or help - Display this HELP list";
    print "\nh         - Display HISTORY of last $cx3_max_cmds commands";

    print "\n\nEnter Perl expression (\"(q)uit\" to QUIT):\n";
}

# List last "$cx3_max_cmds" commands:
sub history {
    my $last_cmd = scalar(@cx3_cmd) - 1;
    my $start_cmd = $last_cmd - $cx3_max_cmds; # Calc range of commands to display
    if ($start_cmd < 1) {
	$start_cmd = 1;  # Last command is less than "$cx3_max_cmds"
    }
    for (my $i = $start_cmd; $i <= $last_cmd; $i++) {
	print "\n[$i]> $cx3_incmd[$i] -> $cx3_cmd[$i] -> $cx3_result[$i]";
    }
    print "\n";
    return " ";
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - cmd_calc.pl

=head2 SYNOPSIS

cmd_calc.pl - Provide a command-line calculator utility for
user-interactive numerical calculations along with useful functions.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This utility brings the Perl interpreter's functionality to the
command-line. The user enters a numerical calculation using any Perl
functions with Perl expression syntax, and then the user presses the
ENTER key to execute the calculation. For example, to calculate 2
divided by 3, enter:

 2/3 <Press the ENTER key>

Note that each line has a number associated with it. If the above
calculation would be entered immediately after starting "cmd_calc.pl",
the calculation would look like this after the ENTER key is pressed:

 [1]> 2/3
 Entry: 2/3 -> 2/3 -> 0.666666666666667

To use the result from this calculation in another calculation, enter
"!1": the "!" indicates that the next number entered will be the
command line number. For example to multiply the result from
command-line number 1 by three, the result would show:

 [2]> !1*3
 Entry: !1*3 -> $cx3_result[1]*3 -> 2

In this case, since the previous command-line's result is being used,
the "!1" could be substituted with "!!".

NOTE: For more complex calculations, it is generally a good idea to
surround previous results calculations with parentheses to insure the
order of calculation is correctly preserved. For example, entering "2
+ 3" will produce the result "5". However, if the next command is
"!!*5" the result will be "17" instead of "25" because the resulting
command substitution will be 2 + 3*5 instead of (2 + 3)*5. As an
alternative, the first input could have been (2 + 3) -- including the
parentheses so that "!!*5" would produce "25". The Perl interpreter
processes the result exactly as entered.

The constants PI and K (K is the ratio of PI/180 for converting
degrees to radians) are also available. These constants are entered
without a leading dollar-sign. However, the user can define Perl
variables by adding a leading dollar-sign to a variable name. For
example to assign the value of the conversion of 180 degrees to
radians in the variable "$angle", enter:

 [3]> $angle = 180*K
 Entry: $angle = 180*K -> $angle = 180*K -> 3.14159265358979

NOTE: Since the user is interactively creating new Perl variables
within an executing Perl program, the new variables cannot be the same
as existing variable names within the main program. To limit the
chance for inadvertently duplicating main program names, all main
program variable names begin with "$cx3_". Variable names used in the
Perl functions are local to the functions in which they are declared.

To display the list of additional useful functions (not including
those functions already defined in Perl), enter "?" or "help" Use
these functions on a command-line as they are listed. While most of
the functions expect numerical arguments, the "deg()" and "hms2sec()"
functions need to have their input argument entered as a string. For
example, to calculate the number of seconds in 1 hour, 2 minutes and
3 seconds, enter:

 [4]> hms2sec("1:02:03")
 Entry: hms2sec("1:02:03") -> hms2sec("1:02:03") -> 3723

NOTE: This function expects the entry format to be HH:MM:SS.sssss
or HH:MM.mmmmm, where lower case "s" indicates decimal seconds and
lower case "m" indicates decimal minutes. The user can modify
this function or create additional functions as needed.

To list the history of previous command-line results, enter "h". For
example, to list the previous example calculations

 [1]> 2/3 -> 2/3 -> 0.666666666666667
 [2]> !1*3 -> $cx3_result[1]*3 -> 2
 [3]> $angle = 180*K -> $angle = 180*K -> 3.14159265358979
 [4]> hms2sec("1:02:03") -> hms2sec("1:02:03") -> 3723
 [5]> h -> h ->
 Entry: h -> h -> h

If a user-defined variable is provided in a formula, then, if
another value is assigned to the variable, the formula command-line
can be executed again by preceding the command line number with "#".
For example, "$angle" is assigned a value of "10":

 [6]> $angle = 10
 Entry: $angle = 10 -> $angle = 10 -> 10

Then, a formula using "$angle" is entered:

 [7]> $angle/2
 Entry: $angle/2 -> $angle/2 -> 5

Next, "$angle" is given a new value of "5"

 [8]> $angle = 5
 Entry: $angle = 5 -> $angle = 5 -> 5

To re-calculate the formula in command-line number 7, enter:

 [9]> #7
 Entry: #7 -> $angle/2 -> 2.5

Note that the formula "$angle/2" is re-calculated with the
new value of "$angle". However, if the value from the original
calculation from command-line number 7 is needed, then enter:

 [10]> !7
 Entry: !7 -> $cx3_result[7] -> 5

To exit "cmd_calc.pl", enter CONTROL-C or enter "q" and press ENTER.

The existing subroutines in this utility have been added when they
were needed to support specific analysis efforts. It's easy to add
your own functions to this utility. Create a new subroutine and add it
above the "help" subroutine. Then, add a brief print statement in the
"help" subroutine to describe the required input parameters.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F cmd_calc.pl > cmd_calc_doc.txt

 In a Linux environment:
   perldoc -otext -F cmd_calc.pl > cmd_calc_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=cmd_calc.pl --outfile=cmd_calc_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the Boeing-standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as Boeing-standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted.)

=head2 AUTHOR

 Gary Richardson
 Strategic Projects & Analysis
 Boeing Research & Technology

=head2 BUGS AND TECHNICAL SUPPORT

If you think you have found a bug in one of our existing data
processing/analysis tools, have ideas on how to improve these
utilities, or simply want help in getting started using the Perl
programming language in your modeling/simulation efforts (including
models other than those developed using AFSIM), please contact us.

 Gary Richardson, Associate Technical Fellow
 Strategic Projects & Analysis
 Boeing Research & Technology
 Tel: 314-232-5451
 E-mail: gary.a.richardson@boeing.com

=head2 COPYRIGHT

Copyright - The Boeing Company, 2009

=cut
