#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "batch_run.pl";
$version = "Version: 07/29/2009";

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
    # Prompt for Perl script name:
    print "\nEnter Perl AFSIM Event file process script name: ";
    chomp($perlname = <STDIN>); # Remove trailing Newline character

    if ($os =~ /^mswin/i) {
	$cmd_line_mode = 0; # Assume running Windows Interactive mode
    }
}
elsif ($nr_args == 1) {
    # Assume running Command-line mode (either Windows or Linux):
    # Get Perl script name from first argument
    $perlname = $ARGV[0];
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

# Check that Perl script has been found:
if (not -R $perlname) {
    print "\n\n*ERROR: Perl script \"$perlname\" NOT FOUND\n";

    if (not $cmd_line_mode) {
	print "\nPress ENTER key to Abort: ";
	<STDIN>;
    }
    die("\nTerminating execution ... \n");

}

# Get list of AFSIM Event files (assuming that all files end in ".evt"
@evt_files = glob "*.evt";

print "\nRunning Perl script \"$perlname\" on AFSIM Event file:";
print "\n------------------------------------------------------";

foreach $evt_file (@evt_files) {
    # Do not reprocess Event files that are already "FILTERED":
    if ($evt_file =~ /_FILTERED\.evt$/) {
	next; # Get next file
    }

    # Create command to execute with specified Perl script for
    #  each AFSIM output Event file name:
    $cmd = "perl $perlname";
    $cmd .= " $evt_file";

    print "\n- $evt_file";

    system($cmd);
    print "\n------------------------------------------------------";
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

=head1 NAME - batch_run.pl

=head2 SYNOPSIS

batch_run.pl - For all AFSIM Event files in the current directory, run
the entered name for the Perl AFSIM Event file processor script. This
script assumes that the entered Perl processing script will generate
output file names. (See Perl script "ev-batch_run.pl" for an example
of a Perl script that creates the output file names and re-directs
output from the processing script to that output file.)

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

If the name of an AFSIM Event file post-processing Perl script is not
entered as the first argument for this script on the command-line,
this script prompts the user to enter the AFSIM Event file
post-processor Perl script. Then, for every AFSIM Event file (with a
".evt" file extension) in the current directory, this "batch run"
script will execute the user-specified Perl script on every AFSIM
Event file (with ".evt" file name extension) in the current directory.

For example, to execute this script for a batch run on all ".evt"
files in the current directory using the "evt_extract_sample1.pl" Perl
script, enter the following on the command-line:

 batch_run.pl evt_extract_sample1.pl

If "batch_run.pl" is run interactively, the user will be prompted to
enter the Perl processing script name.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F batch_run.pl > batch_run_doc.txt

 In a Linux environment:
   perldoc -otext -F batch_run.pl > batch_run_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=batch_run.pl --outfile=batch_run_doc.html

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
