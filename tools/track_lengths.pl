#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# File name: track_lengths.pl
# Version: 07/29/2009

# - This script calculates track times for individual sensors.
# - Unless the $no_updates_flag is set to one (TRUE),
#    the track must be updated at least once for this script
#    to recognize it as a track. The following AFSIM events
#    must be enabled in an AFSIM model run for this script
#    to operate correctly:
#    -- SENSOR_TRACK_INITIATED
#    -- SENSOR_TRACK_UPDATED (Unless $no_updates_flag = 1)
#    -- SENSOR_TRACK_DROPPED
#    -- SENSOR_TURNED_OFF (Unless no events when sensors turn off)

# - If the $no_updates_flag is set to one (TRUE), then the
#    SENSOR_TRACK_UPDATED Event does not need to be enabled.
#    By not enabling the SENSOR_TRACK_UPDATED Event, the resulting
#    size of any AFSIM output Event file will often be significantly
#    smaller than the same AFSIM model run with output of the
#    SENSOR_TRACK_UPDATED Event enabled. However, without including
#    SENSOR_TRACK_UPDATED events, it is possible that some short
#    track time lengths will be listed that result from
#    SENSOR_TRACK_DROPPED Events occuring for a given Track ID
#    immediately after a SENSOR_TRACK_INITIATED Event for that
#    Track ID. While this situation may meet the technical
#    definition for "a track", it is questionable that any
#    realistic actions could be taken on this type of "track".

# Set the $no_updates_flag to zero (FALSE) if there are
#  SENSOR_TRACK_UPDATED Events in the AFSIM output Event file.
#  If there are no SENSOR_TRACK_UPDATED Events in the Event file,
#  set this variable to one (TRUE):
$no_updates_flag = 0;

# Initialize this error-checking flag to zero (FALSE). After
#  processing an Event file, a warning message will be
#  generated if $no_updates_flag is set to zero (SENSOR_TRACK_UPDATED
#  events should be in the Event file) and if no SENSOR_TRACK_UPDATED
#  events are encountered.
$sensor_track_updated_flag = 0;

# Initialize this error-checking flag to zero (FALSE). After
#  processing an Event file, a warning message will be
#  generated if no track length times were output.
$track_output_flag = 0;

# Provide option to enter input Event file from
#  command-line or interactively

# Determine Operating System and mode (Interactive (Windows) or
#  Command-line)
# NOTE: If Windows O/S not detected assume that system is "Linux"
$os = $^O; # Get operating system (Windows = "MsWin..")

$cmd_line_mode = 1; # Default is Linux-style Command-line mode

# Get number of command-line arguments (if any):
$nr_args = scalar(@ARGV);

if ($nr_args == 0) {
    # Prompt user for AFSIM Event file name:
    print "\nEnter AFSIM Event file name for input: ";
    chomp($infilename = <STDIN>); # and remove trailing Newline char

    if ($os =~ /^mswin/i) {
	$cmd_line_mode = 0; # Assume running Windows Interactive mode
    }
}
elsif ($nr_args == 1) {
    # Get AFSIM Event file name from first script argument:
    $infilename = $ARGV[0];
}
else {
    print "\n*ERROR - Too many script arguments";
    print "\n\nUsage: track_lengths.pl <AFSIM Event file>";
    print "\n\n        or only enter \"track_lengths.pl\" to be";
    print "\n          prompted for name of input file\n\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    exit;
}

$rootname = $infilename;  # Create root file name from input file
$rootname =~ s/\.evt$//i; # Remove ".evt" extension

# Open Event file for input:
if (! open(INFH, $infilename)) {
    print "\n*ERROR: File \"$infilename\" could not be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution\n");
}

$max_stop_time = 0; # Init max time encountered (last event time)

%target = (); # Init hash to save target ID accessed by Track ID
%sensor = (); # Init hash to save sensor ID accessed by Track ID

# Init hash for track-start times recorded when SENSOR_TRACK_UPDATED
#  events are included in the Event file and are being processed.
#  The start time of the SENSOR_TRACK_INITIATED event is saved
#  by Track ID in this hash array. When a SENSOR_TRACK_UPDATED
#  event is encountered for this Track ID, the appropriate track-start
#  time is copied to the %valid_track_start hash array
%possible_track_start = ();

