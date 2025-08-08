#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "evt_filter.pl";
$version = "Version: 11/21/2009";

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
    print "\nOutput file \"$outfilename\" Could not be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution ... \n");
}

print "\nWriting Processed output to file: $outfilename";

$linenr = 0; # Initialize line number count

# Initialize variable to hold multi-line event in one long line:
$evt_line = "";

# Initialize variable to hold multi-line event in original format:
$multi_line = "";

###############################################################
# ADD USER-SPECIFIED HEADER LINE TO PROCESSED CSV OUTPUT FILE #
###############################################################
$outline = "Time,Platform,Event,Sensor,Target,Detect Event";

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
    $event_time = "N/A";
    $event_type = "N/A";
    $platform = "N/A";
    $target = "N/A";
    $sensor = "N/A";
    $det_evt = "N/A";

    if ($evt_line =~ /^(\d*\:\d\d:\d\d\.?\d*)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+Sensor:\s+([\w\-]+)\s+.+\s+Detected:\s+(\d)/) {
	# Time units in HHH:MM:SS.sss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
	$sensor = $5;
	$det_evt = $6;
    }
    elsif ($evt_line =~ /^(\d+\.\d+)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+Sensor:\s+([\w\-]+)\s+Detected:\s+(\d)/) {
	# Time units in SSS.sssss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
	$sensor = $5;
	$det_evt = $6;
    }
    elsif ($evt_line =~ /^(\d*\:\d\d:\d\d\.?\d*)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+Sensor:\s+([\w\-]+)\s+/) {
	# Time units in HHH:MM:SS.sss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
	$sensor = $5;
    }
    elsif ($evt_line =~ /^(\d+\.\d+)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+Sensor:\s+([\w\-]+)\s+/) {
	# Time units in SSS.sssss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
	$sensor = $5;
    }
    elsif ($evt_line =~ /^(\d*\:\d\d:\d\d\.?\d*)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+/) {
	# Time units in HHH:MM:SS.sss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
    }
    elsif ($evt_line =~ /^(\d+\.\d+)\s+([\w\-]+)\s+([\w\-]+)\s+([\w\-]+)\s+/) {
	# Time units in SSS.sssss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
	$target = $4;
    }
    elsif ($evt_line =~ /^(\d*\:\d\d:\d\d\.?\d*)\s+([\w\-]+)\s+([\w\-]+)\s+/) {
	# Time units in HHH:MM:SS.sss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
    }
    elsif ($evt_line =~ /^(\d+\.\d+)\s+([\w\-]+)\s+([\w\-]+)\s+/) {
	# Time units in SSS.sssss
	$event_time = $1;
	$event_type = $2;
	$platform = $3;
    }
    elsif ($evt_line =~ /^(\d*\:\d\d:\d\d\.?\d*)\s+([\w\-]+)\s+/) {
	# Time units in HHH:MM:SS.sss
	$event_time = $1;
	$event_type = $2;
    }
    elsif ($evt_line =~ /^(\d+\.\d+)\s+([\w\-]+)\s+/) {
	# Time units in SSS.sssss
	$event_time = $1;
	$event_type = $2;
    }
    else {
	next; # Skip this entry and get next Event
    }

    # If $det_evt equals one or zero, convert
    #  Detection Event flag to "YES" or "NO":
    if ($det_evt eq "1") {
	$det_evt = "YES";
    }
    elsif ($det_evt eq "0") {
	$det_evt = "NO";
    }

    # Format filtered and processed data for output:
    $outline = "$event_time,$platform,$event_type,$sensor,$target,$det_evt";

    # Write processed data to file:
    &writeln(\*OUTFH, $outfilename, $outline);
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

=head1 NAME - evt_filter.pl

=head2 SYNOPSIS

evt_filter.pl - Filter and parse an AFSIM output Event file and write
filtered output to a .CSV file.

=head2 DESCRIPTION

Filter and parse Event Time (in either Hours-Minutes-Seconds
(HH:MM:SS.sss) or in Seconds-Decimal seconds (SSS.ssss)), Platform
Name, Event Type, Platform sensor (if available), Target Name (if
available) and Detection Status (YES or NO - if available) from an
output Event file and generate a Comma-separated-variable (.CSV) file
for displaying this data in either Microsoft "Excel: (Windows
operating system) or Open Office "oocalc" (Linux operating
system). The root name of the AFSIM Event file is also used as the
root name (adding "_OUT.csv" for this output comma-separated-variable
(CSV) format file.

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This script uses the Perl script "evt_extract_BASE.pl" as a
starting-point for added additional functionality for filtering and
processing AFSIM output Event files. See the AFSIM distribution
"tools" folder for this and other useful Perl scripts.

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

 perl -d evt_filter.pl

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
   perldoc -F evt_filter.pl > evt_filter_doc.txt

 In a Linux environment:
   perldoc -otext -F evt_filter.pl > evt_filter_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=evt_filter.pl --outfile=evt_filter_doc.html

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
