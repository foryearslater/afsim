#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "latlon_trans.pl";

$title = $pgm_name . " - Calculate Lat/Long translation";
$version = "Version: 5/22/2009";

# See documentation for this utility at bottom of this script

# ======================================

print "\n$title";
print "\n$version\n";

print "\n\nEnter parameters (or type \"q\" or CNTL-C to Quit):\n";

# Initialize parmeters:
$lat1_str = "00:00:00.0N";
$lon1_str = "000:00:00.0E";
$lat2_str = "00:00:00.0N";
$lon2_str = "000:00:00.0E";
$lat_tr_str = "00:00:00.0N";
$lon_tr_str = "000:00:00.0E";

$good_select_flag = 1; # Set "good-selection" flag to TRUE
$parm_ltr = "";	       # Input selection parameter

$start_flag = 0;       # Suppress clear-screen at start-up
while (1) {
    if ($start_flag) {
	&clear_screen(); # Clear screen
    }
    else {
	$start_flag = 1; # Clear screen on next cycle of while-loop
    }

    ($lat_tr_str, $lon_tr_str) = &translate($lat1_str,
					    $lon1_str,
					    $lat2_str,
					    $lon2_str);

    print "\n";			# Skip a line

    print "\nTranslate from Waypoint #1 to Waypoint #2:";
    print "\n---------------------------------------------";
    printf "\n(1) Latitude  Waypoint #1 =  %s", $lat1_str;
    printf "\n(2) Longitude Waypoint #1 = %s", $lon1_str;
    print "\n";
    printf "\n(3) Latitude  Waypoint #2 =  %s", $lat2_str;
    printf "\n(4) Longitude Waypoint #2 = %s", $lon2_str;
    print "\n---------------------------------------------";
    printf "\nTranslated Latitude  =  %s", $lat_tr_str;
    printf "\nTranslated Longitude = %s", $lon_tr_str;
    print "\n---------------------------------------------";

    if (! $good_select_flag) {
	$good_select_flag = 1;  # Reset flag to TRUE
	print "\n\n* ENTRY ERROR: Invalid entry: \"$parm_ltr\"";
	print "\n   Try again or enter \"q\" or CNTL-C to end this script";
    }

    print "\n\nENTER Line number to change (q or CNTL-C to QUIT): ";
    chomp($parm_ltr = <STDIN>);

    if ($parm_ltr =~ /^1$/i) {
	printf "\nLatitude  Waypoint #1 = %s (ENTER = No change): ",
	$lat1_str;
	# Get possible new latitude
	$lat1_str = &get_lat($lat1_str);
    }
    elsif ($parm_ltr =~ /^2$/i) {
	printf "\nLongitude Waypoint #1 = %s (ENTER = No change): ",
	$lon1_str;
	# Get possible new longitude
	$lon1_str = &get_lon($lon1_str);
    }
    elsif ($parm_ltr =~ /^3$/i) {
	printf "\nLatitude  Waypoint #2 = %s (ENTER = No change): ",
	$lat2_str;
	# Get possible new latitude
	$lat2_str = &get_lat($lat2_str);
    }
    elsif ($parm_ltr =~ /^4$/i) {
	printf "\nLongitude Waypoint #2 = %s (ENTER = No change): ",
	$lon2_str;
	# Get possible new longitude
	$lon2_str = &get_lon($lon2_str);
    }
    elsif ($parm_ltr =~ /(q|exit)/i) {
	last; # Exit this script
    }
    else {
	$good_select_flag = 0;	# Set flag to FALSE: unknown parameter
    }
}

# ------------- SUBROUTINES -----------------

# Subroutine clear_screen() - Clear screen according to
#                             the operating system being used
sub clear_screen {
    my $os = $^O; # Get operating system name
    my $clear_screen_cmd = "unknown";

    # Set clear-screen command according to operating system:
    if ($os =~ /^mswin/i) {
	$clear_screen_cmd = 'cls';
    }
    elsif ($os =~ /^linux/i) {
	$clear_screen_cmd = 'clear';
    }

    # Execute clear-screen command for appropriate operating
    #  system or, if unknown, output 6 blank lines
    if ($clear_screen_cmd eq 'unknown') {
	print "\n\n\n\n\n\n";
    }
    else {
	system($clear_screen_cmd); # Clear screen
    }
    return;
}