# Init hash for saving declared-valid track-start times. This
#  time will be from the %possible_track_start hash if
#  SENSOR_TRACK_UPDATED events are being processed. Otherwise,
#  when no SENSOR_TRACK_UPDATED events are in the Event file
#  and/or the $no_updates_flag variable is set to one, these
#  track-start times will be read directly from SENSOR_TRACK_INITIATED
#  events.
%valid_track_start = ();

# Init hash for saving track-stop times (accesed by
#  Track ID)
%valid_track_stop = ();

$linenr = 0; # Initialize line number counter (for debugging)
# Read lines from input file:
while (<INFH>) {
    $linenr++; # Increment line number
    chomp($inline = $_); # Save input line in $inline

    # Break out words from input line into @data_line array:
    @data_line = split(/\s+/, $inline);

    $event_time = $data_line[0]; # Get Time for this event

    if ($event_time =~ /^(\d*)\:?(\d\d)\:(\d\d\.?\d*)$/) {
	# If required, convert MM:SS.sss or HHH:MM:SS.sss to seconds
	$time_sec = (3600*$1) + (60*$2) + $3;
    }
    elsif ($event_time =~ /^\d+\.?\d*/) {
	$time_sec = $event_time; # Time format already in seconds
    }
    else {
	next; # Other data lines can be skipped
    }

    # Keep track of last Event time encountered in the Event file
    #  to use as stop-times for any tracks that continue until
    #  the end-of-simulation:
    if ($max_stop_time < $time_sec) {
	$max_stop_time = $time_sec; # Update max Event time
    }

    # Process specific sensor-track-related events

    if ($inline =~ /SENSOR_TRACK_INITIATED/) {
	# Process SENSOR_TRACK_INITIATED Events

	$track_id = $data_line[7]; # Track IDs are keys in hash arrays
	$target{$track_id} = $data_line[3];
	$sensor{$track_id} = $data_line[5];

	if ($no_updates_flag) {
	    # Assume this is actual track-start (required
	    #  if no SENSOR_TRACk_UPDATED Events in file):
	    $valid_track_start{$track_id} = $time_sec;
	}
	else {
	    # Record this as possible track-start time, but
	    #  do not consider this a valid track until
	    #  a SENSOR_TRACK_UPDATED event encountered for
	    #  this Track ID:
	    $possible_track_start{$track_id} = $time_sec;
	}
    }
    elsif ($inline =~ /SENSOR_TRACK_UPDATED/) {
	# Process SENSOR_TRACK_UPDATED Events

	# Indicate taht at least one SENSOR_TRACK_UPDATED Event
	#  was encountered in this file:
	$sensor_track_updated_flag = 1; # Set flag to "TRUE"

	$track_id = $data_line[7];
	if (not defined $valid_track_start{$track_id}) {
	    # Possible track-start now confirmed. Record
	    #  this time as a valid track-start:
	    $valid_track_start{$track_id} =
		$possible_track_start{$track_id};
	}
    }
    elsif ($inline =~ /SENSOR_TRACK_DROPPED/) {
	# Process SENSOR_TRACK_DROPPED Events

	$track_id = $data_line[7];
	if (defined $valid_track_start{$track_id}) {
	    # Record the track-stop time for this valid
	    #  Track ID:
	    $valid_track_stop{$track_id} = $time_sec;
	}
    }
    elsif ($inline =~ /SENSOR_TURNED_OFF/) {
	# Process SENSOR_TURNED_OFF Events

	$sensor_name = $data_line[2]; # Get sensor name

	# NOTE: For any existing Track IDs by this sensor
	#       this Event will cause track-stop times
	#       to be created:
	foreach $track_id (keys %valid_track_start) {
	    if(($track_id =~ /^$sensor_name/)           and
	       (not defined $valid_track_stop{$track_id})  ) {
		# Create track-stop times for this sensor's tracks:
		$valid_track_stop{$track_id} = $time_sec;
	    }
	}
    }
}
close INFH; # Close input file

