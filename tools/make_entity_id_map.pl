#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "make_entity_id_map.pl";
$version = "Version: 06/18/2009";

print "\n$pgm_name $version\n";

# See documentation at end of this script for instructions on
#  its use.

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
    print "\nEnter input AFSIM log file: ";
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
$outfilename = $rootfilename . "_DIS.txt"; # Add "_DIS.txt" to name

if (not open(OUTFH, '>'.$outfilename)) {
    print "\nOutput file \"$outfilename\" Could not be opened";
    print "\nPress ENTER key to Abort: ";
    <STDIN>;
    die("\nTerminating execution ... \n");
}

print "\nWriting DIS entity ID output to file: $outfilename\n";

print OUTFH "dis_interface\n";

$data_found_flag = 0; # Set "Data found" flag to "FALSE"

$linenr = 0; # Initiialize Line number counter for input file
while (<INFH>)
{
    chomp($line = $_); # Remove Newline char
    $linenr++; # Increment line number

    if ($line =~ /Created DIS Entity ID=\d+\:\d+\:(\d+)\;/ )
    {
	# Line with "Created DIS Entity ID" found
	# Extract Entity number immediately before semicolon
	# - First number is Site
	# - Second number is Application
	# - Third number is Entity
	$data_found_flag = 1; # Set flag to "TRUE"

	$entity = $1;
        # Old DIS code had a null at the end. Remove it if it is there.
        $entity =~ s/\x00//;

	if ($line = <INFH>)
	{

	    # Next line exists -- Not End-of-file
	    chomp($line); # Remove Newline char
	    $linenr++; # Increment line number

	    if ($line =~ /name\=(.+)\; type/)
	    {
		# Extract AFSIM name
		$name = $1;
	    }
	    else
	    {
		print "\n*ERROR - AFSIM Name not found";
		print "\n         Line number = $linenr";
		print "\n Line = $line\n";

		if (not $cmd_line_mode and $os =~ /^mswin/i)
		{
		    # If not in Command-line mode using Windows environment,
		    # do not close Interactive window until user presses
		    # the ENTER key:
		    print "\nPress ENTER key to Quit: ";
		    <STDIN>;
		}
		die("\nTerminating execution\n");
	    }

	    # If the name is less than 30 characters, pad it out so
	    #  it looks nice
	    if (length($name) < 30)
	    {
		$name = sprintf("%-30s", $name);
	    }
	    print OUTFH "  entity_id  $name  $entity\n";
	}
	else
	{
	    print "\n*ERROR - Unexpected End-of-file encountered";
	    print "\n         Line number = $linenr\n";

	    if (not $cmd_line_mode and $os =~ /^mswin/i)
	    {
		# If not in Command-line mode using Windows environment,
		# do not close Interactive window until user presses
		# the ENTER key:
		print "\nPress ENTER key to Quit: ";
		<STDIN>;
	    }
	    die("\nTerminating execution\n");
	}
    }
}
print OUTFH "end_dis_interface\n";

close (INFH);
close (OUTFH);

if (not $data_found_flag)
{
    print "\n*ERROR - No DIS ID data found in input file: $infilename\n";

    if (not $cmd_line_mode and $os =~ /^mswin/i)
    {
	# If not in Command-line mode using Windows environment,
	# do not close Interactive window until user presses
	# the ENTER key:
	print "\nPress ENTER key to Quit: ";
	<STDIN>;
    }
    die("\nTerminating execution\n");
}

print "\nDONE\n";

if (not $cmd_line_mode and $os =~ /^mswin/i) {
    # If not in Command-line mode using Windows environment ,do not
    #  close Interactive window until user presses ENTER key:
    print "\nPress ENTER key to Quit: ";
    <STDIN>;
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - make_entity_id_map.pl

=head2 SYNOPSIS

make_entity_id_map.pl - For real-time simulations, produce a fixed DIS
entity ID mapping file for every platform in a scenario.

=head2 DESCRIPTION

1) Create an input file that includes the scenario elements that
define the platforms which are to be assigned fixed DIS entity ID's.

=over

=item *

Inside the input file, include a "dis_interface" block which includes
the command "log_created_entities".

=item *

Also set the "end_time" to something small like '10 secs'.

=back

2) Run the simulation and capture standard output to a file.
Example "run xxxxxx.txt > temp.txt"

3) Run this script as follows:

Using Linux operating system:

 ./make_entity_id_map.pl <temp.txt>

If the input file name is not entered as the first argument, the
script will prompt the user for the input file name. Output file name
will be the input file name with "_DIS.txt" added at the end (ex:
temp.txt_DIS.txt).

Using Windows operating system:

Double-click on the Perl script name to be prompted for the input file
name or make the same entry as for Linux (above) in a Command Prompt
window executed in the same folder as the Perl script and the input
file.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F make_entity_id_map.pl > make_entity_id_map_doc.txt

 In a Linux environment:
   perldoc -otext -F make_entity_id_map.pl > make_entity_id_map_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=make_entity_id_map.pl --outfile=make_entity_id_map_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the Boeing-standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as Boeing-standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted.)

=head2 AUTHOR

 Jeff Johnson
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
