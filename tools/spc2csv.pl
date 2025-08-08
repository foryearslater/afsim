#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Set DEBUG level:
#   0 = No Debug
#   1 = Debug with no pauses
#   2 = Debug with pauses
$DEBUG = 0;

$program_name = "spc2csv.pl";
$version = "Ver 07/29/2009";
$usage = "$program_name <input_file.xxx>";
$usage .= "\n or only enter program name to be";
$usage .= "\n prompted for input file name";
$usage .= "\n- Output to file name will be: <input_file.xxx.csv>";

print "\nPerl script: $program_name";
print "\nVersion: $version";

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
    print "\n\nEnter input AFSIM Event file: ";
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
    # Incorrect number of parameters, print Usage line and quit
    if (scalar(@ARGV) > 1) {
	print "\n*ERROR: Incorrect number of parameters";
    }
    print "\n\nUsage: $usage\n";
    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Quit ... ";
	<STDIN>;
    }
    exit;
}

# Open file for input:
if (not open (INFH, $infilename)) {
    print "\n*ERROR - Input file \"$infilename\" could";
    print " not be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Quit ... ";
	<STDIN>;
    }
    exit;
}

# Create output file from input file name - Add ".csv"
#  extension to existing file name:
$outfile = $infilename . ".csv";

# Open file for output:
if (not open (OUTFH, '>'.$outfile)) {
    print "\n*ERROR - Output file \"$outfile\" could";
    print " not be opened\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Quit ... ";
	<STDIN>;
    }
    exit;
}

print "\nWriting output to file: $outfile";

$linenr = 0; # Initialize line number count
# Process input file and write data to output file:
while (<INFH>) {
    chomp($inline = $_); # Get input line
    $linenr++;

    if ($DEBUG >= 1) {
	print "\n*DEBUG: Line number = $linenr";
	print "\n*DEBUG: \$inline = $inline";
	&pause(2);
    }

    # Copy input line to output line for processing:
    $outline = $inline;

    # Substitute each area delimited by Whitespace with a comma:
    $outline =~ s/\s+/\,/g;

    &writeln(\*OUTFH, $outline);
}

close INFH;
close OUTFH;

print "\n----";
print "\nDONE";
print "\n----\n";

if (not $cmd_line_mode) {
    print "\nPress ENTER key to Quit ... ";
    <STDIN>;
}

###############
# SUBROUTINES #
###############

# SUBROUTINE writeln(\*<FILEHANDLE>, <string>) - Write a line to output file
sub writeln {
    my $fh = $_[0];
    my $ln = $_[1];  # Input string to write to output file
    print $fh "$ln\n";  # Write line to specified output file
    if ($DEBUG >= 2) {
        print "\n* DEBUG - writeln() OUT = $ln";
	&pause(2);
    }
}

# SUBROUTINE pause(<DEBUG Level>) - Pause for DEBUG level >= <DEBUG Level>
sub pause {
    if ($DEBUG >= $_[0]) {
        print "\nPress ENTER key to continue ... ";
        <STDIN>;
    }
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - spc2csv.pl

=head2 SYNOPSIS

spc2csv.pl - Convert any Whitespace character groups in the input file
to commas.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This script is useful for converting text data files that have data
items in tabular format with data items on each row separated by
Whitespace character groups into comma-separated-variable (CSV) format
files that can be processed with Microsoft "Excel" (Windows operating
system environment) or "oocalc" (Linux operating system
environment). This script may not be useful if one or more data items
contain more than one number and/or word separated by one or more
space characters, since this space would be interpreted as a data item
separator. (See the similar "tab2csv.pl" Perl script that generates
CSV files using Tab characters instead of Whitespace character groups
as data item separators.)

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

 perl -d spc2csv.pl

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
   perldoc -F spc2csv.pl > spc2csv_doc.txt

 In a Linux environment:
   perldoc -otext -F spc2csv.pl > spc2csv_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=spc2csv.pl --outfile=spc2csv_doc.html

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