# Create output file name from root-name of input file:
$outfilename = $rootname . "_tracks.csv"; # Add "_tracks.csv"

print "\nWriting output to file: $outfilename";

# Open data file for output:
if (! open(OUTFH, '>'.$outfilename)) {
    print "\n*ERROR: File \"$outfilename\" could not be opened";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution\n");
}

# Write output file header line:
$outline = "Track ID,Sensor,Target,Track-start Time (sec)";
$outline .= ",Track-stop Time (sec),Track Length (sec)\n";
print OUTFH $outline;

# Get sorted list of Track IDs for each valid track-start time:
@track_ids = sort(keys %valid_track_start);

foreach $id (@track_ids) {
    if (not defined $valid_track_stop{$id}) {
	# If a track-stop time does not exist for this Track ID,
	#  then track continued through end-of-simulation. Set
	#  track-stop time to end-of-simulation time for this
	#  Track ID:
	$valid_track_stop{$id} = $max_stop_time;
    }
    # Calculate track length (seconds):
    $track_length = $valid_track_stop{$id} - $valid_track_start{$id};

    # Round track length times to nearest tenths of seconds:
    $track_len_out = sprintf("%.1f", $track_length);

    # Create output lines:
    $outline = "$id,$sensor{$id},$target{$id},$valid_track_start{$id}";
    $outline .= ",$valid_track_stop{$id},$track_len_out\n";
    print OUTFH $outline;

    # Record that there was at least one line of track-length output:
    $track_output_flag = 1; # Set variable to "TRUE"
}

close OUTFH; # Close output file

# Generate any applicable warning messages:
if (not $track_output_flag) {
    print "\n\n*WARNING: No track-length times were encountered in this Event file";

    if ((not $no_updates_flag)        and
	(not $sensor_track_updated_flag)  ) {
	print "\n- This script\'s variable \"\$no_updates_flag\" is set to zero (FALSE)";
	print "\n   and no SENSOR_TRACk_UPDATED Events were encountered in this file.";
	print "\n- To process AFSIM Event files that do not contain SENSOR_TRACK_UPDATED";
	print "\n   Events, set the \"\$no_updates_flag\" variable to one (TRUE).";
	print "\n- See comments in this script and its documentation for more details.";
    }
    print "\n\n";
}

print "\n----";
print "\nDONE";
print "\n----\n";

if (not $cmd_line_mode) {
    print "\nPress ENTER key to Quit: ";
    <STDIN>;
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - track_lengths.pl

=head2 SYNOPSIS

track_lengths.pl - Calculate sensor track length times for each target
in an AFSIM output Event file.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This script calculates track times for individual sensors.  Unless the
$no_updates_flag is set to one (TRUE), the track must be updated at
least once for this script to recognize it as a track. The following
AFSIM events must be enabled in an AFSIM model run for this script to
operate correctly:

=over

=item *

SENSOR_TRACK_INITIATED

=item *

SENSOR_TRACK_UPDATED (Unless $no_updates_flag = 1)

=item *

SENSOR_TRACK_DROPPED

=item *

SENSOR_TURNED_OFF (Unless no events when sensors turn off)

=back

IMPORTANT NOTE: The maximum time that a track can be maintained is
determined by the last event time encountered in the Event
file. However, depending on the event types that have been enabled in
an AFSIM model run, the last event encountered may vary. Therefore, to
insure consistent comparisons in an analysis, it may be necessary to
insure that the last SENSOR_TRACK_DROPPED event (or all
SENSOR_TURNED_OFF events have occurred) prior to the end of a model
run. As an alternative, the actual model's end-of-simulation time Perl
variable "$max_stop_time" could be coded as a constant in a copy of
this Perl script.

See the comments embedded in this script for more details.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F track_lengths.pl > track_lengths_doc.txt

 In a Linux environment:
   perldoc -otext -F track_lengths.pl > track_lengths_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=track_lengths.pl --outfile=track_lengths_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted.)

=head2 AUTHORS

 Nicholas Serdar, Joseph Ferrara, Thomas Irish
 Phantom Works
 Integrated Defense Systems

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
