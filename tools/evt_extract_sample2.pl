#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "evt_extract_sample2.pl";
$version = "Version: 07/27/2009";

# Set Debug-leve for script debugging
# Level 0 = No debugging
# Level 1 = Debug output
# Level 2 = Debug output with pauses
$DEBUG = 0;

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

# Create output file name from input file name:
# NOTE: This output file holds extracted data from a
#       (possibly) multi-line AFSIM Event
$rootfilename = $infilename;
$rootfilename =~ s/\.evt$//; # Remove ".evt" extension

$outfilename = $rootfilename . "_OUT.csv"; # Add "_OUT.csv" to name

if (not open(OUTFH, '>'.$outfilename)) {
    print "\nOutput file \"$outfilename\" Could not be opened";
    print "\nPress ENTER key to Abort: ";
    <STDIN>;
    die("\nTerminating execution ... \n");
}

print "\nWriting Processed output to file:           $outfilename";

# Create anothr output file to hold copy of those lines
#  selected for processing, but maintain multi-line format
#  for readability:
$filfilename = $rootfilename . "_FILTERED.evt";

if (not open(FILFH, '>'.$filfilename)) {
    print "\nOutput file \"$filfilename\" Could not be opened";
    print "\nPress ENTER key to Abort: ";
    <STDIN>;
    die("\nTerminating execution ... \n");
}

print "\nWriting Filtered multi-line output to file: $filfilename";

$linenr = 0; # Initialize line number count

# Initialize variable to hold multi-line event in one long line:
$evt_line = "";

# Initialize variable to hold multi-line event in original format:
$multi_line = "";

###############################################################
# ADD USER-SPECIFIED HEADER LINE TO PROCESSED CSV OUTPUT FILE #
###############################################################
$outline = "Time,Sensor ID,Target ID";
$outline .= ",Sensor Type,Track ID";
$outline .= ",Range-to-Target,Range Units";

# Add column in processed output to list range in Nautical Miles
$outline .= ",Range (N Mi)";

# Write header line to file:
&writeln(\*OUTFH, $outfilename, $outline);
###############################################################

while (1) {
    # Get next multi-line event:
    if (&get_event()) {
	last; # Last event encountered
    }

    if ($DEBUG >= 1) {
	print "\n*DEBUG - main(): Line number = $linenr";
	# NOTE: Added "|" char at beginning and end of
	#       Debug output to confirm no leading or
	#       trailing whitespace characters exist
	print "\n         Event line = |$evt_line|";
	&pause(2); # Pause for Debug Level >= 2
    }

    ######################################################
    # ENTER REGULAR EXPRESSION PATTERN MATCHES AND OTHER #
    #  FILTERING/PROCESSING CODE HERE                    #
    ######################################################
    if ($evt_line =~ /^(.+)\sSENSOR_TRACK_INITIATED\s(.+)\s(.+)\sSensor\:\s(.+)\sTrackId\:\s([\w\-\.]+)\s.+\sTruth\:\sRange\:\s(\d+\.?\d*)\s(\w)\s/) {
	$event_time = $1;
	$sensor_id = $2;
	$tgt_id = $3;
	$sensor_type = $4;
	$track_id = $5;
	$range = $6;
	$range_units = $7;

	# Add filter to only output EW and
	#  Acquisition Radar tracks:
	if ($sensor_type !~ /ew_radar/i and $sensor_type !~ /acq_radar/i) {
	    next;  # Skip this Event and get next Event
	}
    }
    else {
	next; # Skip this Event and get next Event
    }

    # Add additional column in processed output to provide
    #  range in Nautical Miles:
    $range_nmi = "Unknown Range Units";
    if ($range_units eq "m") {
	$range_nmi = sprintf("%.3f", $range/1852);
    }

    # Format filtered and processed data for output:
    $outline = "$event_time,$sensor_id,$tgt_id";
    $outline .= ",$sensor_type,$track_id";
    $outline .= ",$range,$range_units,$range_nmi";
    ######################################################

    # Write processed data to file:
    &writeln(\*OUTFH, $outfilename, $outline);

    # Write selected data to another file in original
    #  AFSIM Event output file format:
    &writeln(\*FILFH, $filfilename, $multi_line);
}

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
# SUBROUTINE get_event() - Get next multi-line Event
sub get_event {
    # Declare local variables:
    my ($inln);

    # Initialize Global variable to hold Event as one long line:
    $evt_line = "";

    # Initialize Global variable to hold Event in original format:
    $multi_line = "";

    while (<INFH>) {
	$linenr++; # Increment line count (for debugging)
	chomp($inln = $_); # Get next input line

	# Build Event line in original format:
	if (length($multi_line) == 0) {
	    $multi_line = "$inln\n";
	}
	else {
	    $multi_line .= "$inln\n"; # Add continuation lines
	}

	# Remove any leading and trailing whitespace characters:
	$inln =~ s/^\s+//;
	$inln =~ s/\s+$//;

	if ($DEBUG >= 1) {
	    print "\n*DEBUG - get_event(): Line Number = $linenr";
	    # NOTE: Added "|" char at beginning and end of
	    #       Debug output to confirm no leading or
	    #       trailing whitespace characters exist
	    print "\n         Processed line = |$inln|";
	    &pause(2); # Pause for Debug Level >= 2
	}

	# Check if this line is a continuation line ending in "\":
	if ($inln =~ s/\\$//) {
	    # NOTE: Remove trailing "\" charater in above "if" statement
	    # Continue merging continuation lines
	    $evt_line .= $inln;
	}
	else {
	    $evt_line .= $inln;
	    chomp($multi_line); # Remove last Newline character
	    return 0; # Continue parsing lines
	}
    }
    chomp($multi_line); # Remove last Newline character
    return 1; # End of Event file encountered
}


# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - evt_extract_sample2.pl

=head2 SYNOPSIS

evt_extract_sample2.pl - Filter and parse "LOCAL_TRACK_INITIATED"
events along with selected Event data.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This script uses the Perl script "evt_extract_sample1.pl" (which was
derived from "evt_extract_BASE.pl") as a starting-point for added
additional functionality for filtering and processing AFSIM output
Event files. See the AFSIM distribution "tools" folder for this and
other useful Perl scripts.

The initial baseline Perl script, evt_extract_BASE.pl, provides only
the following features:

=over

=item *

Runs without requiring any changes in both Windows and Linux operating
system environments, including the interactive mode on Windows PCs:
double-click on the script's icon to execute it in its own Command
Window.

=item *

Since there is no code to filter specific Events in the baseline
script, the output files contain all events in the input file.

=item *

Two output files are created with the Event data. The root of the
input file name (ex: the root of file "abcde.evt" is "abcde") is used
as the base for creating the output file names.

=item *

For every AFSIM multi-line Event, output is written to the file
<root_file_name>_FILTERED.evt with the same format that exists in the
input Event file. (For the baseline script, the contents of this file
exactly matches the input Event file.

=item *

For every AFSIM multi-line Event, output of each event is written as
one long text string to the file: <root_file_name>_OUT.csv. A comma is
used to separate each word in the original text string event
line. (Using a ".csv" file extension makes it easy to process this
file using Microsoft "Excel" (Windows) or Open Office "oocalc"
(Linux).

=back

The "evt_extract_sample1.pl" Perl script provides the following
additional features:

=over

=item *

Added top row header line to output file <root_file_name>_OUT.csv that
provides a label for each column of data.

=item *

Added code to filter only "SENSOR_TRACK_INITIATED" Event types, and,
from this event, extract only Time, Sensor ID, Target ID, Sensor Type,
Track ID, Range and Range Units. Only this data is written to the
".csv" output file and only this Event type is written to the ".evt"
output file.

=back

This Perl script provides the following additional features:

=over

=item *

In the header line for the "<root_file_name>_OUT.csv" output file, an
additional column is added to the first row labeled "Range (N Mi)".

=item *

Added filter to include only the Sensor Types of "ew_radar" and
"acq_radar" (case insensitive) for processing and output.

=item *

In section of code where data is extracted, added code to calculate
Range in Nautical Miles from the Event file's default Range in Meters. (A
further improvement could be to add calculations for Ranges in units
other than Meters. In this sample, if the Range Units is not "m", then
the value for Range in Nautical Miles is "Unknown Range Units".)

=back

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

 perl -d evt_extract_sample2.pl

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
   perldoc -F evt_extract_sample2.pl > evt_extract_sample2_doc.txt

 In a Linux environment:
   perldoc -otext -F evt_extract_sample2.pl > evt_extract_sample2_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=evt_extract_sample2.pl --outfile=evt_extract_sample2_doc.html

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
