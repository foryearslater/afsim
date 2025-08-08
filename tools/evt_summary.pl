#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Set Debug-leve for script debugging
# Level 0 = No debugging
# Level 1 = Debug output
# Level 2 = Debug output with pauses
$DEBUG = 0;

$pgm_name = "evt_summary.pl";
$version = "Version: 11/20/2009";

print "\n$pgm_name $version\n";

# Determine Operating System and mode (Interactive (Windows) or
#  Command-line)
# NOTE: If Windows O/S not detected assume that system is "Linux"
$os = $^O; # Get operating system (Windows = "MsWin..")

$cmd_line_mode = 1; # Default is Linux-style Command-line mode

# Get number of command-line arguments (if any):
$nr_args = scalar(@ARGV);

if ($nr_args == 0) {
    # No arguments entered on Command-line (or Interactive in Windows).
    # Prompt for input file name:
    print "\nEnter input AFSIM Event file: ";
    chomp($infilename = <STDIN>); # Remove trailing Newline character

    if ($os =~ /^mswin/i) {
	$cmd_line_mode = 0; # Assume running Windows Interactive mode
    }
}
elsif ($nr_args == 1) {
    # Assume running Command-line mode (either Windows or Linux):
    # Get input file name from first argument
    $infilename = $ARGV[0];
}
else {
    print "\n\n*ERROR: Expecting only one argument that should be";
    print "\n          an AFSIM Event file for input\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution ... \n");
}

if (not open(INFH, $infilename)) {
    print "\nInput file \"$infilename\" Could not be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution ... \n");
}

$linenr = 0; # Initialize line number count

# Initialize variable to hold multi-line event in one long line:
$csv_line = "";

# Initialize array to hold extracted CSV parameters:
@evt_array = ();

# Initialize Event data hash arrays:
%wpn_fired_evts = ();
%wpn_hit_evts = ();
%wpn_missed_evts = ();
%platform_killed_evts = ();
%sensor_detection_attempt_evts = ();

# Initialize cummulative track hash arrays:
%track_start_times = ();
%track_secs = ();

# Initialize variable to save current sim time (seconds):
$sim_time_sec = 0;

while (<INFH>) {
    $linenr++; # Increment Line count
    chomp($inline = $_); # Get line and remove Newline char

    if ($linenr == 1) {
	next; # Skip header line
    }

    # Parse CSV event line and store data in @evt_array:
    &get_event($inline);

    if ($DEBUG >= 1) {
	print "\n*DEBUG - main(): Line number = $linenr";
	# NOTE: Added "|" char at beginning and end of
	#       Debug output to confirm no leading or
	#       trailing whitespace characters exist
	print "\n         CSV line = |$csv_line|";
	print "\n         ----------------------";
	for ($i = 0; $i < scalar(@evt_array); $i++) {
	    print "\n           \$evt_array[$i] = $evt_array[$i]";
	}
	print "\n         ----------------------";
	&pause(2); # Pause for Debug Level >= 2
    }

    ######################################################
    # ENTER REGULAR EXPRESSION PATTERN MATCHES AND OTHER #
    #  FILTERING/PROCESSING CODE HERE                    #
    ######################################################

    # List of parameters being processed:
    $event_time = "N/A";
    $platform = "N/A";
    $event_type = "N/A";
    $sensor = "N/A";
    $target = "N/A";
    $det_evt = "N/A";

    # Store available data in parameters
    if (scalar(@evt_array) == 6) {
	$event_time = $evt_array[0];
	$platform = $evt_array[1];
	$event_type = $evt_array[2];
	$sensor = $evt_array[3];
	$target = $evt_array[4];
	$det_evt = $evt_array[5];
    }
    elsif (scalar(@evt_array) == 5) {
	$event_time = $evt_array[0];
	$platform = $evt_array[1];
	$event_type = $evt_array[2];
	$sensor = $evt_array[3];
	$target = $evt_array[4];
    }
    elsif (scalar(@evt_array) == 4) {
	$event_time = $evt_array[0];
	$platform = $evt_array[1];
	$event_type = $evt_array[2];
	$sensor = $evt_array[3];
    }
    elsif (scalar(@evt_array) == 3) {
	$event_time = $evt_array[0];
	$platform = $evt_array[1];
	$event_type = $evt_array[2];
    }
    elsif (scalar(@evt_array) == 2) {
	$event_time = $evt_array[0];
	$platform = $evt_array[1];
    }
    else {
	print "\n*WARNING: \@evt_array only contains an Event Time\n";
	print "\n            Line number = $linenr";
	print "\n---------------------------------------------------";
	next; # Skip this data line
    }

    if ($event_time =~ /^\d+\:?[0-5]\d\:[0-5]\d\.\d$/) {
	# Convert H:M:S.s or M:S.s to Seconds:
	$sim_time_sec = &hms2sec($event_time);
    }
    elsif ($event_time =~ /^\d+\.\d+$/) {
	$sim_time_sec = $event_time; # Time already in Seconds
    }
    else {
	print "\n*ERROR: Event time format NOT known";
	print "\n        Event time = $event_time\n";

	if (not $cmd_line_mode) {
	    print "\nPress ENTER key to Abort: ";
	    <STDIN>;
	}
	die("\nTerminating Execution\n");
    }

    if ($event_type =~ /^weapon_fired$/i) {
	&proc_wpn_fired_evt();
    }
    elsif ($event_type =~ /^weapon_hit$/i) {
	&proc_wpn_hit_evt();
    }
    elsif ($event_type =~ /^weapon_missed$/i) {
	&proc_wpn_missed_evt();
    }
    elsif ($event_type =~ /^platform_killed$/i) {
	&proc_platform_killed_evt();
    }
    elsif ($event_type =~ /^sensor_detection_attempt$/i) {
	&proc_sensor_detection_attempt_evt();
    }
    elsif (($event_type =~ /^sensor_track_initiated$/i) or
	   ($event_type =~ /^sensor_track_dropped$/i)     ){
	&proc_track_lengths();
    }
    else {
	next; # Process next event line
    }
}