# Subroutine get_lat(<old_latitude>) - Get possible new latitude
sub get_lat {
    my $old_lat = $_[0];    # Get first argument
    my $new_lat = $old_lat; # Init new Lat (if no change)
    my $str = "";	    # Init string for possible new value

    print "\nEnter Latitude (D[D]:MM:SS[.s](N|S)): ";
    chomp($str = <STDIN>);
    # Relmove leading and trailing whitespace:
    $str =~ s/^\s+//;
    $str =~ s/\s+$//;

    if (length($str) > 0) {
	while (1) {
	    if ($str =~ /^\d{1,2}:[0-5]\d:[0-5]\d\.?\d*[ns]$/i) {
		$new_lat = $str;
		last; # Exit while-loop
	    }
	    else {
		print "\nUnknown D:M:S<Quadrant> format (ex: 11:22:33N)";
		print "\nPlease re-enter ...";

		print "\nEnter Latitude (D[D]:MM:SS[.s](N|S)): ";
		chomp($str = <STDIN>);
		# Relmove leading and trailing whitespace:
		$str =~ s/^\s+//;
		$str =~ s/\s+$//;

		if (length($str) == 0) {
		    last; # Exit while-loop
		}
	    }
	}
    }
    return &deg2dms(&dms2deg($new_lat), "LAT"); # Return possibly updated Latitude
}

# Subroutine get_lon(<old_longitude>) - Get possible new longitude
sub get_lon {
    my $old_lon = $_[0];    # Get first argument
    my $new_lon = $old_lon; # Init new value
    my $str = "";	    # Init string for possible new value

    print "\nEnter Longitude (D[DD]:MM:SS[.s](E|W)): ";
    chomp($str = <STDIN>);
    # Relmove leading and trailing whitespace:
    $str =~ s/^\s+//;
    $str =~ s/\s+$//;

    if (length($str) > 0) {
	while (1) {
	    if ($str =~ /^[01]?\d{1,2}:[0-5]\d:[0-5]\d\.?\d*[ew]$/i) {
		$new_lon = $str;
		last; # Exit while-loop
	    }
	    else {
		print "\nUnknown D:M:S<Quadrant> format (ex: 111:22:33E)";
		print "\nPlease re-enter ...";

		print "\nEnter Longitude (D[DD]:MM:SS[.s](E|W)): ";
		chomp($str = <STDIN>);
		# Relmove leading and trailing whitespace:
		$str =~ s/^\s+//;
		$str =~ s/\s+$//;

		if (length($str) == 0) {
		    last; # Exit while-loop
		}
	    }
	}
    }
    return &deg2dms(&dms2deg($new_lon), "LON"); # Return possibly updated Longitude
}

# SUBROUTINE dms2deg("DD:MM:SS.s(N|S|E|W)")
#    - Convert String DD:MM:SS.s<Quadrant> to Decimal
sub dms2deg {
    my $dstr = $_[0]; # Get first argument

    my ($dg, $mn, $sc, $quad); # Local degrees, minutes, seconds, Quandrant
    my $sign = 1;              # Sign is negative for S or W Quadrants

    if ($dstr =~ /^(\d+):(\d\d):(\d\d\.?\d*)([news])$/i) {
	# Format: DD:MM:SS[.sss]<Quadrant>
	$dg = $1;
	$mn = $2;
	$sc = $3;
	$quad = $4;
    }

    if ($quad =~ /[sw]/i) {
	$sign = -1;  # Switch sign to negative for S or W Quadrant
    }

    if ($mn >= 60) {
	print "\n** WARNING: Minutes value greater than or equal to 60";
	print "\n**          Minutes = $mn\n";
    }

    if ($sc >= 60) {
	print "\n** WARNING: Seconds value greater than or equal to 60";
	print "\n**          Seconds = $sc\n";
    }
    return $sign*($dg + $mn/60 + $sc/3600);
}

# SUBROUTINE deg2dms(<decimal degrees, ("LAT"|"LON"))
#    Convert decimal degress to String DD:MM:SS.s<Quadrant>:
sub deg2dms {
    my $dd = $_[0];      # Get decimal degrees
    my $ll_type = $_[1]; # Either "LAT" or "LON"
    my ($quad);          # Declare Quadrant local

    my $sign = 1;   # Positive Sign for N or E Quadrant

    if ($dd < 0) {
	$sign = -1; # Save sign of input argument
	$dd = -$dd; # Work with positive degrees
    }
    if ($ll_type eq "LAT") {
	if ($sign > 0) {
	    $quad = 'N';
	}
	else {
	    $quad = 'S';
	}
    }
    else {
	if ($sign > 0) {
	    $quad = 'E';
	}
	else {
	    $quad = 'W';
	}
    }

    my $dg = int($dd);         # Break out integer degrees
    my $fmn = 60*($dd - $dg);  # Break out float minutes
    my $mn = int($fmn);        # Break out integer minutes
    my $fsc = 60*($fmn - $mn); # Break out float seconds

    # Check if seconds round-up to >= 60 and propagate round-up
    #  to minutes and degrees
    if ($fsc >= 59.95) {
	$fsc = 0.0;
	$mn++; # Increment number of minutes
    }
    if ($mn >= 60) {
	$mn = 0;
	$dg++; # Increment number of degrees
    }
    if ($ll_type eq "LAT") {
	return sprintf("%02d:%02d:%04.1f%s", $dg, $mn, $fsc, $quad);
    }
    else {
	return sprintf("%03d:%02d:%04.1f%s", $dg, $mn, $fsc, $quad);
    }
}

