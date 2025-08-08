#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# File name: ev-combine.pl
# Version: 07/29/2009

# NOTE: Always include "#!/usr/bin/perl -w" as the first line
#       of any Perl script. The "-w" option enables very helpful
#       warning messages for common, but non-fatal, errors
#       that will cause scripts to not function as expected.
#       The "/usr/bin" is the typical path to the
#       Perl interpreter on Linux systems, but it is
#       ignored for Windows installations.

# Read an AFSIM event log and combine multi-line events into
#  a single (potentially very long) line.

# The input is read from standard input (STDIN) and the output is
#  written to standard output (STDOUT).

while (<>) # while end-of-file not encountered in STDIN
{
    # NOTE: Each line of input stored in special Perl variable $_

    chomp; # Remove Newline character from end-of-line

    # If current line ends with a \ character:
    if (/\\$/)   # Remove \ character from end-of-line
    {
	# Remove last character from end-of-line, which should
	#  be a space character for Event files:
        chop;
        print; # Output result to STDOUT
    }
    else
    {
        print "$_\n"; # Otherwise, print last line of Event as-is
    }
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - ev-combine.pl

=head2 SYNOPSIS

ev-combine.pl - Read an AFSIM event log and combine multi-line events
into a single (potentially very long) line.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This is a simple command-line-only Perl script that can be used in a
Linux operating system environment or in a Windows operating system
"Command Window". It can be used as a stand-alone data converter or as
the first processor in a series of filtering/processing tools.  By
converting the multi-line Events into a single long line, other Linux
utilities (such as "egrep") or command-line Perl scripts (for both
Linux and Windows) could be used to more easily filter and process
Event data.

The following command would process Event file "event_sample1.evt" and
output the result to "evt_out.txt":

 ev-combine.pl sample_w_updates.txt > evt_out.txt

NOTE: Depending on how the Linux environment is configured, the above
command may need to be changed to the following:

 ./ev-combine.pl sample_w_updates > evt_out.txt

In a Linux environment, a simple filter using "grep" could be added to
the above command. For example, to filter Event lines with an Event
type of "SENSOR_TRACK_INITIATED", the following command could be
entered:

 ./ev-combine.pl sample_w_updates.evt | grep 'SENSOR_TRACK_INITIATED' > evt_out.txt

For use in a Windows environment, a simple equivalent to the above
"grep" command could be created as follows:

 #!/usr/bin/perl -w
 # File name: event_filter1.pl
 while (<>) { # Read input from STDIN
     if ($_ =~ /SENSOR_TRACK_INITIATED/) {
	 print; # If match, print to STDOUT
     }
 }

Then, run the following commands in a Windows "Command Window":

 ev-combine.pl sample_w_updates.evt > temp1.out
 event_filter1.pl temp1.out > evt_out.txt

A simple addition to the above Perl script can enable it to also
replace whitespace between each word in the output file with a comma,
as follows:

 #!/usr/bin/perl -w
 # File name: event_filter2.pl
 while (<>) { # Read input from STDIN
     # Save input line to variable "$inline" and remove
     #  Newline character from end-of-line:
     chomp($inline = $_);
     # If Event type included in this line:
     if ($inline =~ /SENSOR_TRACK_INITIATED/) {
 	# Make sure there is no trailing whitespace characters to
 	#  avoid ending the line with a comma:
 	$inline =~ s/\s+$//;
 	# Next, replace whitespace between each word with a comma
 	$inline =~ s/\s+/\,/g;
 	print "$inline\n"; # Print output to STDOUT
     }
 }

Now, the resulting output can be processed with Microsoft "Excel"
(Windows environment) or Open Office "oocalc" (Linux environment) after
executing the following commands:

 ev-combine.pl sample_w_updates.evt > temp1.out
 event_filter2.pl temp2.out > evt_out.csv

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F ev-combine.pl > ev-combine_doc.txt

 In a Linux environment:
   perldoc -otext -F ev-combine.pl > ev-combine_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=ev-combine.pl --outfile=ev-combine_doc.html

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