# Create output file name from input file name:
# NOTE: This output file holds extracted data from a
#       (possibly) multi-line AFSIM Event
$rootfilename = $infilename;
$rootfilename =~ s/_OUT\.csv$//i; # Remove original "_OUT.csv" extension

$outfilename = $rootfilename . "_SUMMARY.csv"; # Add "_SUMMARY.csv" to name

if (not open(OUTFH, '>'.$outfilename)) {
    print "\nOutput file \"$outfilename\" Could NOT be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution ... \n");
}

print "\nWriting Processed output to file: $outfilename";

###############################################################
# ADD USER-SPECIFIED HEADER LINE TO PROCESSED CSV OUTPUT FILE #
###############################################################
$outline = "EVENT COUNT SUMMARY";

# Write header line to file:
&writeln(\*OUTFH, $outfilename, $outline);
###############################################################

# Write WEAPON_FIRED Event counts:
&write_counts("WEAPON_FIRED Events",\%wpn_fired_evts);
&write_counts("WEAPON_HIT Events",\%wpn_hit_evts);
&write_counts("WEAPON_MISSED Events",\%wpn_missed_evts);
&write_counts("PLATFORM_KILLED Events",\%platform_killed_evts);
&write_counts("SENSOR_DETECTION_ATTEMPT",
	      \%sensor_detection_attempt_evts);

&write_track_lengths(); # Output cummulative track lengths

print "\nDONE\n";

if (not $cmd_line_mode and $os =~ /^mswin/i) {
    # If not in Command-line mode using Windows environment ,do not
    #  close Interactive window until user presses ENTER key:
    print "\nPress ENTER key to Quit: ";
    <STDIN>;
}

# SUBROUTINE writeln(\*<FILEHANDLE>, <file name>, <string>)
#            - Write a line to output file
sub writeln {
    my $fh = $_[0];        # Get file handle
    my $file_name = $_[1]; # Get output file name
    my $ln = $_[2];        # Get string to write to output file
    print $fh "$ln\n";     # Write line to specified output file
    if ($DEBUG >= 1) {
	print "\n* DEBUG - writeln() FILE: $file_name = $ln";
	&pause(2); # Pause for Debug Level >= 2
    }
}

# SUBROUTINE pause(<DEBUG Level>) - Pause for DEBUG level >= <DEBUG Level>
sub pause {
    if ($DEBUG >= $_[0]) {
	print "\nPress ENTER key to continue ... ";
	<STDIN>;
    }
}

# SUBROUTINE get_event() - Convert Event line into array of data
sub get_event {
    # Get input parameter:
    my $inln = $_[0];

    # Clean-up input line:
    $inln =~ s/^\s+//; # Remove any leading whitespace
    $inln =~ s/\s+$//; # Remove any trailing whitespace
    $inln =~ s/\,+$//; # Remove any trailing commas

    # Load Global array with Event parameters:
    @evt_array = split(/\,/, $inln);
}