# SUBROUTINE translate(<Lat1>, <Lon1>, <Lat2>, <Lon2>)
#    - Calculate translation in Latitude/Longitude
sub translate {
    # Get input paramters
    my $lat1 = $_[0];
    my $lon1 = $_[1];
    my $lat2 = $_[2];
    my $lon2 = $_[3];

    my $delta_lat = &dms2deg($lat2) - &dms2deg($lat1);
    my $delta_lon = &dms2deg($lon2) - &dms2deg($lon1);

    my $lat_tr = &deg2dms($delta_lat, "LAT");
    my $lon_tr = &deg2dms($delta_lon, "LON");

    return ($lat_tr, $lon_tr);
}

# End of Perl source code -- Begin script documentation using
#                            "Plain Old Documentation" format

__END__

=pod

=head1 NAME - latlon_trans.pl

=head2 SYNOPSIS

latlon_trans.pl - Calculate the differences in latitude and longitude
between two entered Latitude-Longitude locations.

=head2 DESCRIPTION

The "Common Modeling Environment" (CME) set of graphical analysis
tools includes the "Scenario Editor" and "VESPA" scenario display
utilities. One useful feature in these tools is the ability to
translate a scenario from one set of Latitude-Longitude coordinates to
another set of coordinates by a specified difference in latitude and
longitude. This Perl script can be used to calculate that difference
by entering the current Latitude-Longitude coordinate for a specific
entity in a scenario and, then, entering the desired new
Latitude-Longitude coordinate for that specific entity. The resulting
differences in latitude and longitude for this translation can be
entered in the Translation dialog box for "Scenario Editor" or
"VESPA". As indicated on the dialog box, once a scenario coordinate
set has been translated and saved using "Scenario Editor" or "VESPA",
there is no built-in "undo" command to reset the coordinate
translation.

=head2 USING THIS PERL SCRIPT ON WINDOWS AND LINUX SYSTEMS

This Perl script will execute without any changes on either a Windows
or Linux PC. It will run from a Windows "Command Prompt" command-line
just as it will on a Linux terminal shell command-line. In addition,
for a Boeing-standard Perl installation on a Windows PC, the script
can be executed by double-clicking on the script's icon, which
resembles a yellow pearl. It is designed to run in its own terminal
window by, first, selecting the line number to be changed and, then,
to enter either a latitude or longitude (along with appropriate N, S,
E or W quadrant) for the selected entry. To the extent possible, the
latitude/longitude entries are checked for correctness, and, if any
errors are detected, the user is prompted to reenter the proper
data. After each entry, the appropriate "clear screen" command for the
operating system being used is sent to the terminal window and the
results of the calculation are displayed. Latitude/Longitude
differences are calculated after each new entry until the user enters
"q" or Control-C when prompted to enter a line number (1 through 4)
for the latitude/longitude entries.

=head2 TROUBLESHOOTING PROBLEMS WITH THIS SCRIPT

This Perl script detects whether it is running on a Linux or Windows
PC, and, from this detection, executes the appropriate "clear screen"
command ("cls" for Windows or "clear" for Linux) to keep the
calculation results anchored at the top of the terminal window. If
this script is being run on a different operating system that it does
not recognize, the "clear screen" command defaults to output of six
blank lines followed by the resulting calculation. If the "clear
screen" command is known for this operating system, then user can
temporarily add a "print $os;" statement in the "clear_screen"
subroutine to determine the operating system name in Perl and, then,
add another "elsif" block to assign the appropriate "clear screen"
command for the operating system being used.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F latlon_trans.pl > latlon_trans_doc.txt

 In a Linux environment:
   perldoc -otext -F latlon_trans.pl > latlon_trans_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=latlon_trans.pl --outfile=latlon_trans_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the Boeing-standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as Boeing-standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted. However, when using the
"make_doc.pl" Perl script, these temporary files will be automatically
deleted.)

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
