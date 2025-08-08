#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# File name: ev-extract.pl
# Version: 07/29/2009

# Extract all of the events out of an AFSIM event log that involve a
# specified player.  The selected events are written exactly as read -
# no combining of multi-line events is performed (use "ev-combine.pl"
# for that).

# The result is written to standard output.

# If two arguments not provided on the command-line, abort:
die("Usage: perl extract.pl input-file player-name\n") if ($#ARGV < 1);

$infilename = $ARGV[0]; # First argument: Name of input file
$playerName = $ARGV[1]; # Second argument: Search keyword

open(INFH, $infilename) or
    die("** ERROR: File \"$infilename\" could not be opened\n");

while (<INFH>)
{
    # NOTE: Contents of each input file line is temporarily
    #       stored in Perl special variable "$_"

    chomp;           # Remove Newline character from end of $_
    @field = split;  # Break out the fields in $_ (whitespace separated)

    # Initialize flag to zero (FALSE), indicating that keyword was
    #  not found in the line of the input file:
    $selected = 0;

    # Check if keyword found in 3rd, 4th, or 5th word of first
    #  line of an Event:
    if (($#field >= 2) && ($field[2] =~ $playerName))
    {
        $selected = 1; # Keyword found in 3rd word of this line
    }
    elsif (($#field >= 3) && ($field[3] =~ $playerName))
    {
        $selected = 1; # Keyword found in 4th word of this line
    }
    elsif (($#field >= 4) && ($field[4] =~ $playerName))
    {
        $selected = 1; # Keyword found in 5th word of this line
    }

    # To enter do-until loop, initialize flag to zero (FALSE),
    #  indicating that last line of Event has been encountered:
    $done = 0;
    do # Loop until no more continuation lines found
    {
	# Only print lines if keyword was found (above):
        print "$_\n" if $selected;

        # If this line doesn't end with a '\', read the next line.
        $done = 1;  # Set flag to indicate last line of Event found
        if (/\\$/)
        {
            $done = 0;   # Set flag to indicate continuation line found
            $_ = <INFH>; # Read next line in input file
            chomp;       # Remove Newline character from end-of-line
        }
    } until $done; # exit loop when $done equals one (TRUE)
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - ev-extract.pl

=head2 SYNOPSIS

ev-extract.pl - Extract all of the events out of an AFSIM output Event
file that involve a specified player.  The selected events are written
exactly as read - no combining of multi-line events is performed (use
"ev-combine.pl" for that).

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This is a simple command-line-only Perl script that can be used in a
Linux operating system environment or in a Windows operating system
"Command Window". It can be used as a stand-alone data converter or as
the first processor in a series of filtering/processing tools.

This script extracts all of the events out of an AFSIM Event file
pertaining to the player name entered as the second argument on the
command-line. The first argument is the name of the Event file. The
selected events are written exactly as read - no combining of
multi-line events is performed (use "ev-combine.pl" for that). The
result is written to standard output.

See the comments embedded in this script's code for more details on
how this script works. Note that the entered keyword only checks the
third, fourth and fifth words in the first line of an Event to
determine if the contents of an Event should be sent to standard
output. In many cases, this is sufficient, but see the
"evt_extract...pl" sample scripts to examine more powerful ways to
process AFSIM output Event files.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F ev-extract.pl > ev-extract_doc.txt

 In a Linux environment:
   perldoc -otext -F ev-extract.pl > ev-extract_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=ev-extract.pl --outfile=ev-extract_doc.html

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