# SUBROUTINE proc_wpn_fired_evt() - Collect WEAPON_FIRED event data
sub proc_wpn_fired_evt {
    # Create hash key for this event:
    my $key = $platform . '#' . $event_type . '#' . $target;

    # Accumulate counts on these events:
    if (exists $wpn_fired_evts{$key}) {
	$wpn_fired_evts{$key} += 1; # Add one to count
    }
    else {
	$wpn_fired_evts{$key} = 1; # Add one to count
    }
}

# SUBROUTINE proc_wpn_hit_evt() - Collect WEAPON_HIT event data
sub proc_wpn_hit_evt {
    # Create hash key for this event:
    my $key = $platform . '#' . $event_type . '#' . $target;

    # Accumulate counts on these events:
    if (exists $wpn_hit_evts{$key}) {
	$wpn_hit_evts{$key} += 1; # Add one to count
    }
    else {
	$wpn_hit_evts{$key} = 1; # Add one to count
    }
}

# SUBROUTINE proc_wpn_missed_evt() - Collect WEAPON_MISSED event data
sub proc_wpn_missed_evt {
    # Create hash key for this event:
    my $key = $platform . '#' . $event_type . '#' . $target;

    # Accumulate counts on these events:
    if (exists $wpn_missed_evts{$key}) {
	$wpn_missed_evts{$key} += 1; # Add one to count
    }
    else {
	$wpn_missed_evts{$key} = 1; # Add one to count
    }
}

# SUBROUTINE proc_platform_killed_evt() - Collect PLATFORM_KILLED event data
sub proc_platform_killed_evt {
    # Create hash key for this event:
    my $key = $platform . '#' . $event_type;

    # Accumulate counts on these events:
    if (exists $platform_killed_evts{$key}) {
	$platform_killed_evts{$key} += 1; # Add one to count
    }
    else {
	$platform_killed_evts{$key} = 1; # Add one to count
    }
}

# SUBROUTINE proc_sensor_detection_attempt_evt() - Collect
#  successful SENSOR_DETECTION_ATTEMPT events
sub proc_sensor_detection_attempt_evt {

    # Create hash key for this event:
    my $key = $platform . '#' . $event_type . '#';
    $key .= $sensor . '#' . $target . '#' . $det_evt;

    # Accumulate counts on these events:
    if (exists $sensor_detection_attempt_evts{$key}) {
	$sensor_detection_attempt_evts{$key} += 1; # Add one to count
    }
    else {
	$sensor_detection_attempt_evts{$key} = 1; # Add one to count
    }
}

# SUBROUTINE proc_track_lengths() - Collect track length data
sub proc_track_lengths {
    # Declare local variables:
    my ($delta_time);

    # Create hash key for this event:
    my $key = $platform . '#' . $sensor . '#' . $target;

    if ($event_type =~ /^sensor_track_initiated$/i) {
	if (exists $track_start_times{$key}) {
	    print "\n*ERROR - SENSOR_TRACK_INITIATED Event already exists";
	    print "\n         for Key = $key";
	    print "\n         Line number = $linenr\n";

	    if (not $cmd_line_mode) {
		print "\nPress ENTER key to Abort: ";
		<STDIN>;
	    }
	    die("\nTerminating execution ... \n");
	}
	else {
	    $track_start_times{$key} = $sim_time_sec;
	}
    }
    elsif ($event_type =~ /^sensor_track_dropped$/i) {
	if (not exists $track_start_times{$key}) {
	    print "\n*ERROR - SENSOR_TRACK_INITIATED Event does NOT exist";
	    print "\n         for Key = $key";
	    print "\n         Line number = $linenr\n";

	    if (not $cmd_line_mode) {
		print "\nPress ENTER key to Abort: ";
		<STDIN>;
	    }
	    die("\nTerminating execution ... \n");
	}
	else {
	    # Calculate additional track time for this key:
	    $delta_time = $sim_time_sec - $track_start_times{$key};

	    # Remove the Track-start time for this key:
	    delete $track_start_times{$key};

	    if (not exists $track_secs{$key}) {
		# Start accumulating track time for this key:
		$track_secs{$key} = $delta_time;
	    }
	    else {
		# Add additional track time for this key:
		$track_secs{$key} += $delta_time;
	    }
	}
    }
    else {
	print "\n*ERROR - Unknown Event type: $event_type";
	print "\n         Line number = $linenr\n";

	if (not $cmd_line_mode) {
	    print "\nPress ENTER key to Abort: ";
	    <STDIN>;
	}
	die("\nTerminating execution ... \n");
    }
}

# SUBROUTINE write_counts("<Event Name>", \%<Event Hash>) - Write Event counts
sub write_counts {
    my $evt_type = $_[0]; # Get Event type
    my %evt = %{$_[1]};   # Get Event hash array

    my ($key); # Declare local variable
    my @keys = sort(keys(%evt)); # Get sorted hash keys

    # Write Header line:
    $outline = "\n$evt_type Events,Count";
    &writeln(\*OUTFH, $outfilename, $outline);

    if (scalar(@keys) == 0) {
	$outline = "NONE";
	&writeln(\*OUTFH, $outfilename, $outline);
    }
    else {
	foreach $key (@keys) {
	    $outline = $key . ',' . $evt{$key};
	    &writeln(\*OUTFH, $outfilename, $outline);
	}
    }
}

# SUBROUTINE write_track_lengths() - Write Track lengths
sub write_track_lengths {

    # Write Header line:
    $outline = "\nPLATFORM#SENSOR#TARGET,Cumm Track Time (sec)";
    &writeln(\*OUTFH, $outfilename, $outline);

    my ($key); # Declare local variable
    my @keys = sort(keys(%track_secs)); # Get sorted hash keys

    if (scalar(@keys) == 0) {
	$outline = "NONE";
	&writeln(\*OUTFH, $outfilename, $outline);
    }
    else {
	foreach $key (@keys) {
	    $outline = $key . ',' . $track_secs{$key};
	    &writeln(\*OUTFH, $outfilename, $outline);
	}
    }
}

# String HH:MM:SS.ss or MM:SS.ss to seconds
sub hms2sec {
    my $hms_str = $_[0]; # Get first argument
    my ($hr, $mn, $sc); # Local variables: hours, minutes, seconds

    if ($hms_str =~ /^(\d+):(\d\d):(\d+\.?\d*)$/) {
	# Format: HH:MM:SS[.sss]
	$hr = $1;
	$mn = $2;
	$sc = $3;
    }
    elsif ($hms_str =~ /^(\d+):(\d+\.?\d*)$/) {
	# Format: MM:SS[.sss]
	$hr = 0;
	$mn = $1;
	$sc = $2;
    }
    else {
	print "\n** ERROR: Input H:M:S.s or M:S.s string NOT found";
	print "\n         Line number = $linenr\n";

	if (not $cmd_line_mode) {
	    print "\nPress ENTER key to Abort: ";
	    <STDIN>;
	}
	die("\nTerminating execution\n");
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

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - evt_summary.pl

=head2 SYNOPSIS

evt_summary.pl - Create summary statistics from EVENTS created by
the Perl script evt_filter.pl.

=head2 DESCRIPTION

This script processes the CSV file created by the "evt_filter.pl" file
to produce summary counts by Platform/Target/Sensor/Weapon (as
appropriate) of:

 WEAPON_FIRED Events
 WEAPON_HIT Events
 WEAPON_MISSED Events
 PLATFORM_KILLED Events
 SENSOR_DETECTION_ATTEMPT Events (Detected = YES, not detected = NO)

Also, for each PLATFORM-Sensor-Target pairing, a summary of
cummulative track time is listed.

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

See the AFSIM distribution "tools" folder for this and other useful
Perl scripts.

=head2 TROUBLESHOOTING PROBLEMS WITH THIS SCRIPT

See the comments in this script's source code to help understand what
each block of code accomplishes. Setting the $DEBUG variable to a
number higher than zero will cause intermediate processing to be
displayed in the terminal window. If $DEBUG is set to one,
intermediate processing will stream up the terminal window until the
script terminates normally or if a fatal execution error is
encountered. Debug output includes the line number of the input file
to help in determining the cause of any errors. For more in-depth
inspection of script processing, run the script on the command-line
using the Perl debugger:

 perl -d evt_summary.pl

For more information on the Perl debugger, access the Windows
ActivePerl Documentation, Perl Core Document "perldebug", or, use the
following command in a Linux (or Windows) terminal window:

 perldoc perldebug

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F evt_summary.pl > evt_summary_doc.txt

 In a Linux environment:
   perldoc -otext -F evt_summary.pl > evt_summary_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=evt_summary.pl --outfile=evt_summary_doc.html

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
